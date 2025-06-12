
<img align="right" width="26%" src="./misc/logo.png">

Blockit
===

A type-safe, template-based blockchain library for C++20 with generic participant authentication and ledger tracking capabilities.


## Features

- **Header-Only Design**: Fully inline functions, no separate compilation required
- **Generic Template Design**: Create blockchains for any data type that has a `to_string()` method
- **Type Safety**: Compile-time enforcement that data types implement required methods
- **Transaction Management**: Create, sign, and validate transactions with custom data types
- **Block Structure**: Organize transactions into blocks with cryptographic hashing
- **Blockchain**: Chain blocks together with hash references and validation
- **Digital Signatures**: RSA-based signing and verification using the Lockey library
- **Timestamp Precision**: Nanosecond-precision timestamps for ordering
- **Priority System**: Transaction priority levels (0-255)
- **SFINAE Type Checking**: Ensures data types have required `to_string()` method at compile time
- **🆕 Generic Authenticator**: Universal participant management for any use case
- **🆕 Capability-Based Authorization**: Flexible permission system for different roles
- **🆕 Metadata Management**: Store and retrieve participant specifications and parameters
- **🆕 State Tracking**: Monitor participant states (active, maintenance, idle, etc.)
- **🆕 Double-Spend Prevention**: Duplicate transaction detection and prevention
- **🆕 Merkle Trees**: Efficient transaction verification with cryptographic proofs
- **🆕 Enhanced Block Validation**: Comprehensive cryptographic integrity checks

## Use Cases

This library is designed for **ledger tracking and coordination systems** across various industries:

- **🤖 Robotics**: Multi-robot coordination with command authorization
- **🚜 Agriculture**: Equipment tracking, sensor networks, maintenance logs
- **🏭 Manufacturing**: Production line coordination and quality control
- **🏥 Healthcare**: Medical device coordination and patient data integrity
- **⚡ Energy**: Smart grid management and meter readings
- **🚛 Supply Chain**: Asset tracking and verification
- **🏙️ Smart Cities**: Infrastructure monitoring and management
- **🔬 Research**: Data integrity and experiment tracking

## Quick Start

The library uses templates throughout - `Transaction<T>`, `Block<T>`, and `Chain<T>` where `T` must have a `to_string()` method.

```cpp
#include "blokit/blokit.hpp"
#include <memory>

// First, create a wrapper for types that don't have to_string()
class StringWrapper {
private:
    std::string value_;
public:
    StringWrapper(const std::string& str) : value_(str) {}
    StringWrapper(const char* str) : value_(str) {}
    std::string to_string() const { return value_; }
};

// Create a crypto instance for signing
auto privateKey = std::make_shared<chain::Crypto>("key_file");

// Create a blockchain using StringWrapper
chain::Chain<StringWrapper> blockchain("my-chain", "genesis-tx", StringWrapper("genesis_data"), privateKey);

// Register participants in the blockchain
blockchain.registerParticipant("device-001", "active", {{"type", "sensor"}, {"location", "field_A"}});
blockchain.registerParticipant("device-002", "active", {{"type", "actuator"}, {"location", "field_B"}});

// Grant capabilities to participants
blockchain.grantCapability("device-001", "READ_DATA");
blockchain.grantCapability("device-002", "WRITE_DATA");

// Create a transaction
chain::Transaction<StringWrapper> tx("tx-001", StringWrapper("transfer_funds"), 100);
tx.signTransaction(privateKey);

// Create a block with transactions
std::vector<chain::Transaction<StringWrapper>> transactions = {tx};
chain::Block<StringWrapper> block(transactions);

// Add block to blockchain (with duplicate detection)
blockchain.addBlock(block);

// Validate the chain (with enhanced cryptographic checks)
bool isValid = blockchain.isValid();
```

### Using Custom Data Types

Any type can be used as long as it implements a `to_string()` method:

```cpp
// Custom data type for payments
struct PaymentData {
    std::string from, to;
    double amount;
    
    // Required: must have to_string() method
    std::string to_string() const {
        return "Payment{from:" + from + ",to:" + to + ",amount:" + std::to_string(amount) + "}";
    }
};

// Create a blockchain for custom payment data
PaymentData payment{"Alice", "Bob", 100.50};
chain::Chain<PaymentData> paymentChain("payment-chain", "pay-genesis", payment, privateKey);

// Create transaction with custom data
chain::Transaction<PaymentData> payTx("pay-001", PaymentData{"Bob", "Charlie", 50.25}, 150);
payTx.signTransaction(privateKey);
```

### Type Requirements

The template system uses SFINAE to ensure your type has a `to_string()` method:

```cpp
// ✅ This works - has to_string() method
struct ValidType {
    int value;
    std::string to_string() const { return std::to_string(value); }
};

// ❌ This fails at compile time - no to_string() method
struct InvalidType {
    int value;
    // Missing to_string() method!
};

// Compile-time error with helpful message:
// "Type T must have a to_string() method"
chain::Transaction<InvalidType> tx; // Won't compile!
```

### Real-World Examples

#### Agricultural Equipment Tracking
```cpp
// Farming operation data structure
struct FarmingOperation {
    std::string equipment_id, operation_type, field_location, crop_type;
    double area_covered;
    std::string to_string() const { /* implementation */ }
};

// Create blockchain for farming operations
chain::Chain<FarmingOperation> farmChain("farm-ops", "genesis", genesis_op, privateKey);

// Register equipment with metadata
farmChain.registerParticipant("tractor-001", "ready", {
    {"model", "John_Deere_8370R"}, {"fuel_capacity", "680L"}
});

// Grant capabilities
farmChain.grantCapability("tractor-001", "TILLAGE");
farmChain.grantCapability("tractor-001", "SEEDING");
```

#### IoT Sensor Network
```cpp
// Sensor reading data structure
struct SensorReading {
    std::string sensor_id, sensor_type, location;
    double value;
    std::string unit;
    std::string to_string() const { /* implementation */ }
};

// Create blockchain for sensor data
chain::Chain<SensorReading> sensorChain("sensors", "init", init_reading, privateKey);

// Register sensors with metadata
sensorChain.registerParticipant("soil-sensor-001", "active", {
    {"type", "soil_moisture"}, {"field", "Field_A"}, {"depth", "15cm"}
});

sensorChain.grantCapability("soil-sensor-001", "READ_SOIL_MOISTURE");
```

#### Robot Coordination
```cpp
// Robot command data structure
struct RobotCommand {
    std::string issuer_robot, target_robot, command;
    int priority_level;
    std::string to_string() const { /* implementation */ }
};

// Create blockchain for robot coordination
chain::Chain<RobotCommand> robotChain("robots", "init", init_cmd, privateKey);

// Register robots and grant capabilities
robotChain.registerParticipant("robot-001", "idle");
robotChain.grantCapability("robot-001", "MOVE");
robotChain.grantCapability("robot-001", "PICK");
```

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Running Examples

```bash
cd build
./main              # Original basic demo
./enhanced_demo     # Robot coordination demo  
./farming_demo      # Agricultural industry demo
```

The **original example** demonstrates:
- Generic template usage with `StringWrapper` 
- Transaction creation and signing with custom data types
- Block creation and validation
- Blockchain construction and management
- Cryptographic operations
- Various advanced scenarios
- Type safety and compile-time checking

The **enhanced example** demonstrates:
- **Robot Coordination**: Multiple robots working together through blockchain authorization
- **Entity Management**: Registration, permissions, and state tracking
- **Ledger Tracking**: Immutable logging for sensors, actuators, and controllers
- **Double-Spend Prevention**: Automatic detection and rejection of duplicate commands
- **Merkle Trees**: Efficient verification of individual transactions in large blocks

The **farming example** demonstrates:
- **Equipment Operations**: Tractors, sprayers, harvesters with metadata and capabilities
- **Sensor Networks**: Soil moisture, weather stations, drone data collection
- **Maintenance Ledger**: Technician records and equipment service tracking
- **Generic Authenticator**: Same system adapted for different agricultural use cases
- **Capability-Based Authorization**: Role-based permissions for different equipment types

## Current Limitations

This implementation is **educational only** and lacks several critical features for production use:

- ❌ No Proof of Work or consensus mechanism
- ❌ No network layer for distributed operation
- ❌ No persistent storage
- ✅ ~~Limited transaction validation~~ **Enhanced transaction validation with entity permissions**
- ✅ ~~No Merkle trees for efficient verification~~ **Merkle trees implemented for efficient verification**
- ✅ ~~No protection against double spending~~ **Double-spend prevention implemented**
- ❌ No difficulty adjustment
- ❌ No fork resolution

## Dependencies

- **Lockey**: Cryptographic library for signing and verification
- **C++20**: Modern C++ features
- **CMake 3.15+**: Build system

## License

This project is open source. See the license file for details.

---
