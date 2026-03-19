#pragma once
// ================================================================
//  Axion Digitaverse - apki.h  [Phase 1 - Real Ed25519]
//  Axion Public Key Infrastructure
//
//  UPGRADED from Phase 0:
//    Before: HMAC-SHA256 simulation (forgeable)
//    After:  Real Ed25519 elliptic curve signatures (unforgeable)
//
//  Ed25519 properties:
//    - 32-byte private seed   (keep secret)
//    - 32-byte public key     (share freely)
//    - 64-byte signature      (authenticates any message)
//    - Verification needs only: message + signature + public key
//    - No private key registry needed for verify (unlike Phase 0)
//
//  Production upgrade path:
//    Replace ed25519.h internals with libsodium:
//      crypto_sign_ed25519_keypair()
//      crypto_sign_ed25519_detached()
//      crypto_sign_ed25519_verify_detached()
//    API is identical - one include swap.
// ================================================================

#include "sha256.h"
#include "ed25519.h"

namespace axion {

// ----------------------------------------------------------------
//  KeyPair - holds all key material for one identity
// ----------------------------------------------------------------
struct KeyPair {
    Bytes       seed;         // 32 bytes - the secret seed
    Bytes       private_key;  // 64 bytes - seed || public_key
    Bytes       public_key;   // 32 bytes - the shareable identity
    std::string address;      // "AXC" + hex(public_key)
};

// ----------------------------------------------------------------
//  APKI - keypair generation, signing, verification
// ----------------------------------------------------------------
class APKI {
public:
    // Generate a deterministic keypair from a passphrase.
    // Same passphrase always produces the same keypair.
    // Production: generate seed from a CSPRNG instead.
    static KeyPair generate(const std::string& passphrase) {
        // Derive 32 seed bytes from passphrase via SHA-256
        std::string seed_hex = sha256(passphrase + "axion-apki-v2-ed25519");
        // Take first 32 bytes (64 hex chars)
        Bytes seed_bytes = from_hex(seed_hex.substr(0, 64));

        auto ekp = ed25519::keypair_from_seed(seed_bytes);

        KeyPair kp;
        kp.seed        = ekp.seed;
        kp.private_key = ekp.private_key;
        kp.public_key  = ekp.public_key;
        kp.address     = "AXC" + to_hex(kp.public_key);
        return kp;
    }

    // Generate from raw 32-byte seed bytes
    static KeyPair generate_from_seed(const Bytes& seed) {
        auto ekp = ed25519::keypair_from_seed(seed);
        KeyPair kp;
        kp.seed        = ekp.seed;
        kp.private_key = ekp.private_key;
        kp.public_key  = ekp.public_key;
        kp.address     = "AXC" + to_hex(kp.public_key);
        return kp;
    }

    // Sign a message string with private_key bytes.
    // Returns 64-byte signature as 128-char hex string.
    static std::string sign(const std::string& message,
                            const Bytes& private_key) {
        Bytes msg(message.begin(), message.end());
        Bytes sig = ed25519::sign(msg, private_key);
        return to_hex(sig);
    }

    // Verify a hex signature against a message and public key bytes.
    // Returns true only if signature is mathematically valid.
    static bool verify(const std::string& message,
                       const std::string& sig_hex,
                       const Bytes& public_key) {
        if (sig_hex.size() != 128) return false;
        Bytes sig = from_hex(sig_hex);
        Bytes msg(message.begin(), message.end());
        return ed25519::verify(msg, sig, public_key);
    }

    // Derive AXC address from public key bytes
    static std::string address_from_pubkey(const Bytes& pub) {
        return "AXC" + to_hex(pub);
    }

    static void print(const KeyPair& kp) {
        std::cout << "  Address    : " << kp.address << "\n";
        std::cout << "  Public key : "
                  << to_hex(kp.public_key).substr(0, 32) << "...\n";
        std::cout << "  Private key: [Ed25519 - 64 bytes - KEEP SECRET]\n";
    }
};

} // namespace axion
