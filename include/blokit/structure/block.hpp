#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <lockey/lockey.hpp>
#include <sstream>
#include <string>
#include <vector>

#include "transaction.hpp"

using namespace std::chrono;

namespace chain {
    template <typename T> class Block {
      public:
        int64_t index_;
        std::string previous_hash_;
        std::string hash_;
        std::vector<Transaction<T>> transactions_;
        int64_t nonce_;
        Timestamp timestamp_;

        Block() = default;
        inline Block(std::vector<Transaction<T>> txns) {
            index_ = 0;
            previous_hash_ = "GENESIS";
            transactions_ = txns;
            nonce_ = 0;
            timestamp_.sec = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
            timestamp_.nanosec =
                duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count() % 1000000000;
            hash_ = calculateHash();
        }

        // Method to calculate the hash of the block
        inline std::string calculateHash() const {
            std::stringstream ss;
            ss << index_ << timestamp_.sec << timestamp_.nanosec << previous_hash_ << nonce_;
            for (const auto &txn : transactions_) {
                ss << txn.toString();
            }

            // Use lockey hash function and convert to hex
            lockey::Lockey crypto(lockey::Lockey::Algorithm::AES_256_GCM, lockey::Lockey::HashAlgorithm::SHA256);
            std::string data = ss.str();
            std::vector<uint8_t> data_vec(data.begin(), data.end());
            auto hash_result = crypto.hash(data_vec);

            if (hash_result.success) {
                return lockey::Lockey::to_hex(hash_result.data);
            } else {
                return ""; // Return empty string on hash failure
            }
        }

        inline bool isValid() const {
            if (index_ < 0 || previous_hash_.empty() || hash_.empty()) {
                std::cout << "Index: " << index_ << " Hash: " << hash_ << "Previous hash: " << previous_hash_
                          << std::endl;
                return false;
            }
            return true;
        }

      private:
        // Helper function to get the current timestamp
        inline std::string getCurrentTime() const {
            std::time_t now = std::time(nullptr);
            char buf[80];
            std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
            return std::string(buf);
        }
    };
} // namespace chain
