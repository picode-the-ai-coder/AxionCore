#pragma once
// ================================================================
//  Axion Digitaverse - identity.h
//  On-chain Identity Registry
//
//  When a user registers:
//    1. APKI generates Ed25519 keypair from their personal info
//    2. A "register" transaction is built containing their details
//       (name, email, etc.) signed with their new private key
//    3. That transaction is submitted to the ADL (chain)
//    4. Their identity is now immutably recorded on-chain
//
//  Login:
//    - User provides their public key + signs a challenge
//    - We look up the public key in the identity registry
//    - Signature verification proves key ownership
//
//  ALL personal data is stored in the ADL transaction `data` field
//  as a pipe-delimited record. It travels with the block and is
//  persisted by the KVStore exactly like any other transaction.
// ================================================================

#include "apki.h"
#include "adl.h"
#include "kvstore.h"
#include <optional>
#include <mutex>

namespace axion {

// ── UserProfile ───────────────────────────────────────────
struct UserProfile {
    std::string public_key_hex;   // 64-char hex = 32-byte Ed25519 pubkey
    std::string address;          // "AXC" + public_key_hex
    std::string full_name;
    std::string email;
    std::string country;
    std::string phone;            // optional
    uint64_t    registered_at;    // Unix ms
    std::string register_tx_id;   // ADL transaction that created this identity
    bool        active = false;

    // Serialize to pipe-delimited string for on-chain storage
    std::string to_data_field() const {
        return "IDENTITY|" + full_name + "|" + email + "|"
             + country + "|" + phone + "|"
             + std::to_string(registered_at) + "|"
             + public_key_hex;
    }

    // Parse from on-chain data field
    static std::optional<UserProfile> from_data_field(
            const std::string& data,
            const std::string& tx_id = "",
            const std::string& pubkey = "") {
        if (data.substr(0, 9) != "IDENTITY|") return std::nullopt;
        std::vector<std::string> parts;
        std::istringstream iss(data);
        std::string tok;
        while (std::getline(iss, tok, '|')) parts.push_back(tok);
        if (parts.size() < 7) return std::nullopt;
        UserProfile p;
        p.full_name      = parts[1];
        p.email          = parts[2];
        p.country        = parts[3];
        p.phone          = parts[4];
        try { p.registered_at = std::stoull(parts[5]); } catch (...) {}
        p.public_key_hex = parts.size() > 6 ? parts[6] : pubkey;
        p.address        = "AXC" + p.public_key_hex;
        p.register_tx_id = tx_id;
        p.active         = true;
        return p;
    }
};

// ── IdentityManager ───────────────────────────────────────
class IdentityManager {
public:
    explicit IdentityManager(const std::string& data_dir)
        : db_(std::make_unique<KVStore>(data_dir + "/identity.axdb"))
    {
        load_from_db();
        std::cout << "[Identity] Registry loaded  users=" << users_.size() << "\n";
    }

    // ── Register a new user ───────────────────────────────
    // Returns {profile, keypair, register_tx}
    // Caller must submit register_tx to the chain.
    struct RegisterResult {
        UserProfile  profile;
        KeyPair      keypair;
        Transaction  register_tx;
        bool         success = false;
        std::string  error;
    };

    RegisterResult register_user(
            const std::string& full_name,
            const std::string& email,
            const std::string& country,
            const std::string& phone = "") {
        RegisterResult res;

        if (full_name.empty() || email.empty() || country.empty()) {
            res.error = "Name, email and country are required.";
            return res;
        }

        // Check duplicate email
        std::lock_guard<std::mutex> lk(mtx_);
        for (const auto& [k, u] : users_) {
            if (u.email == email) {
                res.error = "Email already registered: " + email;
                return res;
            }
        }

        // Generate Ed25519 keypair from deterministic passphrase
        // In production: use OS CSPRNG. For testnet: email + name + timestamp.
        std::string seed_phrase = email + "|" + full_name + "|"
                                + std::to_string(now_ms());
        res.keypair = APKI::generate(seed_phrase);

        // Build the on-chain identity profile
        res.profile.full_name      = full_name;
        res.profile.email          = email;
        res.profile.country        = country;
        res.profile.phone          = phone;
        res.profile.registered_at  = now_ms();
        res.profile.public_key_hex = to_hex(res.keypair.public_key);
        res.profile.address        = res.keypair.address;
        res.profile.active         = true;

        // Build register transaction
        res.register_tx.tx_type       = "register";
        res.register_tx.sender        = res.profile.address;
        res.register_tx.sender_pubkey = res.profile.public_key_hex;
        res.register_tx.receiver      = "axion-identity-registry";
        res.register_tx.amount        = 0.0;
        res.register_tx.timestamp     = res.profile.registered_at;
        res.register_tx.data          = res.profile.to_data_field();
        res.register_tx.compute_id();
        res.register_tx.sign_tx(res.keypair.private_key);

        res.profile.register_tx_id = res.register_tx.tx_id;

        // Cache locally (will also be picked up from chain on restart)
        users_[res.profile.address] = res.profile;
        persist(res.profile);

        res.success = true;
        std::cout << "[Identity] Registered: " << full_name
                  << "  addr=" << res.profile.address.substr(0,20) << "...\n";
        return res;
    }

    // ── Login: verify a signed challenge ─────────────────
    struct LoginResult {
        bool        success = false;
        std::string error;
        UserProfile profile;
    };

    LoginResult login(const std::string& public_key_hex,
                      const std::string& challenge,
                      const std::string& signature_hex) {
        LoginResult res;
        std::lock_guard<std::mutex> lk(mtx_);
        std::string addr = "AXC" + public_key_hex;
        auto it = users_.find(addr);
        if (it == users_.end()) {
            res.error = "Public key not registered.";
            return res;
        }
        Bytes pubkey = from_hex(public_key_hex);
        if (!APKI::verify(challenge, signature_hex, pubkey)) {
            res.error = "Invalid signature — wrong private key.";
            return res;
        }
        res.success = true;
        res.profile = it->second;
        std::cout << "[Identity] Login: " << it->second.full_name << "\n";
        return res;
    }

    // ── Scan a newly accepted block for identity txs ──────
    void index_block(const Block& block) {
        for (const auto& tx : block.transactions) {
            if (tx.tx_type != "register") continue;
            auto prof = UserProfile::from_data_field(
                tx.data, tx.tx_id, tx.sender_pubkey);
            if (!prof) continue;
            std::lock_guard<std::mutex> lk(mtx_);
            if (!users_.count(prof->address)) {
                users_[prof->address] = *prof;
                persist(*prof);
                std::cout << "[Identity] Indexed from chain: "
                          << prof->full_name << "\n";
            }
        }
    }

    // ── Lookups ───────────────────────────────────────────
    bool is_registered(const std::string& address) const {
        std::lock_guard<std::mutex> lk(mtx_);
        return users_.count(address) > 0;
    }

    std::optional<UserProfile> get_by_address(
            const std::string& address) const {
        std::lock_guard<std::mutex> lk(mtx_);
        auto it = users_.find(address);
        if (it == users_.end()) return std::nullopt;
        return it->second;
    }

    std::optional<UserProfile> get_by_pubkey(
            const std::string& pubkey_hex) const {
        return get_by_address("AXC" + pubkey_hex);
    }

    size_t user_count() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return users_.size();
    }

    std::vector<UserProfile> all_users() const {
        std::lock_guard<std::mutex> lk(mtx_);
        std::vector<UserProfile> out;
        for (const auto& [k, u] : users_) out.push_back(u);
        return out;
    }

private:
    mutable std::mutex mtx_;
    std::unique_ptr<KVStore> db_;
    std::unordered_map<std::string, UserProfile> users_;  // keyed by address

    void persist(const UserProfile& p) {
        db_->put("user:" + p.address, p.to_data_field()
            + "|" + p.register_tx_id);
    }

    void load_from_db() {
        auto keys = db_->keys_with_prefix("user:");
        for (const auto& k : keys) {
            std::string val;
            if (!db_->get(k, val)) continue;
            std::string addr = k.substr(5); // strip "user:"
            // Last token after final | is tx_id
            auto last = val.rfind('|');
            std::string tx_id  = (last != std::string::npos)
                                 ? val.substr(last + 1) : "";
            std::string data   = (last != std::string::npos)
                                 ? val.substr(0, last) : val;
            // Extract pubkey from address
            std::string pubkey = addr.size() > 3 ? addr.substr(3) : "";
            auto prof = UserProfile::from_data_field(data, tx_id, pubkey);
            if (prof) users_[addr] = *prof;
        }
    }
};

} // namespace axion
