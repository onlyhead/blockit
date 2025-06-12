#pragma once

#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "signer.hpp"
#include "serializer.hpp"

using namespace std::chrono;

namespace chain {
    struct Timestamp {
        int32_t sec;
        uint32_t nanosec;
        inline Timestamp() : sec(0), nanosec(0) {}
        inline Timestamp(int32_t s, uint32_t ns) : sec(s), nanosec(ns) {}

        // JSON serialization methods
        inline std::string serialize() const {
            return R"({"sec": )" + std::to_string(sec) + R"(, "nanosec": )" + std::to_string(nanosec) + "}";
        }

        inline static Timestamp deserialize(const std::string &data) {
            // Simple JSON parsing for timestamp
            Timestamp result;
            size_t sec_pos = data.find("\"sec\": ") + 7;
            size_t sec_end = data.find(',', sec_pos);
            result.sec = std::stoi(data.substr(sec_pos, sec_end - sec_pos));

            size_t nanosec_pos = data.find("\"nanosec\": ") + 11;
            size_t nanosec_end = data.find('}', nanosec_pos);
            result.nanosec = std::stoul(data.substr(nanosec_pos, nanosec_end - nanosec_pos));

            return result;
        }

        // Binary serialization methods
        inline std::vector<uint8_t> serializeBinary() const {
            std::vector<uint8_t> buffer;
            BinarySerializer::writeUint32(buffer, static_cast<uint32_t>(sec));
            BinarySerializer::writeUint32(buffer, nanosec);
            return buffer;
        }

        inline static Timestamp deserializeBinary(const std::vector<uint8_t>& data) {
            Timestamp result;
            size_t offset = 0;
            result.sec = static_cast<int32_t>(BinarySerializer::readUint32(data, offset));
            result.nanosec = BinarySerializer::readUint32(data, offset);
            return result;
        }
    };

    // SFINAE helper to check if T has a to_string() method
    template <typename T> class has_to_string {
      private:
        template <typename U> static auto test(int) -> decltype(std::declval<U>().to_string(), std::true_type{});
        template <typename> static std::false_type test(...);

      public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };

    template <typename T> class Transaction {
        static_assert(has_to_string<T>::value, "Type T must have a 'to_string() const' method");

      public:
        Timestamp timestamp_;
        int16_t priority_;
        std::string uuid_;
        T function_;
        std::vector<unsigned char> signature_;

        Transaction() = default;
        inline Transaction(std::string uuid, T function, int16_t priority = 100) {
            priority_ = priority;
            uuid_ = uuid;
            function_ = function;
            timestamp_.sec = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
            timestamp_.nanosec =
                duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count() % 1000000000;
        }

        inline void signTransaction(std::shared_ptr<chain::Crypto> privateKey_) {
            signature_ = privateKey_->sign(toString());
        }

        // Method to verify the transaction signature
        inline bool isValid() const {
            if (uuid_.empty() || function_.to_string().empty() || signature_.empty() || priority_ < 0 ||
                priority_ > 255) {
                return false;
            }
            // TODO: Implement actual signature verification
            return true;
        }

        inline std::string toString() const {
            std::stringstream ss;
            ss << timestamp_.sec << timestamp_.nanosec << priority_ << uuid_ << function_.to_string();
            return ss.str();
        }

        // Serialization methods - maintain backward compatibility with string JSON
        
        // Default serialize() method returns JSON string for backward compatibility
        inline std::string serialize() const {
            return serializeJson();
        }
        
        // Binary serialization (explicit method)
        inline std::vector<uint8_t> serializeBinary() const {
            std::vector<uint8_t> buffer;
            
            // Write timestamp
            BinarySerializer::writeUint32(buffer, static_cast<uint32_t>(timestamp_.sec));
            BinarySerializer::writeUint32(buffer, timestamp_.nanosec);
            
            // Write priority 
            BinarySerializer::writeInt16(buffer, priority_);
            
            // Write UUID
            BinarySerializer::writeString(buffer, uuid_);
            
            // Write function data - use TypeSerializer to handle different T capabilities
            auto functionData = TypeSerializer<T>::serializeBinary(function_);
            BinarySerializer::writeBytes(buffer, functionData);
            
            // Write signature
            BinarySerializer::writeBytes(buffer, signature_);
            
            return buffer;
        }
        
        // JSON serialization
        inline std::string serializeJson() const {
            std::stringstream ss;
            ss << R"({)";
            ss << R"("uuid": ")" << JsonSerializer::escapeJson(uuid_) << R"(",)";
            ss << R"("timestamp": {"sec": )" << timestamp_.sec << R"(, "nanosec": )" << timestamp_.nanosec << R"(},)";
            ss << R"("priority": )" << priority_ << R"(,)";
            
            // Handle function serialization using TypeSerializer
            ss << R"("function": )" << TypeSerializer<T>::serializeJson(function_) << R"(,)";
            
            // Encode signature as base64
            std::string signature_b64 = base64Encode(signature_);
            ss << R"("signature": ")" << signature_b64 << R"(")";
            ss << R"(})";
            
            return ss.str();
        }
        
        // Deserialization methods - maintain backward compatibility with string JSON
        
        // Default deserialize() method for JSON string (backward compatibility)
        static Transaction<T> deserialize(const std::string& data) {
            return deserializeJson(data);
        }
        
        // Template deserialize for format specification
        template<typename U = T>
        static Transaction<T> deserialize(const std::vector<uint8_t>& data, 
                                        SerializationFormat format = SerializationFormat::BINARY,
                                        typename std::enable_if<TypeSerializer<U>::supportsBinary() || TypeSerializer<U>::supportsJson()>::type* = 0) {
            if (format == SerializationFormat::BINARY) {
                return deserializeBinary(data);
            } else {
                // Convert bytes back to JSON string
                std::string jsonStr(data.begin(), data.end());
                return deserializeJson(jsonStr);
            }
        }
        
        // Auto-detect format from data
        static Transaction<T> deserializeAuto(const std::vector<uint8_t>& data) {
            // Check if it starts with binary header magic number
            if (data.size() >= 4) {
                uint32_t magic = *reinterpret_cast<const uint32_t*>(data.data());
                if (magic == BinaryHeader::MAGIC_NUMBER) {
                    return deserializeBinary(data);
                }
            }
            
            // Check if it looks like JSON (starts with '{')
            if (!data.empty() && data[0] == '{') {
                std::string jsonStr(data.begin(), data.end());
                return deserializeJson(jsonStr);
            }
            
            // Default to binary
            return deserializeBinary(data);
        }
        
        // Binary deserialization
        static Transaction<T> deserializeBinary(const std::vector<uint8_t>& data) {
            Transaction<T> result;
            size_t offset = 0;
            
            // Read timestamp
            result.timestamp_.sec = static_cast<int32_t>(BinarySerializer::readUint32(data, offset));
            result.timestamp_.nanosec = BinarySerializer::readUint32(data, offset);
            
            // Read priority
            result.priority_ = BinarySerializer::readInt16(data, offset);
            
            // Read UUID
            result.uuid_ = BinarySerializer::readString(data, offset);
            
            // Read function data using TypeSerializer
            auto functionData = BinarySerializer::readBytes(data, offset);
            result.function_ = TypeSerializer<T>::deserializeBinary(functionData);
            
            // Read signature
            result.signature_ = BinarySerializer::readBytesToUChar(data, offset);
            
            return result;
        }
        
        // JSON deserialization 
        static Transaction<T> deserializeJson(const std::string& data) {
            Transaction<T> result;
            
            // Parse UUID
            result.uuid_ = JsonSerializer::extractJsonValue(data, "uuid");
            
            // Parse timestamp
            std::string timestampJson = JsonSerializer::extractJsonValue(data, "timestamp");
            result.timestamp_.sec = std::stoi(JsonSerializer::extractJsonValue(timestampJson, "sec"));
            result.timestamp_.nanosec = static_cast<uint32_t>(std::stoul(JsonSerializer::extractJsonValue(timestampJson, "nanosec")));
            
            // Parse priority
            result.priority_ = static_cast<int16_t>(std::stoi(JsonSerializer::extractJsonValue(data, "priority")));
            
            // Parse function using TypeSerializer
            std::string functionJson = JsonSerializer::extractJsonValue(data, "function");
            result.function_ = TypeSerializer<T>::deserializeJson(functionJson);
            
            // Parse signature
            std::string signature_b64 = JsonSerializer::extractJsonValue(data, "signature");
            result.signature_ = staticBase64Decode(signature_b64);
            
            return result;
        }

      private:
        // Helper functions for base64 encoding/decoding signatures
        inline std::string base64Encode(const std::vector<unsigned char> &data) const {
            // Use the base64Encode from signer.hpp
            return chain::base64Encode(data);
        }

        inline std::vector<unsigned char> base64Decode(const std::string &data) const {
            return chain::base64Decode(data);
        }
        
        // Static helpers for deserialization
        static std::string staticBase64Encode(const std::vector<unsigned char> &data) {
            return chain::base64Encode(data);
        }

        static std::vector<unsigned char> staticBase64Decode(const std::string &data) {
            return chain::base64Decode(data);
        }
    };
} // namespace chain
