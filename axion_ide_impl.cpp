#include "axion_ide.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <imgui.h>
#include <fstream>
#include <filesystem>
#include <chrono>

namespace axion {

// ====================================================================
// Global IDE Instance
// ====================================================================

AxionIDE* g_axion_ide = nullptr;

bool init_axion_ide() {
    if (g_axion_ide != nullptr) {
        return true;
    }
    
    g_axion_ide = new AxionIDE();
    std::cout << "[IDE] Axion IDE initialized" << std::endl;
    
    return true;
}

void shutdown_axion_ide() {
    if (g_axion_ide != nullptr) {
        g_axion_ide->shutdown();
        delete g_axion_ide;
        g_axion_ide = nullptr;
        std::cout << "[IDE] Axion IDE shutdown" << std::endl;
    }
}

AxionIDE* get_ide() {
    return g_axion_ide;
}

// ====================================================================
// AxionIDE Implementation
// ====================================================================

AxionIDE::AxionIDE()
    : is_running_(false),
      show_demo_window_(false),
      active_tab_(Tab::EDITOR),
      window_title_("Axion IDE v2.0"),
      editor_lines_(0),
      default_runtime_(RuntimeType::PYTHON),
      default_gas_limit_(1000000),
      avm_(nullptr),
      last_gas_used_(0) {
    
    std::memset(editor_buffer_, 0, sizeof(editor_buffer_));
    std::memset(deploy_name_, 0, sizeof(deploy_name_));
    std::memset(deploy_creator_, 0, sizeof(deploy_creator_));
    std::memset(func_name_, 0, sizeof(func_name_));
    std::memset(new_filename_, 0, sizeof(new_filename_));

    // Set default file path
    file_base_path_ = "./axion_contracts";

    std::cout << "[IDE] AxionIDE constructor called" << std::endl;
}

AxionIDE::~AxionIDE() {
    if (is_running_) {
        shutdown();
    }
}

bool AxionIDE::initialize(const std::string& window_title) {
    if (is_running_) {
        return true;
    }
    
    std::cout << "[IDE] Initializing Axion IDE..." << std::endl;
    
    window_title_ = window_title;
    avm_ = get_avm();

    if (avm_ == nullptr) {
        std::cerr << "[IDE] WARNING: AVM not initialized" << std::endl;
    }

    is_running_ = true;

    // Create contracts directory if it doesn't exist
    try {
        std::filesystem::create_directories(file_base_path_);
    } catch (...) {
        std::cerr << "[IDE] Warning: Could not create contracts directory" << std::endl;
    }

    // Load existing files
    refresh_file_list();

    add_log("===================================================");
    add_log("Welcome to Axion IDE v2.0");
    add_log("Smart Contract Development Environment");
    add_log("===================================================");
    add_log("Ready to deploy Python smart contracts!");

    std::cout << "[IDE] Axion IDE initialized successfully" << std::endl;
    return true;
}

void AxionIDE::shutdown() {
    if (!is_running_) {
        return;
    }
    
    std::cout << "[IDE] Shutting down Axion IDE..." << std::endl;
    is_running_ = false;
    clear_logs();
    std::cout << "[IDE] Axion IDE shutdown complete" << std::endl;
}

void AxionIDE::render_frame() {
    if (!is_running_) return;
    
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_FirstUseEver);
    
    ImGui::Begin(window_title_.c_str(), nullptr, 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar);
    
    render_menu_bar();
    render_toolbar();
    ImGui::Separator();
    
    if (ImGui::BeginTabBar("IDE_Tabs")) {
        if (ImGui::TabItemButton("Editor")) {
            active_tab_ = Tab::EDITOR;
        }
        if (ImGui::TabItemButton("Files")) {
            active_tab_ = Tab::FILES;
            refresh_file_list();
        }
        if (ImGui::TabItemButton("Test")) {
            active_tab_ = Tab::TEST;
        }
        if (ImGui::TabItemButton("Contracts")) {
            active_tab_ = Tab::CONTRACTS;
        }
        if (ImGui::TabItemButton("Deploy")) {
            active_tab_ = Tab::DEPLOY;
        }
        if (ImGui::TabItemButton("State")) {
            active_tab_ = Tab::STATE;
        }
        if (ImGui::TabItemButton("Events")) {
            active_tab_ = Tab::EVENTS;
        }
        if (ImGui::TabItemButton("Docs")) {
            active_tab_ = Tab::DOCUMENTATION;
        }
        ImGui::EndTabBar();
    }
    
    ImGui::Separator();
    
    ImVec2 remaining = ImGui::GetContentRegionAvail();
    remaining.y -= 120;
    
    ImGui::BeginChild("TabContent", remaining, true);
    
    switch (active_tab_) {
        case Tab::EDITOR: render_editor_panel(); break;
        case Tab::FILES: render_files_panel(); break;
        case Tab::TEST: render_test_panel(); break;
        case Tab::CONTRACTS: render_contracts_panel(); break;
        case Tab::DEPLOY: render_deploy_panel(); break;
        case Tab::STATE: render_state_panel(); break;
        case Tab::EVENTS: render_events_panel(); break;
        case Tab::DOCUMENTATION: render_documentation_panel(); break;
    }
    
    ImGui::EndChild();
    ImGui::Separator();
    render_logs_panel();
    
    ImGui::End();
}

void AxionIDE::render_menu_bar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) clear_editor();
            if (ImGui::MenuItem("Clear")) clear_editor();
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) is_running_ = false;
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Templates")) {
            if (ImGui::MenuItem("Counter")) {
                set_editor_content(get_counter_template());
                add_log("Loaded Counter");
            }
            if (ImGui::MenuItem("Token")) {
                set_editor_content(get_token_template());
                add_log("Loaded Token");
            }
            if (ImGui::MenuItem("Auction")) {
                set_editor_content(get_auction_template());
                add_log("Loaded Auction");
            }
            if (ImGui::MenuItem("Storage")) {
                set_editor_content(get_storage_template());
                add_log("Loaded Storage");
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void AxionIDE::render_toolbar() {
    static uint64_t gas = 1000000;
    static uint64_t min_gas = 10000;
    static uint64_t max_gas = 10000000;
    ImGui::SliderScalar("Gas", ImGuiDataType_U64, &gas, &min_gas, &max_gas);
    default_gas_limit_ = gas;

    ImGui::SameLine();

    const char* runtimes[] = { "Python", "JavaScript" };
    static int rt_idx = 0;
    ImGui::Combo("##runtime", &rt_idx, runtimes, 2);
    default_runtime_ = (rt_idx == 0) ? RuntimeType::PYTHON : RuntimeType::JAVASCRIPT;

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    if (ImGui::Button("Deploy##btn")) {
        if (editor_buffer_[0] != '\0') {
            std::string addr = deploy_contract(editor_buffer_, "python", "0xDev");
            if (!addr.empty()) active_tab_ = Tab::DEPLOY;
        }
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();
    if (ImGui::Button("Clear Logs")) clear_logs();
}

void AxionIDE::render_editor_panel() {
    ImGui::Text("Python Contract Editor");
    ImGui::Separator();

    // Show current file
    if (!current_file_.filename.empty()) {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "File: %s", current_file_.filename.c_str());
        if (current_file_.unsaved_changes) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "* Unsaved");
        }
        ImGui::Separator();
    }

    ImGui::InputTextMultiline("##ed", editor_buffer_, sizeof(editor_buffer_),
        ImVec2(-1, ImGui::GetContentRegionAvail().y - 40), ImGuiInputTextFlags_AllowTabInput);

    int lines = std::count(editor_buffer_, editor_buffer_ + strlen(editor_buffer_), '\n') + 1;
    ImGui::Text("Lines: %d", lines);

    // Save button
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.8f, 1.0f));
    if (ImGui::Button("Save##editor")) {
        if (!current_file_.filename.empty()) {
            save_file(current_file_.filename, editor_buffer_);
            add_log("✓ File saved: " + current_file_.filename);
        } else {
            add_log("No file selected. Go to Files tab to create new.");
        }
    }
    ImGui::PopStyleColor();
}

void AxionIDE::render_files_panel() {
    ImGui::Text("Smart Contract Files");
    ImGui::Separator();

    // Create new file section
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.15f, 0.2f, 1.0f));
    ImGui::BeginChild("##create_new", ImVec2(0, 100), true);

    ImGui::Text("Create New File");
    ImGui::InputText("Filename (*.py)##new", new_filename_, sizeof(new_filename_));

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    if (ImGui::Button("Create", ImVec2(120, 0))) {
        if (new_filename_[0] != '\0') {
            std::string filename = new_filename_;
            if (filename.find(".py") == std::string::npos) {
                filename += ".py";
            }
            if (create_file(filename)) {
                add_log("✓ File created: " + filename);
                std::memset(new_filename_, 0, sizeof(new_filename_));
                refresh_file_list();
            } else {
                add_log("✗ Error creating file");
            }
        } else {
            add_log("Please enter a filename");
        }
    }
    ImGui::PopStyleColor();

    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::Separator();
    ImGui::Text("Your Smart Contract Files (%zu)", file_list_.size());
    ImGui::Separator();

    if (file_list_.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No files yet. Create one above!");
    } else {
        if (ImGui::BeginTable("##files_table", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 300))) {
            ImGui::TableSetupColumn("Filename", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableSetupColumn("Modified", ImGuiTableColumnFlags_WidthFixed, 100);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 150);
            ImGui::TableSetupColumn("Delete", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < file_list_.size(); ++i) {
                ImGui::PushID(i);
                const auto& file = file_list_[i];

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                // Highlight selected file
                if (selected_file_idx_ == (int)i) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
                }

                ImGui::BulletText("%s", file.filename.c_str());

                if (selected_file_idx_ == (int)i) {
                    ImGui::PopStyleColor();
                }

                ImGui::TableNextColumn();
                ImGui::Text("%zu B", file.content.size());

                ImGui::TableNextColumn();
                time_t t = file.modified_time;
                ImGui::Text("%s", std::ctime(&t));

                ImGui::TableNextColumn();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));
                if (ImGui::Button("Open##file", ImVec2(120, 0))) {
                    open_file(file.filename);
                    selected_file_idx_ = i;
                    active_tab_ = Tab::EDITOR;
                    add_log("✓ Opened: " + file.filename);
                }
                ImGui::PopStyleColor();

                ImGui::TableNextColumn();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                if (ImGui::Button("Delete##file", ImVec2(50, 0))) {
                    if (delete_file(file.filename)) {
                        add_log("✓ Deleted: " + file.filename);
                        refresh_file_list();
                    } else {
                        add_log("✗ Error deleting file");
                    }
                }
                ImGui::PopStyleColor();

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
    }
}

void AxionIDE::render_test_panel() {
    ImGui::Text("Test & Execute Smart Contract");
    ImGui::Separator();

    // Upper section: Code preview + Execute button
    ImGui::BeginChild("TestUpper", ImVec2(0, 100), true);
    ImGui::TextWrapped("Code to test:");
    ImGui::TextWrapped("%.500s%s", editor_buffer_, strlen(editor_buffer_) > 500 ? "..." : "");
    ImGui::EndChild();

    ImGui::Separator();

    // Execution controls
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
    if (ImGui::Button("Run Contract (Initialize)", ImVec2(200, 0))) {
        ExecutionResult result = execute_contract_test(editor_buffer_, "initialize");
        last_test_result_.output = result.output;
        last_test_result_.errors = result.error;
        last_test_result_.has_errors = !result.success;
        last_test_result_.execution_time = result.gas_used;
        add_log("✓ Contract execution test complete");
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::InputText("Function", func_name_, sizeof(func_name_));

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));
    if (ImGui::Button("Run Function", ImVec2(120, 0))) {
        std::map<std::string, std::string> args;
        ExecutionResult result = execute_contract_test(editor_buffer_, func_name_, args);
        last_test_result_.output = result.output;
        last_test_result_.errors = result.error;
        last_test_result_.has_errors = !result.success;
        last_test_result_.execution_time = result.gas_used;
        add_log("✓ Function execution test complete");
    }
    ImGui::PopStyleColor();

    ImGui::Separator();

    // Code Analysis
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.3f, 0.1f, 1.0f));
    if (ImGui::Button("Analyze Code")) {
        last_analysis_ = analyze_code(editor_buffer_);
        show_analysis_ = true;
    }
    ImGui::PopStyleColor();

    ImGui::Separator();
    ImGui::Text("Execution Output & Errors");
    ImGui::Separator();

    // Output section with color coding
    ImGui::BeginChild("TestOutput", ImVec2(0, ImGui::GetContentRegionAvail().y - 60), true);

    // Show errors in red
    if (!last_test_result_.errors.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "❌ ERRORS:");
        ImGui::TextWrapped("%s", last_test_result_.errors.c_str());
        ImGui::Spacing();
    }

    // Show warnings in yellow
    if (!last_test_result_.warnings.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "⚠️ WARNINGS:");
        ImGui::TextWrapped("%s", last_test_result_.warnings.c_str());
        ImGui::Spacing();
    }

    // Show output in green
    if (!last_test_result_.output.empty()) {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "✓ OUTPUT:");
        ImGui::TextWrapped("%s", last_test_result_.output.c_str());
        ImGui::Spacing();
    }

    // Show result value
    if (!last_test_result_.result.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "📊 RESULT:");
        ImGui::TextWrapped("%s", last_test_result_.result.c_str());
        ImGui::Spacing();
    }

    // Show gas usage
    if (last_test_result_.execution_time > 0) {
        ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.2f, 1.0f), "⛽ Gas Used: %llu", last_test_result_.execution_time);
    }

    // Show analysis if available
    if (show_analysis_) {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "📝 CODE ANALYSIS:");

        if (last_analysis_.is_valid) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "✓ Code is valid!");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "✗ Code has issues");
        }

        if (!last_analysis_.errors.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Errors:");
            for (const auto& err : last_analysis_.errors) {
                ImGui::BulletText("%s", err.c_str());
            }
        }

        if (!last_analysis_.warnings.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Warnings:");
            for (const auto& warn : last_analysis_.warnings) {
                ImGui::BulletText("%s", warn.c_str());
            }
        }
    }

    ImGui::EndChild();

    // Action buttons
    ImGui::Spacing();
    if (ImGui::Button("Clear Output")) {
        last_test_result_ = TestOutput{};
        show_analysis_ = false;
    }

    ImGui::SameLine();
    if (!last_test_result_.errors.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::Text("❌ Fix errors before deploying!");
        ImGui::PopStyleColor();
    } else if (last_test_result_.output.empty() && !show_analysis_) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Run tests to check for errors");
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        ImGui::Text("✓ Ready to deploy!");
        ImGui::PopStyleColor();
    }
}

void AxionIDE::render_contracts_panel() {
    ImGui::Text("Deployed Contracts (%zu)", contract_list_.size());
    ImGui::Separator();
    
    if (contract_list_.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "None");
    } else {
        for (size_t i = 0; i < contract_list_.size(); ++i) {
            ImGui::PushID(i);
            ImGui::BulletText("%s", contract_list_[i].substr(0, 16).c_str());
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 80);
            if (ImGui::SmallButton("View")) {
                selected_contract_ = contract_list_[i];
                load_contract(contract_list_[i]);
                active_tab_ = Tab::STATE;
            }
            ImGui::PopID();
        }
    }
}

void AxionIDE::render_deploy_panel() {
    ImGui::Text("Deploy Contract");
    ImGui::Separator();
    ImGui::InputText("Name", deploy_name_, sizeof(deploy_name_));
    ImGui::InputText("Creator", deploy_creator_, sizeof(deploy_creator_));
    ImGui::Text("Gas: %llu", default_gas_limit_);
    ImGui::Separator();
    ImGui::Text("Code:");
    ImGui::TextWrapped("%s", editor_buffer_);
    ImGui::Separator();
    
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    if (ImGui::Button("Deploy##deploy", ImVec2(150, 40))) {
        if (editor_buffer_[0]) {
            std::string creator = deploy_creator_[0] ? deploy_creator_ : "0xDev";
            deploy_contract(editor_buffer_, "python", creator);
        }
    }
    ImGui::PopStyleColor();
}

void AxionIDE::render_state_panel() {
    ImGui::Text("Contract State");
    ImGui::Separator();
    
    if (selected_contract_.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Select contract");
    } else {
        ImGui::Text("Contract: %s", selected_contract_.substr(0, 16).c_str());
        update_state_view();
        
        if (current_state_.empty()) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Empty");
        } else {
            for (const auto& s : current_state_) {
                ImGui::BulletText("%s = %s", s.first.c_str(), s.second.c_str());
            }
        }
        
        ImGui::Separator();
        ImGui::InputText("Function", func_name_, sizeof(func_name_));
        if (ImGui::Button("Execute")) {
            std::map<std::string, std::string> args;
            ContractCallResult res = execute_function(selected_contract_, func_name_, args);
            if (res.success) add_log("OK: " + std::to_string(res.gas_used) + " gas");
            else add_log("ERR: " + res.error);
        }
    }
}

void AxionIDE::render_events_panel() {
    ImGui::Text("Contract Events");
    ImGui::Separator();
    
    if (selected_contract_.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Select contract");
    } else {
        auto events = get_contract_events(selected_contract_);
        ImGui::Text("Events: %zu", events.size());
    }
}

void AxionIDE::render_documentation_panel() {
    ImGui::TextWrapped(
        "Axion IDE v2.0\n\n"
        "Smart Contract Development Environment\n\n"
        "FEATURES:\n"
        "• Python editor\n"
        "• Deploy contracts\n"
        "• View state\n"
        "• Monitor events\n\n"
        "STEPS:\n"
        "1. Edit code\n"
        "2. Deploy\n"
        "3. View state"
    );
}

void AxionIDE::render_logs_panel() {
    ImGui::BeginChild("Logs", ImVec2(0, 100), true);
    for (const auto& log : logs_) {
        ImGui::TextWrapped("%s", log.c_str());
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
}

bool AxionIDE::should_close() {
    return !is_running_;
}

void AxionIDE::set_editor_content(const std::string& content) {
    if (content.size() < sizeof(editor_buffer_)) {
        std::strcpy(editor_buffer_, content.c_str());
    }
}

std::string AxionIDE::get_editor_content() const {
    return std::string(editor_buffer_);
}

void AxionIDE::clear_editor() {
    std::memset(editor_buffer_, 0, sizeof(editor_buffer_));
}

std::string AxionIDE::deploy_contract(
    const std::string& code,
    const std::string& runtime,
    const std::string& creator) {
    
    if (!avm_) {
        add_log("ERR: AVM not ready");
        return "";
    }
    
    RuntimeType rt = (runtime == "python") ? RuntimeType::PYTHON : RuntimeType::JAVASCRIPT;
    std::string addr = avm_->deploy_contract(code, rt, creator, default_gas_limit_);
    
    if (!addr.empty()) {
        contract_list_.push_back(addr);
        add_log("Deployed: " + addr.substr(0, 12) + "...");
    }
    return addr;
}

void AxionIDE::load_contract(const std::string& address) {
    selected_contract_ = address;
    current_state_ = get_contract_state(address);
}

void AxionIDE::remove_contract(const std::string& address) {
    contract_list_.erase(
        std::remove(contract_list_.begin(), contract_list_.end(), address),
        contract_list_.end()
    );
}

std::vector<std::string> AxionIDE::get_contracts() const {
    return contract_list_;
}

ContractCallResult AxionIDE::execute_function(
    const std::string& contract_address,
    const std::string& function_name,
    const std::map<std::string, std::string>& args) {
    
    ContractCallResult result;
    result.success = false;
    
    if (!avm_) {
        result.error = "AVM not ready";
        return result;
    }
    
    result = avm_->call_contract(contract_address, function_name, args, "0xUser", 0);
    last_gas_used_ = result.gas_used;
    return result;
}

std::map<std::string, std::string> AxionIDE::get_contract_state(
    const std::string& contract_address) {
    if (!avm_) return {};
    return avm_->get_contract_state(contract_address);
}

void AxionIDE::update_state_view() {
    if (!selected_contract_.empty()) {
        current_state_ = get_contract_state(selected_contract_);
    }
}

std::vector<ContractEvent> AxionIDE::get_contract_events(
    const std::string& contract_address) {
    if (!avm_) return {};
    return avm_->get_contract_events(contract_address, 0, 1000000);
}

void AxionIDE::clear_events() {
    current_events_.clear();
}

uint64_t AxionIDE::estimate_gas(
    const std::string& code,
    const std::string& function_name,
    const std::map<std::string, std::string>& args) {
    return 100 + (code.length() / 100) + args.size() * 10;
}

std::string AxionIDE::get_counter_template() {
    return ContractTemplates::counter_contract();
}

std::string AxionIDE::get_token_template() {
    return ContractTemplates::token_contract();
}

std::string AxionIDE::get_auction_template() {
    return ContractTemplates::auction_contract();
}

std::string AxionIDE::get_storage_template() {
    return ContractTemplates::storage_contract();
}

void AxionIDE::add_log(const std::string& message) {
    std::cout << "[IDE] " << message << std::endl;
    logs_.push_back(message);
    if (logs_.size() > 1000) logs_.erase(logs_.begin());
}

void AxionIDE::clear_logs() {
    logs_.clear();
}

// ============================================================
// Contract Testing
// ============================================================

TestOutput AxionIDE::test_contract(const std::string& code) {
    TestOutput output;

    if (code.empty()) {
        output.errors = "Contract code is empty";
        output.has_errors = true;
        return output;
    }

    // Basic Python syntax checking
    if (code.find("class ") == std::string::npos) {
        output.errors = "Error: Contract must define a class";
        output.has_errors = true;
    }

    if (code.find("def ") == std::string::npos) {
        output.errors = "Error: Contract class must define methods";
        output.has_errors = true;
    }

    if (!output.has_errors) {
        output.output = "✓ Contract structure is valid\n";
        output.output += "✓ Contains class definition\n";
        output.output += "✓ Contains methods\n";
        output.result = "Contract template appears correct";
    }

    return output;
}

TestOutput AxionIDE::test_function(const std::string& code, const std::string& function_name) {
    TestOutput output;

    if (code.empty()) {
        output.errors = "Contract code is empty";
        output.has_errors = true;
        return output;
    }

    if (function_name.empty()) {
        output.errors = "Function name is empty";
        output.has_errors = true;
        return output;
    }

    // Check if function exists in code
    std::string func_search = "def " + function_name + "(";
    if (code.find(func_search) == std::string::npos) {
        output.errors = "Error: Function '" + function_name + "' not found in contract";
        output.has_errors = true;
        return output;
    }

    output.output = "✓ Function '" + function_name + "' found\n";
    output.output += "✓ Function structure is valid\n";
    output.result = "Function ready to execute";

    return output;
}

void AxionIDE::clear_test_output() {
    test_output_.clear();
    last_test_result_ = TestOutput{};
}

CodeAnalysis AxionIDE::analyze_code(const std::string& code) {
    CodeAnalysis analysis;

    if (code.empty()) {
        analysis.errors.push_back("Code is empty");
        analysis.is_valid = false;
        analysis.summary = "No code to analyze";
        return analysis;
    }

    // Check for class definition
    if (code.find("class ") == std::string::npos) {
        analysis.errors.push_back("Missing class definition");
        analysis.is_valid = false;
    }

    // Check for initialize method
    if (code.find("def initialize") == std::string::npos) {
        analysis.warnings.push_back("No initialize() method found");
    }

    // Check for proper indentation
    int indent_errors = 0;
    std::stringstream ss(code);
    std::string line;
    int line_num = 0;
    while (std::getline(ss, line)) {
        line_num++;
        if (line.empty()) continue;

        // Check for tabs vs spaces (Python style)
        if (line[0] == '\t') {
            analysis.warnings.push_back("Line " + std::to_string(line_num) + ": Use spaces instead of tabs");
            indent_errors++;
        }
    }

    // Check code length
    if (code.length() > 65000) {
        analysis.warnings.push_back("Contract code is very large");
    }

    // Check for common Python errors
    if (code.find("self.") != std::string::npos && code.find("def ") == std::string::npos) {
        analysis.warnings.push_back("Found 'self' reference without method definitions");
    }

    // If no errors found
    if (analysis.errors.empty()) {
        analysis.is_valid = true;
        analysis.summary = "✓ Code analysis passed";
    } else {
        analysis.is_valid = false;
        analysis.summary = "✗ Code has errors";
    }

    // Add summary
    analysis.summary += "\nAnalyzed " + std::to_string(line_num) + " lines\n";
    analysis.summary += "Errors: " + std::to_string(analysis.errors.size()) + "\n";
    analysis.summary += "Warnings: " + std::to_string(analysis.warnings.size());

    return analysis;
}

ExecutionResult AxionIDE::execute_contract_test(
    const std::string& code,
    const std::string& function_name,
    const std::map<std::string, std::string>& args) {

    ExecutionResult result;

    if (code.empty()) {
        result.error = "Contract code is empty";
        result.success = false;
        return result;
    }

    if (!avm_) {
        result.error = "AVM not available";
        result.success = false;
        return result;
    }

    // Deploy temporary contract for testing
    std::string test_address = avm_->deploy_contract(code, RuntimeType::PYTHON, "0xTest", 500000);

    if (test_address.empty()) {
        result.error = "Failed to deploy contract for testing";
        result.success = false;
        return result;
    }

    // Execute function
    ContractCallResult call_result = avm_->call_contract(
        test_address,
        function_name,
        args,
        "0xTest",
        0
    );

    result.success = call_result.success;
    result.gas_used = call_result.gas_used;
    result.result_value = call_result.result;
    result.output = "Function executed: " + function_name + "\n";
    result.output += "Gas used: " + std::to_string(call_result.gas_used) + "\n";
    result.output += "Result: " + call_result.result;

    if (!call_result.success) {
        result.error = call_result.error;
    }

    // Get state after execution
    result.state_changes = get_contract_state(test_address);

    add_log("Test execution complete for function: " + function_name);

    return result;
}

// ============================================================
// File Management
// ============================================================

bool AxionIDE::create_file(const std::string& filename) {
    if (file_exists(filename)) {
        add_log("✗ File already exists: " + filename);
        return false;
    }

    std::string filepath = file_base_path_ + "/" + filename;

    try {
        std::ofstream file(filepath);
        if (file) {
            file << "# Smart Contract: " << filename << "\n";
            file << "# Created with Axion IDE v2.0\n\n";
            file << "class Contract:\n";
            file << "    def initialize(self):\n";
            file << "        return {\"status\": \"initialized\"}\n";
            file.close();

            FileEntry entry;
            entry.filename = filename;
            entry.filepath = filepath;
            entry.created_time = std::chrono::system_clock::now().time_since_epoch().count();
            entry.modified_time = entry.created_time;
            entry.unsaved_changes = false;

            file_list_.push_back(entry);
            add_log("✓ File created: " + filename);
            return true;
        }
    } catch (const std::exception& e) {
        add_log("✗ Error creating file: " + std::string(e.what()));
        return false;
    }

    return false;
}

bool AxionIDE::save_file(const std::string& filename, const std::string& content) {
    std::string filepath = file_base_path_ + "/" + filename;

    try {
        std::ofstream file(filepath);
        if (file) {
            file << content;
            file.close();

            // Update file list
            for (auto& f : file_list_) {
                if (f.filename == filename) {
                    f.content = content;
                    f.modified_time = std::chrono::system_clock::now().time_since_epoch().count();
                    f.unsaved_changes = false;
                    current_file_ = f;
                    break;
                }
            }

            add_log("✓ File saved: " + filename);
            return true;
        }
    } catch (const std::exception& e) {
        add_log("✗ Error saving file: " + std::string(e.what()));
        return false;
    }

    return false;
}

bool AxionIDE::open_file(const std::string& filename) {
    std::string filepath = file_base_path_ + "/" + filename;

    try {
        std::ifstream file(filepath);
        if (file) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();

            // Copy to editor buffer
            if (content.size() < sizeof(editor_buffer_)) {
                std::strcpy(editor_buffer_, content.c_str());
            }

            // Update current file
            for (auto& f : file_list_) {
                if (f.filename == filename) {
                    current_file_ = f;
                    current_file_.content = content;
                    current_file_.unsaved_changes = false;
                    break;
                }
            }

            add_log("✓ File opened: " + filename);
            return true;
        }
    } catch (const std::exception& e) {
        add_log("✗ Error opening file: " + std::string(e.what()));
        return false;
    }

    return false;
}

bool AxionIDE::delete_file(const std::string& filename) {
    std::string filepath = file_base_path_ + "/" + filename;

    try {
        if (std::filesystem::exists(filepath)) {
            std::filesystem::remove(filepath);

            // Remove from file list
            file_list_.erase(
                std::remove_if(file_list_.begin(), file_list_.end(),
                    [&filename](const FileEntry& f) { return f.filename == filename; }),
                file_list_.end()
            );

            // Clear current file if it matches
            if (current_file_.filename == filename) {
                current_file_ = FileEntry{};
                std::memset(editor_buffer_, 0, sizeof(editor_buffer_));
            }

            add_log("✓ File deleted: " + filename);
            return true;
        }
    } catch (const std::exception& e) {
        add_log("✗ Error deleting file: " + std::string(e.what()));
        return false;
    }

    return false;
}

void AxionIDE::refresh_file_list() {
    file_list_.clear();

    try {
        if (std::filesystem::exists(file_base_path_)) {
            for (const auto& entry : std::filesystem::directory_iterator(file_base_path_)) {
                if (entry.is_regular_file() && entry.path().extension() == ".py") {
                    std::string filename = entry.path().filename().string();
                    std::string filepath = entry.path().string();

                    FileEntry file_entry;
                    file_entry.filename = filename;
                    file_entry.filepath = filepath;

                    // Read file content
                    std::ifstream file(filepath);
                    if (file) {
                        std::stringstream buffer;
                        buffer << file.rdbuf();
                        file_entry.content = buffer.str();
                        file.close();
                    }

                    // Get file times
                    auto last_write = std::filesystem::last_write_time(entry);
                    file_entry.modified_time = std::chrono::system_clock::now().time_since_epoch().count();

                    file_list_.push_back(file_entry);
                }
            }
        }
    } catch (const std::exception& e) {
        add_log("✗ Error reading files: " + std::string(e.what()));
    }
}

bool AxionIDE::file_exists(const std::string& filename) {
    std::string filepath = file_base_path_ + "/" + filename;
    return std::filesystem::exists(filepath);
}

std::string ContractTemplates::counter_contract() {
    return R"(class Counter:
    def initialize(self):
        self.count = 0
        return {"status": "ready"}
    
    def increment(self):
        self.count += 1
        return {"count": self.count}
    
    def get_count(self):
        return {"count": self.count}
)";
}

std::string ContractTemplates::token_contract() {
    return R"(class Token:
    def initialize(self, supply=1000000):
        self.supply = supply
        return {"status": "ready", "supply": supply}
    
    def transfer(self, to, amount):
        return {"status": "success"}
)";
}

std::string ContractTemplates::auction_contract() {
    return R"(class Auction:
    def initialize(self):
        self.highest_bid = 0
        return {"status": "ready"}
    
    def bid(self, amount):
        if amount > self.highest_bid:
            self.highest_bid = amount
            return {"status": "accepted"}
        return {"status": "rejected"}
)";
}

std::string ContractTemplates::storage_contract() {
    return R"(class Storage:
    def initialize(self):
        self.data = {}
        return {"status": "ready"}
    
    def set(self, key, value):
        self.data[key] = value
        return {"status": "stored"}
    
    def get(self, key):
        return {"value": self.data.get(key, "")}
)";
}

} // namespace axion
