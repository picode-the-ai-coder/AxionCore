#pragma once
// ================================================================
//  Axion Digitaverse - wire.h
//  Binary wire protocol for node-to-node messages.
//
//  Every message on the wire:
//    [4 bytes] magic    = 0x41584E01  ("AXN\x01")
//    [1 byte]  msg_type
//    [4 bytes] payload_len (big-endian)
//    [N bytes] payload  (JSON or binary)
//
//  Message types:
//    0x01  HELLO      - handshake: version, node_id, listen_addr
//    0x02  PING       - keepalive request
//    0x03  PONG       - keepalive response
//    0x04  GET_PEERS  - request peer list
//    0x05  PEERS      - peer list response
//    0x06  NEW_TX     - broadcast a transaction
//    0x07  NEW_BLOCK  - broadcast a mined block
//    0x08  GET_BLOCKS - request blocks from height N
//    0x09  BLOCKS     - block data response
//    0x0A  INV        - inventory: list of known tx/block hashes
// ================================================================

#include "types.h"
#include "adl.h"
#include <cstring>

namespace axion {

static constexpr uint32_t WIRE_MAGIC = 0x41584E01u; // "AXN\x01"

enum class WireType : uint8_t {
    HELLO      = 0x01,
    PING       = 0x02,
    PONG       = 0x03,
    GET_PEERS  = 0x04,
    PEERS      = 0x05,
    NEW_TX     = 0x06,
    NEW_BLOCK  = 0x07,
    GET_BLOCKS = 0x08,
    BLOCKS     = 0x09,
    INV        = 0x0A,
};

// ── Low-level encode/decode helpers ───────────────────────

inline void write_u32(Bytes& out, uint32_t v) {
    out.push_back((v >> 24) & 0xff);
    out.push_back((v >> 16) & 0xff);
    out.push_back((v >>  8) & 0xff);
    out.push_back((v      ) & 0xff);
}

inline void write_u64(Bytes& out, uint64_t v) {
    for (int i = 7; i >= 0; --i)
        out.push_back((v >> (i * 8)) & 0xff);
}

inline uint32_t read_u32(const uint8_t* p) {
    return (uint32_t(p[0])<<24)|(uint32_t(p[1])<<16)|
           (uint32_t(p[2])<<8)|uint32_t(p[3]);
}

inline uint64_t read_u64(const uint8_t* p) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v = (v << 8) | p[i];
    return v;
}

inline void write_str(Bytes& out, const std::string& s) {
    write_u32(out, (uint32_t)s.size());
    out.insert(out.end(), s.begin(), s.end());
}

inline std::string read_str(const uint8_t*& p, const uint8_t* end) {
    if (p + 4 > end) return {};
    uint32_t len = read_u32(p); p += 4;
    if (p + len > end) return {};
    std::string s(reinterpret_cast<const char*>(p), len);
    p += len;
    return s;
}

inline void write_double(Bytes& out, double v) {
    uint64_t bits;
    std::memcpy(&bits, &v, 8);
    write_u64(out, bits);
}

inline double read_double(const uint8_t*& p, const uint8_t* end) {
    if (p + 8 > end) return 0.0;
    uint64_t bits = read_u64(p); p += 8;
    double v;
    std::memcpy(&v, &bits, 8);
    return v;
}

// ── Frame a message ────────────────────────────────────────
inline Bytes frame(WireType type, const Bytes& payload) {
    Bytes out;
    out.reserve(9 + payload.size());
    write_u32(out, WIRE_MAGIC);
    out.push_back(static_cast<uint8_t>(type));
    write_u32(out, (uint32_t)payload.size());
    out.insert(out.end(), payload.begin(), payload.end());
    return out;
}

// ── Transaction serialization ──────────────────────────────
inline Bytes encode_tx(const Transaction& tx) {
    Bytes out;
    write_str(out, tx.tx_id);
    write_str(out, tx.tx_type);
    write_str(out, tx.sender);
    write_str(out, tx.sender_pubkey);
    write_str(out, tx.receiver);
    write_double(out, tx.amount);
    write_u64(out, tx.timestamp);
    write_str(out, tx.signature);
    write_str(out, tx.data);
    return out;
}

inline Transaction decode_tx(const uint8_t*& p, const uint8_t* end) {
    Transaction tx;
    tx.tx_id         = read_str(p, end);
    tx.tx_type       = read_str(p, end);
    tx.sender        = read_str(p, end);
    tx.sender_pubkey = read_str(p, end);
    tx.receiver      = read_str(p, end);
    tx.amount        = read_double(p, end);
    if (p + 8 <= end) { tx.timestamp = read_u64(p); p += 8; }
    tx.signature     = read_str(p, end);
    tx.data          = read_str(p, end);
    return tx;
}

// ── Block serialization ────────────────────────────────────
inline Bytes encode_block(const Block& b) {
    Bytes out;
    // Header
    write_str(out, b.header.version);
    write_u64(out, b.header.index);
    write_str(out, b.header.previous_hash);
    write_str(out, b.header.merkle_root);
    write_u64(out, b.header.timestamp);
    write_u32(out, b.header.nonce);
    write_str(out, b.header.validator);
    write_u32(out, b.header.difficulty);
    write_str(out, b.hash);
    // Transactions
    write_u32(out, (uint32_t)b.transactions.size());
    for (const auto& tx : b.transactions) {
        Bytes txb = encode_tx(tx);
        write_u32(out, (uint32_t)txb.size());
        out.insert(out.end(), txb.begin(), txb.end());
    }
    return out;
}

inline Block decode_block(const uint8_t*& p, const uint8_t* end) {
    Block b;
    b.header.version       = read_str(p, end);
    if (p + 8 <= end) { b.header.index = read_u64(p); p += 8; }
    b.header.previous_hash = read_str(p, end);
    b.header.merkle_root   = read_str(p, end);
    if (p + 8 <= end) { b.header.timestamp = read_u64(p); p += 8; }
    if (p + 4 <= end) { b.header.nonce = read_u32(p); p += 4; }
    b.header.validator     = read_str(p, end);
    if (p + 4 <= end) { b.header.difficulty = read_u32(p); p += 4; }
    b.hash                 = read_str(p, end);
    uint32_t tx_count = 0;
    if (p + 4 <= end) { tx_count = read_u32(p); p += 4; }
    for (uint32_t i = 0; i < tx_count && p < end; ++i) {
        uint32_t tx_len = 0;
        if (p + 4 <= end) { tx_len = read_u32(p); p += 4; }
        const uint8_t* tx_end = std::min(p + tx_len, end);
        b.transactions.push_back(decode_tx(p, tx_end));
        p = tx_end;
    }
    return b;
}

// ── HELLO message ──────────────────────────────────────────
struct HelloMsg {
    std::string version;
    std::string node_id;
    std::string listen_addr;
    uint64_t    chain_height;
    Hash256     best_hash;
};

inline Bytes encode_hello(const HelloMsg& h) {
    Bytes out;
    write_str(out, h.version);
    write_str(out, h.node_id);
    write_str(out, h.listen_addr);
    write_u64(out, h.chain_height);
    write_str(out, h.best_hash);
    return out;
}

inline HelloMsg decode_hello(const uint8_t* p, const uint8_t* end) {
    HelloMsg h;
    h.version      = read_str(p, end);
    h.node_id      = read_str(p, end);
    h.listen_addr  = read_str(p, end);
    if (p + 8 <= end) { h.chain_height = read_u64(p); p += 8; }
    h.best_hash    = read_str(p, end);
    return h;
}

// ── PEERS message ──────────────────────────────────────────
inline Bytes encode_peers(const std::vector<std::string>& addrs) {
    Bytes out;
    write_u32(out, (uint32_t)addrs.size());
    for (const auto& a : addrs) write_str(out, a);
    return out;
}

inline std::vector<std::string> decode_peers(const uint8_t* p,
                                              const uint8_t* end) {
    std::vector<std::string> addrs;
    if (p + 4 > end) return addrs;
    uint32_t n = read_u32(p); p += 4;
    for (uint32_t i = 0; i < n && p < end; ++i)
        addrs.push_back(read_str(p, end));
    return addrs;
}

// ── GET_BLOCKS request ─────────────────────────────────────
inline Bytes encode_get_blocks(uint64_t from_height, uint32_t max_count = 500) {
    Bytes out;
    write_u64(out, from_height);
    write_u32(out, max_count);
    return out;
}

} // namespace axion
