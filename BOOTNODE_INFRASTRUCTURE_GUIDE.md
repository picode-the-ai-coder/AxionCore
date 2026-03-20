# 🌐 AXION IDE AS BOOTNODE - COMPLETE INFRASTRUCTURE GUIDE

## Running Your PC as an Axion Network Bootnode

---

## 📋 Overview

Instead of just being a development tool, Axion IDE can run as a **bootnode** - a critical network node that:
- Discovers and tracks other nodes
- Helps new nodes join the network
- Routes transactions
- Validates smart contracts
- Maintains network state
- Provides RPC endpoints

---

## 🏗️ What You Need to Do

### Phase 1: Network Configuration

#### 1.1 Port Configuration
```
Bootnode Ports:
├─ P2P Port: 30303 (node-to-node communication)
├─ Discovery Port: 30304 (UDP for peer discovery)
├─ RPC Port: 8545 (external API calls)
├─ WS Port: 8546 (WebSocket connections)
├─ Metrics Port: 9090 (Prometheus metrics)
└─ Admin Port: 8551 (admin interface)
```

#### 1.2 Router Configuration
```
Forward ports:
├─ External port 30303 → PC port 30303 (TCP/UDP)
├─ External port 30304 → PC port 30304 (UDP)
├─ External port 8545 → PC port 8545 (TCP)
├─ External port 8546 → PC port 8546 (TCP)
└─ Optional: Configure IPv6
```

#### 1.3 Static IP Setup
```
Windows:
1. Settings → Network & Internet → Advanced network settings
2. Configure static IPv4 address (e.g., 192.168.1.100)
3. Static DNS (e.g., 8.8.8.8, 8.8.4.4)

Linux/Mac:
sudo nano /etc/network/interfaces
# or use netplan for modern Linux
```

### Phase 2: Bootnode Implementation

#### 2.1 Create Bootnode Service
**File**: `bootnode_service.cpp`

```cpp
#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <queue>
#include <ctime>

class BootnodeService {
private:
    std::string node_id_;
    std::string version_ = "1.0.0";
    std::vector<std::string> peer_list_;
    std::map<std::string, PeerInfo> peer_info_;
    std::mutex peer_mutex_;
    
    struct PeerInfo {
        std::string node_id;
        std::string ip_address;
        int port;
        std::string status;  // connected, disconnected, pending
        uint64_t last_seen;
        uint64_t joined_time;
    };

public:
    BootnodeService() {
        generate_node_id();
        start_services();
    }
    
    void generate_node_id() {
        // Generate unique node ID
        node_id_ = "bootnode_" + std::to_string(time(nullptr));
    }
    
    void start_services() {
        std::thread p2p_thread(&BootnodeService::run_p2p_server, this);
        std::thread discovery_thread(&BootnodeService::run_discovery_service, this);
        std::thread rpc_thread(&BootnodeService::run_rpc_server, this);
        std::thread metrics_thread(&BootnodeService::run_metrics_server, this);
        
        p2p_thread.detach();
        discovery_thread.detach();
        rpc_thread.detach();
        metrics_thread.detach();
        
        std::cout << "[BOOTNODE] All services started" << std::endl;
    }
    
    void run_p2p_server() {
        // Listen on port 30303
        // Accept incoming peer connections
        std::cout << "[P2P] Listening on port 30303" << std::endl;
        
        while (true) {
            // Accept new connections
            // Verify peer info
            // Add to peer list
            // Maintain connection pool
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    void run_discovery_service() {
        // Listen on port 30304 (UDP)
        // Respond to peer discovery requests
        // Share peer list with new nodes
        std::cout << "[DISCOVERY] Discovery service started on port 30304" << std::endl;
        
        while (true) {
            // Receive discovery requests
            // Send known peers
            // Update peer status
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    void run_rpc_server() {
        // Listen on port 8545 (HTTP)
        // Listen on port 8546 (WebSocket)
        std::cout << "[RPC] RPC server listening on ports 8545/8546" << std::endl;
        
        while (true) {
            // Handle JSON-RPC requests
            // eth_getNodeInfo
            // eth_peerCount
            // eth_getNetworkInfo
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    void run_metrics_server() {
        // Listen on port 9090
        // Expose Prometheus metrics
        std::cout << "[METRICS] Metrics server listening on port 9090" << std::endl;
        
        while (true) {
            // Collect metrics
            // Export Prometheus format
            
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }
    
    // Add peer to network
    void add_peer(const std::string& node_id, const std::string& ip, int port) {
        std::lock_guard<std::mutex> lock(peer_mutex_);
        
        PeerInfo info;
        info.node_id = node_id;
        info.ip_address = ip;
        info.port = port;
        info.status = "connected";
        info.last_seen = time(nullptr);
        info.joined_time = time(nullptr);
        
        peer_info_[node_id] = info;
        peer_list_.push_back(node_id);
        
        std::cout << "[PEER] Added peer: " << node_id << " (" << ip << ":" << port << ")" << std::endl;
    }
    
    // Remove disconnected peer
    void remove_peer(const std::string& node_id) {
        std::lock_guard<std::mutex> lock(peer_mutex_);
        
        peer_info_.erase(node_id);
        peer_list_.erase(
            std::remove(peer_list_.begin(), peer_list_.end(), node_id),
            peer_list_.end()
        );
        
        std::cout << "[PEER] Removed peer: " << node_id << std::endl;
    }
    
    // Get network stats
    std::string get_network_stats() {
        std::lock_guard<std::mutex> lock(peer_mutex_);
        
        std::string stats;
        stats += "Bootnode: " + node_id_ + "\n";
        stats += "Version: " + version_ + "\n";
        stats += "Connected Peers: " + std::to_string(peer_list_.size()) + "\n";
        stats += "Network Status: Active\n";
        stats += "Uptime: [will be tracked]\n";
        
        return stats;
    }
};
```

#### 2.2 Create Bootnode Configuration
**File**: `bootnode.conf`

```json
{
  "bootnode": {
    "id": "axion-bootnode-1",
    "version": "1.0.0",
    "network": {
      "p2p_port": 30303,
      "discovery_port": 30304,
      "rpc_port": 8545,
      "ws_port": 8546,
      "metrics_port": 9090,
      "admin_port": 8551
    },
    "node": {
      "max_peers": 1000,
      "sync_mode": "fast",
      "pruning": false
    },
    "discovery": {
      "enabled": true,
      "method": "dht",
      "bootstrap_nodes": [
        "enode://node1@ip1:30303",
        "enode://node2@ip2:30303"
      ]
    },
    "database": {
      "path": "./data/chaindata",
      "cache_size": 2048
    },
    "logging": {
      "level": "info",
      "file": "./logs/bootnode.log",
      "max_size": "100MB"
    },
    "metrics": {
      "enabled": true,
      "prometheus_port": 9090,
      "interval": 15
    }
  }
}
```

### Phase 3: System Integration

#### 3.1 Create Windows Service
**File**: `scripts/install_service.ps1`

```powershell
# Run as Administrator

$ServiceName = "AxionBootnode"
$DisplayName = "Axion Network Bootnode"
$ExePath = "C:\AxionCore\build\AxionBootnode.exe"
$Description = "Axion blockchain network bootnode service"

# Create service
New-Service -Name $ServiceName `
            -DisplayName $DisplayName `
            -BinaryPathName $ExePath `
            -StartupType Automatic `
            -Description $Description

# Set recovery actions
sc.exe failure $ServiceName reset= 300 actions= restart/5000/restart/5000/restart/10000

# Start service
Start-Service -Name $ServiceName

Write-Host "Service installed and started"
Get-Service $ServiceName
```

#### 3.2 Create Linux Service
**File**: `/etc/systemd/system/axion-bootnode.service`

```ini
[Unit]
Description=Axion Network Bootnode
After=network.target

[Service]
Type=simple
User=axion
WorkingDirectory=/home/axion/axion-ide
ExecStart=/home/axion/axion-ide/build/axion-bootnode
Restart=on-failure
RestartSec=10

# Resource limits
CPUQuota=80%
MemoryLimit=4G
LimitNOFILE=65536

[Install]
WantedBy=multi-user.target
```

Enable and start:
```bash
sudo systemctl enable axion-bootnode
sudo systemctl start axion-bootnode
sudo systemctl status axion-bootnode
```

### Phase 4: Network Discovery

#### 4.1 Peer Discovery Protocol
```cpp
class PeerDiscovery {
private:
    struct DiscoveryRequest {
        std::string request_id;
        std::string node_id;
        int port;
        std::vector<std::string> capabilities;
    };
    
    struct DiscoveryResponse {
        std::string bootnode_id;
        std::vector<PeerInfo> known_peers;
        std::string network_id;
        uint64_t timestamp;
    };

public:
    DiscoveryResponse handle_discovery_request(const DiscoveryRequest& req) {
        DiscoveryResponse response;
        response.bootnode_id = bootnode_->get_node_id();
        response.known_peers = get_suitable_peers(req.capabilities);
        response.network_id = "axion-mainnet";
        response.timestamp = time(nullptr);
        
        return response;
    }
    
    std::vector<PeerInfo> get_suitable_peers(const std::vector<std::string>& capabilities) {
        // Return peers with matching capabilities
        // Prefer healthy, responsive peers
        // Limit to 20 peers per request
        
        std::vector<PeerInfo> result;
        // ... implementation
        return result;
    }
};
```

#### 4.2 Node Announcement
```
Protocol: UDP broadcast
Port: 30304
Message format:
{
  "type": "peer_announce",
  "node_id": "enode://...",
  "ip": "192.168.1.100",
  "port": 30303,
  "capabilities": ["eth/68", "eth/69"],
  "network_id": "1",
  "timestamp": 1234567890,
  "signature": "0x..."
}
```

### Phase 5: Monitoring & Management

#### 5.1 Create Monitoring Dashboard
**File**: `monitoring/dashboard.html`

```html
<!DOCTYPE html>
<html>
<head>
    <title>Axion Bootnode Dashboard</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        body { font-family: Arial; background: #0D0C1E; color: #E8E6FF; }
        .container { max-width: 1200px; margin: 20px auto; }
        .header { text-align: center; margin-bottom: 30px; }
        .stats { display: grid; grid-template-columns: repeat(4, 1fr); gap: 20px; margin-bottom: 30px; }
        .stat-card { background: #1a1930; padding: 20px; border-radius: 8px; border-left: 4px solid #5B3FD4; }
        .stat-value { font-size: 24px; font-weight: bold; color: #1DC8A0; }
        .stat-label { font-size: 12px; color: #999; margin-top: 5px; }
        .chart-container { background: #1a1930; padding: 20px; border-radius: 8px; margin-bottom: 20px; }
        .peer-list { background: #1a1930; padding: 20px; border-radius: 8px; }
        table { width: 100%; border-collapse: collapse; }
        th { text-align: left; padding: 10px; border-bottom: 1px solid #333; color: #5B3FD4; }
        td { padding: 10px; border-bottom: 1px solid #222; }
        .status-online { color: #1DC8A0; }
        .status-offline { color: #FF6B6B; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🌐 Axion Bootnode Dashboard</h1>
            <p>Real-time network monitoring</p>
        </div>
        
        <div class="stats">
            <div class="stat-card">
                <div class="stat-value" id="peer-count">0</div>
                <div class="stat-label">Connected Peers</div>
            </div>
            <div class="stat-card">
                <div class="stat-value" id="transactions">0</div>
                <div class="stat-label">Transactions/sec</div>
            </div>
            <div class="stat-card">
                <div class="stat-value" id="uptime">0h</div>
                <div class="stat-label">Uptime</div>
            </div>
            <div class="stat-card">
                <div class="stat-value" id="network-health">✓</div>
                <div class="stat-label">Network Health</div>
            </div>
        </div>
        
        <div class="chart-container">
            <canvas id="peersChart"></canvas>
        </div>
        
        <div class="peer-list">
            <h2>Connected Peers</h2>
            <table>
                <thead>
                    <tr>
                        <th>Node ID</th>
                        <th>IP Address</th>
                        <th>Port</th>
                        <th>Status</th>
                        <th>Connected Since</th>
                        <th>Latency</th>
                    </tr>
                </thead>
                <tbody id="peers-table">
                </tbody>
            </table>
        </div>
    </div>
    
    <script>
        // Fetch stats from bootnode API
        async function updateDashboard() {
            const response = await fetch('http://localhost:8545/bootnode/stats');
            const data = await response.json();
            
            // Update stats
            document.getElementById('peer-count').textContent = data.peer_count;
            document.getElementById('transactions').textContent = data.tx_per_sec.toFixed(2);
            document.getElementById('uptime').textContent = formatUptime(data.uptime);
            document.getElementById('network-health').textContent = data.health === 'healthy' ? '✓' : '✗';
            
            // Update peer table
            const tbody = document.getElementById('peers-table');
            tbody.innerHTML = '';
            data.peers.forEach(peer => {
                const row = tbody.insertRow();
                row.innerHTML = `
                    <td>${peer.node_id.substring(0, 10)}...</td>
                    <td>${peer.ip_address}</td>
                    <td>${peer.port}</td>
                    <td><span class="status-${peer.status}">${peer.status}</span></td>
                    <td>${new Date(peer.joined_time * 1000).toLocaleString()}</td>
                    <td>${peer.latency}ms</td>
                `;
            });
        }
        
        function formatUptime(seconds) {
            const hours = Math.floor(seconds / 3600);
            const minutes = Math.floor((seconds % 3600) / 60);
            return `${hours}h ${minutes}m`;
        }
        
        // Update every 5 seconds
        updateDashboard();
        setInterval(updateDashboard, 5000);
    </script>
</body>
</html>
```

### Phase 6: Load Balancing

#### 6.1 Multiple Bootnode Setup
If running multiple bootnodes:

```
Primary Bootnode (Your PC):
├─ Port 30303: Node A
└─ Port 30303: Node B (different IP)

Secondary Bootnodes (Cloud):
├─ AWS Instance
├─ Azure Instance
└─ DigitalOcean Droplet

Load Balancer:
├─ DNS round-robin
├─ GeoDNS routing
└─ Health checks
```

#### 6.2 Failover Configuration
```json
{
  "failover": {
    "primary": {
      "hostname": "bootnode1.axion.io",
      "ip": "YOUR_PC_IP",
      "port": 30303
    },
    "secondary": [
      {
        "hostname": "bootnode2.axion.io",
        "ip": "AWS_IP",
        "port": 30303
      },
      {
        "hostname": "bootnode3.axion.io",
        "ip": "AZURE_IP",
        "port": 30303
      }
    ],
    "health_check_interval": 30,
    "failover_threshold": 3
  }
}
```

---

## 🚀 Setup Instructions

### Step 1: Prepare Your PC
```bash
# Install dependencies
sudo apt install -y build-essential cmake git

# Clone repository
git clone https://github.com/axion/axion-ide.git
cd axion-ide

# Build with bootnode support
mkdir build && cd build
cmake -DENABLE_BOOTNODE=ON ..
cmake --build .
```

### Step 2: Configure Network
```bash
# Check your external IP
curl ifconfig.me

# Configure port forwarding in router
# Forward 30303, 30304, 8545, 8546 to your PC

# Set static IP on your PC
# Note: Port 8545 (RPC) is optional for public access
```

### Step 3: Start Bootnode
```bash
# Linux/Mac
./build/axion-bootnode --config bootnode.conf

# Windows (elevated)
.\build\AxionBootnode.exe --config bootnode.conf
```

### Step 4: Verify Setup
```bash
# Check if bootnode is running
curl http://localhost:8545 -X POST -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"eth_nodeInfo","params":[],"id":1}'

# Check peer count
curl http://localhost:8545 -X POST -H "Content-Type: application/json" \
  -d '{"jsonrpc":"2.0","method":"eth_peerCount","params":[],"id":1}'
```

### Step 5: Access Dashboard
```
Open: http://localhost:3000/dashboard
Or: http://your-pc-ip:3000/dashboard (from other devices)
```

---

## 📊 RPC Methods Available

```json
PEER MANAGEMENT:
{
  "eth_peerCount": "Returns number of connected peers",
  "eth_peers": "List all connected peers",
  "eth_addPeer": "Manually add a peer",
  "eth_removePeer": "Remove a peer",
  "eth_networkInfo": "Get network information"
}

NODE INFO:
{
  "eth_nodeInfo": "Get bootnode information",
  "eth_clientVersion": "Get version",
  "eth_net_version": "Get network ID",
  "eth_syncing": "Get sync status"
}

METRICS:
{
  "eth_metrics": "Get performance metrics",
  "eth_gasPrice": "Get current gas price",
  "eth_blockNumber": "Get current block"
}
```

---

## 💻 Performance Considerations

### Resource Usage
```
Network Bootnode (your PC):
├─ CPU: 2-4 cores typical
├─ RAM: 4-8GB typical
├─ Bandwidth: 5-20 Mbps typical
├─ Storage: 50-500GB (grows with blockchain)
└─ Uptime: 24/7 preferred

vs.

Development IDE:
├─ CPU: 1-2 cores
├─ RAM: 2-4GB
├─ Bandwidth: Low
└─ Storage: 1-5GB
```

### Optimization Tips
```
1. Enable caching
   └─ Cache peer info for 1 hour

2. Compress data
   └─ GZIP responses > 1KB

3. Connection pooling
   └─ Reuse TCP connections

4. Rate limiting
   └─ Limit requests per peer

5. Database optimization
   └─ Index peer lookups
   └─ Archive old data
```

---

## 🔐 Security Considerations

### Firewall Rules
```bash
# Allow necessary ports only
sudo ufw allow 30303/tcp
sudo ufw allow 30304/udp
sudo ufw allow 8545/tcp (optional)
sudo ufw allow 22/tcp (SSH only)

# Deny everything else
sudo ufw default deny incoming
```

### Authentication
```json
{
  "auth": {
    "peers": {
      "method": "signature",
      "verify": true,
      "whitelist": ["bootnode_a", "bootnode_b"]
    },
    "rpc": {
      "method": "api_key",
      "api_keys": ["key_1", "key_2"]
    }
  }
}
```

### Rate Limiting
```cpp
struct RateLimiter {
    std::map<std::string, int> peer_requests;
    int max_requests_per_min = 1000;
    
    bool allow_request(const std::string& peer_id) {
        if (peer_requests[peer_id] > max_requests_per_min) {
            return false;  // Rate limited
        }
        peer_requests[peer_id]++;
        return true;
    }
};
```

---

## 📈 Scaling Strategy

### Phase 1: Single Bootnode (Your PC)
- 100-500 peers
- 1-5 MB/s bandwidth
- Covers local region

### Phase 2: Multiple Bootnodes
- 5,000+ peers
- Load balanced
- Covers multiple regions

### Phase 3: Global Infrastructure
- 50,000+ peers
- CDN integration
- Enterprise setup

---

## 🎯 Key Benefits

✅ **Decentralization**
- Your PC becomes part of the network
- Helps new nodes join
- Reduces single point of failure

✅ **Network Health**
- Provides peer discovery
- Maintains network topology
- Improves connectivity

✅ **Staking Rewards** (if applicable)
- Run bootnode → earn rewards
- Incentivizes participation
- Community contribution

✅ **Control**
- Full visibility into network
- Real-time monitoring
- Custom policies

---

## ⚠️ Challenges

❌ **Continuous Uptime Required**
- Solution: UPS power supply
- Solution: Automated restart
- Solution: Cloud failover

❌ **Bandwidth Usage**
- Solution: Monitor closely
- Solution: Implement throttling
- Solution: Plan for growth

❌ **System Requirements**
- Solution: Upgrade hardware as needed
- Solution: Optimize software
- Solution: Use cloud nodes for failover

---

## 🚀 Next Steps

### TODAY:
1. Read this guide
2. Check hardware specs
3. Plan network setup

### THIS WEEK:
1. Configure port forwarding
2. Build bootnode binary
3. Test locally
4. Secure firewall

### NEXT WEEK:
1. Start bootnode service
2. Register domain
3. Monitor peer connections
4. Setup automated backups

### ONGOING:
1. Monitor performance
2. Update software
3. Maintain uptime
4. Optimize network

---

## 📞 Support & Monitoring

### Logging
```bash
# View logs
tail -f logs/bootnode.log

# Search for errors
grep ERROR logs/bootnode.log

# Monitor in real-time
watch tail logs/bootnode.log
```

### Health Checks
```bash
# Check service status
systemctl status axion-bootnode

# Check network connectivity
netstat -tln | grep 30303

# Check peer count
curl -s http://localhost:8545 -X POST \
  -d '{"jsonrpc":"2.0","method":"eth_peerCount"}' | jq .
```

### Alerts
```json
{
  "alerts": {
    "peer_count_low": {
      "threshold": 10,
      "action": "notify"
    },
    "cpu_high": {
      "threshold": 80,
      "action": "throttle"
    },
    "memory_high": {
      "threshold": 90,
      "action": "cleanup"
    },
    "bandwidth_high": {
      "threshold": 500,
      "action": "limit"
    }
  }
}
```

---

## 🎊 Success Indicators

✓ Bootnode is running
✓ Peers are connecting
✓ RPC is responding
✓ Dashboard is accessible
✓ Network is healthy
✓ Uptime is stable
✓ Performance is good

---

**Your PC is now a critical piece of the Axion network infrastructure!** 🌐

This transforms your development environment into a production network component that helps the entire ecosystem grow and function smoothly.
