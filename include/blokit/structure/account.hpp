#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace chain {

    // Entity state and permission tracker for robot/system coordination
    class EntityManager {
      private:
        std::unordered_set<std::string> authorized_entities_;                          // Entities allowed in the chain
        std::unordered_set<std::string> used_transaction_ids_;                         // Prevent duplicate commands
        std::unordered_map<std::string, std::string> entity_states_;                   // Track entity states
        std::unordered_map<std::string, std::vector<std::string>> entity_permissions_; // Entity permissions

      public:
        EntityManager() = default;

        // Register an entity (robot, system, etc.) in the chain
        inline void registerEntity(const std::string &entity_id, const std::string &initial_state = "inactive") {
            authorized_entities_.insert(entity_id);
            entity_states_[entity_id] = initial_state;
            std::cout << "Entity " << entity_id << " registered with state: " << initial_state << std::endl;
        }

        // Check if entity is authorized to participate in the chain
        inline bool isEntityAuthorized(const std::string &entity_id) const {
            return authorized_entities_.find(entity_id) != authorized_entities_.end();
        }

        // Get entity state
        inline std::string getEntityState(const std::string &entity_id) const {
            auto it = entity_states_.find(entity_id);
            return (it != entity_states_.end()) ? it->second : "unknown";
        }

        // Update entity state
        inline bool updateEntityState(const std::string &entity_id, const std::string &new_state) {
            if (!isEntityAuthorized(entity_id)) {
                std::cout << "Unauthorized entity: " << entity_id << " cannot update state" << std::endl;
                return false;
            }
            entity_states_[entity_id] = new_state;
            return true;
        }

        // Check if transaction has been used (prevent duplicate commands)
        inline bool isTransactionUsed(const std::string &tx_id) const {
            return used_transaction_ids_.find(tx_id) != used_transaction_ids_.end();
        }

        // Mark transaction as used
        inline void markTransactionUsed(const std::string &tx_id) { used_transaction_ids_.insert(tx_id); }

        // Add permission to entity
        inline void grantPermission(const std::string &entity_id, const std::string &permission) {
            if (isEntityAuthorized(entity_id)) {
                entity_permissions_[entity_id].push_back(permission);
            }
        }

        // Check if entity has specific permission
        inline bool hasPermission(const std::string &entity_id, const std::string &permission) const {
            auto it = entity_permissions_.find(entity_id);
            if (it == entity_permissions_.end())
                return false;

            const auto &perms = it->second;
            return std::find(perms.begin(), perms.end(), permission) != perms.end();
        }

        // Execute command with validation
        inline bool executeCommand(const std::string &issuer_entity, const std::string &command,
                                   const std::string &tx_id, const std::string &required_permission = "") {
            // Check for duplicate command
            if (isTransactionUsed(tx_id)) {
                std::cout << "Duplicate command detected: Transaction " << tx_id << " already executed" << std::endl;
                return false;
            }

            // Check if entity is authorized
            if (!isEntityAuthorized(issuer_entity)) {
                std::cout << "Unauthorized entity: " << issuer_entity << " cannot execute commands" << std::endl;
                return false;
            }

            // Check specific permission if required
            if (!required_permission.empty() && !hasPermission(issuer_entity, required_permission)) {
                std::cout << "Entity " << issuer_entity << " lacks permission: " << required_permission << std::endl;
                return false;
            }

            // Execute command
            markTransactionUsed(tx_id);
            std::cout << "Command executed by " << issuer_entity << ": " << command << std::endl;
            return true;
        }

        // Get all authorized entities
        inline std::unordered_set<std::string> getAuthorizedEntities() const { return authorized_entities_; }

        // Print system summary
        inline void printSystemSummary() const {
            std::cout << "=== Entity Manager Summary ===" << std::endl;
            std::cout << "Authorized Entities (" << authorized_entities_.size() << "):" << std::endl;
            for (const auto &entity : authorized_entities_) {
                std::cout << "  " << entity << " (state: " << getEntityState(entity) << ")" << std::endl;

                // Show permissions
                auto perm_it = entity_permissions_.find(entity);
                if (perm_it != entity_permissions_.end() && !perm_it->second.empty()) {
                    std::cout << "    Permissions: ";
                    for (const auto &perm : perm_it->second) {
                        std::cout << perm << " ";
                    }
                    std::cout << std::endl;
                }
            }
            std::cout << "Executed Commands: " << used_transaction_ids_.size() << std::endl;
        }
    };

} // namespace chain
