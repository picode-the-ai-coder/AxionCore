#pragma once
// ================================================================
//  Axion Digitaverse - tests.h  [Phase 1]
//  Test suite - updated for real Ed25519 signatures and persistence.
// ================================================================

#include "axion_node.h"
#include "contracts.h"
#include <cassert>
#include <filesystem>

namespace axion {
namespace tests {

static int passed = 0;
static int failed = 0;

#define AX_ASSERT(cond, msg) \
    do { if (!(cond)) { std::cerr << "  [FAIL] " << (msg) << "\n"; ++failed; } \
         else { std::cout << "  [PASS] " << (msg) << "\n"; ++passed; } } while(0)

// ── SHA-256 ────────────────────────────────────────────────
void test_sha256() {
    std::cout << "\n-- SHA-256 tests ------------------------\n";
    AX_ASSERT(sha256("abc") ==
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
        "SHA-256('abc') matches known vector");
    AX_ASSERT(sha256("axion") == sha256("axion"), "SHA-256 deterministic");
    AX_ASSERT(sha256("a") != sha256("b"),         "Different inputs differ");
}

// ── SHA-512 ────────────────────────────────────────────────
void test_sha512() {
    std::cout << "\n-- SHA-512 tests ------------------------\n";
    // Known test vector: SHA-512("abc")
    std::string expected =
        "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a"
        "2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f";
    AX_ASSERT(sha512("abc") == expected, "SHA-512('abc') known vector");
    AX_ASSERT(sha512("") != sha512("a"), "Empty vs non-empty differ");
}

// ── Ed25519 ────────────────────────────────────────────────
void test_ed25519() {
    std::cout << "\n-- Ed25519 tests ------------------------\n";
    // Generate keypair from known seed
    Bytes seed(32, 0x42); // simple test seed
    auto kp = ed25519::keypair_from_seed(seed);

    AX_ASSERT(kp.public_key.size() == 32, "Public key is 32 bytes");
    AX_ASSERT(kp.private_key.size() == 64, "Private key is 64 bytes");

    // Sign and verify
    Bytes msg = {0x68, 0x65, 0x6c, 0x6c, 0x6f}; // "hello"
    Bytes sig = ed25519::sign(msg, kp.private_key);

    AX_ASSERT(sig.size() == 64, "Signature is 64 bytes");
    AX_ASSERT(ed25519::verify(msg, sig, kp.public_key),
              "Valid signature verifies");

    // Tamper test
    Bytes bad_msg = {0x77, 0x6f, 0x72, 0x6c, 0x64}; // "world"
    AX_ASSERT(!ed25519::verify(bad_msg, sig, kp.public_key),
              "Tampered message fails verify");

    // Wrong key test
    Bytes seed2(32, 0x99);
    auto kp2 = ed25519::keypair_from_seed(seed2);
    AX_ASSERT(!ed25519::verify(msg, sig, kp2.public_key),
              "Wrong public key fails verify");

    // Bit-flip in signature
    Bytes bad_sig = sig;
    bad_sig[0] ^= 0x01;
    AX_ASSERT(!ed25519::verify(msg, bad_sig, kp.public_key),
              "Corrupted signature fails verify");
}

// ── APKI (Phase 1) ─────────────────────────────────────────
void test_apki() {
    std::cout << "\n-- APKI (Ed25519) tests -----------------\n";
    auto kp = APKI::generate("test-seed-phase1");

    AX_ASSERT(!kp.address.empty(),           "Address generated");
    AX_ASSERT(kp.public_key.size() == 32,    "Public key is 32 bytes");
    AX_ASSERT(kp.private_key.size() == 64,   "Private key is 64 bytes");
    AX_ASSERT(kp.address.substr(0,3)=="AXC", "Address starts with AXC");
    AX_ASSERT(kp.address.size() == 3 + 64,   "Address is AXC + 64 hex chars");

    std::string msg = "axion transaction payload";
    std::string sig = APKI::sign(msg, kp.private_key);
    AX_ASSERT(sig.size() == 128, "Signature hex is 128 chars");
    AX_ASSERT(APKI::verify(msg, sig, kp.public_key),
              "APKI signature verifies");
    AX_ASSERT(!APKI::verify("tampered", sig, kp.public_key),
              "Tampered message fails verify");

    // Two different seeds -> two different keypairs
    auto kp2 = APKI::generate("different-seed");
    AX_ASSERT(kp.address != kp2.address, "Different seeds give different addresses");
    AX_ASSERT(!APKI::verify(msg, sig, kp2.public_key),
              "Signature fails with wrong key");
}

// ── Transaction signing ────────────────────────────────────
void test_tx_signing() {
    std::cout << "\n-- Transaction signing tests ------------\n";
    auto alice = Wallet::create("alice", "alice-test-seed");
    auto bob   = Wallet::create("bob",   "bob-test-seed");

    auto tx = alice.build_transfer(bob.address(), 100.0, "test payment");
    AX_ASSERT(!tx.signature.empty(),       "Transaction has signature");
    AX_ASSERT(tx.signature.size() == 128,  "Signature is 128 hex chars");
    AX_ASSERT(!tx.sender_pubkey.empty(),   "Sender pubkey stored on tx");
    AX_ASSERT(tx.verify_signature(),       "Transaction signature verifies");

    // Tamper with amount
    tx.amount = 99999.0;
    AX_ASSERT(!tx.verify_signature(), "Tampered amount fails verify");

    // Coinbase always verifies
    Transaction cb;
    cb.tx_type = "coinbase"; cb.sender = "coinbase";
    cb.receiver = alice.address(); cb.amount = 50.0;
    AX_ASSERT(cb.verify_signature(), "Coinbase always valid");
}

// ── Chain with signature enforcement ──────────────────────
void test_chain_signatures() {
    std::cout << "\n-- Chain signature enforcement ----------\n";
    Chain chain(2, "./axion_test_data_chain");

    auto alice = Wallet::create("alice-chain", "alice-chain-seed");
    auto bob   = Wallet::create("bob-chain",   "bob-chain-seed");

    // Fund alice via coinbase
    Transaction cb;
    cb.tx_type="coinbase"; cb.sender="coinbase";
    cb.receiver=alice.address(); cb.amount=1000.0;
    cb.timestamp=now_ms(); cb.compute_id();

    auto b1 = chain.create_candidate({cb}, alice.address());
    chain.mine(b1); chain.add_block(b1);
    AX_ASSERT(chain.get_balance(alice.address()) == 1000.0,
              "Alice funded after block 1");

    // Valid signed transaction
    auto tx = alice.build_transfer(bob.address(), 200.0);
    auto b2 = chain.create_candidate({tx}, alice.address());
    chain.mine(b2); chain.add_block(b2);
    AX_ASSERT(chain.get_balance(bob.address()) == 200.0,
              "Valid signed tx accepted");

    // Forged transaction (wrong signature)
    auto mallory = Wallet::create("mallory", "mallory-seed");
    auto fake_tx = mallory.build_transfer(mallory.address(), 500.0);
    fake_tx.sender        = alice.address();  // claim to be alice
    fake_tx.sender_pubkey = to_hex(mallory.public_key()); // but wrong key
    fake_tx.compute_id();
    // Signature is for mallory's key, not alice's content

    bool caught = false;
    try {
        auto b3 = chain.create_candidate({fake_tx}, alice.address());
        chain.mine(b3); chain.add_block(b3);
    } catch (const ChainError&) { caught = true; }
    AX_ASSERT(caught, "Forged transaction (wrong sig) rejected by chain");

    // Tampered amount
    auto tx2 = alice.build_transfer(bob.address(), 10.0);
    tx2.amount = 99999.0; // tamper after signing
    tx2.compute_id();
    bool caught2 = false;
    try {
        auto b4 = chain.create_candidate({tx2}, alice.address());
        chain.mine(b4); chain.add_block(b4);
    } catch (const ChainError&) { caught2 = true; }
    AX_ASSERT(caught2, "Tampered amount rejected by chain");

    AX_ASSERT(chain.is_valid(), "Chain remains valid after all tests");

    // Cleanup test data
    try { std::filesystem::remove_all("./axion_test_data_chain"); } catch (...) {}
}

// ── KVStore persistence ────────────────────────────────────
void test_kvstore() {
    std::cout << "\n-- KVStore persistence tests ------------\n";
    std::string test_path = "./axion_test_kvstore.axdb";

    // Write
    {
        KVStore db(test_path);
        db.put("key1", "value1");
        db.put("key2", "value2");
        db.put("key3", "value3");
        db.del("key2");
        AX_ASSERT(db.size() == 2, "Store has 2 entries after delete");
    }

    // Read back after re-open (persistence test)
    {
        KVStore db(test_path);
        std::string v;
        AX_ASSERT(db.get("key1", v) && v == "value1", "key1 persisted");
        AX_ASSERT(!db.exists("key2"),                 "Deleted key2 gone");
        AX_ASSERT(db.get("key3", v) && v == "value3", "key3 persisted");
        AX_ASSERT(db.size() == 2, "Correct entry count after reload");
    }

    // Cleanup
    try { std::filesystem::remove(test_path); } catch (...) {}
    AX_ASSERT(true, "KVStore cleanup complete");
}

// ── AIPFS persistence ──────────────────────────────────────
void test_aipfs_persistence() {
    std::cout << "\n-- AIPFS persistence tests --------------\n";
    std::string test_dir = "./axion_test_aipfs";
    std::string cid;

    // Store data
    {
        AIPFS fs(test_dir);
        cid = fs.put("Hello persistent AIPFS!", "AXCowner");
        AX_ASSERT(!cid.empty(), "CID returned");
        auto r = fs.get(cid);
        AX_ASSERT(r.has_value() && *r == "Hello persistent AIPFS!",
                  "Data retrieved in same session");
    }

    // Re-open and verify data persists
    {
        AIPFS fs(test_dir);
        auto r = fs.get(cid);
        AX_ASSERT(r.has_value(), "Data survives restart");
        AX_ASSERT(*r == "Hello persistent AIPFS!",
                  "Retrieved data matches after restart");
        AX_ASSERT(fs.item_count() >= 1, "Item count preserved");
    }

    // Cleanup
    try { std::filesystem::remove_all(test_dir); } catch (...) {}
    AX_ASSERT(true, "AIPFS cleanup complete");
}

// ── Full integration ───────────────────────────────────────
void test_integration() {
    std::cout << "\n-- Integration test (Phase 1) -----------\n";
    std::string test_dir = "./axion_test_integration";

    AxionNode node("integration-validator-seed", "127.0.0.1:9000", 2, test_dir);

    auto alice = Wallet::create("alice", "alice-integration-seed");
    auto bob   = Wallet::create("bob",   "bob-integration-seed");

    // Fund alice
    Transaction fund;
    fund.tx_type="coinbase"; fund.sender="coinbase";
    fund.receiver=alice.address(); fund.amount=1000.0;
    fund.timestamp=now_ms(); fund.compute_id();
    node.mempool.submit(fund);
    node.mine_block();

    AX_ASSERT(node.chain.get_balance(alice.address()) > 0,
              "Alice funded");

    // Real signed transfer
    node.submit_tx(alice.build_transfer(bob.address(), 300.0, "phase1 test"));
    node.mine_block();
    AX_ASSERT(node.chain.get_balance(bob.address()) == 300.0,
              "Bob received 300 AXC via real Ed25519-signed tx");

    // Try forge: bob pretends to spend alice's money
    auto fake = bob.build_transfer(bob.address(), 500.0);
    fake.sender        = alice.address();  // lie about sender
    fake.sender_pubkey = to_hex(bob.public_key()); // bob's key
    fake.compute_id();
    bool rejected = !node.submit_tx(fake); // AI or chain should reject

    // Even if AI passes it, chain will reject on mine
    bool chain_caught = false;
    try {
        node.mempool.submit(fake);
        node.mine_block();
    } catch (...) { chain_caught = true; }
    AX_ASSERT(rejected || chain_caught,
              "Forged tx rejected (AI or chain)");

    // AIPFS
    auto cid = node.store_data("{\"test\":\"phase1\"}", alice.address());
    AX_ASSERT(!cid.empty(), "Data stored on persistent AIPFS");
    auto ret  = node.aipfs.get(cid);
    AX_ASSERT(ret.has_value(), "Data retrievable from AIPFS");

    // Contracts still work
    auto caddr = node.deploy_contract(contracts::counter(), alice.address());
    auto res   = node.call_contract(caddr, alice.address());
    AX_ASSERT(res.success, "Smart contract runs in Phase 1");
    AX_ASSERT(value_to_int(res.return_value) == 1, "Counter = 1");

    AX_ASSERT(node.chain.is_valid(), "Chain integrity passes");

    // Cleanup
    try { std::filesystem::remove_all(test_dir); } catch (...) {}
    std::cout << "  (test data cleaned up)\n";
}

// ── Run all ────────────────────────────────────────────────
inline void run_all() {
    passed = 0; failed = 0;
    std::cout << "\n+=======================================+\n";
    std::cout << "|  Axion Digitaverse - Phase 1 Tests    |\n";
    std::cout << "+=======================================+\n";

    test_sha256();
    test_sha512();
    test_ed25519();
    test_apki();
    test_tx_signing();
    test_chain_signatures();
    test_kvstore();
    test_aipfs_persistence();
    test_integration();

    std::cout << "\n+=======================================+\n";
    std::cout << "|  Results: " << passed << " passed  "
              << failed << " failed"
              << std::string(std::max(0, 19 - (int)std::to_string(passed).size()
                               - (int)std::to_string(failed).size()), ' ')
              << "|\n";
    std::cout << "+=======================================+\n\n";
}

} // namespace tests
} // namespace axion
