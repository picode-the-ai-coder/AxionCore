#pragma once
// ================================================================
//  Axion Digitaverse - adl.h  [Phase 1]
//  Transaction + Block data structures.
//
//  Phase 1 changes:
//    - signature is now a hex string of a real Ed25519 sig (128 chars)
//    - sender_pubkey stored on tx so verifier has what it needs
//    - sign_tx() and verify_signature() use real Ed25519 via APKI
// ================================================================

#include "sha256.h"
#include "apki.h"

namespace axion {

// ================================================================
//  Transaction
// ================================================================
struct Transaction {
    std::string tx_id;           // SHA-256 of canonical fields
    std::string sender;          // AXC address of sender, or "coinbase"
    std::string sender_pubkey;   // hex of sender's 32-byte Ed25519 public key
    std::string receiver;        // AXC address of receiver
    double      amount;          // AXC amount
    uint64_t    timestamp;       // Unix epoch ms
    std::string signature;       // 128-char hex Ed25519 signature
    std::string data;            // optional: AVM calldata or memo
    std::string tx_type;         // "transfer"|"coinbase"|"contract_deploy"|"contract_call"

    Transaction() : amount(0.0), timestamp(0) {}

    // Canonical serialization for hashing and signing
    std::string serialize() const {
        std::ostringstream oss;
        oss << tx_type << "|" << sender << "|" << receiver << "|"
            << std::fixed << std::setprecision(8) << amount << "|"
            << timestamp << "|" << data;
        return oss.str();
    }

    void compute_id() { tx_id = sha256(serialize()); }

    // Sign with the sender's Ed25519 private key (64 bytes)
    void sign_tx(const Bytes& private_key) {
        signature = APKI::sign(serialize(), private_key);
    }

    // Verify Ed25519 signature using sender_pubkey
    bool verify_signature() const {
        if (tx_type == "coinbase") return true;
        if (sender_pubkey.empty() || signature.empty()) return false;
        Bytes pubkey = from_hex(sender_pubkey);
        return APKI::verify(serialize(), signature, pubkey);
    }

    bool is_coinbase() const { return tx_type == "coinbase"; }

    void print() const {
        std::string from = sender.size() > 14 ? sender.substr(0,14)+"..." : sender;
        std::string to   = receiver.size() > 14 ? receiver.substr(0,14)+"..." : receiver;
        std::cout << "    TX " << tx_id.substr(0,16) << "..."
                  << "  " << from << " -> " << to
                  << "  " << std::fixed << std::setprecision(4)
                  << amount << " AXC  [" << tx_type << "]"
                  << (signature.empty() ? " [UNSIGNED]" : " [signed]")
                  << "\n";
    }
};

// ================================================================
//  BlockHeader
// ================================================================
struct BlockHeader {
    uint64_t    index;
    Hash256     previous_hash;
    Hash256     merkle_root;
    uint64_t    timestamp;
    uint32_t    nonce;
    std::string validator;
    uint32_t    difficulty;
    std::string version;

    BlockHeader()
        : index(0)
        , previous_hash(ZERO_HASH)
        , merkle_root(ZERO_HASH)
        , timestamp(0)
        , nonce(0)
        , difficulty(2)
        , version("1.0.0")
    {}

    std::string serialize() const {
        std::ostringstream oss;
        oss << version << "|" << index << "|" << previous_hash << "|"
            << merkle_root << "|" << timestamp << "|"
            << nonce << "|" << validator << "|" << difficulty;
        return oss.str();
    }
};

// ================================================================
//  Block
// ================================================================
struct Block {
    BlockHeader              header;
    std::vector<Transaction> transactions;
    Hash256                  hash;

    Block() {}

    void compute_hash() { hash = dsha256(header.serialize()); }

    void compute_merkle_root() {
        if (transactions.empty()) { header.merkle_root = ZERO_HASH; return; }
        std::vector<Hash256> layer;
        for (const auto& tx : transactions)
            layer.push_back(sha256(tx.tx_id));
        while (layer.size() > 1) {
            if (layer.size() % 2 != 0) layer.push_back(layer.back());
            std::vector<Hash256> next;
            for (size_t i = 0; i < layer.size(); i += 2)
                next.push_back(sha256(layer[i] + layer[i+1]));
            layer = std::move(next);
        }
        header.merkle_root = layer[0];
    }

    void print() const {
        std::cout << "  Block #" << header.index
                  << "  hash=" << hash.substr(0,16) << "..."
                  << "  txs=" << transactions.size()
                  << "  nonce=" << header.nonce << "\n";
        for (const auto& tx : transactions) tx.print();
    }
};

} // namespace axion
