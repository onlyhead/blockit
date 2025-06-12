#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace chain {

    // Generic authentication and authorization system for blockchain participants
    // Can be used for robots, farming equipment, IoT devices, industrial systems, etc.
    class Authenticator {
      private:
        std::unordered_set<std::string> authorized_participants_;         // Participants allowed in the chain
        std::unordered_set<std::string> used_transaction_ids_;            // Prevent duplicate transactions/actions
        std::unordered_map<std::string, std::string> participant_states_; // Track participant states
        std::unordered_map<std::string, std::vector<std::string>>
            participant_capabilities_; // Participant capabilities/permissions
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
            participant_metadata_; // Additional metadata

      public:
        Authenticator() = default;

        // Register a participant (robot, tractor, sensor, device, etc.) in the chain
        inline void registerParticipant(const std::string &participant_id,
                                        const std::string &initial_state = "inactive",
                                        const std::unordered_map<std::string, std::string> &metadata = {}) {
            authorized_participants_.insert(participant_id);
            participant_states_[participant_id] = initial_state;
            if (!metadata.empty()) {
                participant_metadata_[participant_id] = metadata;
            }
            std::cout << "Participant " << participant_id << " registered with state: " << initial_state << std::endl;
        }

        // Check if participant is authorized to participate in the chain
        inline bool isParticipantAuthorized(const std::string &participant_id) const {
            return authorized_participants_.find(participant_id) != authorized_participants_.end();
        }

        // Get participant state (e.g., "active", "maintenance", "spraying", "idle")
        inline std::string getParticipantState(const std::string &participant_id) const {
            auto it = participant_states_.find(participant_id);
            return (it != participant_states_.end()) ? it->second : "unknown";
        }

        // Update participant state
        inline bool updateParticipantState(const std::string &participant_id, const std::string &new_state) {
            if (!isParticipantAuthorized(participant_id)) {
                std::cout << "Unauthorized participant: " << participant_id << " cannot update state" << std::endl;
                return false;
            }
            participant_states_[participant_id] = new_state;
            return true;
        }

        // Get participant metadata (e.g., location, model, capabilities)
        inline std::string getParticipantMetadata(const std::string &participant_id, const std::string &key) const {
            auto participant_it = participant_metadata_.find(participant_id);
            if (participant_it == participant_metadata_.end())
                return "";

            auto metadata_it = participant_it->second.find(key);
            return (metadata_it != participant_it->second.end()) ? metadata_it->second : "";
        }

        // Set participant metadata
        inline void setParticipantMetadata(const std::string &participant_id, const std::string &key,
                                           const std::string &value) {
            if (isParticipantAuthorized(participant_id)) {
                participant_metadata_[participant_id][key] = value;
            }
        }

        // Check if transaction has been used (prevent duplicate actions/commands)
        inline bool isTransactionUsed(const std::string &tx_id) const {
            return used_transaction_ids_.find(tx_id) != used_transaction_ids_.end();
        }

        // Mark transaction as used
        inline void markTransactionUsed(const std::string &tx_id) { used_transaction_ids_.insert(tx_id); }

        // Add capability to participant (e.g., "SPRAY", "HARVEST", "MOVE", "READ_SENSOR")
        inline void grantCapability(const std::string &participant_id, const std::string &capability) {
            if (isParticipantAuthorized(participant_id)) {
                participant_capabilities_[participant_id].push_back(capability);
            }
        }

        // Remove capability from participant
        inline void revokeCapability(const std::string &participant_id, const std::string &capability) {
            if (isParticipantAuthorized(participant_id)) {
                auto &capabilities = participant_capabilities_[participant_id];
                capabilities.erase(std::remove(capabilities.begin(), capabilities.end(), capability),
                                   capabilities.end());
            }
        }

        // Check if participant has specific capability
        inline bool hasCapability(const std::string &participant_id, const std::string &capability) const {
            auto it = participant_capabilities_.find(participant_id);
            if (it == participant_capabilities_.end())
                return false;

            const auto &capabilities = it->second;
            return std::find(capabilities.begin(), capabilities.end(), capability) != capabilities.end();
        }

        // Validate and record action/transaction with capability checking
        inline bool validateAndRecordAction(const std::string &issuer_participant,
                                            const std::string &action_description, const std::string &tx_id,
                                            const std::string &required_capability = "") {
            // Check for duplicate action
            if (isTransactionUsed(tx_id)) {
                std::cout << "Duplicate action detected: Transaction " << tx_id << " already recorded" << std::endl;
                return false;
            }

            // Check if participant is authorized
            if (!isParticipantAuthorized(issuer_participant)) {
                std::cout << "Unauthorized participant: " << issuer_participant << " cannot perform actions"
                          << std::endl;
                return false;
            }

            // Check specific capability if required
            if (!required_capability.empty() && !hasCapability(issuer_participant, required_capability)) {
                std::cout << "Participant " << issuer_participant << " lacks capability: " << required_capability
                          << std::endl;
                return false;
            }

            // Record action
            markTransactionUsed(tx_id);
            std::cout << "Action recorded by " << issuer_participant << ": " << action_description << std::endl;
            return true;
        }

        // Get all authorized participants
        inline std::unordered_set<std::string> getAuthorizedParticipants() const { return authorized_participants_; }

        // Get participant capabilities
        inline std::vector<std::string> getParticipantCapabilities(const std::string &participant_id) const {
            auto it = participant_capabilities_.find(participant_id);
            return (it != participant_capabilities_.end()) ? it->second : std::vector<std::string>{};
        }

        // Print system summary
        inline void printSystemSummary() const {
            std::cout << "=== Authenticator Summary ===" << std::endl;
            std::cout << "Authorized Participants (" << authorized_participants_.size() << "):" << std::endl;
            for (const auto &participant : authorized_participants_) {
                std::cout << "  " << participant << " (state: " << getParticipantState(participant) << ")" << std::endl;

                // Show capabilities
                auto cap_it = participant_capabilities_.find(participant);
                if (cap_it != participant_capabilities_.end() && !cap_it->second.empty()) {
                    std::cout << "    Capabilities: ";
                    for (const auto &cap : cap_it->second) {
                        std::cout << cap << " ";
                    }
                    std::cout << std::endl;
                }

                // Show metadata
                auto meta_it = participant_metadata_.find(participant);
                if (meta_it != participant_metadata_.end() && !meta_it->second.empty()) {
                    std::cout << "    Metadata: ";
                    for (const auto &meta_pair : meta_it->second) {
                        std::cout << meta_pair.first << "=" << meta_pair.second << " ";
                    }
                    std::cout << std::endl;
                }
            }
            std::cout << "Recorded Actions: " << used_transaction_ids_.size() << std::endl;
        }

        // Serialization methods
        inline std::string serialize() const {
            std::stringstream ss;
            ss << R"({)";

            // Serialize authorized participants
            ss << R"("authorized_participants": [)";
            bool first = true;
            for (const auto &participant : authorized_participants_) {
                if (!first)
                    ss << ",";
                ss << "\"" << participant << "\"";
                first = false;
            }
            ss << R"(],)";

            // Serialize used transaction IDs
            ss << R"("used_transaction_ids": [)";
            first = true;
            for (const auto &tx_id : used_transaction_ids_) {
                if (!first)
                    ss << ",";
                ss << "\"" << tx_id << "\"";
                first = false;
            }
            ss << R"(],)";

            // Serialize participant states
            ss << R"("participant_states": {)";
            first = true;
            for (const auto &[participant, state] : participant_states_) {
                if (!first)
                    ss << ",";
                ss << "\"" << participant << "\": \"" << state << "\"";
                first = false;
            }
            ss << R"(},)";

            // Serialize participant capabilities
            ss << R"("participant_capabilities": {)";
            first = true;
            for (const auto &[participant, capabilities] : participant_capabilities_) {
                if (!first)
                    ss << ",";
                ss << "\"" << participant << "\": [";
                bool first_cap = true;
                for (const auto &cap : capabilities) {
                    if (!first_cap)
                        ss << ",";
                    ss << "\"" << cap << "\"";
                    first_cap = false;
                }
                ss << "]";
                first = false;
            }
            ss << R"(},)";

            // Serialize participant metadata
            ss << R"("participant_metadata": {)";
            first = true;
            for (const auto &[participant, metadata] : participant_metadata_) {
                if (!first)
                    ss << ",";
                ss << "\"" << participant << "\": {";
                bool first_meta = true;
                for (const auto &[key, value] : metadata) {
                    if (!first_meta)
                        ss << ",";
                    ss << "\"" << key << "\": \"" << value << "\"";
                    first_meta = false;
                }
                ss << "}";
                first = false;
            }
            ss << R"(})";

            ss << R"(})";
            return ss.str();
        }

        inline static Authenticator deserialize(const std::string &data) {
            Authenticator result;

            // Parse authorized participants
            size_t auth_start = data.find("\"authorized_participants\": [") + 29;
            size_t auth_end = data.find("],", auth_start);
            std::string auth_data = data.substr(auth_start, auth_end - auth_start);

            // Parse individual participants
            size_t pos = 0;
            while (pos < auth_data.length()) {
                size_t quote_start = auth_data.find("\"", pos);
                if (quote_start == std::string::npos)
                    break;
                size_t quote_end = auth_data.find("\"", quote_start + 1);
                if (quote_end == std::string::npos)
                    break;

                std::string participant = auth_data.substr(quote_start + 1, quote_end - quote_start - 1);
                result.authorized_participants_.insert(participant);
                pos = quote_end + 1;
            }

            // Parse used transaction IDs
            size_t tx_start = data.find("\"used_transaction_ids\": [") + 26;
            size_t tx_end = data.find("],", tx_start);
            std::string tx_data = data.substr(tx_start, tx_end - tx_start);

            pos = 0;
            while (pos < tx_data.length()) {
                size_t quote_start = tx_data.find("\"", pos);
                if (quote_start == std::string::npos)
                    break;
                size_t quote_end = tx_data.find("\"", quote_start + 1);
                if (quote_end == std::string::npos)
                    break;

                std::string tx_id = tx_data.substr(quote_start + 1, quote_end - quote_start - 1);
                result.used_transaction_ids_.insert(tx_id);
                pos = quote_end + 1;
            }

            // Parse participant states (simplified parsing for basic JSON)
            size_t states_start = data.find("\"participant_states\": {") + 24;
            size_t states_end = data.find("},", states_start);
            std::string states_data = data.substr(states_start, states_end - states_start);

            // Simple key-value parsing for states
            pos = 0;
            while (pos < states_data.length()) {
                size_t key_start = states_data.find("\"", pos);
                if (key_start == std::string::npos)
                    break;
                size_t key_end = states_data.find("\"", key_start + 1);
                if (key_end == std::string::npos)
                    break;

                size_t val_start = states_data.find("\"", key_end + 2);
                if (val_start == std::string::npos)
                    break;
                size_t val_end = states_data.find("\"", val_start + 1);
                if (val_end == std::string::npos)
                    break;

                std::string key = states_data.substr(key_start + 1, key_end - key_start - 1);
                std::string value = states_data.substr(val_start + 1, val_end - val_start - 1);
                result.participant_states_[key] = value;
                pos = val_end + 1;
            }

            // Note: Full parsing of capabilities and metadata would require a proper JSON parser
            // For now, this is a basic implementation that handles the core data

            return result;
        }
    };

    // Alias for backward compatibility and specific use cases
    using EntityManager = Authenticator;        // For robot/entity coordination
    using LedgerManager = Authenticator;        // For ledger tracking systems
    using DeviceManager = Authenticator;        // For IoT device management
    using AuthorizationManager = Authenticator; // For backward compatibility

} // namespace chain
