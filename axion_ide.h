#pragma once

#include "avm_v2.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <ctime>

// ====================================================================
// Execution & Testing Structures
// ====================================================================

struct TestOutput {
    std::string output;
    std::string errors;
    std::string warnings;
    bool has_errors = false;
    uint64_t execution_time = 0;
    std::string result;
};

struct CodeAnalysis {
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    bool is_valid = true;
    std::string summary;
};

struct ExecutionResult {
    bool success = false;
    std::string output;
    std::string error;
    uint64_t gas_used = 0;
    std::string result_value;
    std::map<std::string, std::string> state_changes;
};

namespace axion {

// ====================================================================
// Axion IDE - Smart Contract IDE
// ====================================================================

/**
 * @class AxionIDE
 * @brief Integrated Development Environment for Axion Smart Contracts
 * 
 * Features:
 * - Code editor with syntax highlighting
 * - Contract deployment interface
 * - State explorer
 * - Event monitor
 * - Gas calculator
 * - Testing framework
 * - Contract templates
 */
class AxionIDE {
public:
    AxionIDE();
    ~AxionIDE();
    
    // ============================================================
    // IDE Lifecycle
    // ============================================================
    
    bool initialize(const std::string& window_title = "Axion IDE v2.0");
    void shutdown();
    bool is_running() const { return is_running_; }
    
    // ============================================================
    // Main Render Loop
    // ============================================================
    
    void render_frame();
    bool should_close();
    
    // ============================================================
    // Code Editor
    // ============================================================

    void set_editor_content(const std::string& content);
    std::string get_editor_content() const;
    void clear_editor();

    // ============================================================
    // File Management
    // ============================================================

    struct FileEntry {
        std::string filename;
        std::string filepath;
        std::string content;
        uint64_t created_time = 0;
        uint64_t modified_time = 0;
        bool unsaved_changes = false;
    };

    bool create_file(const std::string& filename);
    bool save_file(const std::string& filename, const std::string& content);
    bool open_file(const std::string& filename);
    bool delete_file(const std::string& filename);
    FileEntry get_current_file() const { return current_file_; }
    std::vector<FileEntry> get_file_list() const { return file_list_; }
    void refresh_file_list();
    bool file_exists(const std::string& filename);

    // ============================================================
    // Contract Management
    // ============================================================
    
    std::string deploy_contract(
        const std::string& code,
        const std::string& runtime,
        const std::string& creator
    );
    
    void load_contract(const std::string& address);
    void remove_contract(const std::string& address);
    std::vector<std::string> get_contracts() const;
    
    // ============================================================
    // Contract Execution
    // ============================================================
    
    ContractCallResult execute_function(
        const std::string& contract_address,
        const std::string& function_name,
        const std::map<std::string, std::string>& args
    );
    
    // ============================================================
    // State Explorer
    // ============================================================
    
    std::map<std::string, std::string> get_contract_state(
        const std::string& contract_address
    );
    
    void update_state_view();
    
    // ============================================================
    // Event Monitor
    // ============================================================
    
    std::vector<ContractEvent> get_contract_events(
        const std::string& contract_address
    );
    
    void clear_events();
    
    // ============================================================
    // Gas Calculator
    // ============================================================
    
    uint64_t estimate_gas(
        const std::string& code,
        const std::string& function_name,
        const std::map<std::string, std::string>& args
    );
    
    // ============================================================
    // Templates
    // ============================================================
    
    static std::string get_counter_template();
    static std::string get_token_template();
    static std::string get_auction_template();
    static std::string get_storage_template();
    
    // ============================================================
    // Contract Testing & Execution
    // ============================================================

    TestOutput test_contract(const std::string& code);
    TestOutput test_function(const std::string& code, const std::string& function_name);
    std::vector<std::string> get_test_output() const { return test_output_; }
    void clear_test_output();

    // ============================================================
    // Code Analysis
    // ============================================================

    CodeAnalysis analyze_code(const std::string& code);

    // ============================================================
    // Gas Calculation & Execution
    // ============================================================

    ExecutionResult execute_contract_test(
        const std::string& code,
        const std::string& function_name = "initialize",
        const std::map<std::string, std::string>& args = {}
    );
    
    // ============================================================
    // UI State
    // ============================================================
    
    enum class Tab {
        EDITOR,
        FILES,          // NEW - File Manager Tab
        TEST,
        CONTRACTS,
        DEPLOY,
        STATE,
        EVENTS,
        DOCUMENTATION
    };
    
    void set_active_tab(Tab tab) { active_tab_ = tab; }
    Tab get_active_tab() const { return active_tab_; }
    
    // ============================================================
    // Logging
    // ============================================================
    
    void add_log(const std::string& message);
    void clear_logs();
    std::vector<std::string> get_logs() const { return logs_; }

private:
    // ============================================================
    // Private Rendering Methods
    // ============================================================
    
    void render_menu_bar();
    void render_editor_panel();
    void render_files_panel();      // NEW - File Manager
    void render_test_panel();
    void render_contracts_panel();
    void render_deploy_panel();
    void render_state_panel();
    void render_events_panel();
    void render_documentation_panel();
    void render_logs_panel();
    void render_toolbar();
    
    // ============================================================
    // Internal State
    // ============================================================
    
    bool is_running_;
    bool show_demo_window_;
    
    // UI State
    Tab active_tab_;
    std::string window_title_;
    
    // Editor
    char editor_buffer_[65536];
    char deploy_name_[256];
    char deploy_creator_[256];
    char func_name_[256];
    std::string editor_content_;
    int editor_lines_;

    // File Management
    FileEntry current_file_;
    std::vector<FileEntry> file_list_;
    char new_filename_[256];
    int selected_file_idx_ = -1;
    std::string file_base_path_;

    // Contracts
    std::string selected_contract_;
    std::vector<std::string> contract_list_;

    // Deployment
    RuntimeType default_runtime_;
    uint64_t default_gas_limit_;

    // State
    std::map<std::string, std::string> current_state_;

    // Execution
    std::vector<std::string> logs_;
    std::string last_result_;
    uint64_t last_gas_used_;

    // Testing & Execution
    std::vector<std::string> test_output_;
    std::string test_input_func_;
    std::map<std::string, std::string> test_args_;
    TestOutput last_test_result_;
    CodeAnalysis last_analysis_;
    bool show_analysis_ = false;

    // Events
    std::vector<ContractEvent> current_events_;

    // AVM Reference
    AdvancedAVM* avm_;
};

// ====================================================================
// Template Provider
// ====================================================================

class ContractTemplates {
public:
    static std::string counter_contract();
    static std::string token_contract();
    static std::string auction_contract();
    static std::string storage_contract();
    static std::string voting_contract();
};

// ====================================================================
// Global IDE Instance
// ====================================================================

extern AxionIDE* g_axion_ide;

bool init_axion_ide();
void shutdown_axion_ide();
AxionIDE* get_ide();

} // namespace axion
