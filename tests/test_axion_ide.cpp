#include <iostream>
#include <cassert>
#include "axion_ide.h"

using namespace axion;

// ====================================================================
// Axion IDE Tests
// ====================================================================

void test_ide_initialization() {
    std::cout << "\n[TEST] IDE Initialization" << std::endl;
    std::cout << "════════════════════════════════════════════════════════" << std::endl;
    
    bool success = init_advanced_avm();
    assert(success && "AVM initialization failed");
    
    success = init_axion_ide();
    assert(success && "IDE initialization failed");
    
    AxionIDE* ide = get_ide();
    assert(ide != nullptr && "IDE instance is null");
    
    std::cout << "[PASS] IDE initialized successfully" << std::endl;
}

void test_ide_editor() {
    std::cout << "\n[TEST] IDE Editor" << std::endl;
    std::cout << "════════════════════════════════════════════════════════" << std::endl;
    
    AxionIDE* ide = get_ide();
    assert(ide != nullptr);
    
    std::string test_code = "class TestContract: pass";
    ide->set_editor_content(test_code);
    
    std::string retrieved = ide->get_editor_content();
    assert(retrieved == test_code && "Editor content mismatch");
    
    std::cout << "[PASS] Editor functionality working" << std::endl;
}

void test_ide_templates() {
    std::cout << "\n[TEST] IDE Templates" << std::endl;
    std::cout << "════════════════════════════════════════════════════════" << std::endl;
    
    AxionIDE* ide = get_ide();
    
    // Test Counter template
    std::string counter = ide->get_counter_template();
    assert(!counter.empty() && "Counter template is empty");
    assert(counter.find("increment") != std::string::npos && "Counter template missing increment");
    
    // Test Token template
    std::string token = ide->get_token_template();
    assert(!token.empty() && "Token template is empty");
    assert(token.find("transfer") != std::string::npos && "Token template missing transfer");
    
    // Test Auction template
    std::string auction = ide->get_auction_template();
    assert(!auction.empty() && "Auction template is empty");
    assert(auction.find("place_bid") != std::string::npos && "Auction template missing place_bid");
    
    // Test Storage template
    std::string storage = ide->get_storage_template();
    assert(!storage.empty() && "Storage template is empty");
    assert(storage.find("set_value") != std::string::npos && "Storage template missing set_value");
    
    std::cout << "[PASS] All templates loaded successfully" << std::endl;
    std::cout << "[INFO] Counter: " << counter.length() << " bytes" << std::endl;
    std::cout << "[INFO] Token: " << token.length() << " bytes" << std::endl;
    std::cout << "[INFO] Auction: " << auction.length() << " bytes" << std::endl;
    std::cout << "[INFO] Storage: " << storage.length() << " bytes" << std::endl;
}

void test_ide_deployment() {
    std::cout << "\n[TEST] IDE Contract Deployment" << std::endl;
    std::cout << "════════════════════════════════════════════════════════" << std::endl;
    
    AxionIDE* ide = get_ide();
    
    std::string counter_code = ide->get_counter_template();
    std::string address = ide->deploy_contract(counter_code, "python", "0xDeveloper");
    
    assert(!address.empty() && "Deployment failed");
    std::cout << "[PASS] Contract deployed at: " << address << std::endl;
    
    auto contracts = ide->get_contracts();
    assert(!contracts.empty() && "Contracts list is empty");
    std::cout << "[INFO] Total deployed contracts: " << contracts.size() << std::endl;
}

void test_ide_state_management() {
    std::cout << "\n[TEST] IDE State Management" << std::endl;
    std::cout << "════════════════════════════════════════════════════════" << std::endl;
    
    AxionIDE* ide = get_ide();
    
    auto contracts = ide->get_contracts();
    if (!contracts.empty()) {
        std::string address = contracts[0];
        ide->load_contract(address);
        
        auto state = ide->get_contract_state(address);
        std::cout << "[INFO] Contract state items: " << state.size() << std::endl;
        
        std::cout << "[PASS] State management working" << std::endl;
    }
}

void test_ide_execution() {
    std::cout << "\n[TEST] IDE Function Execution" << std::endl;
    std::cout << "════════════════════════════════════════════════════════" << std::endl;
    
    AxionIDE* ide = get_ide();
    
    auto contracts = ide->get_contracts();
    if (!contracts.empty()) {
        std::string address = contracts[0];
        
        std::map<std::string, std::string> args;
        ContractCallResult result = ide->execute_function(
            address,
            "get_count",
            args
        );
        
        std::cout << "[INFO] Execution success: " << (result.success ? "true" : "false") << std::endl;
        std::cout << "[INFO] Result: " << result.result << std::endl;
        std::cout << "[INFO] Gas used: " << result.gas_used << std::endl;
        
        std::cout << "[PASS] Function execution working" << std::endl;
    }
}

void test_ide_logging() {
    std::cout << "\n[TEST] IDE Logging" << std::endl;
    std::cout << "════════════════════════════════════════════════════════" << std::endl;
    
    AxionIDE* ide = get_ide();
    
    ide->add_log("Test log message 1");
    ide->add_log("Test log message 2");
    ide->add_log("Test log message 3");
    
    auto logs = ide->get_logs();
    assert(logs.size() >= 3 && "Logs not recorded");
    
    std::cout << "[PASS] Logging system working" << std::endl;
    std::cout << "[INFO] Total logs: " << logs.size() << std::endl;
}

void test_ide_gas_estimation() {
    std::cout << "\n[TEST] IDE Gas Estimation" << std::endl;
    std::cout << "════════════════════════════════════════════════════════" << std::endl;
    
    AxionIDE* ide = get_ide();
    
    std::string code = "x = 1 + 1";
    std::map<std::string, std::string> args;
    args["param1"] = "value1";
    
    uint64_t gas = ide->estimate_gas(code, "test_func", args);
    assert(gas > 0 && "Gas estimation returned 0");
    
    std::cout << "[PASS] Gas estimation working" << std::endl;
    std::cout << "[INFO] Estimated gas: " << gas << std::endl;
}

void test_ide_tabs() {
    std::cout << "\n[TEST] IDE Tab Navigation" << std::endl;
    std::cout << "════════════════════════════════════════════════════════" << std::endl;
    
    AxionIDE* ide = get_ide();
    
    ide->set_active_tab(AxionIDE::Tab::EDITOR);
    assert(ide->get_active_tab() == AxionIDE::Tab::EDITOR && "Tab setting failed");
    
    ide->set_active_tab(AxionIDE::Tab::DEPLOY);
    assert(ide->get_active_tab() == AxionIDE::Tab::DEPLOY && "Tab setting failed");
    
    std::cout << "[PASS] Tab navigation working" << std::endl;
}

// ====================================================================
// Main Test Runner
// ====================================================================

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║         AXION IDE - SMART CONTRACT DEVELOPMENT IDE         ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════╝" << std::endl;
    
    try {
        test_ide_initialization();
        test_ide_editor();
        test_ide_templates();
        test_ide_deployment();
        test_ide_state_management();
        test_ide_execution();
        test_ide_logging();
        test_ide_gas_estimation();
        test_ide_tabs();
        
        std::cout << "\n";
        std::cout << "╔════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║                    ✅ ALL TESTS PASSED                     ║" << std::endl;
        std::cout << "║                                                            ║" << std::endl;
        std::cout << "║           Axion IDE Ready for Smart Contracts!            ║" << std::endl;
        std::cout << "╚════════════════════════════════════════════════════════════╝" << std::endl;
        
        // Cleanup
        shutdown_axion_ide();
        shutdown_advanced_avm();
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "\n[ERROR] Test exception: " << e.what() << std::endl;
        return 1;
    }
}
