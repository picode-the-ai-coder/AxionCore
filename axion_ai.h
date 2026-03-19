#pragma once
// ================================================================
//  Axion Digitaverse - axion_ai.h
//  Axion AI Intelligence Layer
//
//  Provides real-time analytics on the network and chain:
//    1. Transaction anomaly detection (z-score + rule heuristics)
//    2. Network load forecasting (EMA over block tx counts)
//    3. Gas fee recommendation based on predicted load
//    4. Validator performance scoring
//
//  Production upgrade:
//    Train models in PyTorch, export to ONNX, load here via
//    ONNX Runtime C++ API for GPU-accelerated inference.
// ================================================================

#include "types.h"
#include "adl.h"
#include <deque>
#include <numeric>
#include <algorithm>

namespace axion {

// ── Anomaly report ─────────────────────────────────────────
struct AnomalyReport {
    std::string tx_id;
    double      score;    // 0.0 (normal) to 1.0 (very suspicious)
    std::string reason;   // semicolon-separated flag names
    bool        flagged;  // score >= ANOMALY_THRESHOLD
};

// ── Load forecast ──────────────────────────────────────────
struct LoadForecast {
    double      predicted_tps;
    double      confidence;
    std::string pressure;   // "low" | "medium" | "high" | "critical"
};

// ── Validator performance score ────────────────────────────
struct ValidatorScore {
    std::string address;
    double      uptime;
    double      efficiency;
    double      score;      // 0-100
};

// ── AxionAI Engine ─────────────────────────────────────────
class AxionAI {
public:
    static constexpr double ANOMALY_THRESHOLD = 0.65;
    static constexpr size_t WINDOW_SIZE       = 100;

    AxionAI() {
        std::cout << "[AxionAI] Engine initialized\n";
    }

    // ── 1. Transaction anomaly detection ──────────────────
    AnomalyReport analyze_tx(const Transaction& tx) {
        AnomalyReport report;
        report.tx_id   = tx.tx_id;
        report.score   = 0.0;
        report.flagged = false;

        // Feature 1: amount z-score vs rolling history
        if (amount_history_.size() >= 5) {
            double mean = std::accumulate(amount_history_.begin(),
                amount_history_.end(), 0.0) / amount_history_.size();
            double sq_sum = 0.0;
            for (double v : amount_history_)
                sq_sum += (v - mean) * (v - mean);
            double stddev = std::sqrt(sq_sum / amount_history_.size());
            if (stddev > 0.0) {
                double z = std::abs(tx.amount - mean) / stddev;
                report.score += std::min(z / 10.0, 0.5);
                if (z > 5.0) report.reason += "large_amount_outlier;";
            }
        }

        // Feature 2: sender sending too frequently
        sender_freq_[tx.sender]++;
        if (sender_freq_[tx.sender] > 10) {
            report.score += 0.2;
            report.reason += "high_frequency_sender;";
        }

        // Feature 3: self-send (definitive suspicious signal)
        if (tx.sender == tx.receiver) {
            report.score += 0.7;
            report.reason += "self_send;";
        }

        // Feature 4: dust transaction
        if (tx.amount < 0.001 && !tx.is_coinbase()) {
            report.score += 0.1;
            report.reason += "dust_tx;";
        }

        report.score   = std::min(report.score, 1.0);
        report.flagged = (report.score >= ANOMALY_THRESHOLD);

        amount_history_.push_back(tx.amount);
        if (amount_history_.size() > WINDOW_SIZE)
            amount_history_.pop_front();

        if (report.flagged)
            std::cout << "[AxionAI] Anomaly detected  tx="
                      << tx.tx_id.substr(0, 16) << "..."
                      << "  score=" << report.score
                      << "  reason=" << report.reason << "\n";

        ++total_analyzed_;
        if (report.flagged) ++total_flagged_;
        return report;
    }

    // ── 2. Record a block for load modeling ───────────────
    void record_block(const Block& block) {
        tx_per_block_.push_back(
            static_cast<double>(block.transactions.size()));
        block_times_.push_back(block.header.timestamp);
        if (tx_per_block_.size() > WINDOW_SIZE) tx_per_block_.pop_front();
        if (block_times_.size()  > WINDOW_SIZE) block_times_.pop_front();
    }

    // ── 3. Predict network load ────────────────────────────
    LoadForecast predict_load() const {
        LoadForecast fc{ 0.0, 0.0, "low" };
        if (tx_per_block_.size() < 2) return fc;

        // Exponential moving average
        double ema = tx_per_block_.front(), alpha = 0.2;
        for (double v : tx_per_block_)
            ema = alpha * v + (1.0 - alpha) * ema;

        double avg_block_time = 5000.0;
        if (block_times_.size() >= 2) {
            double total = 0.0;
            for (size_t i = 1; i < block_times_.size(); ++i)
                total += static_cast<double>(block_times_[i] - block_times_[i-1]);
            avg_block_time = total / static_cast<double>(block_times_.size() - 1);
        }

        fc.predicted_tps = (avg_block_time > 0.0)
            ? ema / (avg_block_time / 1000.0) : 0.0;
        fc.confidence    = std::min(tx_per_block_.size()
                           / static_cast<double>(WINDOW_SIZE), 1.0);

        if      (fc.predicted_tps < 10.0)  fc.pressure = "low";
        else if (fc.predicted_tps < 100.0) fc.pressure = "medium";
        else if (fc.predicted_tps < 500.0) fc.pressure = "high";
        else                                fc.pressure = "critical";

        return fc;
    }

    // ── 4. Gas fee recommendation ──────────────────────────
    double recommend_fee(const std::string& priority = "normal") const {
        auto fc = predict_load();
        double base = 0.01;
        if      (fc.pressure == "medium")   base = 0.05;
        else if (fc.pressure == "high")     base = 0.20;
        else if (fc.pressure == "critical") base = 1.00;

        if      (priority == "fast") return base * 3.0;
        else if (priority == "slow") return base * 0.5;
        return base;
    }

    // ── 5. Validator scoring ───────────────────────────────
    void record_validator_block(const std::string& addr) {
        validator_blocks_[addr]++;
    }

    ValidatorScore score_validator(const std::string& addr,
                                   uint64_t total_blocks) const {
        ValidatorScore vs{ addr, 0.0, 0.0, 0.0 };
        auto it = validator_blocks_.find(addr);
        uint64_t produced = (it != validator_blocks_.end()) ? it->second : 0;
        vs.efficiency = (total_blocks > 0)
            ? std::min(static_cast<double>(produced)
                       / (static_cast<double>(total_blocks) * 0.2), 1.0) : 0.0;
        vs.uptime = vs.efficiency;
        vs.score  = (vs.uptime * 0.4 + vs.efficiency * 0.6) * 100.0;
        return vs;
    }

    void print_stats() const {
        auto fc = predict_load();
        std::cout << "\n-- Axion AI Analytics ------------------\n";
        std::cout << "  Predicted TPS    : "
                  << std::fixed << std::setprecision(2)
                  << fc.predicted_tps << "\n";
        std::cout << "  Network pressure : " << fc.pressure << "\n";
        std::cout << "  Confidence       : "
                  << static_cast<int>(fc.confidence * 100) << "%\n";
        std::cout << "  Recommended fee  : "
                  << recommend_fee() << " AXC\n";
        std::cout << "  Txs analyzed     : " << total_analyzed_ << "\n";
        std::cout << "  Anomalies found  : " << total_flagged_  << "\n";
    }

private:
    std::deque<double>                        amount_history_;
    std::deque<double>                        tx_per_block_;
    std::deque<uint64_t>                      block_times_;
    std::unordered_map<std::string, uint64_t> sender_freq_;
    std::unordered_map<std::string, uint64_t> validator_blocks_;
    uint64_t total_analyzed_ = 0;
    uint64_t total_flagged_  = 0;
};

} // namespace axion
