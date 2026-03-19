#pragma once
// ================================================================
//  Axion Digitaverse - ui_bridge.h
//  NodeBridge — thread-safe interface between the GUI and the node.
//
//  The GUI lives on the UI thread. The node runs background threads
//  (mining, networking, block processing). This bridge:
//    - Exposes synchronous query methods (safe to call from UI)
//    - Provides callback registration for live updates
//    - Queues all write operations through a mutex
//    - Marshals results back to the UI via std::function callbacks
//
//  In the WinUI 3 app, callbacks are dispatched on the UI thread
//  using DispatcherQueue::TryEnqueue(). In the CLI, they run
//  directly. The bridge itself is framework-agnostic.
// ================================================================

#include "axion_node.h"
#include "identity.h"
#include "consensus.h"
#include <functional>
#include <mutex>
#include <atomic>
#include <queue>
#include <optional>

namespace axion {

// ── Callback types ─────────────────────────────────────────
using StringCallback  = std::function<void(const std::string&)>;
using BoolCallback    = std::function<void(bool, const std::string&)>;
using DoubleCallback  = std::function<void(double)>;
using VoidCallback    = std::function<void()>;

// ── Live node stats (updated by background threads) ────────
struct NodeStats {
    uint64_t    chain_height   = 0;
    size_t      peer_count     = 0;
    size_t      mempool_size   = 0;
    bool        is_mining      = false;
    uint64_t    hash_rate      = 0;     // hashes per second
    uint32_t    difficulty     = 2;
    std::string last_block_hash;
    size_t      aipfs_items    = 0;
    size_t      identity_count = 0;
};

// ── NodeBridge ─────────────────────────────────────────────
class NodeBridge {
public:
    // ── Construction ───────────────────────────────────────
    explicit NodeBridge(const std::string& listen_addr  = "0.0.0.0:9000",
                        const std::string& validator_seed = "axion-node-validator",
                        uint32_t           difficulty     = INITIAL_DIFFICULTY,
                        const std::string& data_dir       = "./axion_data")
        : node_(std::make_unique<AxionNode>(
              validator_seed, listen_addr, difficulty, data_dir))
        , identity_(std::make_unique<IdentityManager>(data_dir))
        , miner_(std::make_unique<MiningWorker>(node_->chain))
        , data_dir_(data_dir)
        , listen_addr_(listen_addr)
    {
        // Wire identity indexing into block handler
        node_->net.on_block([this](const Block& b, const std::string&) {
            identity_->index_block(b);
            if (on_new_block_) on_new_block_();
        });
        // Wire miner callback
        miner_->set_mempool(&node_->mempool);
        miner_->start(
            node_->validator_wallet().address(),
            [this](Block b) { handle_mined_block(std::move(b)); }
        );
    }

    ~NodeBridge() {
        miner_->stop();
        // node_ destructor stops network
    }

    // ── Network ────────────────────────────────────────────
    bool start_node() {
        bool ok = node_->start_network();
        if (ok && on_node_started_) on_node_started_();
        return ok;
    }

    void add_seed_peer(const std::string& addr) {
        node_->add_peer(addr);
    }

    // ── Registration ───────────────────────────────────────
    struct RegisterRequest {
        std::string full_name;
        std::string email;
        std::string country;
        std::string phone;
    };

    struct RegisterResponse {
        bool        success       = false;
        std::string error;
        std::string address;       // AXC + hex pubkey
        std::string public_key;    // 64-char hex
        std::string private_key;   // 128-char hex (seed || pubkey)
        std::string tx_id;         // the register tx id
    };

    RegisterResponse do_register(const RegisterRequest& req) {
        RegisterResponse resp;
        auto result = identity_->register_user(
            req.full_name, req.email, req.country, req.phone);
        if (!result.success) {
            resp.error = result.error;
            return resp;
        }
        // Submit register tx to chain
        node_->submit_tx(result.register_tx);

        resp.success     = true;
        resp.address     = result.profile.address;
        resp.public_key  = result.profile.public_key_hex;
        resp.private_key = to_hex(result.keypair.private_key);
        resp.tx_id       = result.register_tx.tx_id;

        std::cout << "[Bridge] Registered: " << req.full_name
                  << "  addr=" << resp.address.substr(0,20) << "...\n";
        return resp;
    }

    // ── Login ───────────────────────────────────────────────
    struct LoginRequest {
        std::string public_key_hex;
        std::string private_key_hex; // used to sign challenge
    };

    struct LoginResponse {
        bool        success = false;
        std::string error;
        UserProfile profile;
        std::string session_pubkey;
    };

    LoginResponse do_login(const LoginRequest& req) {
        LoginResponse resp;
        if (req.public_key_hex.size() != 64) {
            resp.error = "Public key must be 64 hex characters.";
            return resp;
        }
        if (req.private_key_hex.size() != 128) {
            resp.error = "Private key must be 128 hex characters.";
            return resp;
        }
        // Sign a fixed challenge with the private key
        std::string challenge = "axion-login-" + req.public_key_hex;
        Bytes privkey = from_hex(req.private_key_hex);
        std::string sig = APKI::sign(challenge, privkey);

        auto result = identity_->login(req.public_key_hex, challenge, sig);
        if (!result.success) {
            resp.error = result.error;
            return resp;
        }
        resp.success        = true;
        resp.profile        = result.profile;
        resp.session_pubkey = req.public_key_hex;
        // Store active session
        {
            std::lock_guard<std::mutex> lk(session_mtx_);
            session_pubkey_   = req.public_key_hex;
            session_privkey_  = req.private_key_hex;
            session_address_  = result.profile.address;
            session_profile_  = result.profile;
        }
        std::cout << "[Bridge] Login: " << result.profile.full_name << "\n";
        return resp;
    }

    void logout() {
        std::lock_guard<std::mutex> lk(session_mtx_);
        session_pubkey_.clear();
        session_privkey_.clear();
        session_address_.clear();
        std::cout << "[Bridge] Logged out\n";
    }

    bool is_logged_in() const {
        std::lock_guard<std::mutex> lk(session_mtx_);
        return !session_pubkey_.empty();
    }

    UserProfile current_user() const {
        std::lock_guard<std::mutex> lk(session_mtx_);
        return session_profile_;
    }

    std::string current_address() const {
        std::lock_guard<std::mutex> lk(session_mtx_);
        return session_address_;
    }

    // ── Wallet ─────────────────────────────────────────────
    double get_balance(const std::string& address = "") const {
        std::string addr = address.empty() ? current_address() : address;
        return node_->chain.get_balance(addr);
    }

    struct SendRequest {
        std::string to_address;
        double      amount;
        std::string memo;
    };

    struct SendResponse {
        bool        success = false;
        std::string error;
        std::string tx_id;
    };

    SendResponse send_axc(const SendRequest& req) {
        SendResponse resp;
        if (!is_logged_in()) { resp.error = "Not logged in."; return resp; }
        if (req.amount <= 0) { resp.error = "Amount must be positive."; return resp; }

        std::string from_addr, privkey_hex;
        {
            std::lock_guard<std::mutex> lk(session_mtx_);
            from_addr   = session_address_;
            privkey_hex = session_privkey_;
        }

        double bal = node_->chain.get_balance(from_addr);
        if (bal < req.amount) {
            resp.error = "Insufficient balance: have "
                       + std::to_string(bal) + " AXC, need "
                       + std::to_string(req.amount);
            return resp;
        }

        // Build wallet and signed tx
        KeyPair kp = APKI::generate_from_seed(
            from_hex(privkey_hex.substr(0, 64)));
        Wallet w("session", kp);
        Transaction tx = w.build_transfer(req.to_address, req.amount, req.memo);
        if (!node_->submit_tx(tx)) {
            resp.error = "Transaction rejected (signature or AI check).";
            return resp;
        }
        resp.success = true;
        resp.tx_id   = tx.tx_id;
        std::cout << "[Bridge] Send " << req.amount << " AXC -> "
                  << req.to_address.substr(0,20) << "...\n";
        return resp;
    }

    // Deposit (coinbase-style for testnet — in production this is
    // handled by a cross-chain bridge or faucet tx)
    void deposit_axc(double amount, const std::string& address = "") {
        std::string addr = address.empty() ? current_address() : address;
        Transaction cb;
        cb.tx_type  = "coinbase"; cb.sender = "coinbase";
        cb.receiver = addr; cb.amount = amount;
        cb.timestamp = now_ms(); cb.compute_id();
        node_->mempool.submit(cb);
        std::cout << "[Bridge] Deposit queued: " << amount << " AXC -> "
                  << addr.substr(0,20) << "...\n";
    }

    // ── AIPFS File upload ───────────────────────────────────
    struct UploadResponse {
        bool        success = false;
        std::string error;
        std::string cid;
        size_t      size_bytes = 0;
    };

    UploadResponse upload_file(const std::string& filename,
                               const Bytes& file_data) {
        UploadResponse resp;
        if (!is_logged_in()) { resp.error = "Not logged in."; return resp; }
        if (file_data.empty()) { resp.error = "File is empty."; return resp; }

        std::string owner = current_address();
        std::string data_str(file_data.begin(), file_data.end());

        // Store file metadata as prefix
        std::string payload = "FILE|" + filename + "|"
                            + std::to_string(now_ms()) + "|" + data_str;

        resp.cid        = node_->aipfs.put(payload, owner);
        resp.size_bytes = file_data.size();
        resp.success    = true;
        std::cout << "[Bridge] Upload: " << filename
                  << "  " << file_data.size() << "B"
                  << "  cid=" << resp.cid.substr(8,16) << "...\n";
        return resp;
    }

    struct FileInfo {
        std::string filename;
        std::string cid;
        size_t      size_bytes = 0;
        std::string owner;
    };

    std::optional<Bytes> retrieve_file(const std::string& cid) {
        auto data = node_->aipfs.get(cid);
        if (!data) return std::nullopt;
        // Strip "FILE|name|timestamp|" prefix
        size_t third = 0; int bars = 0;
        for (size_t i = 0; i < data->size(); ++i) {
            if ((*data)[i] == '|') { ++bars; if (bars == 3) { third = i+1; break; } }
        }
        Bytes raw(data->begin() + third, data->end());
        return raw;
    }

    // ── Node stats (UI polling) ────────────────────────────
    NodeStats get_stats() const {
        NodeStats s;
        s.chain_height    = node_->chain.height();
        s.peer_count      = node_->net.peer_count();
        s.mempool_size    = node_->mempool.size();
        s.is_mining       = miner_->is_mining();
        s.hash_rate       = miner_->hash_rate();
        s.difficulty      = node_->chain.difficulty();
        s.last_block_hash = node_->chain.last_hash();
        s.aipfs_items     = node_->aipfs.item_count();
        s.identity_count  = identity_->user_count();
        return s;
    }

    // ── Callback registration ──────────────────────────────
    void set_on_node_started(VoidCallback cb)  { on_node_started_ = std::move(cb); }
    void set_on_new_block(VoidCallback cb)     { on_new_block_    = std::move(cb); }
    void set_on_peer_change(VoidCallback cb)   { on_peer_change_  = std::move(cb); }

    // ── Direct accessors for advanced UI ──────────────────
    AxionNode&        node()     { return *node_; }
    IdentityManager&  identity() { return *identity_; }
    MiningWorker&     miner()    { return *miner_; }

private:
    std::unique_ptr<AxionNode>        node_;
    std::unique_ptr<IdentityManager>  identity_;
    std::unique_ptr<MiningWorker>     miner_;
    std::string                       data_dir_;
    std::string                       listen_addr_;

    // Session state
    mutable std::mutex  session_mtx_;
    std::string         session_pubkey_;
    std::string         session_privkey_;
    std::string         session_address_;
    UserProfile         session_profile_;

    // UI callbacks
    VoidCallback on_node_started_;
    VoidCallback on_new_block_;
    VoidCallback on_peer_change_;

    // Handle a block that our miner found
    void handle_mined_block(Block block) {
        // Add coinbase reward tx
        Transaction coinbase;
        coinbase.tx_type  = "coinbase";
        coinbase.sender   = "coinbase";
        coinbase.receiver = node_->validator_wallet().address();
        coinbase.amount   = node_->axc.block_reward(block.header.index);
        coinbase.timestamp = block.header.timestamp;
        coinbase.compute_id();
        block.transactions.insert(block.transactions.begin(), coinbase);
        block.compute_merkle_root();
        block.compute_hash();

        try {
            node_->chain.add_block(block);
            node_->axc.mint(coinbase.receiver, coinbase.amount);
            node_->ai.record_block(block);
            node_->net.update_chain_state(
                node_->chain.height(), node_->chain.last_hash());
            node_->net.broadcast_block(block);
            identity_->index_block(block);
            if (on_new_block_) on_new_block_();
        } catch (const ChainError& e) {
            std::cerr << "[Bridge] Mined block rejected: "
                      << e.what() << "\n";
        }
    }
};

} // namespace axion
