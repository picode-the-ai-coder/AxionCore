#pragma once
// ================================================================
//  Axion Digitaverse - mempool.h
//  Pending transaction pool (memory pool).
//  Transactions sit here until a validator mines a block.
// ================================================================

#include "adl.h"
#include <unordered_map>
#include <algorithm>
#include <mutex>

namespace axion {

class Mempool {
public:
    // Submit a transaction. Returns false if already known or invalid.
    bool submit(const Transaction& tx) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (pool_.count(tx.tx_id)) return false;
        if (tx.amount <= 0.0)      return false;
        pool_[tx.tx_id] = tx;
        std::cout << "[Mempool] +tx " << tx.tx_id.substr(0, 16)
                  << "...  " << std::fixed << std::setprecision(2)
                  << tx.amount << " AXC\n";
        return true;
    }

    // Pull up to max_count transactions for block building.
    // Sorted by amount descending (simple fee proxy).
    std::vector<Transaction> drain(size_t max_count = 100) {
        std::lock_guard<std::mutex> lock(mtx_);
        std::vector<Transaction> selected;
        for (auto& [id, tx] : pool_) {
            selected.push_back(tx);
            if (selected.size() >= max_count) break;
        }
        std::sort(selected.begin(), selected.end(),
            [](const Transaction& a, const Transaction& b) {
                return a.amount > b.amount;
            });
        for (const auto& tx : selected)
            pool_.erase(tx.tx_id);
        return selected;
    }

    void remove(const std::string& tx_id) {
        std::lock_guard<std::mutex> lock(mtx_);
        pool_.erase(tx_id);
    }

    bool contains(const std::string& tx_id) const {
        std::lock_guard<std::mutex> lock(mtx_);
        return pool_.count(tx_id) > 0;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return pool_.size();
    }

    void print() const {
        std::lock_guard<std::mutex> lock(mtx_);
        std::cout << "[Mempool] " << pool_.size() << " pending txs\n";
        for (const auto& [id, tx] : pool_) tx.print();
    }

private:
    mutable std::mutex                            mtx_;
    std::unordered_map<std::string, Transaction>  pool_;
};

} // namespace axion
