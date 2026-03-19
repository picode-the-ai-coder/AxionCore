# 🚀 Axion Digitaverse - Quick Start & Installation Guide

**Version 2.0** | Get your node running in 5 minutes

---

## 📥 Installation

### Windows

#### Option 1: Automated Installer (Recommended)

1. Download `AxionDigitaverse-2.0.0-Windows.exe` from https://axiondigitaverse.com
2. Double-click the installer
3. Follow the wizard
4. Click "Launch Axion Digitaverse" when done

**That's it!** Your node is running.

#### Option 2: Manual (ZIP)

1. Download `AxionDigitaverse-2.0.0-Windows.zip`
2. Extract to `C:\Program Files\AxionDigitaverse\`
3. Double-click `AxionDigitaverse.exe` or `axion_node.exe`

---

### macOS

#### Option 1: DMG Installer

1. Download `AxionDigitaverse-2.0.0-Darwin.dmg`
2. Double-click to mount
3. Drag `AxionDigitaverse.app` to `/Applications`
4. Launch from Spotlight (Cmd+Space, type "Axion")

#### Option 2: Homebrew

```bash
brew tap picode-the-ai-coder/axion
brew install axiondigitaverse
axiondigitaverse &
```

#### Option 3: Manual (TAR.GZ)

```bash
tar xzf AxionDigitaverse-2.0.0-Darwin.tar.gz
cd AxionDigitaverse
./AxionDigitaverse.app/Contents/MacOS/AxionDigitaverse
```

---

### Linux

#### Ubuntu/Debian

```bash
# Download
wget https://github.com/picode-the-ai-coder/AxionCore/releases/download/v2.0.0/AxionDigitaverse-2.0.0-Linux.deb

# Install
sudo apt-get install ./AxionDigitaverse-2.0.0-Linux.deb

# Run GUI
axiondigitaverse

# OR run CLI node
axion_node &
```

#### Fedora/RedHat/CentOS

```bash
# Download
wget https://github.com/picode-the-ai-coder/AxionCore/releases/download/v2.0.0/AxionDigitaverse-2.0.0-Linux.rpm

# Install
sudo rpm -i AxionDigitaverse-2.0.0-Linux.rpm

# Run
axiondigitaverse
```

#### Any Linux (TAR.GZ)

```bash
# Download
wget https://github.com/picode-the-ai-coder/AxionCore/releases/download/v2.0.0/AxionDigitaverse-2.0.0-Linux.tar.gz

# Extract
tar xzf AxionDigitaverse-2.0.0-Linux.tar.gz
cd AxionDigitaverse-2.0.0-Linux

# Run
./bin/axiondigitaverse      # GUI
# OR
./bin/axion_node            # CLI
```

---

## 🚀 First Launch

### Desktop GUI

When you launch the GUI for the first time:

```
┌────────────────────────────────────────┐
│   Axion Digitaverse                    │
├────────────────────────────────────────┤
│                                        │
│  [Register] [Login] [Wallet] ...       │
│                                        │
│  Status: Connecting to network...      │
│  Chain Height: Loading...              │
│  Peers: 0/16                           │
│                                        │
└────────────────────────────────────────┘
```

**Step 1: Register**
- Create a new wallet
- System generates Ed25519 keypair
- **Save your private key!** (password manager recommended)

**Step 2: Login**
- Paste your public and private keys
- Create session

**Step 3: Wallet**
- View balance
- Send/receive transactions
- View history

**Step 4: Files (AIPFS)**
- Upload files (get CID)
- Retrieve files by CID
- List all uploaded files

**Step 5: Node Status**
- Monitor chain height
- See connected peers
- View mining stats

### Headless CLI Node

For server operators:

```bash
# Default (port 9000)
axion_node

# Custom port
axion_node 9001

# Connect to seed peer
axion_node 9001 seed-1.axiondigitaverse.com:9000

# Output:
# [Node] Started  validator=axc1a2b3c4d5e6f7g8h9i0j1k2l3m4n5o6p7...
# [Network] Listening on 0.0.0.0:9001
# [Network] Connecting to seed-1.axiondigitaverse.com:9000...
# [Sync] Syncing blocks from peers... 0% → 100%
# [Mining] Starting miner...
# [Block] Mined block #1 in 15.2s
```

---

## 🔧 Configuration

### Node Configuration File

Create `axion.conf` in your data directory:

```ini
# Network
listen_port=9000
max_peers=16

# Mining
mining_enabled=true
mining_threads=4

# Storage
data_dir=./axion_data
max_blockchain_size=50GB

# Logging
log_level=INFO
log_file=axion.log
```

### Environment Variables

```bash
# Override data directory
export AXION_DATA_DIR=/mnt/fast-ssd/axion_data

# Run as seed node
export AXION_SEED_NODE=1

# Mining threads
export AXION_MINING_THREADS=8

# Start node
axion_node
```

---

## 📊 System Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| CPU | 2 cores | 4+ cores |
| RAM | 2 GB | 8+ GB |
| Disk | 20 GB | 100+ GB (SSD) |
| Network | 1 Mbps | 10+ Mbps |
| OS | Windows 10, macOS 12, Ubuntu 20.04 | Latest stable |

---

## 🌐 Network Connection

### Port Forwarding (for home users)

If you're behind a NAT/router:

1. Forward external port 9000 → your machine's port 9000
2. Router configuration varies (see your router manual)
3. Test: https://www.canyouseeme.org/ (enter 9000)

### Firewall

Allow inbound TCP port 9000:

**Windows:**
```powershell
netsh advfirewall firewall add rule name="Axion" dir=in action=allow protocol=tcp localport=9000
```

**Linux:**
```bash
sudo ufw allow 9000/tcp
```

**macOS:**
```bash
# System Preferences → Security & Privacy → Firewall → Firewall Options
# Add AxionDigitaverse
```

---

## 🔍 Verification

### Check Node is Running

```bash
# Linux/macOS
netstat -tuln | grep 9000
# Should show: tcp 0 0 0.0.0.0:9000 0.0.0.0:* LISTEN

# Windows
netstat -an | find "9000"
```

### View Node Logs

```bash
# Linux/macOS
tail -f axion.log

# Windows
Get-Content axion.log -Tail 20 -Wait
```

### Connect to Peer

```bash
# Check connected peers in GUI
# Node Status page → "Peers" section

# Or via CLI
curl http://localhost:9001/api/peers
```

---

## ⚠️ Important - Security

### Private Key Safety

🔴 **CRITICAL**: Your private key controls your funds

**DO:**
- ✅ Save in password manager (1Password, Bitwarden, etc.)
- ✅ Keep on encrypted drive
- ✅ Write down recovery phrase
- ✅ Store securely offline

**DON'T:**
- ❌ Share with anyone
- ❌ Post in forums/social media
- ❌ Store in email or cloud without encryption
- ❌ Leave on unencrypted hard drive

### Wallet Backups

Backup your `axion_data/` directory regularly:

```bash
# Linux/macOS
tar czf axion_backup_$(date +%s).tar.gz axion_data/

# Windows (PowerShell)
Compress-Archive -Path axion_data -DestinationPath "axion_backup_$(Get-Date -Format 'yyyyMMddHHmmss').zip"
```

---

## 🔄 Updates

### Checking for Updates

1. Visit https://axiondigitaverse.com
2. Compare your version (Help → About)
3. Download new version if available

### Updating

**Option 1: Clean Install**
1. Backup `axion_data/` directory
2. Uninstall old version
3. Install new version
4. Restore `axion_data/`

**Option 2: In-Place Update**
1. Stop node
2. Download new binary
3. Replace old binary
4. Restart node

---

## 🆘 Troubleshooting

### "Network Unreachable"

**Solution:**
- Check firewall allows port 9000
- Check internet connection
- Check if seed nodes are up: https://status.axiondigitaverse.com

### "Node won't start"

**Solution:**
- Check logs: `axion.log`
- Check port 9000 is free: `netstat -tuln | grep 9000`
- Try different port: `axion_node 9001`

### "Stuck syncing"

**Solution:**
- Check peer count (should be > 0)
- Check network connection
- Restart node: Stop and start again

### "Out of disk space"

**Solution:**
- Blockchain grows ~1 GB per day
- Use different `data_dir` on larger drive
- Or prune old blocks (not recommended)

---

## 📚 Next Steps

- **Join Discord**: https://discord.gg/axiondigitaverse
- **Read Docs**: https://docs.axiondigitaverse.com
- **Join Community**: https://github.com/picode-the-ai-coder/AxionCore/discussions
- **Report Issues**: https://github.com/picode-the-ai-coder/AxionCore/issues

---

## 💬 Support

- **Email**: support@axiondigitaverse.com
- **Discord**: https://discord.gg/axiondigitaverse
- **GitHub Issues**: https://github.com/picode-the-ai-coder/AxionCore/issues
- **FAQ**: https://faq.axiondigitaverse.com

Enjoy your Axion node! 🚀
