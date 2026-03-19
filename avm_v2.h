#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <functional>
#include <cstdint>
#include <stdexcept>
#include <iostream>

namespace axion {

// ====================================================================
// ADVANCED VM v2.0 - Multi-Language Smart Contract Execution
// Supports: Python, JavaScript, Future: WebAssembly
// ====================================================================

enum class RuntimeType {
    PYTHON = 0,
    JAVASCRIPT = 1,
    WASM = 2  // Future
};

enum class ContractStatus {
    PENDING,
    DEPLOYED,
    EXECUTING,
    COMPLETED,
    FAILED,
    PAUSED
};

// ====================================================================
// Contract Event
// ====================================================================

struct ContractEvent {
    std::string event_type;
    std::string contract_address;
    std::map<std::string, std::string> data;
    uint64_t block_number;
    uint32_t tx_index;
    uint64_t timestamp;
};

// ====================================================================
// Contract Call Result
// ====================================================================

struct ContractCallResult {
    bool success;
    std::string result;  // Return value as JSON string
    std::string error;
    uint64_t gas_used;
    std::vector<ContractEvent> events;
    std::map<std::string, std::string> state_changes;
};

// ====================================================================
// Smart Contract Definition
// ====================================================================

class SmartContract {
public:
    std::string address;          // Contract address (hash)
    std::string code;             // Source code
    RuntimeType runtime;          // Language
    std::string code_hash;        // SHA256 of code
    
    std::map<std::string, std::string> state;  // Persistent state
    
    std::string creator;          // Deployer address
    uint64_t creation_block;
    uint64_t creation_timestamp;
    
    ContractStatus status;
    uint64_t gas_limit;
    uint64_t gas_used_total;
    
    std::vector<ContractEvent> events;
    
    // Constructor
    SmartContract() 
        : runtime(RuntimeType::PYTHON),
          creation_block(0),
          creation_timestamp(0),
          status(ContractStatus::PENDING),
          gas_limit(1000000),
          gas_used_total(0) {}
    
    // Serialize to JSON
    std::string to_json() const;
    
    // Deserialize from JSON
    static SmartContract from_json(const std::string& json);
};

// ====================================================================
// Python Runtime Environment
// ====================================================================

class PythonRuntime {
public:
    explicit PythonRuntime(uint64_t gas_limit = 1000000);
    ~PythonRuntime();
    
    // Initialize Python interpreter
    bool initialize();
    
    // Execute Python function
    ContractCallResult execute(
        const std::string& code,
        const std::string& function_name,
        const std::map<std::string, std::string>& args,
        const std::map<std::string, std::string>& contract_state
    );
    
    // Get current gas usage
    uint64_t get_gas_used() const { return gas_used_; }
    
    // Reset state for new execution
    void reset();
    
private:
    uint64_t gas_limit_;
    uint64_t gas_used_;
    bool initialized_;
    
    // Python sandbox utilities
    bool setup_sandbox();
    void teardown_sandbox();
};

// ====================================================================
// JavaScript (V8) Runtime Environment
// ====================================================================

class JavaScriptRuntime {
public:
    explicit JavaScriptRuntime(uint64_t gas_limit = 1000000);
    ~JavaScriptRuntime();
    
    // Initialize V8 engine
    bool initialize();
    
    // Execute JavaScript function
    ContractCallResult execute(
        const std::string& code,
        const std::string& function_name,
        const std::map<std::string, std::string>& args,
        const std::map<std::string, std::string>& contract_state
    );
    
    // Get current gas usage
    uint64_t get_gas_used() const { return gas_used_; }
    
    // Reset state
    void reset();
    
private:
    uint64_t gas_limit_;
    uint64_t gas_used_;
    bool initialized_;
};

// ====================================================================
// Advanced VM v2.0 - Main Class
// ====================================================================

class AdvancedAVM {
public:
    AdvancedAVM();
    ~AdvancedAVM();
    
    // ================================================================
    // Contract Deployment
    // ================================================================
    
    /**
     * Deploy a new smart contract
     * @param code Source code of the contract
     * @param runtime Language (PYTHON or JAVASCRIPT)
     * @param creator Address of deployer
     * @param gas_limit Maximum gas allowed
     * @return Contract address (hash of code + creator)
     */
    std::string deploy_contract(
        const std::string& code,
        RuntimeType runtime,
        const std::string& creator,
        uint64_t gas_limit = 1000000
    );
    
    // ================================================================
    // Contract Execution
    // ================================================================
    
    /**
     * Call a function in a deployed contract
     * @param contract_address Address of deployed contract
     * @param function_name Function to call
     * @param args Function arguments (as JSON string)
     * @param caller Address of caller
     * @param value AXC amount to send
     * @return Result with return value, gas used, events
     */
    ContractCallResult call_contract(
        const std::string& contract_address,
        const std::string& function_name,
        const std::map<std::string, std::string>& args,
        const std::string& caller,
        uint64_t value = 0
    );
    
    /**
     * Static call (read-only, no state changes)
     */
    ContractCallResult static_call(
        const std::string& contract_address,
        const std::string& function_name,
        const std::map<std::string, std::string>& args
    );
    
    // ================================================================
    // Contract Management
    // ================================================================
    
    /**
     * Get contract by address
     */
    SmartContract* get_contract(const std::string& address);
    
    /**
     * Get contract state
     */
    std::map<std::string, std::string> get_contract_state(
        const std::string& contract_address
    );
    
    /**
     * Get contract code
     */
    std::string get_contract_code(const std::string& contract_address);
    
    /**
     * Pause/resume contract
     */
    void pause_contract(const std::string& contract_address);
    void resume_contract(const std::string& contract_address);
    
    /**
     * Get contract events
     */
    std::vector<ContractEvent> get_contract_events(
        const std::string& contract_address,
        uint64_t from_block = 0,
        uint64_t to_block = UINT64_MAX
    );
    
    // ================================================================
    // Validation & Analysis
    // ================================================================
    
    /**
     * Validate contract code before deployment
     */
    bool validate_contract(
        const std::string& code,
        RuntimeType runtime
    );
    
    /**
     * Check if contract exists
     */
    bool contract_exists(const std::string& contract_address);
    
    /**
     * Get contract statistics
     */
    struct ContractStats {
        uint64_t total_gas_used;
        uint64_t call_count;
        uint64_t event_count;
        std::string last_execution_time;
    };
    
    ContractStats get_contract_stats(const std::string& contract_address);
    
    // ================================================================
    // Storage Management
    // ================================================================
    
    /**
     * Persist contracts to disk
     */
    bool save_contracts(const std::string& filepath);
    
    /**
     * Load contracts from disk
     */
    bool load_contracts(const std::string& filepath);
    
    /**
     * Clear all contracts (dangerous!)
     */
    void clear_all_contracts();
    
    // ================================================================
    // Utilities
    // ================================================================
    
    /**
     * Get total contracts deployed
     */
    uint64_t get_contract_count() const {
        return contracts_.size();
    }
    
    /**
     * List all contract addresses
     */
    std::vector<std::string> list_contracts() const;
    
    /**
     * Get VM version
     */
    static std::string get_version() {
        return "AdvancedAVM v2.0.0";
    }

private:
    // Storage
    std::map<std::string, std::shared_ptr<SmartContract>> contracts_;
    
    // Runtimes
    std::unique_ptr<PythonRuntime> python_runtime_;
    std::unique_ptr<JavaScriptRuntime> js_runtime_;
    
    // Utilities
    std::string compute_contract_address(
        const std::string& code,
        const std::string& creator,
        uint64_t nonce
    );
    
    uint64_t contract_nonce_;
};

// ====================================================================
// Global VM Instance
// ====================================================================

extern AdvancedAVM* g_advanced_avm;

/**
 * Initialize the Advanced VM
 */
bool init_advanced_avm();

/**
 * Shutdown the Advanced VM
 */
void shutdown_advanced_avm();

/**
 * Get global AVM instance
 */
inline AdvancedAVM* get_avm() {
    return g_advanced_avm;
}

} // namespace axion
