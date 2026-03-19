#pragma once
// ================================================================
//  Axion Digitaverse - tcp.h
//  Real TCP socket primitives.
//  POSIX sockets (Linux/macOS). On Windows: Winsock2 (same API).
//
//  Provides:
//    TcpSocket  - single connected socket (send/recv)
//    TcpServer  - listens, accepts new connections
// ================================================================

#include "types.h"
#include <string>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <cerrno>

#ifdef _WIN32
  // winsock2.h is already included in platform.h  
  #pragma comment(lib,"ws2_32.lib")
  typedef SOCKET sock_t;
  #define INVALID_SOCK INVALID_SOCKET
  #define SOCK_ERR     SOCKET_ERROR
  #define close_sock   closesocket
  static void sock_init() {
      static bool done = false;
      if (!done) { WSADATA w; WSAStartup(MAKEWORD(2,2),&w); done=true; }
  }
#else
  #include <sys/socket.h>
  #include <sys/types.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <netdb.h>
  #include <poll.h>
  typedef int sock_t;
  #define INVALID_SOCK (-1)
  #define SOCK_ERR     (-1)
  #define close_sock   ::close
  static void sock_init() {}
#endif

namespace axion {

// ── TcpSocket: one bidirectional connection ────────────────
class TcpSocket {
public:
    TcpSocket() : fd_(INVALID_SOCK) { sock_init(); }

    explicit TcpSocket(sock_t fd) : fd_(fd) { sock_init(); }

    ~TcpSocket() { close(); }

    // Non-copyable, movable
    TcpSocket(const TcpSocket&)            = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;
    TcpSocket(TcpSocket&& o) noexcept : fd_(o.fd_) { o.fd_ = INVALID_SOCK; }
    TcpSocket& operator=(TcpSocket&& o) noexcept {
        close(); fd_ = o.fd_; o.fd_ = INVALID_SOCK; return *this;
    }

    bool valid() const { return fd_ != INVALID_SOCK; }

    // Connect to host:port (blocking, with timeout_ms)
    bool connect(const std::string& host, uint16_t port,
                 int timeout_ms = 5000) {
        close();
        fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd_ == INVALID_SOCK) return false;

        // Set non-blocking for connect-with-timeout
        set_nonblocking(true);

        struct sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port);

        // Resolve hostname
        struct addrinfo hints{}, *res = nullptr;
        hints.ai_family   = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0 || !res)
            { close(); return false; }
        addr.sin_addr = reinterpret_cast<struct sockaddr_in*>(res->ai_addr)->sin_addr;
        freeaddrinfo(res);

        int r = ::connect(fd_, reinterpret_cast<struct sockaddr*>(&addr),
                          sizeof(addr));
        if (r == 0) { set_nonblocking(false); return true; }

#ifdef _WIN32
        if (WSAGetLastError() != WSAEWOULDBLOCK) { close(); return false; }
#else
        if (errno != EINPROGRESS) { close(); return false; }
#endif

        // Wait for connection via poll/select
#ifdef _WIN32
        WSAPOLLFD pfd{};
        pfd.fd     = fd_;
        pfd.events = POLLWRNORM;
        int pr = WSAPoll(&pfd, 1, timeout_ms);
#else
        struct pollfd pfd{};
        pfd.fd     = (int)fd_;
        pfd.events = POLLOUT;
        int pr = poll(&pfd, 1, timeout_ms);
#endif
        if (pr <= 0) { close(); return false; }

        // Check socket error
        int err = 0;
        socklen_t elen = sizeof(err);
        getsockopt(fd_, SOL_SOCKET, SO_ERROR,
                   reinterpret_cast<char*>(&err), &elen);
        if (err != 0) { close(); return false; }

        set_nonblocking(false);
        set_tcp_nodelay();
        return true;
    }

    // Send all bytes (blocking)
    bool send_all(const Bytes& data) {
        size_t sent = 0;
        while (sent < data.size()) {
            int n = ::send(fd_,
                reinterpret_cast<const char*>(data.data() + sent),
                (int)(data.size() - sent), 0);
            if (n <= 0) return false;
            sent += (size_t)n;
        }
        return true;
    }

    // Send a length-prefixed message (4-byte big-endian length + payload)
    bool send_msg(const Bytes& payload) {
        uint32_t len = htonl(static_cast<uint32_t>(payload.size()));
        Bytes frame(4 + payload.size());
        std::memcpy(frame.data(), &len, 4);
        std::memcpy(frame.data() + 4, payload.data(), payload.size());
        return send_all(frame);
    }

    // Receive a length-prefixed message (blocks until full message arrives)
    bool recv_msg(Bytes& payload, int timeout_ms = 30000) {
        // Read 4-byte length header
        uint8_t hdr[4];
        if (!recv_exact(hdr, 4, timeout_ms)) return false;
        uint32_t len;
        std::memcpy(&len, hdr, 4);
        len = ntohl(len);
        if (len == 0 || len > 64 * 1024 * 1024) return false; // sanity: max 64MB

        payload.resize(len);
        return recv_exact(payload.data(), len, timeout_ms);
    }

    void close() {
        if (fd_ != INVALID_SOCK) {
            close_sock(fd_);
            fd_ = INVALID_SOCK;
        }
    }

    sock_t fd() const { return fd_; }

private:
    sock_t fd_;

    void set_nonblocking(bool nb) {
#ifdef _WIN32
        u_long mode = nb ? 1 : 0;
        ioctlsocket(fd_, FIONBIO, &mode);
#else
        int flags = fcntl(fd_, F_GETFL, 0);
        if (nb) fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
        else    fcntl(fd_, F_SETFL, flags & ~O_NONBLOCK);
#endif
    }

    void set_tcp_nodelay() {
        int flag = 1;
        setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY,
                   reinterpret_cast<char*>(&flag), sizeof(flag));
    }

    bool recv_exact(uint8_t* buf, size_t n, int timeout_ms) {
        size_t got = 0;
        while (got < n) {
            // Wait for data with timeout
#ifdef _WIN32
            WSAPOLLFD pfd{};
            pfd.fd     = fd_;
            pfd.events = POLLRDNORM;
            int pr = WSAPoll(&pfd, 1, timeout_ms);
#else
            struct pollfd pfd{};
            pfd.fd     = (int)fd_;
            pfd.events = POLLIN;
            int pr = poll(&pfd, 1, timeout_ms);
#endif
            if (pr <= 0) return false;  // timeout or error

            int r = ::recv(fd_,
                reinterpret_cast<char*>(buf + got),
                (int)(n - got), 0);
            if (r <= 0) return false;
            got += (size_t)r;
        }
        return true;
    }
};

// ── TcpServer: listens for incoming connections ────────────
class TcpServer {
public:
    TcpServer() : fd_(INVALID_SOCK) { sock_init(); }
    ~TcpServer() { stop(); }

    TcpServer(const TcpServer&)            = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    // Bind and listen on host:port
    bool listen(const std::string& host, uint16_t port,
                int backlog = 64) {
        stop();
        fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd_ == INVALID_SOCK) return false;

        // Reuse address
        int yes = 1;
        setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<char*>(&yes), sizeof(yes));

        struct sockaddr_in addr{};
        addr.sin_family      = AF_INET;
        addr.sin_port        = htons(port);
        if (host == "0.0.0.0" || host.empty()) {
            addr.sin_addr.s_addr = INADDR_ANY;
        } else {
#ifdef _WIN32
            struct sockaddr_in sa{};
            int len = sizeof(sa);
            if (InetPtonA(AF_INET, host.c_str(), &addr.sin_addr.s_addr) != 1) {
                return false;
            }
#else
            inet_pton(AF_INET, host.c_str(), &addr.sin_addr.s_addr);
#endif
        }

        if (::bind(fd_, reinterpret_cast<struct sockaddr*>(&addr),
                   sizeof(addr)) == SOCK_ERR)
            { stop(); return false; }

        if (::listen(fd_, backlog) == SOCK_ERR)
            { stop(); return false; }

        port_ = port;
        host_ = host;
        return true;
    }

    // Accept one connection (blocks until one arrives or timeout_ms)
    // Returns valid TcpSocket on success, invalid on failure/timeout
    TcpSocket accept(int timeout_ms = 1000) {
#ifdef _WIN32
        WSAPOLLFD pfd{};
        pfd.fd     = fd_;
        pfd.events = POLLRDNORM;
        if (WSAPoll(&pfd, 1, timeout_ms) <= 0) return TcpSocket{};
#else
        struct pollfd pfd{};
        pfd.fd     = (int)fd_;
        pfd.events = POLLIN;
        if (poll(&pfd, 1, timeout_ms) <= 0) return TcpSocket{};
#endif

        struct sockaddr_in peer{};
        socklen_t plen = sizeof(peer);
        sock_t client = ::accept(fd_,
            reinterpret_cast<struct sockaddr*>(&peer), &plen);
        if (client == INVALID_SOCK) return TcpSocket{};

        // Disable Nagle on accepted socket
        int flag = 1;
        setsockopt(client, IPPROTO_TCP, TCP_NODELAY,
                   reinterpret_cast<char*>(&flag), sizeof(flag));
        return TcpSocket{client};
    }

    bool valid() const { return fd_ != INVALID_SOCK; }
    uint16_t port() const { return port_; }

    void stop() {
        if (fd_ != INVALID_SOCK) { close_sock(fd_); fd_ = INVALID_SOCK; }
    }

private:
    sock_t      fd_;
    uint16_t    port_ = 0;
    std::string host_;
};

} // namespace axion
