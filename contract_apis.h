#pragma once

#include <string>
#include <map>
#include <vector>
#include <iostream>

namespace axion {

// ====================================================================
// Python Smart Contract Base Class
// This file serves as documentation for what Python contracts
// can do when running in the Axion VM
// ====================================================================

/**
 * SmartContract Base Class for Python
 * 
 * All Axion Python smart contracts should inherit from this class.
 * It provides access to blockchain state, events, and transfers.
 * 
 * Example:
 * 
 *   from axion import SmartContract
 *   
 *   class MyContract(SmartContract):
 *       def __init__(self):
 *           super().__init__()
 *           self.initialize()
 *       
 *       def initialize(self):
 *           self.set_state("owner", msg.sender)
 *           self.emit_event("Initialized", {"owner": msg.sender})
 *       
 *       def do_something(self, value):
 *           self.set_state("last_value", value)
 *           return {"success": True, "stored": value}
 */

class PythonContractAPI {
public:
    // ================================================================
    // State Management
    // ================================================================
    
    /**
     * Set contract state (persisted to blockchain)
     * 
     * Example:
     *   self.set_state("balance", 1000)
     *   self.set_state("owner", msg.sender)
     */
    void set_state(const std::string& key, const std::string& value);
    
    /**
     * Get contract state
     * 
     * Example:
     *   balance = self.get_state("balance")
     */
    std::string get_state(const std::string& key);
    
    /**
     * Get all state keys
     */
    std::vector<std::string> get_state_keys();
    
    /**
     * Delete state entry
     */
    void delete_state(const std::string& key);
    
    // ================================================================
    // Events
    // ================================================================
    
    /**
     * Emit an event (logged on blockchain)
     * 
     * Example:
     *   self.emit_event("Transfer", {
     *       "from": msg.sender,
     *       "to": recipient,
     *       "amount": amount
     *   })
     */
    void emit_event(const std::string& event_type,
                   const std::map<std::string, std::string>& data);
    
    /**
     * Get all events for this contract
     */
    std::vector<std::map<std::string, std::string>> get_events();
    
    // ================================================================
    // Transfers & Payments
    // ================================================================
    
    /**
     * Transfer AXC to another address
     * 
     * Example:
     *   if self.transfer("axc1abc123", 100):
     *       self.emit_event("Transferred", {"to": "axc1abc123", "amount": 100})
     */
    bool transfer(const std::string& to_address, uint64_t amount);
    
    /**
     * Get contract balance
     */
    uint64_t get_balance();
    
    // ================================================================
    // Blockchain Context
    // ================================================================
    
    /**
     * Get current block number
     * 
     * Example:
     *   current_block = self.block_number()
     */
    uint64_t block_number();
    
    /**
     * Get current timestamp
     * 
     * Example:
     *   now = self.block_timestamp()
     */
    uint64_t block_timestamp();
    
    /**
     * Get message sender
     * 
     * Example:
     *   sender = self.msg_sender()
     */
    std::string msg_sender();
    
    /**
     * Get message value (AXC sent)
     * 
     * Example:
     *   amount = self.msg_value()
     */
    uint64_t msg_value();
    
    /**
     * Get this contract's address
     * 
     * Example:
     *   my_address = self.address()
     */
    std::string address();
    
    // ================================================================
    // Contract Interaction
    // ================================================================
    
    /**
     * Call another contract
     * 
     * Example:
     *   result = self.call_contract(
     *       "axc1def456",
     *       "transfer",
     *       {"to": "axc1ghi789", "amount": 50}
     *   )
     */
    std::string call_contract(
        const std::string& contract_address,
        const std::string& function_name,
        const std::map<std::string, std::string>& args
    );
    
    /**
     * Static call (read-only)
     */
    std::string static_call(
        const std::string& contract_address,
        const std::string& function_name,
        const std::map<std::string, std::string>& args
    );
    
    // ================================================================
    // Utilities
    // ================================================================
    
    /**
     * Require condition (revert if false)
     * 
     * Example:
     *   self.require(balance >= amount, "Insufficient balance")
     */
    void require(bool condition, const std::string& message);
    
    /**
     * Assert condition
     */
    void assert_(bool condition, const std::string& message);
    
    /**
     * Revert execution
     */
    void revert(const std::string& reason);
    
    /**
     * Log a message (for debugging)
     */
    void log(const std::string& message);
    
    // ================================================================
    // Cryptography
    // ================================================================
    
    /**
     * SHA256 hash
     */
    std::string sha256(const std::string& data);
    
    /**
     * Keccak256 hash
     */
    std::string keccak256(const std::string& data);
    
    /**
     * Recover address from signature
     */
    std::string recover_address(
        const std::string& message,
        const std::string& signature
    );
};

} // namespace axion

/**
 * Python Contract Example
 * 
 * from axion import SmartContract
 * 
 * class Counter(SmartContract):
 *     def initialize(self):
 *         self.set_state("count", "0")
 *         self.set_state("owner", msg.sender)
 *     
 *     def increment(self):
 *         count = int(self.get_state("count"))
 *         self.set_state("count", str(count + 1))
 *         self.emit_event("Incremented", {"count": str(count + 1)})
 *         return {"count": count + 1}
 *     
 *     def get_count(self):
 *         return {"count": int(self.get_state("count"))}
 *     
 *     def reset(self):
 *         self.require(msg.sender == self.get_state("owner"), "Not owner")
 *         self.set_state("count", "0")
 *         self.emit_event("Reset", {})
 */

// ====================================================================
// JavaScript Smart Contract Base Class
// ====================================================================

namespace axion {

/**
 * SmartContract Base Class for JavaScript
 * 
 * All Axion JavaScript smart contracts should inherit from this class.
 * 
 * Example:
 * 
 *   class MyContract extends SmartContract {
 *       constructor() {
 *           super();
 *           this.initialize();
 *       }
 *       
 *       initialize() {
 *           this.setState("owner", msg.sender);
 *           this.emitEvent("Initialized", {owner: msg.sender});
 *       }
 *       
 *       doSomething(value) {
 *           this.setState("lastValue", value);
 *           return {success: true, stored: value};
 *       }
 *   }
 */

class JavaScriptContractAPI {
public:
    // ================================================================
    // State Management
    // ================================================================
    
    /**
     * Set contract state
     */
    void setState(const std::string& key, const std::string& value);
    
    /**
     * Get contract state
     */
    std::string getState(const std::string& key);
    
    /**
     * Delete state
     */
    void deleteState(const std::string& key);
    
    // ================================================================
    // Events
    // ================================================================
    
    /**
     * Emit event
     */
    void emitEvent(const std::string& eventType,
                  const std::map<std::string, std::string>& data);
    
    // ================================================================
    // Transfers
    // ================================================================
    
    /**
     * Transfer AXC
     */
    bool transfer(const std::string& toAddress, uint64_t amount);
    
    /**
     * Get balance
     */
    uint64_t getBalance();
    
    // ================================================================
    // Context
    // ================================================================
    
    /**
     * Get blockchain info
     */
    uint64_t blockNumber();
    uint64_t blockTimestamp();
    std::string msgSender();
    uint64_t msgValue();
    std::string address();
    
    // ================================================================
    // Contract Calls
    // ================================================================
    
    /**
     * Call another contract
     */
    std::string callContract(
        const std::string& contractAddress,
        const std::string& functionName,
        const std::map<std::string, std::string>& args
    );
    
    // ================================================================
    // Utilities
    // ================================================================
    
    /**
     * Require condition
     */
    void require(bool condition, const std::string& message);
    
    /**
     * Revert
     */
    void revert(const std::string& reason);
};

} // namespace axion

/**
 * JavaScript Contract Example
 * 
 * class Counter extends SmartContract {
 *     constructor() {
 *         super();
 *         this.initialize();
 *     }
 *     
 *     initialize() {
 *         this.setState("count", "0");
 *         this.setState("owner", msg.sender);
 *     }
 *     
 *     increment() {
 *         let count = parseInt(this.getState("count"));
 *         this.setState("count", (count + 1).toString());
 *         this.emitEvent("Incremented", {count: (count + 1).toString()});
 *         return {count: count + 1};
 *     }
 *     
 *     getCount() {
 *         return {count: parseInt(this.getState("count"))};
 *     }
 * }
 */
