#pragma once

#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "signer.hpp"

using namespace std::chrono;

namespace chain {
    struct Timestamp {
        int32_t sec;
        uint32_t nanosec;

        Timestamp() : sec(0), nanosec(0) {}
        Timestamp(int32_t s, uint32_t ns) : sec(s), nanosec(ns) {}
    };

    class Transaction {
      public:
        Timestamp timestamp_;
        int16_t priority_;
        std::string uuid_;
        std::string function_;
        std::vector<unsigned char> signature_;

        Transaction() = default;
        Transaction(std::string uuid, std::string function, int16_t priority = 100) {
            priority_ = priority;
            uuid_ = uuid;
            function_ = function;
            timestamp_.sec = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
            timestamp_.nanosec =
                duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count() % 1000000000;
        }

        void signTransaction(std::shared_ptr<chain::Crypto> privateKey_) { signature_ = privateKey_->sign(toString()); }

        // bool verifyTransaction(std::shared_ptr<chain::OpenSSLPublic> publicKey_) {
        // return publicKey_->verify(toString(), signature_);
        // }

        // Method to verify the transaction signature
        bool isValid() const {
            if (uuid_.empty() || function_.empty() || signature_.empty() || priority_ < 0 || priority_ > 255) {
                return false;
            }
            // TODO: Implement actual signature verification
            return true;
        }

        std::string toString() const {
            std::stringstream ss;
            ss << timestamp_.sec << timestamp_.nanosec << priority_ << uuid_ << function_;
            return ss.str();
        }
    };
} // namespace chain
