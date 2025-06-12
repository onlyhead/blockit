#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <lockey/lockey.hpp>
#include <sstream>
#include <string>
#include <vector>

#include "merkle.hpp"
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
        std::string merkle_root_; // Merkle root for transaction integrity

        Block() = default;
        inline Block(std::vector<Transaction<T>> txns) {
            index_ = 0;
            previous_hash_ = "GENESIS";
            transactions_ = txns;
            nonce_ = 0;
            timestamp_.sec = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
            timestamp_.nanosec =
                duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count() % 1000000000;

            // Build Merkle tree for transactions
            buildMerkleTree();
            hash_ = calculateHash();
        }

        // Build Merkle tree from transactions
        inline void buildMerkleTree() {
            std::vector<std::string> tx_strings;
            for (const auto &txn : transactions_) {
                tx_strings.push_back(txn.toString());
            }
            MerkleTree temp_tree(tx_strings);
            merkle_root_ = temp_tree.getRoot();
        }

        // Method to calculate the hash of the block
        inline std::string calculateHash() const {
            std::stringstream ss;
            ss << index_ << timestamp_.sec << timestamp_.nanosec << previous_hash_ << nonce_;
            // Use Merkle root instead of iterating through all transactions
            ss << merkle_root_;

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
            // Basic field validation
            if (index_ < 0 || previous_hash_.empty() || hash_.empty()) {
                std::cout << "Basic validation failed - Index: " << index_ << " Hash: " << hash_
                          << " Previous hash: " << previous_hash_ << std::endl;
                return false;
            }

            // Verify block hash is correct
            if (hash_ != calculateHash()) {
                std::cout << "Block hash validation failed - stored vs calculated hash mismatch" << std::endl;
                return false;
            }

            // Verify Merkle root
            std::vector<std::string> tx_strings;
            for (const auto &txn : transactions_) {
                tx_strings.push_back(txn.toString());
            }
            MerkleTree verification_tree(tx_strings);
            if (merkle_root_ != verification_tree.getRoot()) {
                std::cout << "Merkle root validation failed" << std::endl;
                return false;
            }

            // Validate all transactions
            for (const auto &txn : transactions_) {
                if (!txn.isValid()) {
                    std::cout << "Transaction validation failed for: " << txn.uuid_ << std::endl;
                    return false;
                }
            }

            return true;
        }

        // Verify a specific transaction is in this block using Merkle proof
        inline bool verifyTransaction(size_t transaction_index) const {
            if (transaction_index >= transactions_.size()) {
                return false;
            }

            // Create temporary Merkle tree for verification
            std::vector<std::string> tx_strings;
            for (const auto &txn : transactions_) {
                tx_strings.push_back(txn.toString());
            }
            MerkleTree verification_tree(tx_strings);

            // Get proof and verify
            auto proof = verification_tree.getProof(transaction_index);
            return verification_tree.verifyProof(transactions_[transaction_index].toString(), transaction_index, proof);
        }

        // Get block summary for debugging
        inline void printBlockSummary() const {
            std::cout << "=== Block Summary ===" << std::endl;
            std::cout << "Index: " << index_ << std::endl;
            std::cout << "Transactions: " << transactions_.size() << std::endl;
            std::cout << "Merkle Root: " << merkle_root_.substr(0, 16) << "..." << std::endl;
            std::cout << "Block Hash: " << hash_.substr(0, 16) << "..." << std::endl;
            std::cout << "Previous Hash: " << previous_hash_.substr(0, 16) << "..." << std::endl;
            std::cout << "Timestamp: " << timestamp_.sec << "." << timestamp_.nanosec << std::endl;
            std::cout << "Is Valid: " << (isValid() ? "YES" : "NO") << std::endl;
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
