# 🚀 Axion Digitaverse v2.0.0 Release Notes

**Official Launch - Complete Blockchain Platform**

---

## 🎉 What's New in v2.0.0

### Core Features ✨

- **Real Cryptography**: Ed25519 elliptic curve digital signatures
- **Proof-of-Work Consensus**: Dynamic difficulty adjustment targeting 15-second blocks
- **P2P Networking**: Real TCP mesh with up to 16 peer connections per node
- **Smart Contracts**: Axion Virtual Machine (AVM) with gas metering
- **Distributed Storage**: AIPFS for content-addressed file storage
- **Cross-Platform**: Windows, macOS, and Linux support
- **Beautiful GUI**: ImGui-based desktop application
- **Headless CLI**: Node software for servers and mining

### Phase 2 Upgrades

✅ Real TCP P2P networking (not in-process simulation)
✅ Block sync from peers on startup
✅ Transaction and block propagation
✅ Peer discovery and connection management
✅ Network bootstrap via public seed nodes

---

## 📦 Downloads

### Windows
- **GUI App**: `AxionDigitaverse-2.0.0-Windows.exe` (Installer)
- **CLI Node**: Included in installer, also available standalone

### macOS
- **GUI App**: `AxionDigitaverse-2.0.0-Darwin.dmg` (DMG Image)
- **CLI Node**: Included in DMG

### Linux
- **Ubuntu/Debian**: `AxionDigitaverse-2.0.0-Linux.deb`
- **RedHat/CentOS**: `AxionDigitaverse-2.0.0-Linux.rpm`
- **Universal**: `AxionDigitaverse-2.0.0-Linux.tar.gz`

---

## 🚀 Quick Start

### For Users (GUI)

```bash
# Windows
AxionDigitaverse-2.0.0-Windows.exe
# Follow installer wizard

# macOS
# Double-click .dmg, drag to Applications

# Linux
sudo apt install ./AxionDigitaverse-2.0.0-Linux.deb
axiondigitaverse
```

### For Operators (CLI Node)

```bash
# Default (port 9000)
axion_node

# With custom port
axion_node 9001

# Connect to seed peer
axion_node 9001 seed-1.axiondigitaverse.com:9000
```

---

## 📖 Documentation

- **[QUICKSTART.md](QUICKSTART.md)** - Installation in 5 minutes
- **[DEPLOYMENT.md](DEPLOYMENT.md)** - Operations guide
- **[NODE_SETUP.md](NODE_SETUP.md)** - Node configuration
- **[ARCHITECTURE.md](ARCHITECTURE_MAP.md)** - System design

---

## 🌐 Public Seed Nodes

Bootstrap your node with any of these:

```
seed-1.axiondigitaverse.com:9000    (US East)
seed-2.axiondigitaverse.com:9000    (EU Central)
seed-3.axiondigitaverse.com:9000    (Asia Pacific)
```

---

## 💻 System Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| CPU | 2 cores | 4+ cores |
| RAM | 2 GB | 8+ GB |
| Disk | 20 GB | 100+ GB (SSD) |
| Network | 1 Mbps | 10+ Mbps |
| OS | Windows 10, macOS 12, Ubuntu 20.04 | Latest stable |

---

## 🔐 Security Notes

⚠️ **This is testnet phase software**
- Do NOT use with real money
- Do NOT share your private keys
- Backup your keys in password manager
- Run on trusted networks

---

## 🐛 Known Issues

### None reported yet!

Please report issues: https://github.com/picode-the-ai-coder/AxionCore/issues

---

## 📊 Network Stats

- **Block Time**: ~15 seconds (target)
- **Max Peers**: 16 connections per node
- **TPS**: 100-1000 transactions/second
- **Supply**: 1 billion AXC
- **Halving**: Every 210,000 blocks (~4 years)

---

## 🤝 Community

- **Discord**: https://discord.gg/axiondigitaverse
- **GitHub**: https://github.com/picode-the-ai-coder/AxionCore
- **Twitter**: @AxionDigitaverse
- **Website**: https://axiondigitaverse.com

---

## 📝 Changelog

### v2.0.0 (This Release)

**New Features:**
- Real TCP P2P networking implementation
- Public seed nodes infrastructure
- Distributed network bootstrap
- Block sync from peers
- Transaction propagation
- Peer management and discovery

**Improvements:**
- Cross-platform build system (Windows/macOS/Linux)
- Automated installer creation
- GitHub Actions CI/CD pipeline
- Enhanced documentation
- Website and download portal

**Fixes:**
- APIENTRY macro conflict resolution
- ImGuiCol_ScrollbarGrabHov compatibility
- inet_addr() deprecation warning
- poll() cross-platform compatibility

### v1.0.0

- Initial single-node blockchain
- Ed25519 signature verification
- Persistent storage via KVStore
- Smart contract VM (AVM)
- AIPFS storage system
- Desktop GUI application

---

## 🙏 Credits

Built with ❤️ by the Axion Foundation

Special thanks to:
- Dear ImGui team for excellent UI framework
- GLFW contributors for window management
- Ed25519 researchers and cryptographers
- Community testers and early adopters

---

## 📄 License

MIT License - See LICENSE file for details

---

## 🔄 Upgrade Instructions

### From v1.0.0 to v2.0.0

1. **Backup your data**:
   ```bash
   tar czf axion_backup.tar.gz axion_data/
   ```

2. **Download new version** from releases

3. **Install fresh**:
   ```bash
   # Windows/macOS: Run new installer
   # Linux: apt install new .deb package
   ```

4. **Data migration**:
   - Blockchain data is compatible
   - Wallets and keys are compatible
   - Your balances preserved!

---

## 🚀 Getting Started

1. **Download** from https://axiondigitaverse.com
2. **Install** for your operating system
3. **Run** AxionDigitaverse GUI or axion_node CLI
4. **Wait** for initial sync (5-30 minutes)
5. **Create wallet** or import existing
6. **Start transacting!**

---

## 📞 Support

Having issues?

1. Check **[QUICKSTART.md](QUICKSTART.md)** Troubleshooting section
2. Review **[NODE_SETUP.md](NODE_SETUP.md)** for configuration
3. Search **[GitHub Issues](https://github.com/picode-the-ai-coder/AxionCore/issues)**
4. Ask in **[Discord](https://discord.gg/axiondigitaverse)**
5. Email: support@axiondigitaverse.com

---

## 🎯 Roadmap

**Q2 2026:**
- Mobile wallet (iOS/Android)
- REST API server
- Docker support
- Performance optimizations

**Q3 2026:**
- Proof-of-Stake transition
- Sharding for scalability
- Cross-chain bridges

**Q4 2026:**
- Enhanced smart contracts
- Multi-signature wallets
- Hardware wallet support

---

**Welcome to the Axion Digitaverse!** 🌟

Download now: https://axiondigitaverse.com
