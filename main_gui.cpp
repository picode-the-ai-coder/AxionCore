// ================================================================
//  Axion Digitaverse - main_gui.cpp
//  Complete desktop GUI using Dear ImGui + GLFW + OpenGL3.
//
//  Pages:
//    1. Register   — personal info → Ed25519 keypair → on-chain identity
//    2. Login      — public + private key → session
//    3. Wallet     — AXC balance, deposit, send (signed tx)
//    4. Files      — AIPFS upload / retrieve / list
//    5. Node       — live node stats, peers, blocks, mining
//
//  Build:
//    mkdir build && cd build
//    cmake .. -DCMAKE_BUILD_TYPE=Release
//    cmake --build . --config Release
//
//  Run:
//    ./AxionDigitaverse [port] [seed_peer]
//    e.g. AxionDigitaverse.exe 9001 192.168.1.5:9000
// ================================================================

// ── ImGui + GLFW + OpenGL ──────────────────────────────────
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

// ── Axion backend ──────────────────────────────────────────
#include "ui_bridge.h"
#include "tests.h"
<<<<<<< HEAD
#include "axion_ide.h"
=======
>>>>>>> 8d74da462f0f1bf0ba5e8c8729e1834a15296503

// ── STL ────────────────────────────────────────────────────
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <memory>
#include <cstdio>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>

// ── Platform file dialog (Windows) ────────────────────────
#ifdef _WIN32
  #include <windows.h>
  #include <commdlg.h>
  #pragma comment(lib,"comdlg32.lib")
#endif

// ================================================================
//  Global state
// ================================================================
static std::unique_ptr<axion::NodeBridge> g_bridge;

// ── Axion colour palette ───────────────────────────────────
static const ImVec4 COL_BG        = {0.051f, 0.047f, 0.118f, 1.0f}; // #0D0C1E
static const ImVec4 COL_SURFACE   = {0.071f, 0.067f, 0.165f, 1.0f}; // #12112A
static const ImVec4 COL_BORDER    = {0.165f, 0.137f, 0.376f, 1.0f}; // #2A2360
static const ImVec4 COL_PURPLE    = {0.357f, 0.247f, 0.831f, 1.0f}; // #5B3FD4
static const ImVec4 COL_BLUE      = {0.231f, 0.545f, 0.831f, 1.0f}; // #3B8BD4
static const ImVec4 COL_TEAL      = {0.114f, 0.784f, 0.627f, 1.0f}; // #1DC8A0
static const ImVec4 COL_ACCENT    = {0.498f, 0.435f, 0.910f, 1.0f}; // #7F6FE8
static const ImVec4 COL_TEXT      = {0.910f, 0.902f, 1.000f, 1.0f}; // #E8E6FF
static const ImVec4 COL_MUTED     = {0.533f, 0.502f, 0.690f, 1.0f}; // #8880B0
static const ImVec4 COL_SUCCESS   = {0.133f, 0.788f, 0.482f, 1.0f}; // #22C97B
static const ImVec4 COL_WARNING   = {0.941f, 0.627f, 0.125f, 1.0f}; // #F0A020
static const ImVec4 COL_DANGER    = {0.910f, 0.251f, 0.251f, 1.0f}; // #E84040
static const ImVec4 COL_DARKRED   = {0.227f, 0.063f, 0.063f, 1.0f}; // #3A1010

// ── ImGui helper: coloured text ────────────────────────────
static void TextColored(ImVec4 c, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ImGui::PushStyleColor(ImGuiCol_Text, c);
    ImGui::TextV(fmt, args);
    ImGui::PopStyleColor();
    va_end(args);
}

// ── Button with Axion purple style ────────────────────────
static bool PrimaryButton(const char* label, float width = 0) {
    ImGui::PushStyleColor(ImGuiCol_Button,        COL_PURPLE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COL_ACCENT);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  COL_BLUE);
    bool r = ImGui::Button(label, {width, 36});
    ImGui::PopStyleColor(3);
    return r;
}

// ── Button with border-only style ─────────────────────────
static bool SecondaryButton(const char* label, float width = 0) {
    ImGui::PushStyleColor(ImGuiCol_Button,        {0,0,0,0});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.3f,0.25f,0.6f,0.4f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  COL_ACCENT);
    ImGui::PushStyleColor(ImGuiCol_Text,          COL_ACCENT);
    bool r = ImGui::Button(label, {width, 32});
    ImGui::PopStyleColor(4);
    return r;
}

// ── Section header ─────────────────────────────────────────
static void SectionHeader(const char* text) {
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]); // bold font
    TextColored(COL_TEXT, "%s", text);
    ImGui::PopFont();
    ImGui::Spacing();
}

// ── Card begin/end (padded bordered region) ────────────────
static void CardBegin(const char* id) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, COL_SURFACE);
    ImGui::PushStyleColor(ImGuiCol_Border,  COL_BORDER);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,   12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize,  1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {20.0f, 16.0f});
    ImGui::BeginChild(id, {0, 0}, ImGuiChildFlags_Borders |
                                   ImGuiChildFlags_AutoResizeY);
}
static void CardEnd() {
    ImGui::EndChild();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
}

// ── Label + value row ──────────────────────────────────────
static void LabelValue(const char* label, const char* value) {
    TextColored(COL_MUTED, "%s", label);
    ImGui::SameLine(140);
    TextColored(COL_TEXT, "%s", value);
}

// ── Monospace value with copy button ──────────────────────
static void MonoCopyField(const char* label, const std::string& value,
                          const char* btn_id) {
    TextColored(COL_MUTED, "%s", label);
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]); // mono
    ImGui::PushStyleColor(ImGuiCol_Text, COL_TEAL);
    char buf[512];
    snprintf(buf, sizeof(buf), "%.60s%s",
             value.c_str(), value.size() > 60 ? "..." : "");
    ImGui::TextWrapped("%s", buf);
    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::PushID(btn_id);
    if (SecondaryButton("Copy")) {
        ImGui::SetClipboardText(value.c_str());
    }
    ImGui::PopID();
}

// ── Status badge ──────────────────────────────────────────
static void StatusBadge(const char* text, ImVec4 col) {
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::PushStyleColor(ImGuiCol_Button, {col.x*0.2f, col.y*0.2f, col.z*0.2f, 1.0f});
    ImGui::SmallButton(text);
    ImGui::PopStyleColor(2);
}

// ── Separator with label ──────────────────────────────────
static void LabeledSeparator(const char* label) {
    ImGui::Spacing();
    TextColored(COL_MUTED, "%s", label);
    ImGui::Separator();
    ImGui::Spacing();
}

// ── Platform file dialog ──────────────────────────────────
static std::string OpenFileDialog() {
#ifdef _WIN32
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.lpstrFile   = filename;
    ofn.nMaxFile    = sizeof(filename);
    ofn.Flags       = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileNameA(&ofn)) return std::string(filename);
#endif
    return {};
}

static std::string SaveFileDialog(const std::string& default_name) {
#ifdef _WIN32
    char filename[MAX_PATH] = {};
    strncpy_s(filename, default_name.c_str(), sizeof(filename)-1);
    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.lpstrFile   = filename;
    ofn.nMaxFile    = sizeof(filename);
    ofn.Flags       = OFN_OVERWRITEPROMPT;
    if (GetSaveFileNameA(&ofn)) return std::string(filename);
#else
    return "./" + default_name;
#endif
    return {};
}

// ================================================================
//  Page state structs
// ================================================================

struct RegisterState {
    char name[128]    = {};
    char email[128]   = {};
    char country[64]  = {};
    char phone[32]    = {};
    std::string error_msg;
    bool show_result  = false;
    axion::NodeBridge::RegisterResponse result;
    bool working      = false;
    std::string status_msg;
};

struct LoginState {
    char pubkey[256]  = {};
    char privkey[256] = {};
    std::string error_msg;
    bool working      = false;
};

struct WalletState {
    double balance        = 0.0;
    char recipient[256]   = {};
    double send_amount    = 10.0;
    char memo[128]        = {};
    std::string send_error;
    std::string send_ok;
    double deposit_amount = 100.0;
    std::string deposit_status;
    bool balance_dirty    = true;

    struct TxRow {
        std::string summary;
        std::string tx_id;
        std::string amount_str;
        bool        outgoing = false;
    };
    std::vector<TxRow> txs;
    bool txs_dirty = true;
};

struct FilesState {
    std::string selected_path;
    std::string selected_name;
    size_t      selected_size = 0;
    std::string upload_cid;
    std::string upload_error;
    bool        uploading = false;
    char        retrieve_cid[512] = {};
    std::string retrieve_status;
    std::string last_cid;
};

struct NodeState {
    axion::NodeStats stats;
    char peer_addr[256] = {};
    std::string peer_connect_status;
    bool connect_working = false;

    struct BlockRow {
        std::string height;
        std::string hash;
        std::string info;
        std::string tx_count;
    };
    std::vector<BlockRow> blocks;
    int tick = 0;
};

// ================================================================
//  Apply Axion theme to ImGui
// ================================================================
static void ApplyAxionTheme() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding   = 12.0f;
    s.ChildRounding    = 10.0f;
    s.FrameRounding    =  8.0f;
    s.PopupRounding    =  8.0f;
    s.ScrollbarRounding = 6.0f;
    s.GrabRounding     =  6.0f;
    s.TabRounding      =  8.0f;
    s.WindowBorderSize = 1.0f;
    s.FrameBorderSize  = 1.0f;
    s.ItemSpacing      = {8, 8};
    s.ItemInnerSpacing = {6, 4};
    s.FramePadding     = {12, 8};
    s.WindowPadding    = {20, 16};
    s.IndentSpacing    = 18.0f;
    s.ScrollbarSize    = 10.0f;

    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]          = COL_BG;
    c[ImGuiCol_ChildBg]           = COL_SURFACE;
    c[ImGuiCol_PopupBg]           = COL_SURFACE;
    c[ImGuiCol_Border]            = COL_BORDER;
    c[ImGuiCol_BorderShadow]      = {0,0,0,0};
    c[ImGuiCol_FrameBg]           = {0.12f, 0.11f, 0.23f, 1.0f};
    c[ImGuiCol_FrameBgHovered]    = {0.18f, 0.15f, 0.35f, 1.0f};
    c[ImGuiCol_FrameBgActive]     = {0.22f, 0.18f, 0.42f, 1.0f};
    c[ImGuiCol_TitleBg]           = COL_SURFACE;
    c[ImGuiCol_TitleBgActive]     = {0.12f, 0.10f, 0.28f, 1.0f};
    c[ImGuiCol_MenuBarBg]         = COL_SURFACE;
    c[ImGuiCol_ScrollbarBg]       = COL_BG;
    c[ImGuiCol_ScrollbarGrab]     = {0.30f, 0.25f, 0.55f, 1.0f};
    // ImGuiCol_ScrollbarGrabHov was removed in ImGui v1.90+
    // Use ImGuiCol_ScrollbarGrab styling instead
    c[ImGuiCol_CheckMark]         = COL_TEAL;
    c[ImGuiCol_SliderGrab]        = COL_PURPLE;
    c[ImGuiCol_SliderGrabActive]  = COL_ACCENT;
    c[ImGuiCol_Button]            = {0.20f, 0.17f, 0.40f, 1.0f};
    c[ImGuiCol_ButtonHovered]     = {0.32f, 0.27f, 0.60f, 1.0f};
    c[ImGuiCol_ButtonActive]      = COL_PURPLE;
    c[ImGuiCol_Header]            = {0.22f, 0.18f, 0.44f, 0.6f};
    c[ImGuiCol_HeaderHovered]     = {0.28f, 0.23f, 0.55f, 0.8f};
    c[ImGuiCol_HeaderActive]      = COL_ACCENT;
    c[ImGuiCol_Separator]         = COL_BORDER;
    c[ImGuiCol_ResizeGrip]        = COL_BORDER;
    c[ImGuiCol_ResizeGripHovered] = COL_ACCENT;
    c[ImGuiCol_Tab]               = {0.14f, 0.12f, 0.30f, 1.0f};
    c[ImGuiCol_TabHovered]        = COL_ACCENT;
    c[ImGuiCol_TabActive]         = COL_PURPLE;
    c[ImGuiCol_TabUnfocused]      = {0.10f, 0.09f, 0.22f, 1.0f};
    c[ImGuiCol_TabUnfocusedActive]= {0.18f, 0.15f, 0.38f, 1.0f};
    c[ImGuiCol_PlotLines]         = COL_TEAL;
    c[ImGuiCol_PlotHistogram]     = COL_PURPLE;
    c[ImGuiCol_TextSelectedBg]    = {0.35f, 0.28f, 0.65f, 0.5f};
    c[ImGuiCol_NavHighlight]      = COL_ACCENT;
    c[ImGuiCol_Text]              = COL_TEXT;
    c[ImGuiCol_TextDisabled]      = COL_MUTED;
}

// ================================================================
//  Page: Register
// ================================================================
static void RenderRegisterPage(RegisterState& st) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
    SectionHeader("Create your identity");
    TextColored(COL_MUTED, "Enter your details. APKI will generate a unique Ed25519 keypair.");
    ImGui::Spacing(); ImGui::Spacing();

    if (!st.show_result) {
        // ── Input form ─────────────────────────────────────
        CardBegin("##regform");

        ImGui::PushItemWidth(-1);

        TextColored(COL_MUTED, "Full name *");
        ImGui::InputText("##name", st.name, sizeof(st.name));
        ImGui::Spacing();

        TextColored(COL_MUTED, "Email address *");
        ImGui::InputText("##email", st.email, sizeof(st.email));
        ImGui::Spacing();

        float w = (ImGui::GetContentRegionAvail().x - 12) * 0.5f;
        ImGui::PushItemWidth(w);
        TextColored(COL_MUTED, "Country *");
        ImGui::InputText("##country", st.country, sizeof(st.country));
        ImGui::SameLine(0, 12);
        TextColored(COL_MUTED, "Phone (optional)");
        ImGui::InputText("##phone", st.phone, sizeof(st.phone));
        ImGui::PopItemWidth();

        ImGui::PopItemWidth();
        ImGui::Spacing();

        if (!st.error_msg.empty()) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, COL_DARKRED);
            ImGui::BeginChild("##regerr", {0,0}, ImGuiChildFlags_Borders |
                              ImGuiChildFlags_AutoResizeY);
            TextColored(COL_DANGER, "%s", st.error_msg.c_str());
            ImGui::EndChild();
            ImGui::PopStyleColor();
            ImGui::Spacing();
        }

        bool disabled = st.working;
        if (disabled) ImGui::BeginDisabled();
        if (PrimaryButton(st.working ? "Generating keypair..." : "Generate keypair and register",
                          ImGui::GetContentRegionAvail().x)) {
            // Validate
            if (strlen(st.name) == 0 || strlen(st.email) == 0 ||
                strlen(st.country) == 0) {
                st.error_msg = "Name, email and country are required.";
            } else if (std::string(st.email).find('@') == std::string::npos) {
                st.error_msg = "Please enter a valid email address.";
            } else {
                st.error_msg.clear();
                st.working = true;
                // Run on thread pool so UI stays responsive
                auto* pst = &st;
                std::thread([pst]() {
                    axion::NodeBridge::RegisterRequest req;
                    req.full_name = pst->name;
                    req.email     = pst->email;
                    req.country   = pst->country;
                    req.phone     = pst->phone;
                    pst->result   = g_bridge->do_register(req);
                    pst->working  = false;
                    pst->show_result = pst->result.success;
                    if (!pst->result.success)
                        pst->error_msg = pst->result.error;
                }).detach();
            }
        }
        if (disabled) ImGui::EndDisabled();

        CardEnd();

    } else {
        // ── Keypair result ─────────────────────────────────
        CardBegin("##regresult");

        StatusBadge(" Identity created! ", COL_SUCCESS);
        ImGui::Spacing();
        TextColored(COL_MUTED,
            "Save your keys now. Your private key cannot be recovered if lost.");
        ImGui::Spacing();

        LabeledSeparator("Your AXC address");
        MonoCopyField("", st.result.address, "cpy_addr");

        LabeledSeparator("Public key  (share freely — this is your identity)");
        MonoCopyField("", st.result.public_key, "cpy_pub");

        LabeledSeparator("Private key  (NEVER share — keep in a password manager)");
        ImGui::PushStyleColor(ImGuiCol_Text, COL_WARNING);
        ImGui::TextWrapped("%.80s%s", st.result.private_key.c_str(),
                           st.result.private_key.size() > 80 ? "..." : "");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        if (SecondaryButton("Copy##privk"))
            ImGui::SetClipboardText(st.result.private_key.c_str());
        ImGui::Spacing();

        LabeledSeparator("Identity transaction (recorded on ADL)");
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);
        TextColored(COL_TEAL, "%.48s...", st.result.tx_id.c_str());
        ImGui::PopFont();
        TextColored(COL_MUTED, "Your identity is now permanently on the blockchain.");
        ImGui::Spacing();

        CardEnd();
        ImGui::Spacing();

        if (PrimaryButton("Proceed to login",
                          ImGui::GetContentRegionAvail().x)) {
            // Signal tab switch — handled by caller
            st.status_msg = "__goto_login__";
        }
    }
}

// ================================================================
//  Page: Login
// ================================================================
static void RenderLoginPage(LoginState& st, bool& logged_in) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 32);

    float avail = ImGui::GetContentRegionAvail().x;
    float card_w = std::min(avail, 500.0f);
    ImGui::SetCursorPosX((avail - card_w) * 0.5f + ImGui::GetCursorPosX());

    SectionHeader("Log in to your account");
    TextColored(COL_MUTED, "Paste your public and private keys. "
                "Your private key never leaves this device.");
    ImGui::Spacing(); ImGui::Spacing();

    CardBegin("##loginform");
    ImGui::PushItemWidth(-1);

    TextColored(COL_MUTED, "Public key  (64 hex characters)");
    ImGui::InputText("##lpub", st.pubkey, sizeof(st.pubkey));
    ImGui::Spacing();

    TextColored(COL_MUTED, "Private key  (128 hex characters)");
    ImGui::InputText("##lpriv", st.privkey, sizeof(st.privkey),
                     ImGuiInputTextFlags_Password);
    ImGui::Spacing();

    ImGui::PopItemWidth();

    if (!st.error_msg.empty()) {
        TextColored(COL_DANGER, "%s", st.error_msg.c_str());
        ImGui::Spacing();
    }

    bool disabled = st.working;
    if (disabled) ImGui::BeginDisabled();
    if (PrimaryButton(st.working ? "Verifying..." : "Log in",
                      ImGui::GetContentRegionAvail().x)) {
        // Strip spaces
        std::string pub = st.pubkey, priv = st.privkey;
        pub.erase(std::remove_if(pub.begin(), pub.end(), ::isspace), pub.end());
        priv.erase(std::remove_if(priv.begin(), priv.end(), ::isspace), priv.end());

        if (pub.size() != 64) {
            st.error_msg = "Public key must be exactly 64 hex characters.";
        } else if (priv.size() != 128) {
            st.error_msg = "Private key must be exactly 128 hex characters.";
        } else {
            st.error_msg.clear();
            st.working   = true;
            std::thread([pub, priv, &st, &logged_in]() {
                axion::NodeBridge::LoginRequest req;
                req.public_key_hex  = pub;
                req.private_key_hex = priv;
                auto resp = g_bridge->do_login(req);
                st.working = false;
                if (!resp.success) {
                    st.error_msg = resp.error;
                } else {
                    logged_in = true;
                    // Clear sensitive data from buffers
                    memset(st.privkey, 0, sizeof(st.privkey));
                }
            }).detach();
        }
    }
    if (disabled) ImGui::EndDisabled();
    ImGui::Spacing();

    CardEnd();
}

// ================================================================
//  Page: Wallet
// ================================================================
static void RenderWalletPage(WalletState& st) {
    // Refresh balance if needed
    if (st.balance_dirty && g_bridge->is_logged_in()) {
        st.balance     = g_bridge->get_balance();
        st.balance_dirty = false;
    }

    SectionHeader("Wallet");
    ImGui::Spacing();

    // ── Balance hero card ──────────────────────────────────
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg,
            ImVec4{0.12f, 0.07f, 0.31f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_Border, COL_PURPLE);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 16.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {24.0f, 20.0f});
        ImGui::BeginChild("##walbal", {0,0},
            ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);

        TextColored(COL_MUTED, "AXC balance");
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(4) << st.balance << "  AXC";
        TextColored(COL_TEXT, "%s", oss.str().c_str());
        ImGui::PopFont();

        auto user = g_bridge->current_user();
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);
        TextColored(COL_MUTED, "%.32s...", user.address.c_str());
        ImGui::PopFont();

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 80);
        if (SecondaryButton("Refresh##bal")) {
            st.balance_dirty = true;
            st.txs_dirty     = true;
        }
        ImGui::EndChild();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(3);
    }

    ImGui::Spacing();

    // ── Deposit + Send side by side ────────────────────────
    float half = (ImGui::GetContentRegionAvail().x - 12) * 0.5f;

    // Deposit
    ImGui::PushID("deposit_card");
    CardBegin("##dep");
    TextColored(COL_TEXT, "Deposit AXC");
    TextColored(COL_MUTED, "Testnet faucet — adds AXC to your balance");
    ImGui::Spacing();
    ImGui::PushItemWidth(200);
    ImGui::InputDouble("Amount##dep", &st.deposit_amount, 10.0, 100.0, "%.0f");
    ImGui::PopItemWidth();
    ImGui::Spacing();
    if (PrimaryButton("Deposit##btn", half - 40)) {
        if (st.deposit_amount > 0) {
            g_bridge->deposit_axc(st.deposit_amount);
            st.deposit_status = "Queued — appears after next block.";
            st.balance_dirty  = true;
        }
    }
    if (!st.deposit_status.empty()) {
        ImGui::Spacing();
        TextColored(COL_SUCCESS, "%s", st.deposit_status.c_str());
    }
    CardEnd();
    ImGui::PopID();

    ImGui::SameLine(0, 12);

    // Send
    ImGui::PushID("send_card");
    CardBegin("##snd");
    TextColored(COL_TEXT, "Send AXC");
    TextColored(COL_MUTED, "Signs transaction with your private key");
    ImGui::Spacing();
    ImGui::PushItemWidth(-1);
    TextColored(COL_MUTED, "Recipient (AXC...)");
    ImGui::InputText("##recip", st.recipient, sizeof(st.recipient));
    ImGui::Spacing();
    TextColored(COL_MUTED, "Amount (AXC)");
    ImGui::InputDouble("##sendamt", &st.send_amount, 1.0, 10.0, "%.4f");
    ImGui::Spacing();
    TextColored(COL_MUTED, "Memo (optional)");
    ImGui::InputText("##memo", st.memo, sizeof(st.memo));
    ImGui::PopItemWidth();
    ImGui::Spacing();
    if (!st.send_error.empty()) TextColored(COL_DANGER, "%s", st.send_error.c_str());
    if (!st.send_ok.empty())    TextColored(COL_SUCCESS, "%s", st.send_ok.c_str());
    ImGui::Spacing();
    if (PrimaryButton("Send  (Ed25519 signed)", half - 40)) {
        st.send_error.clear(); st.send_ok.clear();
        axion::NodeBridge::SendRequest req;
        req.to_address = st.recipient;
        req.amount     = st.send_amount;
        req.memo       = st.memo;
        if (req.to_address.substr(0,3) != "AXC" || req.to_address.size() < 10) {
            st.send_error = "Address must start with AXC.";
        } else {
            auto resp = g_bridge->send_axc(req);
            if (!resp.success) {
                st.send_error = resp.error;
            } else {
                st.send_ok     = "Sent! TX: " + resp.tx_id.substr(0,16) + "...";
                st.balance_dirty = true;
                st.txs_dirty     = true;
                memset(st.recipient, 0, sizeof(st.recipient));
                memset(st.memo, 0, sizeof(st.memo));
            }
        }
    }
    CardEnd();
    ImGui::PopID();

    // ── Transaction history ────────────────────────────────
    ImGui::Spacing();

    if (st.txs_dirty) {
        st.txs.clear();
        std::string my_addr = g_bridge->current_address();
        auto& chain = g_bridge->node().chain;
        for (uint64_t h = chain.height(); h > 0 && st.txs.size() < 50; --h) {
            const auto* blk = chain.get_block(h);
            if (!blk) continue;
            for (const auto& tx : blk->transactions) {
                if (tx.sender != my_addr && tx.receiver != my_addr) continue;
                WalletState::TxRow row;
                row.outgoing = (tx.sender == my_addr);
                std::string peer = row.outgoing
                    ? tx.receiver.substr(0,16) + "..." : tx.sender.substr(0,16) + "...";
                row.summary  = (row.outgoing ? "Sent to " : "Received from ") + peer;
                row.tx_id    = tx.tx_id.substr(0,20) + "...";
                std::ostringstream a;
                a << std::fixed << std::setprecision(4) << tx.amount << " AXC";
                row.amount_str = (row.outgoing ? "-" : "+") + a.str();
                st.txs.push_back(row);
            }
        }
        st.txs_dirty = false;
    }

    CardBegin("##txhistory");
    TextColored(COL_TEXT, "Recent transactions");
    ImGui::Spacing();
    if (st.txs.empty()) {
        TextColored(COL_MUTED, "No transactions yet.");
    } else {
        if (ImGui::BeginTable("##txtable", 3,
            ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV)) {
            ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("TX ID",       ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Amount",      ImGuiTableColumnFlags_WidthFixed, 130);
            ImGui::TableHeadersRow();
            for (const auto& tx : st.txs) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                TextColored(COL_TEXT, "%s", tx.summary.c_str());
                ImGui::TableNextColumn();
                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);
                TextColored(COL_MUTED, "%s", tx.tx_id.c_str());
                ImGui::PopFont();
                ImGui::TableNextColumn();
                TextColored(tx.outgoing ? COL_DANGER : COL_SUCCESS,
                            "%s", tx.amount_str.c_str());
            }
            ImGui::EndTable();
        }
    }
    CardEnd();
}

// ================================================================
//  Page: Files (AIPFS)
// ================================================================
static void RenderFilesPage(FilesState& st) {
    SectionHeader("Decentralized storage  (AIPFS)");
    TextColored(COL_MUTED,
        "Files are content-addressed and synced across all Axion nodes.");
    ImGui::Spacing(); ImGui::Spacing();

    // ── Upload card ────────────────────────────────────────
    CardBegin("##upload_card");
    TextColored(COL_TEXT, "Upload a file");
    ImGui::Spacing();

    if (st.selected_path.empty()) {
        if (PrimaryButton("Browse files...", 180)) {
            std::string path = OpenFileDialog();
            if (!path.empty()) {
                st.selected_path = path;
                st.selected_name =
                    std::filesystem::path(path).filename().string();
                st.selected_size =
                    std::filesystem::file_size(path);
                st.upload_cid.clear();
                st.upload_error.clear();
            }
        }
        ImGui::SameLine();
        TextColored(COL_MUTED, "or drag a file onto this window");
    } else {
        // Show selected file info
        TextColored(COL_TEAL, "  %s", st.selected_name.c_str());
        ImGui::SameLine();
        std::string sz_str = st.selected_size >= 1024*1024
            ? std::to_string(st.selected_size/(1024*1024)) + " MB"
            : st.selected_size >= 1024
            ? std::to_string(st.selected_size/1024) + " KB"
            : std::to_string(st.selected_size) + " B";
        TextColored(COL_MUTED, "(%s)", sz_str.c_str());
        ImGui::Spacing();

        if (!st.uploading) {
            if (PrimaryButton("Upload to AIPFS", 180)) {
                st.uploading = true;
                st.upload_error.clear();
                auto path = st.selected_path;
                auto name = st.selected_name;
                std::thread([path, name, &st]() {
                    std::ifstream f(path, std::ios::binary);
                    if (!f) { st.upload_error = "Cannot read file."; st.uploading=false; return; }
                    axion::Bytes data((std::istreambuf_iterator<char>(f)),
                                      std::istreambuf_iterator<char>());
                    auto resp = g_bridge->upload_file(name, data);
                    if (resp.success) {
                        st.upload_cid   = resp.cid;
                        st.last_cid     = resp.cid;
                    } else {
                        st.upload_error = resp.error;
                    }
                    st.uploading = false;
                }).detach();
            }
            ImGui::SameLine();
            if (SecondaryButton("Clear")) {
                st.selected_path.clear();
                st.selected_name.clear();
            }
        } else {
            ImGui::ProgressBar(-1.0f * (float)ImGui::GetTime(), {180,20},
                               "Uploading...");
        }

        if (!st.upload_error.empty())
            TextColored(COL_DANGER, "%s", st.upload_error.c_str());
    }

    // Upload result
    if (!st.upload_cid.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, {0.05f,0.16f,0.08f,1.0f});
        ImGui::BeginChild("##urok", {0,0},
            ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
        TextColored(COL_SUCCESS, "Uploaded!");
        TextColored(COL_MUTED, "CID:");
        ImGui::SameLine();
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);
        TextColored(COL_TEAL, "%.60s%s", st.upload_cid.c_str(),
                    st.upload_cid.size()>60?"...":"");
        ImGui::PopFont();
        ImGui::SameLine();
        if (SecondaryButton("Copy CID"))
            ImGui::SetClipboardText(st.upload_cid.c_str());
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    CardEnd();
    ImGui::Spacing();

    // ── Retrieve by CID ────────────────────────────────────
    CardBegin("##retrieve_card");
    TextColored(COL_TEXT, "Retrieve by CID");
    ImGui::Spacing();
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 110);
    ImGui::InputText("##cid_input", st.retrieve_cid, sizeof(st.retrieve_cid));
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (SecondaryButton("Retrieve", 96)) {
        std::string cid = st.retrieve_cid;
        std::thread([cid, &st]() {
            auto data = g_bridge->retrieve_file(cid);
            if (!data || data->empty()) {
                st.retrieve_status = "Not found in local store.";
            } else {
                std::string path = SaveFileDialog("retrieved_file");
                if (!path.empty()) {
                    std::ofstream f(path, std::ios::binary);
                    f.write(reinterpret_cast<const char*>(data->data()),
                            (std::streamsize)data->size());
                    st.retrieve_status = "Saved: " + path;
                } else {
                    st.retrieve_status = "Found " +
                        std::to_string(data->size()) + " bytes (save cancelled).";
                }
            }
        }).detach();
    }
    if (!st.retrieve_status.empty()) {
        ImGui::Spacing();
        TextColored(COL_MUTED, "%s", st.retrieve_status.c_str());
    }
    CardEnd();
    ImGui::Spacing();

    // ── Stats ──────────────────────────────────────────────
    CardBegin("##fstats");
    auto stats = g_bridge->get_stats();
    LabelValue("Items in AIPFS store:", std::to_string(stats.aipfs_items).c_str());
    CardEnd();
}

// ================================================================
<<<<<<< HEAD
//  Page: Smart Contract IDE
// ================================================================
static void RenderIDEPage(axion::AxionIDE* ide) {
    if (!ide) return;
    ide->render_frame();
}

// ================================================================
=======
>>>>>>> 8d74da462f0f1bf0ba5e8c8729e1834a15296503
//  Page: Node Status
// ================================================================
static void RenderNodePage(NodeState& st) {
    st.tick++;

    // Refresh stats every 60 frames (~1s)
    if (st.tick % 60 == 0 || st.tick == 1) {
        st.stats = g_bridge->get_stats();

        // Refresh block list
        st.blocks.clear();
        auto& chain = g_bridge->node().chain;
        for (uint64_t h = chain.height(); h > 0 && chain.height()-h < 20; --h) {
            const auto* blk = chain.get_block(h);
            if (!blk) continue;
            NodeState::BlockRow row;
            row.height   = "#" + std::to_string(blk->header.index);
            row.hash     = blk->hash.substr(0,24) + "...";
            row.tx_count = std::to_string(blk->transactions.size()) + " txs";
            row.info     = "nonce=" + std::to_string(blk->header.nonce)
                         + "  diff=" + std::to_string(blk->header.difficulty);
            st.blocks.push_back(row);
        }
    }

    SectionHeader("Node status");
    ImGui::Spacing();

    // ── 4 stat cards ──────────────────────────────────────
    float tile = (ImGui::GetContentRegionAvail().x - 36) * 0.25f;
    auto StatTile = [&](const char* id, const char* label,
                        const std::string& val, ImVec4 col) {
        ImGui::PushID(id);
        CardBegin("##st");
        TextColored(COL_MUTED, "%s", label);
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        TextColored(col, "%s", val.c_str());
        ImGui::PopFont();
        CardEnd();
        ImGui::PopID();
        ImGui::SameLine(0, 12);
    };

    std::string hr_str = (st.stats.is_mining && st.stats.hash_rate > 0)
        ? (st.stats.hash_rate >= 1000
            ? std::to_string(st.stats.hash_rate/1000) + "k"
            : std::to_string(st.stats.hash_rate))
        : "—";

    StatTile("h",  "Chain height", std::to_string(st.stats.chain_height), COL_TEXT);
    StatTile("p",  "Peers",        std::to_string(st.stats.peer_count),   COL_BLUE);
    StatTile("mp", "Mempool",      std::to_string(st.stats.mempool_size), COL_WARNING);
    StatTile("hr", "Hash/s",       hr_str,                                COL_TEAL);
    ImGui::NewLine();
    ImGui::Spacing();

    // ── Mining status ──────────────────────────────────────
    CardBegin("##minstatus");
    {
        ImVec4 dot_col = st.stats.is_mining ? COL_WARNING : COL_MUTED;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        pos.x += 4; pos.y += 8;
        dl->AddCircleFilled(pos, 5.0f, ImGui::ColorConvertFloat4ToU32(dot_col));
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 16);
        if (st.stats.is_mining) {
            TextColored(COL_WARNING, "Mining block #%llu...",
                        (unsigned long long)(st.stats.chain_height + 1));
        } else {
            TextColored(COL_MUTED, "Mining idle");
        }
    }
    ImGui::Spacing();
    LabelValue("Difficulty:",   (std::to_string(st.stats.difficulty) + " leading zeros").c_str());
    LabelValue("Tip hash:",     (st.stats.last_block_hash.substr(0,32) + "...").c_str());
    LabelValue("Registered users:", std::to_string(st.stats.identity_count).c_str());
    LabelValue("AIPFS items:",  std::to_string(st.stats.aipfs_items).c_str());
    CardEnd();
    ImGui::Spacing();

    // ── Connect to peer ────────────────────────────────────
    CardBegin("##addpeer");
    TextColored(COL_TEXT, "Connect to peer");
    TextColored(COL_MUTED, "Enter IP:port of another Axion node");
    ImGui::Spacing();
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 110);
    ImGui::InputText("##peeraddr", st.peer_addr, sizeof(st.peer_addr),
                     0, nullptr, nullptr);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    bool pdis = st.connect_working;
    if (pdis) ImGui::BeginDisabled();
    if (PrimaryButton("Connect", 96)) {
        std::string addr = st.peer_addr;
        if (!addr.empty()) {
            st.connect_working    = true;
            st.peer_connect_status.clear();
            std::thread([addr, &st]() {
                g_bridge->add_seed_peer(addr);
                std::this_thread::sleep_for(std::chrono::seconds(3));
                auto s = g_bridge->get_stats();
                st.peer_connect_status = s.peer_count > 0
                    ? "Connected! Peers: " + std::to_string(s.peer_count)
                    : "Could not connect to " + addr;
                st.connect_working = false;
            }).detach();
        }
    }
    if (pdis) ImGui::EndDisabled();
    if (!st.peer_connect_status.empty()) {
        ImGui::Spacing();
        bool ok = st.peer_connect_status.find("Could") == std::string::npos;
        TextColored(ok ? COL_SUCCESS : COL_DANGER,
                    "%s", st.peer_connect_status.c_str());
    }
    CardEnd();
    ImGui::Spacing();

    // ── Recent blocks ──────────────────────────────────────
    CardBegin("##recentblocks");
    TextColored(COL_TEXT, "Recent blocks");
    ImGui::Spacing();
    if (st.blocks.empty()) {
        TextColored(COL_MUTED, "No blocks yet — mining will create the first block.");
    } else {
        if (ImGui::BeginTable("##blktable", 4,
            ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
            ImGuiTableFlags_ScrollY, {0, 220})) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("#",    ImGuiTableColumnFlags_WidthFixed, 50);
            ImGui::TableSetupColumn("Hash", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Info", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("TXs",  ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableHeadersRow();
            for (const auto& b : st.blocks) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                TextColored(COL_PURPLE, "%s", b.height.c_str());
                ImGui::TableNextColumn();
                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);
                TextColored(COL_TEAL, "%s", b.hash.c_str());
                ImGui::PopFont();
                ImGui::TableNextColumn();
                TextColored(COL_MUTED, "%s", b.info.c_str());
                ImGui::TableNextColumn();
                TextColored(COL_TEXT, "%s", b.tx_count.c_str());
            }
            ImGui::EndTable();
        }
    }
    CardEnd();
}

// ================================================================
//  Main
// ================================================================
int main(int argc, char* argv[]) {
    // ── Parse arguments ────────────────────────────────────
    uint16_t    port     = 9000;
    std::string seed_peer;
    if (argc >= 2) try { port = (uint16_t)std::stoi(argv[1]); } catch (...) {}
    if (argc >= 3) seed_peer = argv[2];

    std::string data_dir = "./axion_data_" + std::to_string(port);

    // ── Run tests first (silent in release build) ──────────
#ifndef NDEBUG
    axion::tests::run_all();
#endif

    // ── Initialise backend node ────────────────────────────
    g_bridge = std::make_unique<axion::NodeBridge>(
        "0.0.0.0:" + std::to_string(port),
        "axion-node-" + std::to_string(port),
        axion::INITIAL_DIFFICULTY,
        data_dir);

    g_bridge->start_node();
    if (!seed_peer.empty()) g_bridge->add_seed_peer(seed_peer);

    // ── GLFW init ──────────────────────────────────────────
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_SAMPLES, 4);  // anti-aliasing

    GLFWwindow* window = glfwCreateWindow(
        1280, 800, "Axion Digitaverse", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // vsync

    // ── ImGui init ─────────────────────────────────────────
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Load fonts — regular, bold, mono
    // Embedded default font (no file needed)
    io.Fonts->AddFontDefault();                     // [0] regular
    io.Fonts->AddFontDefault();                     // [1] "bold" — replace with actual bold
    io.Fonts->AddFontDefault();                     // [2] "mono" — replace with actual mono
    // To load real fonts, replace the lines above with:
    // io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeuib.ttf", 18.0f);
    // io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\cascadiacode.ttf", 13.0f);

    ApplyAxionTheme();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

<<<<<<< HEAD
    // ── Initialize IDE ────────────────────────────────────
    axion::init_advanced_avm();
    axion::init_axion_ide();
    axion::AxionIDE* ide = axion::get_ide();
    if (ide) {
        ide->initialize("Axion Smart Contract IDE");
    }

=======
>>>>>>> 8d74da462f0f1bf0ba5e8c8729e1834a15296503
    // ── Page state ─────────────────────────────────────────
    RegisterState reg_st{};
    LoginState    log_st{};
    WalletState   wal_st{};
    FilesState    fil_st{};
    NodeState     node_st{};
    bool          logged_in  = false;
    int           active_tab = 0;  // 0=Register,1=Login,2=Wallet,3=Files,4=Node

    // ── Render loop ────────────────────────────────────────
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int fb_w, fb_h;
        glfwGetFramebufferSize(window, &fb_w, &fb_h);

        // ── Full-screen main window ────────────────────────
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize({(float)fb_w, (float)fb_h});
        ImGui::Begin("##mainwin", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoResize     |
            ImGuiWindowFlags_NoBringToFrontOnFocus);

        // ── Title bar ──────────────────────────────────────
        {
            ImGui::PushStyleColor(ImGuiCol_ChildBg,
                ImVec4{0.071f, 0.067f, 0.165f, 1.0f});
            ImGui::BeginChild("##titlebar", {(float)fb_w, 44},
                              ImGuiChildFlags_None);

            ImGui::SetCursorPos({12, 12});
            TextColored(COL_ACCENT, "Axion");
            ImGui::SameLine();
            TextColored(COL_TEXT, "Digitaverse");

            // Right: node status
            auto stats = g_bridge->get_stats();
            std::string status_str;
            ImVec4 status_col;
            if (stats.peer_count > 0) {
                status_str = " Node online  " + std::to_string(stats.peer_count) + " peers ";
                status_col = COL_SUCCESS;
            } else {
                status_str = " No peers ";
                status_col = COL_DANGER;
            }
            if (stats.is_mining) {
                status_str += " | Mining #" + std::to_string(stats.chain_height+1);
                status_col = COL_WARNING;
            }

            float sw = ImGui::CalcTextSize(status_str.c_str()).x + 16;
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - sw + 12);
            StatusBadge(status_str.c_str(), status_col);
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
        ImGui::Spacing();

        // ── Sidebar navigation ─────────────────────────────
        const float SIDEBAR_W = 200.0f;
        float content_h = (float)fb_h - 60;

        ImGui::PushStyleColor(ImGuiCol_ChildBg,
            ImVec4{0.071f, 0.067f, 0.165f, 1.0f});
        ImGui::BeginChild("##sidebar", {SIDEBAR_W, content_h},
                          ImGuiChildFlags_None);

        struct NavItem { const char* icon; const char* label; int idx; bool needs_login; };
        NavItem nav[] = {
            {" +", " Register",    0, false},
            {" >", " Login",       1, false},
            {" $", " Wallet",      2, true },
            {" #", " Files",       3, true },
            {" @", " Node",        4, false},
<<<<<<< HEAD
            {" ⚙️ ", " IDE",        5, true },
=======
>>>>>>> 8d74da462f0f1bf0ba5e8c8729e1834a15296503
        };

        ImGui::SetCursorPosY(16);
        for (auto& n : nav) {
            bool selected = (active_tab == n.idx);
            bool locked   = (n.needs_login && !logged_in);

            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Button, COL_PURPLE);
                ImGui::PushStyleColor(ImGuiCol_Text,   COL_TEXT);
            } else if (locked) {
                ImGui::PushStyleColor(ImGuiCol_Button, {0,0,0,0});
                ImGui::PushStyleColor(ImGuiCol_Text,   COL_MUTED);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, {0,0,0,0});
                ImGui::PushStyleColor(ImGuiCol_Text,   COL_TEXT);
            }

            std::string label = std::string(n.icon) + n.label;
            if (locked) label += " (login)";

            ImGui::SetCursorPosX(0);
            if (ImGui::Button(label.c_str(), {SIDEBAR_W, 40})) {
                if (locked) {
                    active_tab = 1; // redirect to login
                } else {
                    active_tab = n.idx;
                    // Mark wallet/tx dirty on switch
                    if (n.idx == 2) {
                        wal_st.balance_dirty = true;
                        wal_st.txs_dirty     = true;
                    }
                }
            }
            ImGui::PopStyleColor(2);
        }

        // ── User info at bottom of sidebar ────────────────
        if (logged_in) {
            ImGui::SetCursorPosY(content_h - 70);
            ImGui::Separator();
            ImGui::Spacing();
            auto user = g_bridge->current_user();
            TextColored(COL_TEXT, "  %s", user.full_name.c_str());
            TextColored(COL_MUTED, "  %.18s...", user.address.c_str());
            ImGui::Spacing();
            ImGui::SetCursorPosX(8);
            if (SecondaryButton("Log out", SIDEBAR_W - 16)) {
                g_bridge->logout();
                logged_in  = false;
                active_tab = 1;
                wal_st = WalletState{};
                fil_st = FilesState{};
            }
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();

        // ── Page content ──────────────────────────────────
        ImGui::SameLine(0, 0);
        ImGui::BeginChild("##content", {0, content_h}, ImGuiChildFlags_None);

        // Update logged_in state from bridge
        logged_in = g_bridge->is_logged_in();

        // Check register page "goto login" signal
        if (reg_st.status_msg == "__goto_login__") {
            active_tab = 1;
            reg_st.status_msg.clear();
        }

        switch (active_tab) {
            case 0: RenderRegisterPage(reg_st);          break;
            case 1: RenderLoginPage(log_st, logged_in);
                    if (logged_in) {
                        active_tab = 2;
                        wal_st.balance_dirty = true;
                    }
                    break;
            case 2: RenderWalletPage(wal_st);            break;
            case 3: RenderFilesPage(fil_st);             break;
            case 4: RenderNodePage(node_st);             break;
<<<<<<< HEAD
            case 5: RenderIDEPage(axion::get_ide());     break;
=======
>>>>>>> 8d74da462f0f1bf0ba5e8c8729e1834a15296503
        }

        ImGui::EndChild();
        ImGui::End();

        // ── Render ─────────────────────────────────────────
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.051f, 0.047f, 0.118f, 1.0f); // COL_BG
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // ── Cleanup ────────────────────────────────────────────
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    g_bridge.reset();
<<<<<<< HEAD

    // Shutdown IDE
    axion::shutdown_axion_ide();
    axion::shutdown_advanced_avm();

=======
>>>>>>> 8d74da462f0f1bf0ba5e8c8729e1834a15296503
    return 0;
}
