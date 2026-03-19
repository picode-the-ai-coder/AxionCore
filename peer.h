#pragma once
// ================================================================
//  Axion Digitaverse - peer.h
//  A single live peer connection.
//  Each Peer has:
//    - A TcpSocket (the actual TCP connection)
//    - A dedicated receive thread (reads messages, dispatches)
//    - A send queue (thread-safe outbound messages)
//    - State: CONNECTING -> HANDSHAKED -> ACTIVE -> DEAD
// ================================================================

#include "tcp.h"
#include "wire.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <queue>
#include <condition_variable>
#include <chrono>

namespace axion {

enum class PeerState { CONNECTING, HANDSHAKED, ACTIVE, DEAD };

struct PeerInfo {
    std::string node_id;
    std::string listen_addr;  // "host:port" for reconnects
    uint64_t    chain_height = 0;
    Hash256     best_hash;
};

// Callback type: (peer_id, msg_type, payload_bytes)
using MsgCallback = std::function<void(const std::string&, WireType, const Bytes&)>;
using DeadCallback = std::function<void(const std::string&)>;

// ── Peer ──────────────────────────────────────────────────
class Peer {
public:
    Peer(TcpSocket sock, const std::string& addr,
         MsgCallback on_msg, DeadCallback on_dead)
        : sock_(std::move(sock))
        , remote_addr_(addr)
        , state_(PeerState::CONNECTING)
        , on_msg_(std::move(on_msg))
        , on_dead_(std::move(on_dead))
        , running_(true)
    {
        recv_thread_ = std::thread(&Peer::recv_loop, this);
        send_thread_ = std::thread(&Peer::send_loop, this);
    }

    ~Peer() { stop(); }

    Peer(const Peer&) = delete;
    Peer& operator=(const Peer&) = delete;

    // Enqueue an outbound message (non-blocking)
    void send(WireType type, const Bytes& payload) {
        if (state_ == PeerState::DEAD) return;
        Bytes framed = frame(type, payload);
        {
            std::lock_guard<std::mutex> lk(send_mtx_);
            send_queue_.push(std::move(framed));
        }
        send_cv_.notify_one();
    }

    void stop() {
        running_ = false;
        sock_.close();
        send_cv_.notify_all();
        if (recv_thread_.joinable()) recv_thread_.join();
        if (send_thread_.joinable()) send_thread_.join();
        state_ = PeerState::DEAD;
    }

    PeerState state()  const { return state_.load(); }
    bool      active() const { return state_ == PeerState::ACTIVE; }
    bool      dead()   const { return state_ == PeerState::DEAD; }

    const PeerInfo&    info()   const { return info_; }
    const std::string& addr()   const { return remote_addr_; }

    void set_info(const PeerInfo& pi) {
        info_  = pi;
        state_ = PeerState::ACTIVE;
    }

    // Bytes sent/received for diagnostics
    uint64_t bytes_sent()     const { return bytes_sent_; }
    uint64_t bytes_received() const { return bytes_recv_; }

private:
    TcpSocket               sock_;
    std::string             remote_addr_;
    std::atomic<PeerState>  state_;
    MsgCallback             on_msg_;
    DeadCallback            on_dead_;
    PeerInfo                info_;
    std::atomic<bool>       running_;
    std::thread             recv_thread_;
    std::thread             send_thread_;
    std::mutex              send_mtx_;
    std::condition_variable send_cv_;
    std::queue<Bytes>       send_queue_;
    std::atomic<uint64_t>   bytes_sent_{0};
    std::atomic<uint64_t>   bytes_recv_{0};

    // ── Receive loop (dedicated thread) ───────────────────
    void recv_loop() {
        while (running_) {
            Bytes payload;
            // Read the 9-byte header: magic(4) + type(1) + len(4)
            uint8_t hdr[9];
            if (!recv_exact(hdr, 9)) break;

            uint32_t magic = read_u32(hdr);
            if (magic != WIRE_MAGIC) break;  // protocol error

            WireType type = static_cast<WireType>(hdr[4]);
            uint32_t plen = read_u32(hdr + 5);
            if (plen > 64 * 1024 * 1024) break;  // too large

            payload.resize(plen);
            if (plen > 0 && !recv_exact(payload.data(), plen)) break;

            bytes_recv_ += 9 + plen;

            // Dispatch
            if (on_msg_) {
                std::string peer_id = info_.node_id.empty()
                                      ? remote_addr_ : info_.node_id;
                on_msg_(peer_id, type, payload);
            }
        }
        // Connection lost
        state_ = PeerState::DEAD;
        running_ = false;
        send_cv_.notify_all();
        std::string pid = info_.node_id.empty() ? remote_addr_ : info_.node_id;
        if (on_dead_) on_dead_(pid);
    }

    // ── Send loop (dedicated thread) ──────────────────────
    void send_loop() {
        while (running_) {
            std::unique_lock<std::mutex> lk(send_mtx_);
            send_cv_.wait(lk, [this]{
                return !send_queue_.empty() || !running_;
            });
            while (!send_queue_.empty()) {
                Bytes msg = std::move(send_queue_.front());
                send_queue_.pop();
                lk.unlock();
                if (!sock_.send_all(msg)) {
                    running_ = false;
                    return;
                }
                bytes_sent_ += msg.size();
                lk.lock();
            }
        }
    }

    bool recv_exact(uint8_t* buf, size_t n) {
        size_t got = 0;
        while (got < n && running_) {
#ifdef _WIN32
            WSAPOLLFD pfd{};
            pfd.fd     = sock_.fd();
            pfd.events = POLLRDNORM;
            if (WSAPoll(&pfd, 1, 2000) <= 0) continue; // 2s timeout, retry
#else
            struct pollfd pfd{};
            pfd.fd     = (int)sock_.fd();
            pfd.events = POLLIN;
            if (poll(&pfd, 1, 2000) <= 0) continue; // 2s timeout, retry
#endif
            int r = ::recv(sock_.fd(),
                reinterpret_cast<char*>(buf + got),
                (int)(n - got), 0);
            if (r <= 0) return false;
            got += (size_t)r;
        }
        return got == n;
    }
};

} // namespace axion
