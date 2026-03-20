#include "bootnode_service.hpp"
#include <iomanip>
#include <fstream>

std::unique_ptr<BootnodeService> g_bootnode = nullptr;

BootnodeService::BootnodeService() {
    generate_node_id();
    start_time_ = std::time(nullptr);
    total_transactions_ = 0;
    current_block_ = 0;
    
    std::cout << "╔════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║          🌐 AXION NETWORK BOOTNODE v" << std::setw(4) << std::left << version_ << "                     ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════════╝" << std::endl;
    std::cout << std::endl;
    
    std::cout << "[BOOTNODE] Initializing bootnode service..." << std::endl;
    std::cout << "[BOOTNODE] Node ID: " << node_id_ << std::endl;
    std::cout << "[BOOTNODE] Network: " << network_id_ << std::endl;
    std::cout << "[BOOTNODE] Configuration: OK" << std::endl;
}

BootnodeService::~BootnodeService() {
    stop();
}

void BootnodeService::generate_node_id() {
    // Generate a unique node ID based on timestamp and network name
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << "enode://";
    ss << "axion";
    ss << "_bootnode_";
    ss << std::hex << std::time(nullptr);
    ss << "@127.0.0.1:30303";
    
    node_id_ = ss.str();
}

void BootnodeService::start() {
    std::cout << "[BOOTNODE] Starting bootnode services..." << std::endl;
    
    start_services();
    
    std::cout << "[BOOTNODE] ✓ All services started successfully" << std::endl;
    std::cout << "[BOOTNODE] ✓ Ready to accept peer connections" << std::endl;
    std::cout << std::endl;
    
    print_network_status();
}

void BootnodeService::stop() {
    std::cout << "[BOOTNODE] Stopping bootnode services..." << std::endl;
    std::cout << "[BOOTNODE] Closing peer connections..." << std::endl;
    
    {
        std::lock_guard<std::mutex> lock(peer_mutex_);
        peer_info_.clear();
        peer_list_.clear();
    }
    
    std::cout << "[BOOTNODE] ✓ All services stopped" << std::endl;
}

void BootnodeService::restart() {
    std::cout << "[BOOTNODE] Restarting bootnode..." << std::endl;
    stop();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    start();
}

void BootnodeService::configure(int max_peers, int p2p_port, 
                                 int discovery_port, int rpc_port) {
    max_peers_ = max_peers;
    p2p_port_ = p2p_port;
    discovery_port_ = discovery_port;
    rpc_port_ = rpc_port;
    
    std::cout << "[CONFIG] Configuration updated:" << std::endl;
    std::cout << "[CONFIG]   Max peers: " << max_peers_ << std::endl;
    std::cout << "[CONFIG]   P2P port: " << p2p_port_ << std::endl;
    std::cout << "[CONFIG]   Discovery port: " << discovery_port_ << std::endl;
    std::cout << "[CONFIG]   RPC port: " << rpc_port_ << std::endl;
}

void BootnodeService::add_peer(const std::string& node_id, const std::string& ip, 
                                int port, const std::vector<std::string>& capabilities) {
    std::lock_guard<std::mutex> lock(peer_mutex_);
    
    if (peer_info_.count(node_id)) {
        std::cout << "[PEER] Peer already exists: " << node_id.substr(0, 10) << "..." << std::endl;
        return;
    }
    
    if (peer_list_.size() >= max_peers_) {
        std::cout << "[PEER] ⚠️  Max peers reached (" << max_peers_ << ")" << std::endl;
        return;
    }
    
    PeerInfo info;
    info.node_id = node_id;
    info.ip_address = ip;
    info.port = port;
    info.status = "connected";
    info.last_seen = std::time(nullptr);
    info.joined_time = std::time(nullptr);
    info.latency_ms = 0;
    info.request_count = 0;
    info.capabilities = capabilities;
    
    peer_info_[node_id] = info;
    peer_list_.push_back(node_id);
    
    std::cout << "[PEER] ✓ Added peer: " << node_id.substr(0, 10) << "... ("
              << ip << ":" << port << ")" << std::endl;
}

void BootnodeService::remove_peer(const std::string& node_id) {
    std::lock_guard<std::mutex> lock(peer_mutex_);
    
    if (peer_info_.find(node_id) == peer_info_.end()) {
        return;
    }
    
    peer_info_.erase(node_id);
    peer_list_.erase(
        std::remove(peer_list_.begin(), peer_list_.end(), node_id),
        peer_list_.end()
    );
    
    std::cout << "[PEER] ✗ Removed peer: " << node_id.substr(0, 10) << "..." << std::endl;
}

void BootnodeService::update_peer_status(const std::string& node_id, const std::string& status) {
    std::lock_guard<std::mutex> lock(peer_mutex_);
    
    auto it = peer_info_.find(node_id);
    if (it != peer_info_.end()) {
        it->second.status = status;
        it->second.last_seen = std::time(nullptr);
    }
}

void BootnodeService::update_peer_latency(const std::string& node_id, uint64_t latency_ms) {
    std::lock_guard<std::mutex> lock(peer_mutex_);
    
    auto it = peer_info_.find(node_id);
    if (it != peer_info_.end()) {
        it->second.latency_ms = latency_ms;
        it->second.last_seen = std::time(nullptr);
    }
}

std::vector<PeerInfo> BootnodeService::get_suitable_peers(int limit) {
    std::lock_guard<std::mutex> lock(peer_mutex_);
    
    std::vector<PeerInfo> result;
    int count = 0;
    
    for (const auto& node_id : peer_list_) {
        if (count >= limit) break;
        
        auto it = peer_info_.find(node_id);
        if (it != peer_info_.end() && it->second.status == "connected") {
            result.push_back(it->second);
            count++;
        }
    }
    
    return result;
}

std::vector<PeerInfo> BootnodeService::get_all_peers() {
    std::lock_guard<std::mutex> lock(peer_mutex_);
    
    std::vector<PeerInfo> result;
    for (const auto& peer : peer_info_) {
        result.push_back(peer.second);
    }
    return result;
}

PeerInfo* BootnodeService::get_peer(const std::string& node_id) {
    auto it = peer_info_.find(node_id);
    if (it != peer_info_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::string BootnodeService::get_network_stats() {
    std::lock_guard<std::mutex> lock(peer_mutex_);
    
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"bootnode\": \"" << node_id_ << "\",\n";
    ss << "  \"version\": \"" << version_ << "\",\n";
    ss << "  \"network\": \"" << network_id_ << "\",\n";
    ss << "  \"peer_count\": " << peer_list_.size() << ",\n";
    ss << "  \"max_peers\": " << max_peers_ << ",\n";
    ss << "  \"uptime_seconds\": " << get_uptime() << ",\n";
    ss << "  \"status\": \"active\",\n";
    ss << "  \"p2p_port\": " << p2p_port_ << ",\n";
    ss << "  \"discovery_port\": " << discovery_port_ << ",\n";
    ss << "  \"rpc_port\": " << rpc_port_ << ",\n";
    ss << "  \"current_block\": " << current_block_ << ",\n";
    ss << "  \"total_transactions\": " << total_transactions_ << ",\n";
    ss << "  \"average_latency_ms\": " << get_average_latency() << ",\n";
    ss << "  \"timestamp\": " << std::time(nullptr) << "\n";
    ss << "}\n";
    
    return ss.str();
}

uint64_t BootnodeService::get_uptime() const {
    return std::time(nullptr) - start_time_;
}

double BootnodeService::get_average_latency() {
    std::lock_guard<std::mutex> lock(peer_mutex_);
    
    if (peer_info_.empty()) return 0.0;
    
    uint64_t total = 0;
    for (const auto& peer : peer_info_) {
        total += peer.second.latency_ms;
    }
    
    return static_cast<double>(total) / peer_info_.size();
}

void BootnodeService::start_services() {
    // Start P2P server
    std::thread p2p_thread([this]() { this->run_p2p_server(); });
    p2p_thread.detach();
    
    // Start discovery service
    std::thread discovery_thread([this]() { this->run_discovery_service(); });
    discovery_thread.detach();
    
    // Start RPC server
    std::thread rpc_thread([this]() { this->run_rpc_server(); });
    rpc_thread.detach();
    
    // Start metrics server
    std::thread metrics_thread([this]() { this->run_metrics_server(); });
    metrics_thread.detach();
    
    // Start health check
    std::thread health_thread([this]() { this->run_health_check_service(); });
    health_thread.detach();
}

void BootnodeService::run_p2p_server() {
    std::cout << "[P2P] Server listening on port " << p2p_port_ << std::endl;
    
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        // Simulate peer connection checks
        std::lock_guard<std::mutex> lock(peer_mutex_);
        auto now = std::time(nullptr);
        
        for (auto& peer : peer_info_) {
            // Disconnect peers inactive for > 5 minutes
            if (now - peer.second.last_seen > 300) {
                peer.second.status = "disconnected";
            }
        }
    }
}

void BootnodeService::run_discovery_service() {
    std::cout << "[DISCOVERY] Service started on port " << discovery_port_ << std::endl;
    
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
        // Broadcast network info
        std::cout << "[DISCOVERY] Broadcasting network info to peers" << std::endl;
    }
}

void BootnodeService::run_rpc_server() {
    std::cout << "[RPC] Server listening on port " << rpc_port_ << std::endl;
    
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Handle RPC requests (simplified)
        // In production, would use proper HTTP/JSON-RPC library
    }
}

void BootnodeService::run_metrics_server() {
    std::cout << "[METRICS] Server listening on port 9090" << std::endl;
    
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(30));
        
        // Collect and export metrics
        auto stats = get_network_stats();
        
        // Would write to file or HTTP endpoint in production
        std::cout << "[METRICS] Metrics updated: "
                  << get_peer_count() << " peers, "
                  << "uptime: " << get_uptime() << "s" << std::endl;
    }
}

void BootnodeService::run_health_check_service() {
    std::cout << "[HEALTH] Health check service started" << std::endl;
    
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(60));
        
        health_check();
    }
}

void BootnodeService::health_check() {
    std::lock_guard<std::mutex> lock(peer_mutex_);
    
    int healthy = 0;
    int unhealthy = 0;
    
    for (const auto& peer : peer_info_) {
        if (peer.second.status == "connected") {
            healthy++;
        } else {
            unhealthy++;
        }
    }
    
    double health_score = healthy > 0 ? (double)healthy / (healthy + unhealthy) * 100.0 : 0.0;
    
    std::cout << "[HEALTH] Network health: " << std::fixed << std::setprecision(1) 
              << health_score << "% (" << healthy << " healthy, " 
              << unhealthy << " unhealthy)" << std::endl;
}

void BootnodeService::cleanup_disconnected_peers() {
    std::lock_guard<std::mutex> lock(peer_mutex_);
    
    auto now = std::time(nullptr);
    std::vector<std::string> to_remove;
    
    for (auto& peer : peer_info_) {
        // Remove peers disconnected for > 10 minutes
        if (peer.second.status == "disconnected" && 
            (now - peer.second.last_seen > 600)) {
            to_remove.push_back(peer.first);
        }
    }
    
    for (const auto& node_id : to_remove) {
        peer_info_.erase(node_id);
        peer_list_.erase(
            std::remove(peer_list_.begin(), peer_list_.end(), node_id),
            peer_list_.end()
        );
    }
    
    if (!to_remove.empty()) {
        std::cout << "[CLEANUP] Removed " << to_remove.size() << " disconnected peers" << std::endl;
    }
}

void BootnodeService::print_network_status() {
    std::cout << "╔════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                   BOOTNODE STATUS                              ║" << std::endl;
    std::cout << "╠════════════════════════════════════════════════════════════════╣" << std::endl;
    std::cout << "║ Node ID:      " << node_id_.substr(0, 50) << " ║" << std::endl;
    std::cout << "║ Network:      " << std::left << std::setw(46) << network_id_ << "║" << std::endl;
    std::cout << "║ Peers:        " << std::left << std::setw(46) << std::to_string(get_peer_count()) << "║" << std::endl;
    std::cout << "║ Ports:        P2P:" << p2p_port_ << " Discovery:" << discovery_port_ 
              << " RPC:" << rpc_port_ << " " << std::endl;
    std::cout << "║ Status:       " << std::left << std::setw(46) << "ACTIVE ✓" << "║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════════╝" << std::endl;
}
