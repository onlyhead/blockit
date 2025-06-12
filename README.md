
<img align="right" width="26%" src="./misc/logo.png">

Blockit
===

A header-only blockchain/ledger library for C++20.

**⚠️ Note: This is a learning/educational implementation. Not suitable for production use.**

## Features

- **Transaction Management**: Create, sign, and validate transactions
- **Block Structure**: Organize transactions into blocks with cryptographic hashing
- **Blockchain**: Chain blocks together with hash references
- **Digital Signatures**: RSA-based signing and verification using the Lockey library
- **Timestamp Precision**: Nanosecond-precision timestamps for ordering
- **Priority System**: Transaction priority levels (0-255)

## Quick Start

```cpp
#include "blokit/blokit.hpp"
#include <memory>

// Create a crypto instance for signing
auto privateKey = std::make_shared<chain::Crypto>("key_file");

// Create a transaction
chain::Transaction tx("tx-001", "transfer", 100);
tx.signTransaction(privateKey);

// Create a block with transactions
std::vector<chain::Transaction> transactions = {tx};
chain::Block block(transactions);

// Create a blockchain
chain::Chain blockchain("my-chain", "genesis-tx", "genesis", privateKey);
blockchain.addBlock(block);

// Validate the chain
bool isValid = blockchain.isValid();
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
- Transaction creation and signing
- Block creation and validation
- Blockchain construction and management
- Cryptographic operations
- Various advanced scenarios

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
