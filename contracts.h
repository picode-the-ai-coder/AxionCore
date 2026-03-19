#pragma once
// ================================================================
//  Axion Digitaverse - contracts.h
//  Example smart contracts written as AVM bytecode.
//
//  In production these are compiled from a high-level language
//  (Rust -> WASM -> AVM). Here they are hand-assembled.
//
//  Contracts:
//    1. SimpleStore   - persist and retrieve one value
//    2. Counter       - auto-incrementing counter
//    3. AXCEscrow     - time-locked AXC escrow vault
//    4. SimpleToken   - ERC-20 style fungible token
// ================================================================

#include "avm.h"

namespace axion {
namespace contracts {

// ── 1. SimpleStore ─────────────────────────────────────────
// Stores an initial value, returns it on every call.
inline std::vector<Instruction> simple_store(int64_t initial = 42) {
    return {
        { Op::PUSH,   initial },
        { Op::STORE,  std::string("value") },
        { Op::LOAD,   std::string("value") },
        { Op::RETURN, int64_t(0) }
    };
}

// ── 2. Counter ─────────────────────────────────────────────
// Increments a persistent counter by 1 on every call.
// Returns the new counter value.
inline std::vector<Instruction> counter() {
    return {
        { Op::LOAD,   std::string("count") },  // push current count
        { Op::PUSH,   int64_t(1) },            // push 1
        { Op::ADD,    int64_t(0) },            // count + 1
        { Op::DUP,    int64_t(0) },            // dup so we can return it
        { Op::STORE,  std::string("count") },  // save new count
        { Op::RETURN, int64_t(0) }             // return new count
    };
}

// ── 3. AXCEscrow ───────────────────────────────────────────
// Locks the AXC sent with the call.
// Records depositor, amount, and unlock block height.
// Emits "AXCLocked" event.
inline std::vector<Instruction> axc_escrow() {
    return {
        { Op::CALLER,  int64_t(0) },
        { Op::STORE,   std::string("depositor") },
        { Op::TXVAL,   int64_t(0) },
        { Op::STORE,   std::string("amount") },
        { Op::HEIGHT,  int64_t(0) },
        { Op::PUSH,    int64_t(100) },            // lock for 100 blocks
        { Op::ADD,     int64_t(0) },
        { Op::STORE,   std::string("unlock_height") },
        { Op::PUSH,    std::string("AXCLocked") },
        { Op::EMIT,    int64_t(0) },
        { Op::LOAD,    std::string("amount") },
        { Op::RETURN,  int64_t(0) }
    };
}

// ── 4. SimpleToken (ERC-20 style) ─────────────────────────
// Mints total_supply tokens to the deployer (caller).
// Records balance in contract storage.
// Emits "TokensMinted" event.
inline std::vector<Instruction> simple_token(int64_t total_supply = 1'000'000) {
    return {
        // Balance[caller] = total_supply
        { Op::CALLER,  int64_t(0) },
        { Op::DUP,     int64_t(0) },
        { Op::PUSH,    total_supply },
        { Op::SSTORE,  int64_t(0) },
        // total_supply key
        { Op::PUSH,    total_supply },
        { Op::STORE,   std::string("total_supply") },
        // emit event
        { Op::PUSH,    std::string("TokensMinted") },
        { Op::EMIT,    int64_t(0) },
        // return supply
        { Op::LOAD,    std::string("total_supply") },
        { Op::RETURN,  int64_t(0) }
    };
}

} // namespace contracts
} // namespace axion
