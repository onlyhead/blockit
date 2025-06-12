
<img align="right" width="26%" src="./misc/logo.png">

Blockit
===

A type-safe, template-based blockchain library for C++20 with compile-time validation.


## Features

- **Generic Template Design**: Create blockchains for any data type that has a `to_string()` method
- **Type Safety**: Compile-time enforcement that data types implement required methods
- **Transaction Management**: Create, sign, and validate transactions with custom data types
- **Block Structure**: Organize transactions into blocks with cryptographic hashing
- **Blockchain**: Chain blocks together with hash references and validation
- **Digital Signatures**: RSA-based signing and verification using the Lockey library
- **Timestamp Precision**: Nanosecond-precision timestamps for ordering
- **Priority System**: Transaction priority levels (0-255)
- **SFINAE Type Checking**: Ensures data types have required `to_string()` method at compile time

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

// Create a transaction
chain::Transaction<StringWrapper> tx("tx-001", StringWrapper("transfer_funds"), 100);
tx.signTransaction(privateKey);

// Create a block with transactions
std::vector<chain::Transaction<StringWrapper>> transactions = {tx};
chain::Block<StringWrapper> block(transactions);

// Add block to blockchain
blockchain.addBlock(block);

// Validate the chain
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

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Running Examples

```bash
cd build
./main
```

The example demonstrates:
- Generic template usage with `StringWrapper` 
- Transaction creation and signing with custom data types
- Block creation and validation
- Blockchain construction and management
- Cryptographic operations
- Various advanced scenarios
- Type safety and compile-time checking

## Current Limitations

This implementation is **educational only** and lacks several critical features for production use:

- ❌ No Proof of Work or consensus mechanism
- ❌ No network layer for distributed operation
- ❌ No persistent storage
- ❌ Limited transaction validation
- ❌ No Merkle trees for efficient verification
- ❌ No protection against double spending
- ❌ No difficulty adjustment
- ❌ No fork resolution

## Dependencies

- **Lockey**: Cryptographic library for signing and verification
- **C++20**: Modern C++ features
- **CMake 3.15+**: Build system

## License

This project is open source. See the license file for details.

---
