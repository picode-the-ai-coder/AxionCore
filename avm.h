#pragma once
// ================================================================
//  Axion Digitaverse - avm.h
//  Axion Virtual Machine (AVM)
//
//  A stack-based bytecode interpreter for smart contracts.
//
//  Features:
//    - 30 opcodes: stack, arithmetic, comparison, control flow,
//      persistent storage, blockchain context, events, hashing
//    - Gas metering (prevents infinite loops)
//    - Per-contract isolated key-value storage
//    - Event emission system
//
//  Production upgrade:
//    Replace with wasm3 or wasmer-c-api WASM runtime for
//    full language support (Rust, C++, AssemblyScript).
// ================================================================

#include "types.h"
#include "sha256.h"
#include <stack>
#include <unordered_map>
#include <functional>
#include <variant>
#include <optional>

namespace axion {

// ── Opcodes ────────────────────────────────────────────────
enum class Op : uint8_t {
    // Stack manipulation
    PUSH   = 0x01,   // push literal value onto stack
    POP    = 0x02,   // discard top of stack
    DUP    = 0x03,   // duplicate top of stack
    SWAP   = 0x04,   // swap top two items

    // Arithmetic
    ADD    = 0x10,
    SUB    = 0x11,
    MUL    = 0x12,
    DIV    = 0x13,   // integer division (throws on div-by-zero)
    MOD    = 0x14,

    // Comparison (push 1 if true, 0 if false)
    EQ     = 0x20,
    NEQ    = 0x21,
    LT     = 0x22,
    GT     = 0x23,
    LTE    = 0x24,
    GTE    = 0x25,

    // Control flow
    JMP    = 0x30,   // unconditional jump to offset
    JIF    = 0x31,   // jump if top != 0
    JIFN   = 0x32,   // jump if top == 0
    HALT   = 0x33,   // stop, return top of stack
    RETURN = 0x34,   // return top of stack

    // Persistent contract storage
    STORE  = 0x40,   // STORE <key>: pop value, write to storage[key]
    LOAD   = 0x41,   // LOAD  <key>: push storage[key]
    SSTORE = 0x42,   // SSTORE: pop key then value, write storage[key]=value
    SLOAD  = 0x43,   // SLOAD: pop key, push storage[key]

    // Blockchain context (read-only)
    CALLER  = 0x50,  // push caller address string
    BALANCE = 0x51,  // push caller AXC balance * 1e8 (integer satoshis)
    HEIGHT  = 0x52,  // push current block height
    TXVAL   = 0x53,  // push transaction value * 1e8

    // Events
    EMIT   = 0x60,   // pop event name string, emit as log event

    // Crypto
    HASH   = 0x70,   // pop string, push sha256(string)
};

// ── Gas cost per opcode ────────────────────────────────────
inline uint64_t gas_cost(Op op) {
    switch (op) {
    case Op::PUSH: case Op::POP: case Op::DUP: case Op::SWAP:   return 1;
    case Op::ADD:  case Op::SUB: case Op::MUL:                   return 3;
    case Op::DIV:  case Op::MOD:                                 return 5;
    case Op::EQ: case Op::NEQ:
    case Op::LT: case Op::GT: case Op::LTE: case Op::GTE:       return 2;
    case Op::JMP: case Op::JIF: case Op::JIFN:                   return 4;
    case Op::STORE: case Op::SSTORE:                             return 20;
    case Op::LOAD:  case Op::SLOAD:                              return 5;
    case Op::HASH:                                               return 30;
    case Op::EMIT:                                               return 10;
    default:                                                     return 1;
    }
}

// ── Value type: integer or string ─────────────────────────
using VMValue = std::variant<int64_t, std::string>;

inline std::string value_to_str(const VMValue& v) {
    if (std::holds_alternative<int64_t>(v))
        return std::to_string(std::get<int64_t>(v));
    return std::get<std::string>(v);
}

inline int64_t value_to_int(const VMValue& v) {
    if (std::holds_alternative<int64_t>(v))
        return std::get<int64_t>(v);
    try { return std::stoll(std::get<std::string>(v)); }
    catch (...) { return 0; }
}

// ── A single bytecode instruction ─────────────────────────
struct Instruction {
    Op      opcode;
    VMValue operand;  // literal value for PUSH, key for STORE/LOAD
    Instruction(Op op, VMValue val = int64_t(0))
        : opcode(op), operand(std::move(val)) {}
};

// ── Deployed contract record ───────────────────────────────
struct Contract {
    std::string                              address;
    std::vector<Instruction>                 bytecode;
    std::unordered_map<std::string, VMValue> storage;  // persistent state
    std::string                              owner;
    uint64_t                                 deploy_height;
};

// ── Context passed into every contract call ────────────────
struct ExecContext {
    std::string caller;
    double      value;           // AXC sent with call
    uint64_t    block_height;
    double      caller_balance;
};

// ── Result returned from a contract call ──────────────────
struct ExecResult {
    bool                     success;
    VMValue                  return_value;
    uint64_t                 gas_used;
    std::string              error;
    std::vector<std::string> events;
};

// ── AVM Engine ─────────────────────────────────────────────
class AVM {
public:
    explicit AVM(uint64_t default_gas_limit = 1'000'000)
        : default_gas_limit_(default_gas_limit) {}

    // Deploy a contract. Returns its unique address.
    std::string deploy(std::vector<Instruction> bytecode,
                       const std::string& owner,
                       uint64_t block_height)
    {
        Contract c;
        c.address       = sha256(owner
                            + std::to_string(bytecode.size())
                            + std::to_string(block_height));
        c.bytecode      = std::move(bytecode);
        c.owner         = owner;
        c.deploy_height = block_height;
        std::string addr = c.address;
        contracts_[addr] = std::move(c);
        std::cout << "[AVM] Contract deployed: " << addr.substr(0, 16)
                  << "...  owner=" << owner.substr(0, 12) << "...\n";
        return addr;
    }

    // Execute a contract call
    ExecResult call(const std::string& contract_addr,
                    const ExecContext& ctx,
                    uint64_t gas_limit = 0)
    {
        ExecResult result{ false, int64_t(0), 0, "", {} };

        auto it = contracts_.find(contract_addr);
        if (it == contracts_.end()) {
            result.error = "Contract not found: " + contract_addr;
            return result;
        }

        Contract& contract = it->second;
        uint64_t  limit    = gas_limit ? gas_limit : default_gas_limit_;
        std::stack<VMValue> stack;
        size_t pc = 0;

        while (pc < contract.bytecode.size()) {
            const Instruction& instr = contract.bytecode[pc];
            uint64_t cost = gas_cost(instr.opcode);
            if (result.gas_used + cost > limit) {
                result.error = "Out of gas";
                return result;
            }
            result.gas_used += cost;

            switch (instr.opcode) {

            case Op::PUSH:  stack.push(instr.operand); break;

            case Op::POP:
                if (!stack.empty()) stack.pop();
                break;

            case Op::DUP:
                if (!stack.empty()) stack.push(stack.top());
                break;

            case Op::SWAP: {
                if (stack.size() < 2) { result.error = "SWAP underflow"; return result; }
                VMValue a = stack.top(); stack.pop();
                VMValue b = stack.top(); stack.pop();
                stack.push(a); stack.push(b);
                break;
            }

            case Op::ADD: { auto b=pop(stack); auto a=pop(stack); stack.push(value_to_int(a)+value_to_int(b)); break; }
            case Op::SUB: { auto b=pop(stack); auto a=pop(stack); stack.push(value_to_int(a)-value_to_int(b)); break; }
            case Op::MUL: { auto b=pop(stack); auto a=pop(stack); stack.push(value_to_int(a)*value_to_int(b)); break; }
            case Op::DIV: {
                auto b=pop(stack); auto a=pop(stack);
                if (value_to_int(b)==0) { result.error="Division by zero"; return result; }
                stack.push(value_to_int(a)/value_to_int(b)); break;
            }
            case Op::MOD: {
                auto b=pop(stack); auto a=pop(stack);
                if (value_to_int(b)==0) { result.error="Modulo by zero"; return result; }
                stack.push(value_to_int(a)%value_to_int(b)); break;
            }

            case Op::EQ:  { auto b=pop(stack); auto a=pop(stack); stack.push(int64_t(value_to_str(a)==value_to_str(b)?1:0)); break; }
            case Op::NEQ: { auto b=pop(stack); auto a=pop(stack); stack.push(int64_t(value_to_str(a)!=value_to_str(b)?1:0)); break; }
            case Op::LT:  { auto b=pop(stack); auto a=pop(stack); stack.push(int64_t(value_to_int(a)<value_to_int(b)?1:0)); break; }
            case Op::GT:  { auto b=pop(stack); auto a=pop(stack); stack.push(int64_t(value_to_int(a)>value_to_int(b)?1:0)); break; }
            case Op::LTE: { auto b=pop(stack); auto a=pop(stack); stack.push(int64_t(value_to_int(a)<=value_to_int(b)?1:0)); break; }
            case Op::GTE: { auto b=pop(stack); auto a=pop(stack); stack.push(int64_t(value_to_int(a)>=value_to_int(b)?1:0)); break; }

            case Op::JMP:  pc = static_cast<size_t>(value_to_int(instr.operand)); continue;
            case Op::JIF:  { auto c=pop(stack); if(value_to_int(c)!=0){ pc=static_cast<size_t>(value_to_int(instr.operand)); continue; } break; }
            case Op::JIFN: { auto c=pop(stack); if(value_to_int(c)==0){ pc=static_cast<size_t>(value_to_int(instr.operand)); continue; } break; }

            case Op::STORE: {
                std::string key = value_to_str(instr.operand);
                contract.storage[key] = stack.empty() ? VMValue{int64_t(0)} : pop(stack);
                break;
            }
            case Op::LOAD: {
                std::string key = value_to_str(instr.operand);
                auto sit = contract.storage.find(key);
                stack.push(sit != contract.storage.end() ? sit->second : VMValue{int64_t(0)});
                break;
            }
            case Op::SSTORE: {
                if (stack.size() < 2) { result.error="SSTORE underflow"; return result; }
                VMValue val = pop(stack);
                std::string key = value_to_str(pop(stack));
                contract.storage[key] = val;
                break;
            }
            case Op::SLOAD: {
                std::string key = value_to_str(pop(stack));
                auto sit = contract.storage.find(key);
                stack.push(sit != contract.storage.end() ? sit->second : VMValue{int64_t(0)});
                break;
            }

            case Op::CALLER:  stack.push(ctx.caller); break;
            case Op::BALANCE: stack.push(int64_t(ctx.caller_balance * 1e8)); break;
            case Op::HEIGHT:  stack.push(int64_t(ctx.block_height)); break;
            case Op::TXVAL:   stack.push(int64_t(ctx.value * 1e8)); break;

            case Op::EMIT: {
                std::string ev = value_to_str(stack.empty() ? VMValue{std::string("event")} : pop(stack));
                result.events.push_back(ev);
                std::cout << "[AVM] Event emitted: " << ev << "\n";
                break;
            }

            case Op::HASH: {
                std::string s = value_to_str(stack.empty() ? VMValue{std::string("")} : pop(stack));
                stack.push(sha256(s));
                break;
            }

            case Op::RETURN:
            case Op::HALT:
                result.success      = true;
                result.return_value = stack.empty() ? VMValue{int64_t(0)} : stack.top();
                return result;

            default:
                result.error = "Unknown opcode";
                return result;
            }
            ++pc;
        }

        result.success      = true;
        result.return_value = stack.empty() ? VMValue{int64_t(0)} : stack.top();
        return result;
    }

    // Read a value from contract storage without executing
    std::optional<VMValue> read_storage(const std::string& contract_addr,
                                        const std::string& key) const {
        auto it = contracts_.find(contract_addr);
        if (it == contracts_.end()) return std::nullopt;
        auto sit = it->second.storage.find(key);
        if (sit == it->second.storage.end()) return std::nullopt;
        return sit->second;
    }

    bool contract_exists(const std::string& addr) const {
        return contracts_.count(addr) > 0;
    }

    void print_contracts() const {
        std::cout << "\n-- Deployed Contracts (" << contracts_.size() << ") ----\n";
        for (const auto& [addr, c] : contracts_)
            std::cout << "  " << addr.substr(0, 16) << "..."
                      << "  owner=" << c.owner.substr(0, 12) << "..."
                      << "  storage_keys=" << c.storage.size() << "\n";
    }

private:
    std::unordered_map<std::string, Contract> contracts_;
    uint64_t default_gas_limit_;

    static VMValue pop(std::stack<VMValue>& s) {
        VMValue v = s.top(); s.pop(); return v;
    }
};

} // namespace axion
