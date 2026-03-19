#pragma once
// ================================================================
//  Axion Digitaverse - axion_node.h  [Phase 2 - Real Node]
//
//  A fully operational blockchain node:
//    - Real Ed25519 signing (Phase 1)
//    - Persistent chain + AIPFS storage (Phase 1)
//    - Real TCP P2P networking (Phase 2)
//    - Block sync from peers on startup (Phase 2)
//    - Transaction/block propagation over the network (Phase 2)
// ================================================================

#include "chain.h"
#include "mempool.h"
#include "axc.h"
#include "avm.h"
#include "network.h"
#include "aipfs.h"
#include "axion_ai.h"
#include "wallet.h"

namespace axion {

class AxionNode {
public:
    Chain          chain;
    Mempool        mempool;
    AVM            avm;
    AIPFS          aipfs;
    AXCLedger      axc;
    AxionAI        ai;

    // validator_wallet_ declared BEFORE net because net's constructor
    // uses validator_wallet_.address() in the initialiser list.
    // (C++ initialises in declaration order, not initialiser-list order.)
private:
    Wallet         validator_wallet_;
public:
    NetworkManager net;

    // ── Constructor ────────────────────────────────────────
    explicit AxionNode(const std::string& validator_seed,
                       const std::string& listen_addr = "0.0.0.0:9000",
                       uint32_t           difficulty   = 2,
                       const std::string& data_dir     = "./axion_data")
        : chain(difficulty, data_dir)
        , avm(500'000)
        , aipfs(data_dir)
        , validator_wallet_(Wallet::create("validator", validator_seed))
        , net(listen_addr,
              validator_wallet_.address(),
              chain.height(),
              chain.last_hash())
    {
        // Wire network -> chain + mempool handlers
        net.on_transaction([this](const Transaction& tx,
                                  const std::string& /*from*/) {
            if (!tx.is_coinbase() && !tx.verify_signature()) {
                std::cout << "[Node] Rejected tx (bad sig): "
                          << tx.tx_id.substr(0,16) << "\n";
                return;
            }
            Transaction t = tx;
            auto report = ai.analyze_tx(t);
            if (report.flagged && report.score > 0.90) return;
            mempool.submit(tx);
        });

        net.on_block([this](const Block& block, const std::string& from) {
            handle_incoming_block(block, from);
        });

        net.on_sync_request([this](uint64_t from_height,
                                   uint32_t max_count) -> std::vector<Block> {
            std::vector<Block> result;
            for (uint64_t i = from_height;
                 i <= chain.height() && result.size() < max_count; ++i) {
                const Block* b = chain.get_block(i);
                if (b) result.push_back(*b);
            }
            std::cout << "[Node] Sync: serving " << result.size()
                      << " blocks from height " << from_height << "\n";
            return result;
        });

        std::cout << "[Node] Started  validator="
                  << validator_wallet_.address() << "\n\n";
    }

    // ── Start the network layer ────────────────────────────
    bool start_network() {
        return net.start();
    }

    // ── Connect to a seed peer ─────────────────────────────
    void add_peer(const std::string& addr) {
        net.add_seed(addr);
        net.connect_to(addr);
    }

    // ── Submit a transaction ───────────────────────────────
    bool submit_tx(Transaction tx) {
        tx.compute_id();
        if (!tx.is_coinbase() && !tx.verify_signature()) {
            std::cout << "[Node] TX rejected: bad signature\n";
            return false;
        }
        auto report = ai.analyze_tx(tx);
        if (report.flagged && report.score > 0.90) {
            std::cout << "[Node] TX rejected: anomaly score="
                      << report.score << "\n";
            return false;
        }
        if (!mempool.submit(tx)) return false;
        net.broadcast_tx(tx);
        return true;
    }

    // ── Mine a block ───────────────────────────────────────
    bool mine_block(size_t max_txs = 50) {
        auto txs = mempool.drain(max_txs);

        Transaction coinbase;
        coinbase.tx_type   = "coinbase";
        coinbase.sender    = "coinbase";
        coinbase.receiver  = validator_wallet_.address();
        coinbase.amount    = axc.block_reward(chain.height() + 1);
        coinbase.timestamp = now_ms();
        coinbase.compute_id();
        txs.insert(txs.begin(), coinbase);

        auto block = chain.create_candidate(txs, validator_wallet_.address());
        chain.mine(block);
        try {
            chain.add_block(block);
        } catch (const ChainError& e) {
            std::cerr << "[Node] Mine failed: " << e.what() << "\n";
            return false;
        }

        axc.mint(validator_wallet_.address(), coinbase.amount);
        ai.record_block(block);
        ai.record_validator_block(validator_wallet_.address());
        net.update_chain_state(chain.height(), chain.last_hash());
        net.broadcast_block(block);
        return true;
    }

    // ── Handle block received from network ─────────────────
    void handle_incoming_block(const Block& block, const std::string& from) {
        if (chain.get_block_by_hash(block.hash)) return;

        if (block.header.previous_hash == chain.last_hash()) {
            Block mb = block;
            try {
                chain.add_block(mb);
                axc.mint(block.header.validator,
                         axc.block_reward(block.header.index));
                ai.record_block(block);
                net.update_chain_state(chain.height(), chain.last_hash());
                std::cout << "[Node] Accepted peer block #"
                          << block.header.index << " from "
                          << from.substr(0, 12) << "\n";
            } catch (const ChainError& e) {
                std::cerr << "[Node] Rejected peer block: "
                          << e.what() << "\n";
            }
        } else if (block.header.index > chain.height() + 1) {
            std::cout << "[Node] Behind peer (#" << block.header.index
                      << " > #" << chain.height() << ") — syncing...\n";
        }
    }

    // ── Deploy a smart contract ────────────────────────────
    std::string deploy_contract(const std::vector<Instruction>& bytecode,
                                const std::string& owner_address) {
        std::string serial;
        for (const auto& i : bytecode)
            serial += std::to_string(static_cast<int>(i.opcode)) + ";";
        std::string cid  = aipfs.put(serial, owner_address);
        std::string addr = avm.deploy(bytecode, owner_address, chain.height());
        std::cout << "[Node] Contract deployed  addr="
                  << addr.substr(0, 16) << "...\n";
        return addr;
    }

    // ── Call a smart contract ──────────────────────────────
    ExecResult call_contract(const std::string& contract_addr,
                             const std::string& caller_address,
                             double   value = 0.0,
                             uint64_t gas   = 100'000) {
        ExecContext ctx;
        ctx.caller         = caller_address;
        ctx.value          = value;
        ctx.block_height   = chain.height();
        ctx.caller_balance = chain.get_balance(caller_address);
        return avm.call(contract_addr, ctx, gas);
    }

    // ── Store data on AIPFS ───────────────────────────────
    std::string store_data(const std::string& data,
                           const std::string& owner) {
        return aipfs.put(data, owner);
    }

    // ── Full status ────────────────────────────────────────
    void print_status() const {
        chain.print_summary();
        chain.print_balances();
        axc.print_supply();
        aipfs.print_stats();
        ai.print_stats();
        avm.print_contracts();
        net.print_status();
    }

    const Wallet& validator_wallet() const { return validator_wallet_; }
};

} // namespace axion
