// ================================================================
//  Axion Digitaverse - main.cpp  [Phase 2 - Real Node]
//
//  Usage:
//    ./axion_node                          -- start on default port 9000
//    ./axion_node 9001                     -- start on port 9001
//    ./axion_node 9001 127.0.0.1:9000      -- start on 9001, connect to 9000
//    ./axion_node 9001 seed1:port,seed2:port  -- multiple seeds
//
//  The node will:
//    1. Load existing chain from disk (or create genesis if none)
//    2. Start TCP listener
//    3. Connect to seed peers
//    4. Sync chain from peers if behind
//    5. Accept transactions and mine blocks
// ================================================================

#include "axion_node.h"
#include "contracts.h"
#include "tests.h"
#include <csignal>
#include <sstream>

// ── Graceful shutdown ──────────────────────────────────────
static std::atomic<bool> g_running{true};
static void sighandler(int) { g_running = false; }

// ── Parse comma-separated seed list ───────────────────────
static std::vector<std::string> parse_seeds(const std::string& s) {
    std::vector<std::string> seeds;
    std::istringstream iss(s);
    std::string tok;
    while (std::getline(iss, tok, ','))
        if (!tok.empty()) seeds.push_back(tok);
    return seeds;
}

int main(int argc, char* argv[]) {
    std::cout << R"(
+====================================================+
|          AXION DIGITAVERSE  v2.0.0                 |
|   Real Ed25519 + Persistent Storage + TCP P2P      |
+====================================================+
)";

    // ── Parse arguments ────────────────────────────────────
    uint16_t    port       = 9000;
    std::string listen_addr= "0.0.0.0:9000";
    std::vector<std::string> seeds;
    bool        run_tests  = true;

    if (argc >= 2) {
        try { port = (uint16_t)std::stoi(argv[1]); }
        catch (...) {}
        listen_addr = "0.0.0.0:" + std::to_string(port);
    }
    if (argc >= 3) seeds = parse_seeds(argv[2]);
    if (argc >= 4 && std::string(argv[3]) == "--no-tests") run_tests = false;

    // ── Run test suite first ───────────────────────────────
    if (run_tests) {
        axion::tests::run_all();
        std::cout << "\n";
    }

    // ── Start the node ─────────────────────────────────────
    std::string data_dir = "./axion_data_" + std::to_string(port);
    axion::AxionNode node("primary-validator-" + std::to_string(port),
                          listen_addr, 2, data_dir);

    // Start networking
    if (!node.start_network()) {
        std::cerr << "[FATAL] Could not bind " << listen_addr << "\n";
        return 1;
    }

    // Connect to seeds
    for (const auto& seed : seeds) {
        std::cout << "[Main] Connecting to seed: " << seed << "\n";
        node.add_peer(seed);
    }

    // ── Create wallets for demo ────────────────────────────
    auto alice = axion::Wallet::create("alice", "alice-wallet-" + std::to_string(port));
    auto bob   = axion::Wallet::create("bob",   "bob-wallet-"   + std::to_string(port));
    alice.print();

    // ── Signal handler ─────────────────────────────────────
    std::signal(SIGINT,  sighandler);
    std::signal(SIGTERM, sighandler);

    std::cout << "\n[Main] Node running. Press Ctrl+C to stop.\n";
    std::cout << "[Main] Peers: " << node.net.peer_count() << "\n\n";

    // ── Main loop ──────────────────────────────────────────
    int tick = 0;
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ++tick;

        // Mine a block every 10 seconds if we have mempool txs
        // or every 30 seconds regardless (to keep chain moving)
        bool should_mine = (tick % 10 == 0 && node.mempool.size() > 0)
                        || (tick % 30 == 0);

        if (should_mine) {
            std::cout << "[Main] Mining block #" << (node.chain.height()+1)
                      << "  mempool=" << node.mempool.size()
                      << "  peers="   << node.net.peer_count() << "\n";
            node.mine_block();
        }

        // Every 5s: submit a demo transfer if alice has balance
        if (tick % 5 == 0 && tick <= 60) {
            double alice_bal = node.chain.get_balance(alice.address());
            if (alice_bal >= 10.0) {
                auto tx = alice.build_transfer(bob.address(), 1.0, "demo");
                node.submit_tx(tx);
            }
        }

        // Every 60s: print status
        if (tick % 60 == 0) {
            node.print_status();
        }

        // Seed alice on first block
        if (tick == 2 && node.chain.height() == 0) {
            axion::Transaction fund;
            fund.tx_type  = "coinbase"; fund.sender = "coinbase";
            fund.receiver = alice.address(); fund.amount = 10000.0;
            fund.timestamp = axion::now_ms(); fund.compute_id();
            node.mempool.submit(fund);
        }
    }

    std::cout << "\n[Main] Shutting down...\n";
    node.print_status();
    std::cout << "[Main] Chain height: " << node.chain.height() << "\n";
    std::cout << "[Main] Goodbye.\n";
    return 0;
}
