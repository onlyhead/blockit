#pragma once

#include <chrono>
#include <iostream>
#include <vector>

#include "account.hpp"
#include "block.hpp"

using namespace std::chrono;

namespace chain {
    template <typename T> class Chain {
      public:
        std::string uuid_;
        Timestamp timestamp_;
        std::vector<Block<T>> blocks_;
        EntityManager entity_manager_; // For robot/entity coordination and duplicate prevention

        Chain() = default;
        inline Chain(std::string s_uuid, std::string t_uuid, T function, std::shared_ptr<chain::Crypto> privateKey_,
                     int16_t priority = 100) {
            Transaction<T> genesisTransaction(t_uuid, function, priority);
            genesisTransaction.signTransaction(privateKey_);
            Block<T> genesisBlock({genesisTransaction});
            blocks_.push_back(genesisBlock);
            uuid_ = s_uuid;
        }

        inline Chain(std::string s_uuid, std::string t_uuid, T function, int16_t priority = 100) {
            Transaction<T> genesisTransaction(t_uuid, function, priority);
            Block<T> genesisBlock({genesisTransaction});
            blocks_.push_back(genesisBlock);
            uuid_ = s_uuid;
        }

        // Method to add a new block to the blockchain
        inline void addBlock(const Block<T> &newBlock) {
            Block<T> blockToAdd = newBlock;
            blockToAdd.previous_hash_ = blocks_.back().hash_;
            blockToAdd.index_ = blocks_.back().index_ + 1;

            // Validate transactions against entity manager
            for (const auto &txn : blockToAdd.transactions_) {
                // Check for duplicate transaction
                if (entity_manager_.isTransactionUsed(txn.uuid_)) {
                    std::cout << "Duplicate transaction detected: " << txn.uuid_ << std::endl;
                    return;
                }

                // For robot coordination, you could add entity authorization checks here
                // For example: if (!entity_manager_.isEntityAuthorized(txn.issuer_entity_)) { return; }
            }

            // Rebuild Merkle tree and recalculate hash after updating previous_hash and index
            blockToAdd.buildMerkleTree();
            blockToAdd.hash_ = blockToAdd.calculateHash();

            if (!blockToAdd.isValid()) {
                std::cout << "Invalid block attempted to be added to the blockchain" << std::endl;
                return;
            }

            // Mark all transactions as used
            for (const auto &txn : blockToAdd.transactions_) {
                entity_manager_.markTransactionUsed(txn.uuid_);
            }

            std::cout << "Adding block to chain" << std::endl;
            blocks_.push_back(blockToAdd);
        }

        inline void addBlock(std::string uuid, T function, std::shared_ptr<chain::Crypto> privateKey_,
                             int16_t priority = 100) {
            Transaction<T> genesisTransaction(uuid, function, priority);
            genesisTransaction.signTransaction(privateKey_);
            Block<T> genesisBlock({genesisTransaction});
            addBlock(genesisBlock);
        }

        // Method to validate the integrity of the blockchain
        inline bool isValid() const {
            if (blocks_.empty()) {
                return false;
            }
            if (blocks_.size() == 1) {
                return blocks_[0].isValid();
            }
            for (size_t i = 1; i < blocks_.size(); i++) {
                const Block<T> &currentBlock_ = blocks_[i];
                const Block<T> &previousBlock = blocks_[i - 1];

                // Enhanced block validation
                if (!currentBlock_.isValid()) {
                    std::cout << "Block " << i << " failed validation" << std::endl;
                    return false;
                }

                // Check if the current block's previous hash matches the hash of the previous block
                if (currentBlock_.previous_hash_ != previousBlock.hash_) {
                    std::cout << "Block chain link broken at block " << i << std::endl;
                    return false;
                }
            }
            return true;
        }

        // Register an entity (robot/system) in the blockchain
        inline void registerEntity(const std::string &entity_id, const std::string &initial_state = "inactive") {
            entity_manager_.registerEntity(entity_id, initial_state);
        }

        // Check if entity is authorized
        inline bool isEntityAuthorized(const std::string &entity_id) const {
            return entity_manager_.isEntityAuthorized(entity_id);
        }

        // Update entity state
        inline bool updateEntityState(const std::string &entity_id, const std::string &new_state) {
            return entity_manager_.updateEntityState(entity_id, new_state);
        }

        // Grant permission to entity
        inline void grantPermission(const std::string &entity_id, const std::string &permission) {
            entity_manager_.grantPermission(entity_id, permission);
        }

        // Execute a command through the blockchain
        inline bool executeCommand(const std::string &issuer_entity, const std::string &command,
                                   const std::string &tx_id, const std::string &required_permission = "") {
            return entity_manager_.executeCommand(issuer_entity, command, tx_id, required_permission);
        }

        // Get blockchain statistics
        inline void printChainSummary() const {
            std::cout << "=== Blockchain Summary ===" << std::endl;
            std::cout << "Chain UUID: " << uuid_ << std::endl;
            std::cout << "Total Blocks: " << blocks_.size() << std::endl;
            std::cout << "Chain Valid: " << (isValid() ? "YES" : "NO") << std::endl;

            size_t total_transactions = 0;
            for (const auto &block : blocks_) {
                total_transactions += block.transactions_.size();
            }
            std::cout << "Total Transactions: " << total_transactions << std::endl;

            if (!blocks_.empty()) {
                std::cout << "Genesis Block Hash: " << blocks_[0].hash_.substr(0, 16) << "..." << std::endl;
                std::cout << "Latest Block Hash: " << blocks_.back().hash_.substr(0, 16) << "..." << std::endl;
            }

            std::cout << "\nEntity Manager:" << std::endl;
            entity_manager_.printSystemSummary();
        }
    };
} // namespace chain
