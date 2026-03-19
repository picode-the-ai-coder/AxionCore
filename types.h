#pragma once
// ================================================================
//  Axion Digitaverse - types.h
//  Shared primitive types and utilities used by every module.
//  Include this first whenever you need Bytes, Hash256, etc.
// ================================================================

#include "platform.h"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <ctime>

namespace axion {

// ── Primitive type aliases ─────────────────────────────────
using Bytes   = std::vector<uint8_t>;
using Hash256 = std::string;    // 64-char lowercase hex string
using PubKey  = std::string;    // hex-encoded public key
using Sig     = std::string;    // hex-encoded signature

// ── Constants ──────────────────────────────────────────────
inline const Hash256 ZERO_HASH(64, '0');

// ── Base error type ────────────────────────────────────────
struct AxionError : public std::runtime_error {
    explicit AxionError(const std::string& msg)
        : std::runtime_error("[Axion] " + msg) {}
};

// ── Timestamp: milliseconds since Unix epoch ───────────────
inline uint64_t now_ms() {
    return static_cast<uint64_t>(std::time(nullptr)) * 1000ULL;
}

// ── Hex utilities ──────────────────────────────────────────
inline std::string to_hex(const Bytes& data) {
    std::ostringstream oss;
    for (auto b : data)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    return oss.str();
}

inline Bytes from_hex(const std::string& hex) {
    Bytes out;
    for (size_t i = 0; i + 1 < hex.size(); i += 2) {
        unsigned int byte = 0;
        std::istringstream ss(hex.substr(i, 2));
        ss >> std::hex >> byte;
        out.push_back(static_cast<uint8_t>(byte));
    }
    return out;
}

} // namespace axion
