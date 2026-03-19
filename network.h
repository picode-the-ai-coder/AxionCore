#pragma once
// ================================================================
//  Axion Digitaverse - network.h  [Phase 2 - Real TCP]
//
//  Replaces the in-process simulation with real TCP networking.
//  Uses POSIX sockets + std::thread. No Boost required.
//
//  Each connected peer gets its own recv + send thread.
//  NetworkManager dispatches to chain handlers.
// ================================================================

#include "peer.h"
#include "types.h"
#include "adl.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <chrono>
#include <algorithm>

namespace axion {

inline std::pair<std::string,uint16_t> parse_addr(const std::string& addr) {
    auto colon = addr.rfind(':');
    if (colon == std::string::npos) return {"127.0.0.1", 9000};
    return { addr.substr(0, colon), (uint16_t)std::stoi(addr.substr(colon + 1)) };
}

using TxHandler    = std::function<void(const Transaction&, const std::string&)>;
using BlockHandler = std::function<void(const Block&,       const std::string&)>;
using SyncHandler  = std::function<std::vector<Block>(uint64_t, uint32_t)>;

class NetworkManager {
public:
    static constexpr int MAX_PEERS = 16;
    static constexpr int MIN_PEERS = 3;

    explicit NetworkManager(const std::string& listen_addr,
                            const std::string& node_id,
                            uint64_t chain_height = 0,
                            const Hash256& best_hash = ZERO_HASH)
        : listen_addr_(listen_addr), node_id_(node_id)
        , chain_height_(chain_height), best_hash_(best_hash)
        , running_(false) {}

    ~NetworkManager() { stop(); }
    NetworkManager(const NetworkManager&)            = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    bool start() {
        auto [host, port] = parse_addr(listen_addr_);
        if (!server_.listen(host, port)) {
            std::cerr << "[Net] Failed to bind " << listen_addr_ << "\n";
            return false;
        }
        running_ = true;
        accept_thread_ = std::thread(&NetworkManager::accept_loop, this);
        manage_thread_ = std::thread(&NetworkManager::manage_loop, this);
        std::cout << "[Net] Listening on " << listen_addr_
                  << "  id=" << node_id_.substr(0,12) << "...\n";
        return true;
    }

    void stop() {
        running_ = false;
        server_.stop();
        if (accept_thread_.joinable()) accept_thread_.join();
        if (manage_thread_.joinable()) manage_thread_.join();
        std::lock_guard<std::mutex> lk(peers_mtx_);
        for (auto& [k,p] : peers_) p->stop();
        peers_.clear();
    }

    bool connect_to(const std::string& addr) {
        if (addr == listen_addr_) return false;
        {
            std::lock_guard<std::mutex> lk(peers_mtx_);
            if ((int)peers_.size() >= MAX_PEERS) return false;
            if (peers_.count(addr)) return true;
        }
        auto [host, port] = parse_addr(addr);
        TcpSocket sock;
        if (!sock.connect(host, port, 5000)) {
            std::cerr << "[Net] Connect failed: " << addr << "\n";
            return false;
        }
        std::cout << "[Net] Connected to " << addr << "\n";
        add_peer(std::move(sock), addr);
        return true;
    }

    void add_seed(const std::string& addr) {
        std::lock_guard<std::mutex> lk(peers_mtx_);
        known_addrs_.insert(addr);
    }

    void on_transaction(TxHandler h)    { tx_handler_    = std::move(h); }
    void on_block(BlockHandler h)       { block_handler_  = std::move(h); }
    void on_sync_request(SyncHandler h) { sync_handler_   = std::move(h); }

    void broadcast_tx(const Transaction& tx) {
        Bytes payload = encode_tx(tx);
        broadcast(WireType::NEW_TX, payload);
        std::lock_guard<std::mutex> lk(peers_mtx_);
        seen_txs_.insert(tx.tx_id);
    }

    void broadcast_block(const Block& block) {
        Bytes payload = encode_block(block);
        broadcast(WireType::NEW_BLOCK, payload);
        std::lock_guard<std::mutex> lk(peers_mtx_);
        seen_blocks_.insert(block.hash);
    }

    void update_chain_state(uint64_t height, const Hash256& hash) {
        chain_height_ = height;
        best_hash_    = hash;
    }

    size_t peer_count() const {
        std::lock_guard<std::mutex> lk(peers_mtx_);
        size_t n = 0;
        for (auto& [k,p] : peers_) if (p->active()) ++n;
        return n;
    }

    std::vector<std::string> active_peer_addrs() const {
        std::lock_guard<std::mutex> lk(peers_mtx_);
        std::vector<std::string> addrs;
        for (auto& [k,p] : peers_)
            if (p->active()) addrs.push_back(p->info().listen_addr);
        return addrs;
    }

    void print_status() const {
        std::cout << "\n-- Network ----------------------------\n";
        std::cout << "  Listen : " << listen_addr_ << "\n";
        std::cout << "  Peers  : " << peer_count() << "\n";
        std::lock_guard<std::mutex> lk(peers_mtx_);
        for (auto& [k,p] : peers_) {
            if (!p->dead())
                std::cout << "  " << p->addr()
                          << "  active=" << p->active()
                          << "  sent="   << p->bytes_sent()   << "B"
                          << "  recv="   << p->bytes_received() << "B\n";
        }
    }

private:
    std::string         listen_addr_, node_id_;
    std::atomic<uint64_t> chain_height_;
    Hash256             best_hash_;
    std::atomic<bool>   running_;
    TcpServer           server_;
    std::thread         accept_thread_, manage_thread_;
    mutable std::mutex  peers_mtx_;
    std::unordered_map<std::string, std::unique_ptr<Peer>> peers_;
    std::unordered_set<std::string> known_addrs_, seen_txs_, seen_blocks_;
    TxHandler    tx_handler_;
    BlockHandler block_handler_;
    SyncHandler  sync_handler_;

    void accept_loop() {
        while (running_) {
            TcpSocket client = server_.accept(500);
            if (!client.valid()) continue;
            std::cout << "[Net] Inbound connection\n";
            add_peer(std::move(client), "inbound:" + std::to_string(now_ms()));
        }
    }

    void manage_loop() {
        int tick = 0;
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            ++tick;
            {
                std::lock_guard<std::mutex> lk(peers_mtx_);
                for (auto it = peers_.begin(); it != peers_.end(); ) {
                    if (it->second->dead()) it = peers_.erase(it);
                    else ++it;
                }
            }
            // Reconnect to known peers if below minimum
            if ((int)peer_count() < MIN_PEERS) {
                std::vector<std::string> todo;
                {
                    std::lock_guard<std::mutex> lk(peers_mtx_);
                    for (const auto& addr : known_addrs_)
                        if (!peers_.count(addr) && addr != listen_addr_)
                            todo.push_back(addr);
                }
                for (const auto& a : todo) {
                    connect_to(a);
                    if ((int)peer_count() >= MIN_PEERS) break;
                }
            }
            // Peer exchange every 30s
            if (tick % 6 == 0) {
                auto addrs = active_peer_addrs();
                addrs.push_back(listen_addr_);
                broadcast(WireType::PEERS, encode_peers(addrs));
            }
            // Ping every 30s
            if (tick % 6 == 0) broadcast(WireType::PING, {});
        }
    }

    void add_peer(TcpSocket sock, const std::string& key) {
        auto mcb = [this](const std::string& pid, WireType t, const Bytes& b){
            handle(pid, t, b);
        };
        auto dcb = [this](const std::string& pid){
            std::cout << "[Net] Peer lost: " << pid << "\n";
        };
        auto peer = std::make_unique<Peer>(std::move(sock), key,
                                          std::move(mcb), std::move(dcb));
        // Send HELLO
        HelloMsg hello{ "1.0.0", node_id_, listen_addr_,
                         chain_height_.load(), best_hash_ };
        peer->send(WireType::HELLO, encode_hello(hello));
        std::lock_guard<std::mutex> lk(peers_mtx_);
        peers_[key] = std::move(peer);
    }

    void handle(const std::string& peer_id, WireType type, const Bytes& payload) {
        const uint8_t* p   = payload.data();
        const uint8_t* end = p + payload.size();
        switch (type) {

        case WireType::HELLO: {
            HelloMsg h = decode_hello(p, end);
            std::cout << "[Net] HELLO from " << h.listen_addr
                      << " height=" << h.chain_height << "\n";
            PeerInfo pi{ h.node_id, h.listen_addr, h.chain_height, h.best_hash };
            {
                std::lock_guard<std::mutex> lk(peers_mtx_);
                for (auto& [k, peer] : peers_) {
                    if (!peer->dead() && peer->state() != PeerState::ACTIVE) {
                        peer->set_info(pi);
                        known_addrs_.insert(h.listen_addr);
                        // If they have more chain, request sync
                        if (h.chain_height > chain_height_.load()) {
                            peer->send(WireType::GET_BLOCKS,
                                encode_get_blocks(chain_height_.load() + 1));
                        }
                        break;
                    }
                }
            }
            break;
        }

        case WireType::PING:
            for_peer(peer_id, [](Peer& p){ p.send(WireType::PONG, {}); });
            break;

        case WireType::PONG: break;

        case WireType::GET_PEERS: {
            auto addrs = active_peer_addrs();
            addrs.push_back(listen_addr_);
            for_peer(peer_id, [&](Peer& p){
                p.send(WireType::PEERS, encode_peers(addrs));
            });
            break;
        }

        case WireType::PEERS: {
            auto addrs = decode_peers(p, end);
            std::lock_guard<std::mutex> lk(peers_mtx_);
            for (const auto& a : addrs)
                if (a != listen_addr_) known_addrs_.insert(a);
            break;
        }

        case WireType::NEW_TX: {
            Transaction tx = decode_tx(p, end);
            bool already = false;
            { std::lock_guard<std::mutex> lk(peers_mtx_);
              already = !seen_txs_.insert(tx.tx_id).second; }
            if (already) return;
            if (tx_handler_) tx_handler_(tx, peer_id);
            broadcast_except(WireType::NEW_TX, payload, peer_id);
            break;
        }

        case WireType::NEW_BLOCK:
        case WireType::BLOCKS: {
            Block block = decode_block(p, end);
            bool already = false;
            { std::lock_guard<std::mutex> lk(peers_mtx_);
              already = !seen_blocks_.insert(block.hash).second; }
            if (already) return;
            if (block_handler_) block_handler_(block, peer_id);
            if (type == WireType::NEW_BLOCK)
                broadcast_except(WireType::NEW_BLOCK, payload, peer_id);
            break;
        }

        case WireType::GET_BLOCKS: {
            if (!sync_handler_ || p + 12 > end) break;
            uint64_t from   = read_u64(p); p += 8;
            uint32_t maxcnt = read_u32(p);
            auto blocks = sync_handler_(from, std::min(maxcnt, 500u));
            for_peer(peer_id, [&](Peer& peer) {
                for (const auto& blk : blocks)
                    peer.send(WireType::BLOCKS, encode_block(blk));
            });
            break;
        }

        default: break;
        }
    }

    void broadcast(WireType type, const Bytes& payload) {
        std::lock_guard<std::mutex> lk(peers_mtx_);
        for (auto& [k,p] : peers_) if (p->active()) p->send(type, payload);
    }

    void broadcast_except(WireType type, const Bytes& payload,
                          const std::string& exc) {
        std::lock_guard<std::mutex> lk(peers_mtx_);
        for (auto& [k,p] : peers_)
            if (p->active() && p->info().node_id != exc)
                p->send(type, payload);
    }

    void for_peer(const std::string& id,
                  std::function<void(Peer&)> fn) {
        std::lock_guard<std::mutex> lk(peers_mtx_);
        for (auto& [k,p] : peers_)
            if (p->info().node_id == id || k == id) { fn(*p); return; }
    }
};

} // namespace axion
