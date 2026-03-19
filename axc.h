#pragma once
// ================================================================
//  Axion Digitaverse - axc.h
//  AXC Coin tokenomics engine.
//
//  Supply model:
//    Total supply   : 1,000,000,000 AXC
//    Genesis alloc  :   400,000,000 AXC (ecosystem + team fund)
//    Block reward   : 50 AXC (halves every 210,000 blocks ~4 years)
//    Fee burn       : 1% of every transaction fee is burned
//    Staking APY    : ~12% at launch
// ================================================================

#include "types.h"
#include <unordered_map>

namespace axion {

class AXCLedger {
public:
    static constexpr double TOTAL_SUPPLY         = 1'000'000'000.0;
    static constexpr double GENESIS_ALLOCATION   =   400'000'000.0;
    static constexpr double INITIAL_BLOCK_REWARD =            50.0;
    static constexpr double HALVING_BLOCKS        =       210'000.0;
    static constexpr double MIN_TX_FEE           =             0.01;
    static constexpr double FEE_BURN_RATE        =             0.01;  // 1%

    AXCLedger() : circulating_(0.0), burned_(0.0), staked_(0.0) {
        mint("axion-genesis-fund", GENESIS_ALLOCATION);
        std::cout << "[AXC] Genesis allocation: "
                  << GENESIS_ALLOCATION << " AXC\n";
    }

    // Block reward at a given height (halves every HALVING_BLOCKS)
    double block_reward(uint64_t block_height) const {
        int halvings = static_cast<int>(block_height / HALVING_BLOCKS);
        if (halvings >= 64) return 0.0;
        return INITIAL_BLOCK_REWARD / std::pow(2.0, static_cast<double>(halvings));
    }

    // Mint new coins (only for block rewards and genesis)
    bool mint(const std::string& address, double amount) {
        if (circulating_ + amount > TOTAL_SUPPLY) return false;
        balances_[address] += amount;
        circulating_       += amount;
        return true;
    }

    // Transfer AXC between addresses
    bool transfer(const std::string& from, const std::string& to,
                  double amount, double fee = MIN_TX_FEE) {
        double total = amount + fee;
        if (balances_[from] < total) return false;
        balances_[from]  -= total;
        balances_[to]    += amount;
        double burn       = fee * FEE_BURN_RATE;
        burned_          += burn;
        circulating_     -= burn;
        return true;
    }

    // Stake AXC (locks balance, earns validator rewards)
    bool stake(const std::string& address, double amount) {
        if (balances_[address] < amount) return false;
        balances_[address]        -= amount;
        staked_balances_[address] += amount;
        staked_                   += amount;
        std::cout << "[AXC] Staked " << amount << " AXC  by "
                  << address.substr(0, 12) << "...\n";
        return true;
    }

    bool unstake(const std::string& address, double amount) {
        if (staked_balances_[address] < amount) return false;
        staked_balances_[address] -= amount;
        balances_[address]        += amount;
        staked_                   -= amount;
        return true;
    }

    double get_balance(const std::string& addr) const {
        auto it = balances_.find(addr);
        return (it != balances_.end()) ? it->second : 0.0;
    }

    double get_staked(const std::string& addr) const {
        auto it = staked_balances_.find(addr);
        return (it != staked_balances_.end()) ? it->second : 0.0;
    }

    double circulating()  const { return circulating_; }
    double burned()       const { return burned_; }
    double total_staked() const { return staked_; }

    void print_supply() const {
        std::cout << "\n-- AXC Supply --------------------------\n";
        std::cout << "  Total supply : " << TOTAL_SUPPLY   << " AXC\n";
        std::cout << "  Circulating  : " << circulating_   << " AXC\n";
        std::cout << "  Staked       : " << staked_        << " AXC\n";
        std::cout << "  Burned       : " << burned_        << " AXC\n";
        std::cout << "  Remaining    : " << TOTAL_SUPPLY - circulating_ << " AXC\n";
    }

private:
    std::unordered_map<std::string, double> balances_;
    std::unordered_map<std::string, double> staked_balances_;
    double circulating_, burned_, staked_;
};

} // namespace axion
