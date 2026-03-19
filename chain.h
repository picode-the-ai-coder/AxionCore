#pragma once
// ================================================================
//  Axion Digitaverse - chain.h  [Phase 1]
//  The canonical Axion Decentralized Ledger.
//
//  Phase 1 upgrades:
//    1. Ed25519 signature verification in block validation
//       (Rule 7 added to the existing 6-rule validator)
//    2. Persistent storage via KVStore
//       - Every accepted block written to disk immediately
//       - Chain restored from disk on node restart
//    3. Persistent balance index (survives restarts)
// ================================================================

#include "adl.h"
#include "mempool.h"
#include "kvstore.h"
#include <unordered_map>
#include <optional>
#include <sstream>

namespace axion {

struct ChainError : public AxionError {
    explicit ChainError(const std::string& msg)
        : AxionError("[Chain] " + msg) {}
};

// ── Serialization helpers ──────────────────────────────────
// Simple pipe-delimited text encoding for block storage.
// Replace with protobuf/msgpack for production.

static std::string serialize_tx(const Transaction& tx) {
    std::ostringstream oss;
    oss << tx.tx_type << "|" << tx.tx_id << "|"
        << tx.sender  << "|" << tx.sender_pubkey << "|"
        << tx.receiver << "|"
        << std::fixed << std::setprecision(8) << tx.amount << "|"
        << tx.timestamp << "|" << tx.signature << "|" << tx.data;
    return oss.str();
}

static Transaction deserialize_tx(const std::string& s) {
    Transaction tx;
    std::vector<std::string> parts;
    std::istringstream iss(s);
    std::string tok;
    while (std::getline(iss, tok, '|')) parts.push_back(tok);
    if (parts.size() < 9) return tx;
    tx.tx_type       = parts[0];
    tx.tx_id         = parts[1];
    tx.sender        = parts[2];
    tx.sender_pubkey = parts[3];
    tx.receiver      = parts[4];
    try { tx.amount  = std::stod(parts[5]); } catch (...) {}
    try { tx.timestamp = std::stoull(parts[6]); } catch (...) {}
    tx.signature     = parts[7];
    tx.data          = parts.size() > 8 ? parts[8] : "";
    return tx;
}

static std::string serialize_block(const Block& b) {
    std::ostringstream oss;
    // Header line
    oss << b.header.version << "|"
        << b.header.index   << "|"
        << b.header.previous_hash << "|"
        << b.header.merkle_root   << "|"
        << b.header.timestamp     << "|"
        << b.header.nonce         << "|"
        << b.header.validator     << "|"
        << b.header.difficulty    << "|"
        << b.hash                 << "\n";
    // Transaction count
    oss << b.transactions.size() << "\n";
    for (const auto& tx : b.transactions)
        oss << serialize_tx(tx) << "\n";
    return oss.str();
}

static Block deserialize_block(const std::string& s) {
    Block b;
    std::istringstream iss(s);
    std::string line;
    // Header line
    if (!std::getline(iss, line)) return b;
    {
        std::vector<std::string> p;
        std::istringstream hss(line);
        std::string tok;
        while (std::getline(hss, tok, '|')) p.push_back(tok);
        if (p.size() < 9) return b;
        b.header.version       = p[0];
        try { b.header.index   = std::stoull(p[1]); } catch (...) {}
        b.header.previous_hash = p[2];
        b.header.merkle_root   = p[3];
        try { b.header.timestamp = std::stoull(p[4]); } catch (...) {}
        try { b.header.nonce   = std::stoul(p[5]); } catch (...) {}
        b.header.validator     = p[6];
        try { b.header.difficulty = std::stoul(p[7]); } catch (...) {}
        b.hash                 = p[8];
    }
    // Transaction count
    size_t tx_count = 0;
    if (std::getline(iss, line))
        try { tx_count = std::stoull(line); } catch (...) {}
    for (size_t i = 0; i < tx_count; ++i) {
        if (!std::getline(iss, line)) break;
        b.transactions.push_back(deserialize_tx(line));
    }
    return b;
}

// ── Chain ──────────────────────────────────────────────────
class Chain {
public:
    explicit Chain(uint32_t difficulty = 2,
                   const std::string& data_dir = "./axion_data")
        : difficulty_(difficulty)
        , storage_(data_dir)
        , db_(storage_.open("chain"))
        , balances_db_(storage_.open("balances"))
    {
        // Try to load existing chain from disk
        if (!load_from_disk()) {
            // No existing chain — create genesis
            chain_.push_back(make_genesis());
            index_balances(chain_[0]);
            persist_block(chain_[0]);
            std::cout << "[Chain] New chain created  genesis="
                      << chain_[0].hash.substr(0,16) << "...\n";
        } else {
            std::cout << "[Chain] Restored from disk  height="
                      << height() << "  blocks=" << block_count() << "\n";
        }
    }

    // ── Accessors ──────────────────────────────────────────
    uint64_t     height()      const { return chain_.back().header.index; }
    Hash256      last_hash()   const { return chain_.back().hash; }
    size_t       block_count() const { return chain_.size(); }
    const Block& tip()         const { return chain_.back(); }
    const Block& genesis()     const { return chain_[0]; }
    uint32_t     difficulty()  const { return difficulty_; }

    const Block* get_block_by_hash(const Hash256& h) const {
        auto it = hash_to_index_.find(h);
        if (it == hash_to_index_.end()) return nullptr;
        return get_block(it->second);
    }

    const Block* get_block(uint64_t idx) const {
        if (idx >= chain_.size()) return nullptr;
        return &chain_[static_cast<size_t>(idx)];
    }

    double get_balance(const std::string& address) const {
        auto it = balances_.find(address);
        return (it != balances_.end()) ? it->second : 0.0;
    }

    // ── Build a candidate block ────────────────────────────
    Block create_candidate(std::vector<Transaction> txs,
                           const std::string& validator_address) const {
        Block b;
        b.header.index         = height() + 1;
        b.header.previous_hash = last_hash();
        b.header.timestamp     = now_ms();
        b.header.validator     = validator_address;
        b.header.difficulty    = difficulty_;
        b.header.version       = "1.0.0";
        b.header.nonce         = 0;
        b.transactions         = std::move(txs);
        for (auto& tx : b.transactions)
            if (tx.tx_id.empty()) tx.compute_id();
        b.compute_merkle_root();
        return b;
    }

    // ── Proof-of-Work mining ───────────────────────────────
    void mine(Block& block) const {
        std::string target(difficulty_, '0');
        uint32_t attempts = 0;
        while (true) {
            block.header.nonce = attempts++;
            block.compute_hash();
            if (block.hash.substr(0, difficulty_) == target) break;
        }
        std::cout << "[Chain] Mined #" << block.header.index
                  << "  nonce=" << block.header.nonce
                  << "  hash=" << block.hash.substr(0,16) << "...\n";
    }

    // ── Validate + append block ────────────────────────────
    void add_block(Block& block) {
        validate_block(block);
        index_balances(block);
        hash_to_index_[block.hash] = static_cast<size_t>(block.header.index);
        chain_.push_back(block);
        persist_block(block);
        persist_balances();
        rebuild_tx_index();
        std::cout << "[Chain] Accepted #" << block.header.index
                  << "  txs=" << block.transactions.size()
                  << "  hash=" << block.hash.substr(0,16) << "...\n";
    }

    void mine_and_add(Block& block) { mine(block); add_block(block); }

    // ── Full chain integrity check ─────────────────────────
    bool is_valid() const {
        for (size_t i = 1; i < chain_.size(); ++i) {
            const Block& prev = chain_[i-1];
            const Block& curr = chain_[i];
            if (curr.hash != dsha256(curr.header.serialize())) return false;
            if (curr.header.previous_hash != prev.hash)         return false;
            if (curr.header.index != prev.header.index + 1)     return false;
            if (curr.header.difficulty > 0) {
                std::string tgt(curr.header.difficulty, '0');
                if (curr.hash.substr(0, curr.header.difficulty) != tgt)
                    return false;
            }
        }
        return true;
    }

    void print_summary() const {
        std::cout << "\n+---------------------------------------+\n";
        std::cout << "|  Axion Decentralized Ledger (ADL)     |\n";
        std::cout << "+---------------------------------------+\n";
        std::cout << "  Height     : " << height()      << "\n";
        std::cout << "  Blocks     : " << block_count() << "\n";
        std::cout << "  Difficulty : " << difficulty_   << "\n";
        std::cout << "  Data dir   : " << storage_.data_dir() << "\n";
        std::cout << "  Tip hash   : " << last_hash().substr(0,32) << "...\n";
        std::cout << "  Valid      : " << (is_valid() ? "YES" : "NO") << "\n";
        std::cout << "+---------------------------------------+\n\n";
    }

    void print_balances() const {
        std::cout << "\n-- Balances ----------------------------\n";
        for (const auto& [addr, bal] : balances_)
            if (bal > 0.0)
                std::cout << "  " << addr.substr(0,20) << "...  "
                          << std::fixed << std::setprecision(4)
                          << bal << " AXC\n";
    }

private:
    std::vector<Block>                       chain_;
    std::unordered_map<Hash256, size_t>      hash_to_index_;
    std::unordered_map<std::string, const Transaction*> tx_index_;
    std::unordered_map<std::string, double>  balances_;
    uint32_t                                 difficulty_;
    StorageManager                           storage_;
    KVStore&                                 db_;
    KVStore&                                 balances_db_;

    // ── Genesis block ──────────────────────────────────────
    static Block make_genesis() {
        Block g;
        g.header.index          = 0;
        g.header.previous_hash  = ZERO_HASH;
        g.header.timestamp      = 1700000000000ULL;
        g.header.validator      = "axion-genesis";
        g.header.difficulty     = 0;
        g.header.version        = "1.0.0";
        g.header.nonce          = 0;
        g.header.merkle_root    = ZERO_HASH;
        g.compute_hash();
        return g;
    }

    // ── 7-rule block validation (Rule 7 = Ed25519 sigs) ────
    void validate_block(Block& block) const {
        const Block& prev = chain_.back();

        // Rule 1: correct height
        if (block.header.index != prev.header.index + 1)
            throw ChainError("Wrong index: got "
                + std::to_string(block.header.index)
                + " expected " + std::to_string(prev.header.index + 1));

        // Rule 2: hash chain link
        if (block.header.previous_hash != prev.hash)
            throw ChainError("Previous hash mismatch at #"
                + std::to_string(block.header.index));

        // Rule 3: hash integrity
        if (block.hash != dsha256(block.header.serialize()))
            throw ChainError("Block hash invalid at #"
                + std::to_string(block.header.index));

        // Rule 4: PoW difficulty target
        if (block.header.difficulty > 0) {
            std::string tgt(block.header.difficulty, '0');
            if (block.hash.substr(0, block.header.difficulty) != tgt)
                throw ChainError("PoW target not met at #"
                    + std::to_string(block.header.index));
        }

        // Rule 5: Merkle root consistency
        Block tmp; tmp.transactions = block.transactions;
        tmp.compute_merkle_root();
        if (tmp.header.merkle_root != block.header.merkle_root)
            throw ChainError("Merkle root mismatch at #"
                + std::to_string(block.header.index));

        // Rule 6: sufficient balance
        for (const auto& tx : block.transactions) {
            if (tx.is_coinbase()) continue;
            if (tx.amount <= 0.0)
                throw ChainError("Non-positive amount in tx: " + tx.tx_id);
            double bal = get_balance(tx.sender);
            if (bal < tx.amount)
                throw ChainError("Insufficient balance for "
                    + tx.sender.substr(0,12) + " (has "
                    + std::to_string(bal) + " needs "
                    + std::to_string(tx.amount) + ")");
        }

        // Rule 7 [Phase 1 NEW]: Ed25519 signature verification
        for (const auto& tx : block.transactions) {
            if (tx.is_coinbase()) continue;
            if (!tx.verify_signature())
                throw ChainError(
                    "Invalid Ed25519 signature on tx: " + tx.tx_id
                    + " from " + tx.sender.substr(0,12));
        }
    }

    void index_balances(const Block& block) {
        for (const auto& tx : block.transactions) {
            if (!tx.is_coinbase())
                balances_[tx.sender]   -= tx.amount;
            balances_[tx.receiver] += tx.amount;
        }
    }

    void rebuild_tx_index() {
        tx_index_.clear();
        for (auto& blk : chain_)
            for (auto& tx : blk.transactions)
                tx_index_[tx.tx_id] = &tx;
    }

    // ── Persistence ────────────────────────────────────────
    void persist_block(const Block& block) {
        std::string key = "block:" + std::to_string(block.header.index);
        db_.put(key, serialize_block(block));
        // Also store hash -> index mapping
        db_.put("hash:" + block.hash, std::to_string(block.header.index));
        // Store latest height
        db_.put("meta:height", std::to_string(block.header.index));
    }

    void persist_balances() {
        for (const auto& [addr, bal] : balances_)
            balances_db_.put(addr, std::to_string(bal));
    }

    bool load_from_disk() {
        std::string height_str;
        if (!db_.get("meta:height", height_str)) return false;

        uint64_t stored_height = 0;
        try { stored_height = std::stoull(height_str); } catch (...) { return false; }

        chain_.clear();
        balances_.clear();

        for (uint64_t i = 0; i <= stored_height; ++i) {
            std::string key = "block:" + std::to_string(i);
            std::string val;
            if (!db_.get(key, val)) {
                std::cerr << "[Chain] Missing block " << i << " in storage\n";
                return false;
            }
            Block b = deserialize_block(val);
            hash_to_index_[b.hash] = static_cast<size_t>(b.header.index);
            chain_.push_back(b);
        }

        // Restore balances from balance store
        for (auto& blk : chain_)
            index_balances(blk);

        rebuild_tx_index();
        return !chain_.empty();
    }
};

} // namespace axion
