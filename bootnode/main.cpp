#include "bootnode_service.hpp"
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <sstream>

std::unique_ptr<BootnodeService> g_bootnode = nullptr;

void signal_handler(int signal) {
    std::cout << "\n[SHUTDOWN] Received signal " << signal << std::endl;
    
    if (g_bootnode) {
        std::cout << "[SHUTDOWN] Stopping bootnode gracefully..." << std::endl;
        g_bootnode->stop();
    }
    
    std::cout << "[SHUTDOWN] Bootnode stopped" << std::endl;
    exit(0);
}

void print_banner() {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                                   ║\n";
    std::cout << "║            🌐 AXION NETWORK BOOTNODE v1.0.0 🌐                  ║\n";
    std::cout << "║                                                                   ║\n";
    std::cout << "║        Professional blockchain network infrastructure             ║\n";
    std::cout << "║        Peer discovery | Network routing | Node management         ║\n";
    std::cout << "║                                                                   ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
}

void print_help() {
    std::cout << "\nUsage: axion-bootnode [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help              Show this help message\n";
    std::cout << "  --version           Show version information\n";
    std::cout << "  --p2p-port <port>   Set P2P port (default: 30303)\n";
    std::cout << "  --discovery <port>  Set discovery port (default: 30304)\n";
    std::cout << "  --rpc-port <port>   Set RPC port (default: 8545)\n";
    std::cout << "  --max-peers <n>     Set maximum peers (default: 1000)\n";
    std::cout << "  --test-mode         Run in test mode with demo peers\n";
    std::cout << "\n";
}

void demo_mode() {
    std::cout << "[DEMO] Starting bootnode in demonstration mode...\n\n";
    
    // Add some demo peers
    g_bootnode->add_peer(
        "enode://peer1@192.168.1.101:30303",
        "192.168.1.101",
        30303,
        {"eth/68", "eth/69"}
    );
    
    g_bootnode->add_peer(
        "enode://peer2@192.168.1.102:30303",
        "192.168.1.102",
        30303,
        {"eth/68", "eth/69"}
    );
    
    g_bootnode->add_peer(
        "enode://peer3@192.168.1.103:30303",
        "192.168.1.103",
        30303,
        {"eth/68", "eth/69"}
    );
    
    g_bootnode->add_peer(
        "enode://peer4@192.168.1.104:30303",
        "192.168.1.104",
        30303,
        {"eth/68", "eth/69"}
    );
    
    std::cout << "[DEMO] Added 4 demo peers\n";
    std::cout << "[DEMO] Run commands:\n";
    std::cout << "       peer_count          - Show number of connected peers\n";
    std::cout << "       network_stats       - Show network statistics\n";
    std::cout << "       add_peer <ip>       - Add a peer\n";
    std::cout << "       list_peers          - List all peers\n";
    std::cout << "       help                - Show all commands\n";
    std::cout << "       quit                - Stop bootnode\n";
    std::cout << "\n";
}

void process_command(const std::string& command) {
    if (command == "peer_count") {
        std::cout << "[INFO] Connected peers: " << g_bootnode->get_peer_count() << "\n";
    }
    else if (command == "network_stats") {
        std::cout << g_bootnode->get_network_stats();
    }
    else if (command == "list_peers") {
        auto peers = g_bootnode->get_all_peers();
        std::cout << "\n[PEERS] Total: " << peers.size() << "\n";
        std::cout << "┌" << std::string(70, '─') << "┐\n";
        
        int count = 0;
        for (const auto& peer : peers) {
            std::cout << "│ " << (count + 1) << ". " 
                      << peer.node_id.substr(0, 20) << "... ("
                      << peer.ip_address << ":" << peer.port << ") | "
                      << peer.status << "\n";
            count++;
            if (count >= 10) {
                std::cout << "│ ... and " << (peers.size() - 10) << " more peers\n";
                break;
            }
        }
        
        std::cout << "└" << std::string(70, '─') << "┘\n";
    }
    else if (command == "node_info") {
        std::cout << "\n[INFO] Node Information:\n";
        std::cout << "  Node ID: " << g_bootnode->get_node_id() << "\n";
        std::cout << "  Version: " << g_bootnode->get_version() << "\n";
        std::cout << "  Network: " << g_bootnode->get_network_id() << "\n";
    }
    else if (command == "uptime") {
        uint64_t uptime = g_bootnode->get_uptime();
        int hours = uptime / 3600;
        int minutes = (uptime % 3600) / 60;
        int seconds = uptime % 60;
        
        std::cout << "[INFO] Uptime: " << hours << "h " << minutes << "m " << seconds << "s\n";
    }
    else if (command.substr(0, 8) == "add_peer") {
        std::istringstream iss(command);
        std::string cmd, ip;
        iss >> cmd >> ip;
        
        if (!ip.empty()) {
            std::string node_id = "enode://peer_" + ip + "@" + ip + ":30303";
            g_bootnode->add_peer(node_id, ip, 30303);
        } else {
            std::cout << "[ERROR] Usage: add_peer <ip>\n";
        }
    }
    else if (command == "help") {
        std::cout << "\n[HELP] Available commands:\n";
        std::cout << "  peer_count          - Show number of connected peers\n";
        std::cout << "  list_peers          - List all connected peers\n";
        std::cout << "  network_stats       - Show network statistics (JSON)\n";
        std::cout << "  node_info           - Show bootnode information\n";
        std::cout << "  uptime              - Show bootnode uptime\n";
        std::cout << "  add_peer <ip>       - Add a peer (e.g., add_peer 192.168.1.100)\n";
        std::cout << "  help                - Show this help message\n";
        std::cout << "  quit/exit           - Stop bootnode gracefully\n";
        std::cout << "\n";
    }
    else if (command == "quit" || command == "exit") {
        std::cout << "[SHUTDOWN] Stopping bootnode...\n";
        g_bootnode->stop();
        exit(0);
    }
    else if (!command.empty()) {
        std::cout << "[ERROR] Unknown command: '" << command << "'\n";
        std::cout << "[INFO] Type 'help' for available commands\n";
    }
}

int main(int argc, char* argv[]) {
    print_banner();
    
    // Parse command line arguments
    int p2p_port = 30303;
    int discovery_port = 30304;
    int rpc_port = 8545;
    int max_peers = 1000;
    bool test_mode = false;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            print_help();
            return 0;
        }
        else if (arg == "--version" || arg == "-v") {
            std::cout << "Axion Bootnode v1.0.0\n";
            return 0;
        }
        else if (arg == "--p2p-port" && i + 1 < argc) {
            p2p_port = std::stoi(argv[++i]);
        }
        else if (arg == "--discovery" && i + 1 < argc) {
            discovery_port = std::stoi(argv[++i]);
        }
        else if (arg == "--rpc-port" && i + 1 < argc) {
            rpc_port = std::stoi(argv[++i]);
        }
        else if (arg == "--max-peers" && i + 1 < argc) {
            max_peers = std::stoi(argv[++i]);
        }
        else if (arg == "--test-mode") {
            test_mode = true;
        }
    }
    
    // Initialize bootnode
    g_bootnode = std::make_unique<BootnodeService>();
    
    // Configure
    g_bootnode->configure(max_peers, p2p_port, discovery_port, rpc_port);
    
    // Start
    g_bootnode->start();
    
    // Register signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    // Demo mode
    if (test_mode) {
        demo_mode();
    }
    
    // Interactive command loop
    std::cout << "\n[READY] Bootnode ready for commands (type 'help' for commands)\n\n";
    
    std::string command;
    while (std::getline(std::cin, command)) {
        if (!command.empty()) {
            process_command(command);
        }
        std::cout << "> ";
    }
    
    return 0;
}
