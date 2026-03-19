#pragma once
// ================================================================
//  Axion Digitaverse - consensus.h
//  Consensus rules and difficulty auto-adjustment.
//
//  Rules:
//    1. Longest valid chain wins (most cumulative work)
//    2. Difficulty adjusts every ADJUST_INTERVAL blocks
//       to target BLOCK_TIME_MS milliseconds per block
//    3. Fork resolution: when a competing chain arrives from
//       a peer with higher height, switch if it's valid
//    4. Mining runs in a background thread; stops when a
//       peer block arrives at the same height
// ================================================================

#include "chain.h"
#include <atomic>
#include <thread>
#include <functional>
#include <chrono>

namespace axion {

// ── Consensus parameters ──────────────────────────────────
static constexpr uint64_t BLOCK_TIME_MS     = 15'000;   // 15 seconds target
static constexpr uint64_t ADJUST_INTERVAL   = 10;       // adjust every 10 blocks
static constexpr uint32_t MIN_DIFFICULTY    = 1;
static constexpr uint32_t MAX_DIFFICULTY    = 8;
static constexpr uint32_t INITIAL_DIFFICULTY= 2;

// ── Difficulty calculator ─────────────────────────────────
inline uint32_t calculate_difficulty(const Chain& chain) {
    uint64_t height = chain.height();
    if (height < ADJUST_INTERVAL) return INITIAL_DIFFICULTY;
    if (height % ADJUST_INTERVAL != 0) return chain.difficulty();

    // Get timestamp of block ADJUST_INTERVAL blocks ago
    const Block* old_block = chain.get_block(height - ADJUST_INTERVAL);
    const Block* new_block = chain.get_block(height);
    if (!old_block || !new_block) return chain.difficulty();

    uint64_t elapsed_ms = new_block->header.timestamp
                        - old_block->header.timestamp;
    if (elapsed_ms == 0) elapsed_ms = 1;

    uint64_t target_ms = BLOCK_TIME_MS * ADJUST_INTERVAL;

    // Ratio: actual / target
    // If actual < target/2: too fast -> increase difficulty
    // If actual > target*2: too slow -> decrease difficulty
    uint32_t current = chain.difficulty();
    if (elapsed_ms < target_ms / 2 && current < MAX_DIFFICULTY)
        return current + 1;
    if (elapsed_ms > target_ms * 2 && current > MIN_DIFFICULTY)
        return current - 1;
    return current;
}

// ── MiningWorker: mines blocks in background ─────────────
class MiningWorker {
public:
    using BlockReadyCallback = std::function<void(Block)>;

    explicit MiningWorker(Chain& chain)
        : chain_(chain), running_(false), mining_(false) {}

    ~MiningWorker() { stop(); }

    // Start / stop the mining thread
    void start(const std::string& miner_address,
               BlockReadyCallback on_block_ready) {
        if (running_) return;
        miner_address_ = miner_address;
        on_ready_      = std::move(on_block_ready);
        running_       = true;
        worker_        = std::thread(&MiningWorker::mine_loop, this);
        std::cout << "[Mining] Worker started  miner="
                  << miner_address_.substr(0,20) << "...\n";
    }

    void stop() {
        running_ = false;
        interrupt_current();
        if (worker_.joinable()) worker_.join();
    }

    // Called when a peer block arrives — interrupt current PoW
    void interrupt_current() { interrupt_ = true; }

    bool is_mining() const { return mining_.load(); }

    uint64_t hash_rate() const { return hash_rate_.load(); }

    void set_mempool(Mempool* mp) { mempool_ = mp; }

private:
    Chain&               chain_;
    Mempool*             mempool_    = nullptr;
    std::string          miner_address_;
    BlockReadyCallback   on_ready_;
    std::atomic<bool>    running_{false};
    std::atomic<bool>    mining_{false};
    std::atomic<bool>    interrupt_{false};
    std::atomic<uint64_t> hash_rate_{0};
    std::thread          worker_;

    void mine_loop() {
        while (running_) {
            // Wait a moment before starting next block
            // (allow peer messages to arrive first)
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if (!running_) break;

            // Recalculate difficulty
            uint32_t diff = calculate_difficulty(chain_);

            // Pull transactions from mempool
            std::vector<Transaction> txs;
            if (mempool_) txs = mempool_->drain(50);

            // Build candidate block
            auto block = chain_.create_candidate(txs, miner_address_);
            block.header.difficulty = diff;
            block.compute_merkle_root();

            // Mine it
            mining_    = true;
            interrupt_ = false;
            bool found = do_pow(block);
            mining_    = false;

            if (found && running_) {
                std::cout << "[Mining] Found block #"
                          << block.header.index
                          << "  diff=" << diff
                          << "  nonce=" << block.header.nonce
                          << "  hash=" << block.hash.substr(0,16)
                          << "...\n";
                if (on_ready_) on_ready_(std::move(block));
            } else if (!found) {
                // Interrupted by peer block — put txs back
                if (mempool_) for (auto& tx : txs) mempool_->submit(tx);
            }
        }
    }

    // Proof-of-Work inner loop
    bool do_pow(Block& block) {
        std::string target(block.header.difficulty, '0');
        uint64_t   attempts = 0;
        auto       start    = std::chrono::steady_clock::now();

        for (uint32_t nonce = 0; nonce < 0xFFFFFFFFu; ++nonce) {
            if (interrupt_ || !running_) return false;
            block.header.nonce = nonce;
            block.compute_hash();
            ++attempts;

            if (block.hash.substr(0, block.header.difficulty) == target)
                return true;

            // Update hash rate every 10k attempts
            if (attempts % 10000 == 0) {
                auto now = std::chrono::steady_clock::now();
                auto ms  = std::chrono::duration_cast<
                    std::chrono::milliseconds>(now - start).count();
                if (ms > 0)
                    hash_rate_ = (attempts * 1000ULL) / (uint64_t)ms;
            }
        }
        return false;
    }
};

// ── ForkResolver: switch to a longer valid chain ──────────
class ForkResolver {
public:
    // Returns true if we should switch to the new chain
    // (called when we receive a chain from a peer that
    //  has more blocks than ours)
    static bool should_switch(const Chain& our_chain,
                              uint64_t peer_height,
                              const Hash256& peer_tip_hash) {
        if (peer_height <= our_chain.height()) return false;
        // Peer has strictly more blocks — switch
        return true;
    }

    // Apply incoming blocks one by one in order
    // Returns number of blocks successfully applied
    static size_t apply_blocks(Chain& chain,
                               Mempool& mempool,
                               const std::vector<Block>& blocks,
                               MiningWorker& miner) {
        size_t applied = 0;
        for (auto b : blocks) {   // copy for mutability
            try {
                if (b.header.index != chain.height() + 1) continue;
                if (b.header.previous_hash != chain.last_hash()) continue;
                miner.interrupt_current();
                chain.add_block(b);
                // Remove any txs in the new block from mempool
                for (const auto& tx : b.transactions)
                    mempool.remove(tx.tx_id);
                ++applied;
            } catch (const ChainError& e) {
                std::cerr << "[Fork] Block rejected: " << e.what() << "\n";
                break;
            }
        }
        if (applied > 0)
            std::cout << "[Fork] Applied " << applied
                      << " peer blocks. New height=" << chain.height() << "\n";
        return applied;
    }
};

} // namespace axion
