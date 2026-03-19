# 🔧 Node Setup & Configuration Guide

**Advanced node configuration for operators**

---

## Configuration Files

### axion.conf

Create `axion.conf` in your data directory:

```ini
[Network]
listen_port=9000
max_peers=16
peer_timeout_ms=5000

[Mining]
mining_enabled=true
mining_threads=4
difficulty_target=2

[Storage]
data_dir=./axion_data
max_blockchain_size=50GB
enable_pruning=false

[Logging]
log_level=INFO
log_file=axion.log
log_max_size_mb=100
```

### Environment Variables

```bash
# Linux/macOS
export AXION_PORT=9000
export AXION_DATA_DIR=/mnt/ssd/axion_data
export AXION_LOG_LEVEL=DEBUG
axion_node

# Windows PowerShell
$env:AXION_PORT=9000
$env:AXION_DATA_DIR=D:\axion_data
$env:AXION_LOG_LEVEL=DEBUG
.\axion_node.exe
```

---

## Advanced Configuration

### Mining Configuration

```ini
[Mining]
# Number of CPU threads for mining
mining_threads=8

# Only mine if connected to at least N peers
min_peers_for_mining=1

# Stop mining if behind on chain
max_sync_lag_blocks=10

# Miner fee percentage (0.0 - 1.0)
miner_fee_percent=0.05
```

### Network Tuning

```ini
[Network]
# Connection parameters
connection_timeout_ms=5000
read_timeout_ms=30000
write_timeout_ms=30000

# Peer management
peer_ban_duration_sec=3600
max_inbound_connections=20
max_outbound_connections=10

# Message limits
max_message_size_bytes=33554432  # 32MB
```

### Database Optimization

```ini
[Storage]
# Block cache
cache_size_mb=512

# Index settings
enable_block_index=true
enable_address_index=true

# Pruning (not recommended)
enable_pruning=false
prune_height=100000
```

---

## Performance Tuning

### For Low-End Systems (1GB RAM)

```ini
[Mining]
mining_threads=1

[Storage]
cache_size_mb=64

[Network]
max_peers=4
```

### For High-End Systems (8GB+ RAM)

```ini
[Mining]
mining_threads=8

[Storage]
cache_size_mb=1024

[Network]
max_peers=32  # (limited to 16 by code)
```

---

## Monitoring & Health Checks

### Check Node Status

```bash
# Check if running
pgrep axion_node

# View logs
tail -f axion.log

# Check network connections
netstat -an | grep 9000

# Monitor resources
top -p $(pgrep axion_node)
```

### Health Check Endpoint

```bash
# If HTTP metrics enabled
curl http://localhost:9001/api/stats
```

**Response:**
```json
{
  "chain_height": 12345,
  "peer_count": 8,
  "mempool_size": 256,
  "is_mining": true,
  "difficulty": 3,
  "circulating_supply": 500000000
}
```

---

## Troubleshooting

### Node Won't Start

```bash
# Check logs
cat axion.log | head -50

# Verify port is free
netstat -an | grep 9000

# Try different port
axion_node 9001
```

### Stuck on Sync

```bash
# Check peer count
netstat -an | grep -c 9000

# Check chain height
grep "Block" axion.log | tail -5

# Restart if stuck > 1 hour
systemctl restart axion
```

### High Memory Usage

```bash
# Reduce cache
cache_size_mb=128

# Reduce peers
max_peers=4

# Prune old blocks (careful!)
enable_pruning=true
prune_height=50000
```

### Network Issues

```bash
# Test DNS seeds
nslookup seed-1.axiondigitaverse.com

# Test connectivity
ping seed-1.axiondigitaverse.com

# Check firewall
sudo ufw status
```

---

## Security Best Practices

### Firewall Configuration

```bash
# Linux UFW
sudo ufw allow 9000/tcp comment "Axion node"
sudo ufw enable

# macOS
# System Preferences → Security & Privacy → Firewall → Firewall Options

# Windows Firewall
New-NetFirewallRule -DisplayName "Axion" -Direction Inbound -Action Allow -Protocol TCP -LocalPort 9000
```

### Key Management

```bash
# Secure key backup
tar czf - axion_data/ | gpg --symmetric | dd of=backup.tar.gz.gpg

# Restore from backup
gpg -d backup.tar.gz.gpg | tar xz
```

### Regular Maintenance

```bash
# Weekly: Check logs
grep ERROR axion.log

# Monthly: Update dependencies
git pull
cmake .. -G Ninja
cmake --build .

# Quarterly: Full backup
tar czf axion_backup_$(date +%Y%m%d).tar.gz axion_data/
```

---

## Running Multiple Nodes

### Node #1 - Port 9000

```bash
axion_node 9000 &
```

### Node #2 - Port 9001 (Connected to Node #1)

```bash
axion_node 9001 127.0.0.1:9000 &
```

### Node #3 - Port 9002 (External seed)

```bash
axion_node 9002 seed-1.axiondigitaverse.com:9000 &
```

---

## Systemd Service Management

### Create Service File

```ini
# /etc/systemd/system/axion.service

[Unit]
Description=Axion Digitaverse Node
After=network.target

[Service]
Type=simple
User=axion
WorkingDirectory=/home/axion/axion
ExecStart=/home/axion/axion/build/bin/axion_node 9000
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

### Manage Service

```bash
# Enable on startup
sudo systemctl enable axion

# Start/stop/restart
sudo systemctl start axion
sudo systemctl stop axion
sudo systemctl restart axion

# View status
sudo systemctl status axion
sudo journalctl -u axion -f
```

---

## Docker Support (Future)

When Docker support is added:

```dockerfile
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y build-essential cmake ninja-build git libgl1-mesa-dev
WORKDIR /app
COPY . .
RUN mkdir build && cd build && cmake .. -G Ninja && cmake --build . --config Release
EXPOSE 9000
CMD ["./build/bin/axion_node", "9000"]
```

```bash
docker build -t axiondigitaverse:latest .
docker run -p 9000:9000 axiondigitaverse:latest
```

---

For more help, see:
- **Deployment Guide**: DEPLOYMENT.md
- **Quick Start**: QUICKSTART.md
- **Architecture**: ARCHITECTURE_MAP.md
