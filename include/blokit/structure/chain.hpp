#pragma once

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "auth.hpp"
#include "block.hpp"

using namespace std::chrono;

namespace chain {
    template <typename T> class Chain {
      public:
        std::string uuid_;
        Timestamp timestamp_;
        std::vector<Block<T>> blocks_;
        EntityManager entity_manager_; // For participant authentication and authorization management

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
        inline bool addBlock(const Block<T> &newBlock) {
            Block<T> blockToAdd = newBlock;
            blockToAdd.previous_hash_ = blocks_.back().hash_;
            blockToAdd.index_ = blocks_.back().index_ + 1;

            // Validate transactions against entity manager
            for (const auto &txn : blockToAdd.transactions_) {
                // Check for duplicate transaction
                if (entity_manager_.isTransactionUsed(txn.uuid_)) {
                    std::cout << "Duplicate transaction detected: " << txn.uuid_ << std::endl;
                    return false;
                }

                // For robot coordination, you could add entity authorization checks here
                // For example: if (!entity_manager_.isEntityAuthorized(txn.issuer_entity_)) { return false; }
            }

            // Rebuild Merkle tree and recalculate hash after updating previous_hash and index
            blockToAdd.buildMerkleTree();
            blockToAdd.hash_ = blockToAdd.calculateHash();

            if (!blockToAdd.isValid()) {
                std::cout << "Invalid block attempted to be added to the blockchain" << std::endl;
                return false;
            }

            // Mark all transactions as used
            for (const auto &txn : blockToAdd.transactions_) {
                entity_manager_.markTransactionUsed(txn.uuid_);
            }

            std::cout << "Adding block to chain" << std::endl;
            blocks_.push_back(blockToAdd);
            return true;
        }

        inline bool addBlock(std::string uuid, T function, std::shared_ptr<chain::Crypto> privateKey_,
                             int16_t priority = 100) {
            Transaction<T> genesisTransaction(uuid, function, priority);
            genesisTransaction.signTransaction(privateKey_);
            Block<T> genesisBlock({genesisTransaction});
            return addBlock(genesisBlock);
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

        // Register an entity/participant in the blockchain
        inline void registerEntity(const std::string &entity_id, const std::string &initial_state = "inactive") {
            entity_manager_.registerParticipant(entity_id, initial_state);
        }

        // Generic method for registering participants with metadata
        inline void registerParticipant(const std::string &participant_id,
                                        const std::string &initial_state = "inactive",
                                        const std::unordered_map<std::string, std::string> &metadata = {}) {
            entity_manager_.registerParticipant(participant_id, initial_state, metadata);
        }

        // Check if entity/participant is authorized
        inline bool isEntityAuthorized(const std::string &entity_id) const {
            return entity_manager_.isParticipantAuthorized(entity_id);
        }

        inline bool isParticipantAuthorized(const std::string &participant_id) const {
            return entity_manager_.isParticipantAuthorized(participant_id);
        }

        // Update entity/participant state
        inline bool updateEntityState(const std::string &entity_id, const std::string &new_state) {
            return entity_manager_.updateParticipantState(entity_id, new_state);
        }

        inline bool updateParticipantState(const std::string &participant_id, const std::string &new_state) {
            return entity_manager_.updateParticipantState(participant_id, new_state);
        }

        // Grant permission/capability
        inline void grantPermission(const std::string &entity_id, const std::string &permission) {
            entity_manager_.grantCapability(entity_id, permission);
        }

        inline void grantCapability(const std::string &participant_id, const std::string &capability) {
            entity_manager_.grantCapability(participant_id, capability);
        }

        // Metadata management
        inline std::string getParticipantMetadata(const std::string &participant_id, const std::string &key) const {
            return entity_manager_.getParticipantMetadata(participant_id, key);
        }

        inline void setParticipantMetadata(const std::string &participant_id, const std::string &key,
                                           const std::string &value) {
            entity_manager_.setParticipantMetadata(participant_id, key, value);
        }

        // Execute a command through the blockchain
        inline bool executeCommand(const std::string &issuer_entity, const std::string &command,
                                   const std::string &tx_id, const std::string &required_permission = "") {
            return entity_manager_.validateAndRecordAction(issuer_entity, command, tx_id, required_permission);
        }

        // Generic action validation and recording
        inline bool validateAndRecordAction(const std::string &issuer_participant,
                                            const std::string &action_description, const std::string &tx_id,
                                            const std::string &required_capability = "") {
            return entity_manager_.validateAndRecordAction(issuer_participant, action_description, tx_id,
                                                           required_capability);
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

            std::cout << "\nAuthenticator:" << std::endl;
            entity_manager_.printSystemSummary();
        }

        // Additional missing methods for comprehensive chain management

        // Check if participant is registered
        inline bool isParticipantRegistered(const std::string &participant_id) const {
            return entity_manager_.isParticipantAuthorized(participant_id);
        }

        // Check if participant can perform specific action (has capability)
        inline bool canParticipantPerform(const std::string &participant_id, const std::string &capability) const {
            return entity_manager_.hasCapability(participant_id, capability);
        }

        // Revoke capability from participant
        inline void revokeCapability(const std::string &participant_id, const std::string &capability) {
            entity_manager_.revokeCapability(participant_id, capability);
        }

        // Check if chain is valid (alias for isValid)
        inline bool isChainValid() const { return isValid(); }

        // Get chain length
        inline size_t getChainLength() const { return blocks_.size(); }

        // Get last block
        inline const Block<T> &getLastBlock() const {
            if (blocks_.empty()) {
                throw std::runtime_error("Chain is empty");
            }
            return blocks_.back();
        }

        // Check if transaction is used
        inline bool isTransactionUsed(const std::string &tx_id) const {
            return entity_manager_.isTransactionUsed(tx_id);
        }

        // Serialization methods
        inline std::string serialize() const {
            std::stringstream ss;
            ss << R"({)";
            ss << R"("uuid": ")" << uuid_ << R"(",)";
            ss << R"("timestamp": )" << timestamp_.serialize() << R"(,)";
            ss << R"("blocks": [)";

            for (size_t i = 0; i < blocks_.size(); ++i) {
                ss << blocks_[i].serialize();
                if (i < blocks_.size() - 1) {
                    ss << ",";
                }
            }

            ss << R"(],)";
            ss << R"("entity_manager": )" << entity_manager_.serialize();
            ss << R"(})";

            return ss.str();
        }

        inline static Chain<T> deserialize(const std::string &data) {
            Chain<T> result;

            // Parse UUID
            size_t uuid_start = data.find("\"uuid\": \"") + 9;
            size_t uuid_end = data.find("\"", uuid_start);
            result.uuid_ = data.substr(uuid_start, uuid_end - uuid_start);

            // Parse timestamp
            size_t ts_start = data.find("\"timestamp\": ") + 13;
            size_t ts_end = data.find("},", ts_start) + 1;
            result.timestamp_ = Timestamp::deserialize(data.substr(ts_start, ts_end - ts_start));

            // Parse blocks array
            size_t blocks_array_start = data.find("\"blocks\": [") + 11;
            size_t blocks_array_end = data.find("],", blocks_array_start);
            std::string blocks_array_data = data.substr(blocks_array_start, blocks_array_end - blocks_array_start);

            // Parse individual blocks
            int brace_count = 0;
            size_t block_start = 0;

            for (size_t i = 0; i < blocks_array_data.length(); ++i) {
                if (blocks_array_data[i] == '{') {
                    if (brace_count == 0)
                        block_start = i;
                    brace_count++;
                } else if (blocks_array_data[i] == '}') {
                    brace_count--;
                    if (brace_count == 0) {
                        std::string block_data = blocks_array_data.substr(block_start, i - block_start + 1);
                        result.blocks_.push_back(Block<T>::deserialize(block_data));
                    }
                }
            }

            // Parse entity manager
            size_t entity_start = data.find("\"entity_manager\": ") + 18;
            size_t entity_end = data.rfind("}");
            std::string entity_data = data.substr(entity_start, entity_end - entity_start);
            result.entity_manager_ = EntityManager::deserialize(entity_data);

            return result;
        }

        // File I/O methods
        inline bool saveToFile(const std::string &filename) const {
            try {
                std::ofstream file(filename);
                if (!file.is_open()) {
                    std::cerr << "Failed to open file for writing: " << filename << std::endl;
                    return false;
                }

                file << serialize();
                file.close();

                std::cout << "Blockchain saved to " << filename << std::endl;
                return true;
            } catch (const std::exception &e) {
                std::cerr << "Error saving blockchain: " << e.what() << std::endl;
                return false;
            }
        }

        inline bool loadFromFile(const std::string &filename) {
            try {
                std::ifstream file(filename);
                if (!file.is_open()) {
                    std::cerr << "Failed to open file for reading: " << filename << std::endl;
                    return false;
                }

                std::stringstream buffer;
                buffer << file.rdbuf();
                file.close();

                *this = Chain<T>::deserialize(buffer.str());

                std::cout << "Blockchain loaded from " << filename << std::endl;
                return true;
            } catch (const std::exception &e) {
                std::cerr << "Error loading blockchain: " << e.what() << std::endl;
                return false;
            }
        }
    };
} // namespace chain
