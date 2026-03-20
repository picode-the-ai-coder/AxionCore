#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <thread>
#include <mutex>
#include <queue>
#include <ctime>
#include <memory>
#include <chrono>
#include <string>
#include <sstream>
#include <algorithm>

/**
 * @class BootnodeService
 * @brief Core bootnode service for Axion network
 * 
 * Manages:
 * - Peer discovery and connection
 * - Network topology
 * - RPC endpoints
 * - Metrics collection
 */

struct PeerInfo {
    std::string node_id;
    std::string ip_address;
    int port;
    std::string status;           // "connected", "disconnected", "pending"
    uint64_t last_seen;
    uint64_t joined_time;
    uint64_t latency_ms;
    int request_count;
    std::vector<std::string> capabilities;
};

class BootnodeService {
private:
    std::string node_id_;
    std::string version_ = "1.0.0";
    std::string network_id_ = "axion-mainnet";
    std::vector<std::string> peer_list_;
    std::map<std::string, PeerInfo> peer_info_;
    std::mutex peer_mutex_;
    
    // Stats
    uint64_t start_time_;
    uint64_t total_transactions_;
    uint64_t current_block_;
    
    // Configuration
    int max_peers_ = 1000;
    int p2p_port_ = 30303;
    int discovery_port_ = 30304;
    int rpc_port_ = 8545;
    
public:
    BootnodeService();
    ~BootnodeService();
    
    // Service lifecycle
    void start();
    void stop();
    void restart();
    
    // Peer management
    void add_peer(const std::string& node_id, const std::string& ip, 
                  int port, const std::vector<std::string>& capabilities = {});
    void remove_peer(const std::string& node_id);
    void update_peer_status(const std::string& node_id, const std::string& status);
    void update_peer_latency(const std::string& node_id, uint64_t latency_ms);
    
    // Discovery
    std::vector<PeerInfo> get_suitable_peers(int limit = 20);
    std::vector<PeerInfo> get_all_peers();
    PeerInfo* get_peer(const std::string& node_id);
    
    // Statistics
    std::string get_network_stats();
    int get_peer_count() const { return peer_list_.size(); }
    uint64_t get_uptime() const;
    double get_average_latency();
    
    // Node info
    std::string get_node_id() const { return node_id_; }
    std::string get_version() const { return version_; }
    std::string get_network_id() const { return network_id_; }
    
    // Configuration
    void configure(int max_peers, int p2p_port, int discovery_port, int rpc_port);
    
private:
    void generate_node_id();
    void start_services();
    void cleanup_disconnected_peers();
    void health_check();
    
    // Service threads
    void run_p2p_server();
    void run_discovery_service();
    void run_rpc_server();
    void run_metrics_server();
    void run_health_check_service();
};

// Singleton instance
extern std::unique_ptr<BootnodeService> g_bootnode;
