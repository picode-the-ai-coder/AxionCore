#include <iostream>
#include <cassert>
#include "avm_v2.h"

using namespace axion;

// ====================================================================
// AVM Tests
// ====================================================================

void test_avm_initialization() {
    std::cout << "\n[TEST] AVM Initialization" << std::endl;
    std::cout << "================================" << std::endl;
    
    bool success = init_advanced_avm();
    assert(success && "AVM initialization failed");
    
    AdvancedAVM* avm = get_avm();
    assert(avm != nullptr && "AVM instance is null");
    
    std::cout << "[PASS] AVM initialized successfully" << std::endl;
}

void test_contract_deployment() {
    std::cout << "\n[TEST] Contract Deployment" << std::endl;
    std::cout << "================================" << std::endl;
    
    AdvancedAVM* avm = get_avm();
    assert(avm != nullptr);
    
    // Simple Python contract code
    std::string contract_code = R"(
class Counter:
    def initialize(self):
        self.set_state("count", "0")
    
    def increment(self):
        count = int(self.get_state("count"))
        self.set_state("count", str(count + 1))
        return {"count": count + 1}
)";
    
    std::string address = avm->deploy_contract(
        contract_code,
        RuntimeType::PYTHON,
        "0xCreator123",
        1000000
    );
    
    assert(!address.empty() && "Contract deployment returned empty address");
    assert(avm->contract_exists(address) && "Contract not found after deployment");
    
    std::cout << "[PASS] Contract deployed at: " << address << std::endl;
}

void test_contract_state() {
    std::cout << "\n[TEST] Contract State Management" << std::endl;
    std::cout << "================================" << std::endl;
    
    AdvancedAVM* avm = get_avm();
    
    std::string code = "class Test: pass";
    std::string address = avm->deploy_contract(code, RuntimeType::PYTHON, "0xTest", 1000000);
    
    // Get state (should be empty)
    auto state = avm->get_contract_state(address);
    assert(state.empty() && "State should be empty initially");
    
    std::cout << "[PASS] Contract state management working" << std::endl;
}

void test_contract_listing() {
    std::cout << "\n[TEST] Contract Listing" << std::endl;
    std::cout << "================================" << std::endl;
    
    AdvancedAVM* avm = get_avm();
    
    auto contracts = avm->list_contracts();
    std::cout << "[INFO] Total contracts deployed: " << contracts.size() << std::endl;
    
    assert(contracts.size() > 0 && "Should have deployed contracts");
    
    for (const auto& addr : contracts) {
        std::cout << "  - " << addr << std::endl;
    }
    
    std::cout << "[PASS] Contract listing working" << std::endl;
}

void test_contract_stats() {
    std::cout << "\n[TEST] Contract Statistics" << std::endl;
    std::cout << "================================" << std::endl;
    
    AdvancedAVM* avm = get_avm();
    auto contracts = avm->list_contracts();
    
    if (!contracts.empty()) {
        auto stats = avm->get_contract_stats(contracts[0]);
        std::cout << "[INFO] Contract stats:" << std::endl;
        std::cout << "  - Gas used: " << stats.total_gas_used << std::endl;
        std::cout << "  - Call count: " << stats.call_count << std::endl;
        std::cout << "  - Event count: " << stats.event_count << std::endl;
        std::cout << "[PASS] Contract statistics retrieved" << std::endl;
    }
}

void test_avm_version() {
    std::cout << "\n[TEST] AVM Version" << std::endl;
    std::cout << "================================" << std::endl;
    
    std::string version = AdvancedAVM::get_version();
    std::cout << "[INFO] AVM Version: " << version << std::endl;
    
    assert(version.find("v2") != std::string::npos && "Version should contain v2");
    
    std::cout << "[PASS] AVM version correct" << std::endl;
}

void test_contract_pause_resume() {
    std::cout << "\n[TEST] Contract Pause/Resume" << std::endl;
    std::cout << "================================" << std::endl;
    
    AdvancedAVM* avm = get_avm();
    
    std::string code = "class Test: pass";
    std::string address = avm->deploy_contract(code, RuntimeType::PYTHON, "0xTest2", 1000000);
    
    auto contract = avm->get_contract(address);
    assert(contract != nullptr);
    assert(contract->status == ContractStatus::DEPLOYED);
    
    // Pause
    avm->pause_contract(address);
    contract = avm->get_contract(address);
    assert(contract->status == ContractStatus::PAUSED);
    
    // Resume
    avm->resume_contract(address);
    contract = avm->get_contract(address);
    assert(contract->status == ContractStatus::DEPLOYED);
    
    std::cout << "[PASS] Contract pause/resume working" << std::endl;
}

void test_runtime_type() {
    std::cout << "\n[TEST] Runtime Types" << std::endl;
    std::cout << "================================" << std::endl;
    
    AdvancedAVM* avm = get_avm();
    
    // Python runtime
    std::string python_code = "class PyContract: pass";
    std::string py_addr = avm->deploy_contract(
        python_code,
        RuntimeType::PYTHON,
        "0xPy",
        1000000
    );
    
    auto py_contract = avm->get_contract(py_addr);
    assert(py_contract->runtime == RuntimeType::PYTHON);
    
    // JavaScript runtime
    std::string js_code = "class JsContract {}";
    std::string js_addr = avm->deploy_contract(
        js_code,
        RuntimeType::JAVASCRIPT,
        "0xJs",
        1000000
    );
    
    auto js_contract = avm->get_contract(js_addr);
    assert(js_contract->runtime == RuntimeType::JAVASCRIPT);
    
    std::cout << "[PASS] Multiple runtime types supported" << std::endl;
}

// ====================================================================
// Main Test Runner
// ====================================================================

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║         PHASE 2 - ADVANCED AVM v2.0 TEST SUITE             ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════╝" << std::endl;
    
    try {
        test_avm_initialization();
        test_contract_deployment();
        test_contract_state();
        test_contract_listing();
        test_contract_stats();
        test_avm_version();
        test_contract_pause_resume();
        test_runtime_type();
        
        std::cout << "\n";
        std::cout << "╔════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║                    ✅ ALL TESTS PASSED                     ║" << std::endl;
        std::cout << "║                                                            ║" << std::endl;
        std::cout << "║  Phase 2 Advanced AVM is ready for full implementation!   ║" << std::endl;
        std::cout << "╚════════════════════════════════════════════════════════════╝" << std::endl;
        
        // Cleanup
        shutdown_advanced_avm();
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "\n[ERROR] Test exception: " << e.what() << std::endl;
        return 1;
    }
}
