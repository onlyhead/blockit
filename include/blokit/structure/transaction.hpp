#pragma once

#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "signer.hpp"

using namespace std::chrono;

namespace chain {
    struct Timestamp {
        int32_t sec;
        uint32_t nanosec;
        inline Timestamp() : sec(0), nanosec(0) {}
        inline Timestamp(int32_t s, uint32_t ns) : sec(s), nanosec(ns) {}
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
        static_assert(has_to_string<T>::value,
                      "Type T must have a 'operator std::string() const { return value_; }' method");

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

        // bool verifyTransaction(std::shared_ptr<chain::OpenSSLPublic> publicKey_) {
        // return publicKey_->verify(toString(), signature_);
        // }

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
    };
} // namespace chain
