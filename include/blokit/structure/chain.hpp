#pragma once

#include <chrono>
#include <iostream>
#include <vector>

#include "block.hpp"

using namespace std::chrono;

namespace chain {
    template <typename T> class Chain {
      public:
        std::string uuid_;
        Timestamp timestamp_;
        std::vector<Block<T>> blocks_;

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
            // Recalculate hash after updating previous_hash and index
            blockToAdd.hash_ = blockToAdd.calculateHash();
            if (!blockToAdd.isValid()) {
                std::cout << "Invalid block attempted to be added to the blockchain" << std::endl;
                return;
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
                return true;
            }
            for (size_t i = 1; i < blocks_.size(); i++) {
                const Block<T> &currentBlock_ = blocks_[i];
                const Block<T> &previousBlock = blocks_[i - 1];

                // Check if the current block's hash is correct
                if (currentBlock_.hash_ != currentBlock_.calculateHash()) {
                    return false;
                }

                // Check if the current block's previous hash matches the hash of the previous block
                if (currentBlock_.previous_hash_ != previousBlock.hash_) {
                    return false;
                }
            }
            return true;
        }
    };
} // namespace chain
