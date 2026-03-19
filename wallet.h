#pragma once
// ================================================================
//  Axion Digitaverse - wallet.h  [Phase 1]
//  Wallet: key management and signed transaction builder.
//
//  Phase 1 changes:
//    - Stores real Ed25519 KeyPair (Bytes-based keys)
//    - build_transfer() sets sender_pubkey on every tx
//    - All transactions signed with real Ed25519 private key
// ================================================================

#include "apki.h"
#include "adl.h"

namespace axion {

class Wallet {
public:
    Wallet(const std::string& name, const KeyPair& kp)
        : name_(name), kp_(kp) {}

    // Create a wallet from a passphrase (deterministic)
    static Wallet create(const std::string& name,
                         const std::string& passphrase) {
        auto kp = APKI::generate(passphrase);
        std::cout << "[Wallet] Created '" << name
                  << "'  address=" << kp.address << "\n";
        return Wallet(name, kp);
    }

    // Build a signed transfer transaction
    Transaction build_transfer(const std::string& to_address,
                                double amount,
                                const std::string& memo = "") const {
        Transaction tx;
        tx.tx_type       = "transfer";
        tx.sender        = kp_.address;
        tx.sender_pubkey = to_hex(kp_.public_key);
        tx.receiver      = to_address;
        tx.amount        = amount;
        tx.timestamp     = now_ms();
        tx.data          = memo;
        tx.compute_id();
        tx.sign_tx(kp_.private_key);
        return tx;
    }

    // Build a contract deployment transaction
    Transaction build_deploy(const std::string& contract_cid,
                              double gas_fee = 1.0) const {
        Transaction tx;
        tx.tx_type       = "contract_deploy";
        tx.sender        = kp_.address;
        tx.sender_pubkey = to_hex(kp_.public_key);
        tx.receiver      = "axion-avm";
        tx.amount        = gas_fee;
        tx.timestamp     = now_ms();
        tx.data          = contract_cid;
        tx.compute_id();
        tx.sign_tx(kp_.private_key);
        return tx;
    }

    // Build a contract call transaction
    Transaction build_call(const std::string& contract_addr,
                            double value,
                            const std::string& calldata = "") const {
        Transaction tx;
        tx.tx_type       = "contract_call";
        tx.sender        = kp_.address;
        tx.sender_pubkey = to_hex(kp_.public_key);
        tx.receiver      = contract_addr;
        tx.amount        = value;
        tx.timestamp     = now_ms();
        tx.data          = calldata;
        tx.compute_id();
        tx.sign_tx(kp_.private_key);
        return tx;
    }

    const std::string& address()    const { return kp_.address; }
    const Bytes&       public_key() const { return kp_.public_key; }
    const std::string& name()       const { return name_; }

    void print() const {
        std::cout << "\n-- Wallet: " << name_ << " ----------------\n";
        APKI::print(kp_);
    }

private:
    std::string name_;
    KeyPair     kp_;
};

} // namespace axion
