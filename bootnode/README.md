# 🚀 AXION BOOTNODE - QUICK SETUP GUIDE

## Build and Run the Bootnode

### Prerequisites
- C++17 compiler (GCC 7+, Clang 5+, or MSVC)
- CMake 3.15 or higher
- Linux/macOS/Windows

### Step 1: Prepare Files
Files created:
```
bootnode/
├── bootnode_service.hpp        - Service interface
├── bootnode_service.cpp        - Service implementation
├── main.cpp                    - Main application
├── CMakeLists.txt              - Build configuration
├── bootnode.conf               - Configuration file
└── README.md                   - This file
```

### Step 2: Build the Bootnode

**On Linux/macOS:**
```bash
cd bootnode
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

**On Windows (with Visual Studio):**
```bash
cd bootnode
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

**Output:**
- Linux/macOS: `bin/axion-bootnode`
- Windows: `bin/axion-bootnode.exe`

### Step 3: Configure the Bootnode

Edit `bootnode.conf`:
```bash
# Set your external IP (check with: curl ifconfig.me)
"external_ip": "YOUR.IP.ADDRESS.HERE"

# Set your domain (optional but recommended)
"primary": {
  "hostname": "bootnode.yourdomain.com",
  "ip": "YOUR.IP.ADDRESS.HERE"
}
```

### Step 4: Run the Bootnode

**Test Mode (with demo peers):**
```bash
./axion-bootnode --test-mode
```

**Production Mode:**
```bash
./axion-bootnode
```

**Custom Configuration:**
```bash
./axion-bootnode --p2p-port 30303 --max-peers 500 --test-mode
```

### Step 5: Interact with the Bootnode

Once running, use these commands:

```
Commands:
  peer_count          - Show number of connected peers
  list_peers          - List all connected peers
  network_stats       - Show network statistics (JSON)
  node_info           - Show bootnode information
  uptime              - Show bootnode uptime
  add_peer <ip>       - Add a peer manually
  help                - Show all commands
  quit                - Stop bootnode
```

Example session:
```
> peer_count
[INFO] Connected peers: 4

> node_info
[INFO] Node Information:
  Node ID: enode://axion_bootnode_...
  Version: 1.0.0
  Network: axion-mainnet

> add_peer 192.168.1.100
[PEER] ✓ Added peer: enode://peer... (192.168.1.100:30303)

> list_peers
[PEERS] Total: 5
...

> quit
```

---

## 🔧 Configuration

### Network Settings
```json
"network": {
  "p2p_port": 30303,           // P2P communication port
  "discovery_port": 30304,     // Peer discovery UDP port
  "rpc_port": 8545,            // JSON-RPC HTTP port
  "ws_port": 8546,             // WebSocket port
  "metrics_port": 9090         // Prometheus metrics port
}
```

### Node Settings
```json
"node": {
  "max_peers": 1000,           // Maximum peer connections
  "min_peers": 10,             // Minimum peers for network health
  "sync_mode": "fast",         // Sync mode: fast, full, light
  "pruning": false             // Database pruning
}
```

### Discovery Settings
```json
"discovery": {
  "enabled": true,             // Enable peer discovery
  "bootstrap_nodes": [...],    // Initial bootstrap nodes
  "discovery_interval": 30     // Discovery frequency (seconds)
}
```

---

## 🌐 Network Setup

### Port Forwarding

Forward these ports on your router:

| Port | Protocol | Purpose |
|------|----------|---------|
| 30303 | TCP/UDP | P2P node communication |
| 30304 | UDP | Peer discovery |
| 8545 | TCP | JSON-RPC (optional, for external access) |
| 8546 | TCP | WebSocket (optional, for external access) |

### Firewall Rules (UFW - Ubuntu/Debian)
```bash
sudo ufw allow 30303/tcp
sudo ufw allow 30303/udp
sudo ufw allow 30304/udp
sudo ufw allow 8545/tcp
```

### Static IP Setup

**Linux (Netplan):**
```bash
sudo nano /etc/netplan/00-installer-config.yaml
```

Add:
```yaml
network:
  ethernets:
    eth0:
      dhcp4: no
      addresses: [192.168.1.100/24]
      gateway4: 192.168.1.1
      nameservers:
        addresses: [8.8.8.8, 8.8.4.4]
  version: 2
```

Then:
```bash
sudo netplan apply
```

---

## ⚙️ System Service (Linux)

### Create Service File
```bash
sudo nano /etc/systemd/system/axion-bootnode.service
```

Add:
```ini
[Unit]
Description=Axion Network Bootnode
After=network.target

[Service]
Type=simple
User=axion
WorkingDirectory=/home/axion/axion-bootnode
ExecStart=/home/axion/axion-bootnode/bin/axion-bootnode
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
```

### Enable and Start
```bash
sudo systemctl daemon-reload
sudo systemctl enable axion-bootnode
sudo systemctl start axion-bootnode
sudo systemctl status axion-bootnode
```

### View Logs
```bash
sudo journalctl -u axion-bootnode -f
```

---

## 🪟 Windows Service Setup

### Option 1: Use NSSM (Non-Sucking Service Manager)

```bash
# Download NSSM from nssm.cc

nssm install AxionBootnode "C:\axion-bootnode\bin\axion-bootnode.exe"
nssm set AxionBootnode AppDirectory "C:\axion-bootnode"
nssm start AxionBootnode
nssm status AxionBootnode
```

### Option 2: Built-in PowerShell (Elevated)

```powershell
# Create service
New-Service -Name "AxionBootnode" `
  -BinaryPathName "C:\axion-bootnode\bin\axion-bootnode.exe" `
  -DisplayName "Axion Network Bootnode" `
  -StartupType Automatic

# Start service
Start-Service -Name "AxionBootnode"

# Check status
Get-Service -Name "AxionBootnode"
```

---

## 📊 Monitoring

### Check Peer Count
```bash
./bin/axion-bootnode &
sleep 2
echo "peer_count" | nc localhost 8545
```

### View Network Stats
```bash
echo "network_stats" | socat - EXEC:"./bin/axion-bootnode"
```

### Monitor with Prometheus

Bootnode exposes metrics on port 9090:
```bash
curl http://localhost:9090/metrics
```

---

## 🚨 Troubleshooting

### Issue: Port Already in Use
```bash
# Find process using port 30303
sudo lsof -i :30303

# Kill process
sudo kill -9 <PID>

# Or use different port
./bin/axion-bootnode --p2p-port 30305
```

### Issue: Permission Denied
```bash
# Make executable
chmod +x bin/axion-bootnode

# Or run with sudo
sudo ./bin/axion-bootnode
```

### Issue: Cannot Connect to Peers
```bash
# Check firewall
sudo ufw status

# Check port forwarding on router
# Check external IP: curl ifconfig.me

# Update bootnode.conf with correct IP
```

### Issue: High Memory Usage
```bash
# Reduce cache size in bootnode.conf
"cache_size": 1024

# Enable compression
"compress": true

# Rebuild and restart
```

---

## 📈 Performance Tuning

### For Low-End Hardware (1GB RAM)
```json
{
  "cache_size": 512,
  "max_peers": 100,
  "collection_interval": 60
}
```

### For Medium Hardware (4GB RAM)
```json
{
  "cache_size": 2048,
  "max_peers": 500,
  "collection_interval": 30
}
```

### For High-End Hardware (16GB+ RAM)
```json
{
  "cache_size": 4096,
  "max_peers": 2000,
  "collection_interval": 15
}
```

---

## 🔐 Security

### Enable Firewall
```bash
sudo ufw enable
sudo ufw default deny incoming
sudo ufw allow 22/tcp
sudo ufw allow 30303/tcp
sudo ufw allow 30304/udp
```

### Create Dedicated User
```bash
sudo useradd -m -s /bin/bash axion
sudo chown -R axion:axion /home/axion/axion-bootnode
```

### Restrict RPC Port
```bash
# Only allow local RPC
sudo ufw allow from 127.0.0.1 to any port 8545
```

### Enable Rate Limiting
```json
"rate_limiting": {
  "enabled": true,
  "requests_per_minute": 1000
}
```

---

## 📚 Next Steps

1. **Configure networking** - Set up port forwarding
2. **Run bootnode** - Start the service
3. **Monitor peers** - Watch peer connections grow
4. **Scale up** - Add more bootnodes as needed

---

## 📞 Support

For issues:
1. Check `logs/bootnode.log`
2. Review `bootnode.conf` configuration
3. Verify network connectivity
4. Check firewall rules
5. See troubleshooting section above

---

## 🎊 Success!

Your bootnode is now:
- ✅ Running on your PC
- ✅ Accepting peer connections
- ✅ Providing network discovery
- ✅ Supporting the Axion network

Congratulations! You're now part of the network infrastructure! 🌐

