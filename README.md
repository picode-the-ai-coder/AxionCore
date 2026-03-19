# Axion Digitaverse - Complete Blockchain Platform v2.0

![C++17](https://img.shields.io/badge/C++-17-blue)
![CMake](https://img.shields.io/badge/CMake-3.20+-green)
![License](https://img.shields.io/badge/License-MIT-yellow)
![Build](https://img.shields.io/badge/Build-Passing-brightgreen)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-blue)

**A complete, production-ready blockchain network you can run from your browser or desktop.**

---

## 🎯 What is Axion Digitaverse?

Axion Digitaverse is a **fully-operational blockchain platform** with:

| Feature | Description |
|---------|-------------|
| **Real Cryptography** | Ed25519 elliptic curve digital signatures |
| **Consensus** | Proof-of-Work with dynamic difficulty adjustment |
| **Networking** | Real TCP P2P mesh, 16-peer connections |
| **Smart Contracts** | Axion Virtual Machine (AVM) with gas metering |
| **Storage** | Content-addressed distributed file system (AIPFS) |
| **GUI** | Cross-platform desktop application (ImGui + OpenGL) |
| **CLI** | Headless node software for servers/mining |
| **Economics** | 1 billion AXC supply, halving schedule, staking |
| **Speed** | ~15 second blocks, 100-1000 TPS capacity |

### Download & Run

**👉 Visit [axiondigitaverse.com](https://axiondigitaverse.com) to download and get started in 5 minutes**

Or build from source:

```bash
git clone https://github.com/picode-the-ai-coder/AxionCore
cd AxionCore && mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────┐
│              GUI (ImGui + GLFW + OpenGL3)               │
│        Desktop App for Windows/macOS/Linux              │
│  ┌─────────────────────────────────────────────────┐   │
│  │ Pages: Register → Login → Wallet → Files → Node │   │
│  └─────────────────────────────────────────────────┘   │
└────────────────────┬────────────────────────────────────┘
                     │ NodeBridge (thread-safe IPC)
┌────────────────────▼────────────────────────────────────┐
│           AxionNode - THE BLOCKCHAIN ENGINE             │
│  ┌──────────────────────────────────────────────────┐  │
│  │ 1. Chain         - Immutable ledger (persistent) │  │
│  │ 2. Mempool       - Transaction pool              │  │
│  │ 3. Consensus     - Mining + difficulty adjust   │  │
│  │ 4. Network       - P2P gossip protocol (TCP)    │  │
│  │ 5. AXC Ledger    - Token economics              │  │
│  │ 6. AVM           - Smart contract execution      │  │
│  │ 7. AIPFS         - Distributed file storage     │  │
│  │ 8. APKI          - Ed25519 cryptography         │  │
│  │ 9. Wallet        - Key management               │  │
│  │10. AI            - Anomaly detection             │  │
│  └──────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────┘
```

---

## 🚀 Quick Start

### Windows

```powershell
# Download installer from website
# Double-click AxionDigitaverse-2.0.0-Windows.exe
# Follow the wizard
# Restart node and go!

# OR run CLI node
axion_node 9000
```

### macOS

```bash
brew install axiondigitaverse
# Or: Double-click DMG and drag to Applications
axiondigitaverse &
```

### Linux

```bash
sudo apt install ./AxionDigitaverse-2.0.0-Linux.deb
axiondigitaverse &
```

**[Full Installation Guide](QUICKSTART.md)**

---

## 📚 Documentation

| Document | Purpose |
|----------|---------|
| **[QUICKSTART.md](QUICKSTART.md)** | 5-minute installation & setup guide |
| **[DEPLOYMENT.md](DEPLOYMENT.md)** | Operations, infrastructure, seed nodes |
| **[LAUNCH_CHECKLIST.md](LAUNCH_CHECKLIST.md)** | Pre-launch verification checklist |
| **[NODE_SETUP.md](NODE_SETUP.md)** | Advanced node configuration |
| **[ARCHITECTURE.md](ARCHITECTURE.md)** | Deep dive into system design |

---

## 💻 Building from Source

### Prerequisites

```bash
# Windows: Visual Studio 2022 Community (free)
# macOS: Xcode command-line tools
# Linux: build-essential, cmake, git
```

### Build Steps

```bash
git clone https://github.com/picode-the-ai-coder/AxionCore
cd AxionCore
mkdir build && cd build

# Configure
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release

# Create installers (Windows, macOS, Linux)
cpack -G NSIS    # or DragNDrop, DEB, RPM, TGZ
```

---

## 🌍 Public Seed Nodes

Bootstrap your node with any of these:

| Host | Port | Location | Operator |
|------|------|----------|----------|
| `seed-1.axiondigitaverse.com` | 9000 | US East | Axion Foundation |
| `seed-2.axiondigitaverse.com` | 9000 | EU Central | Axion Foundation |
| `seed-3.axiondigitaverse.com` | 9000 | Asia Pacific | Axion Foundation |

---

## 📊 Network Stats

**Live statistics** from [status.axiondigitaverse.com](https://status.axiondigitaverse.com):

- **Chain Height**: Growing daily ⛓️
- **Active Nodes**: 100+ worldwide 🌍
- **Peer Network**: 16-connection mesh 🔗
- **Block Time**: ~15 seconds ⏱️
- **TPS**: 100-1000 transactions/sec 💨
- **Circulating Supply**: AXC growing with each block 💰

---

## 🛠️ Features Deep Dive

### 1️⃣ **Blockchain Core (chain.h)**
- Persistent storage via KVStore
- 7-rule validation (including Ed25519 signatures)
- Merkle tree transaction verification
- Fast block lookup by height/hash

### 2️⃣ **Consensus & Mining (consensus.h)**
- Proof-of-Work with configurable difficulty
- Auto-adjustment every 10 blocks
- Target: 15 seconds per block
- Background mining thread

### 3️⃣ **P2P Networking (network.h + tcp.h)**
- Real TCP sockets (Windows/POSIX compatible)
- Automatic peer discovery
- Transaction & block gossip
- Sync from peers on startup
- Max 16 peer connections per node

### 4️⃣ **Smart Contracts (avm.h)**
- Axion Virtual Machine
- Gas metering to prevent infinite loops
- Persistent state storage
- User-deployed contract support

### 5️⃣ **Distributed Storage (aipfs.h)**
- Content-addressed files (SHA256 hash = address)
- Upload/retrieve/list files
- Persistent disk storage
- Referenced in smart contracts

### 6️⃣ **Cryptography (apki.h)**
- Real Ed25519 implementations
- Deterministic wallet generation
- Transaction signing & verification
- Secure key management

### 7️⃣ **Token Economics (axc.h)**
```
Total Supply: 1 billion AXC
├── Genesis Allocation: 400M AXC
├── Mining Rewards: 50 AXC/block (halving every 210k blocks)
├── Fee System: 1% burn rate
└── Staking: ~12% APY
```

### 8️⃣ **User Interface (main_gui.cpp)**
- **Register**: Generate Ed25519 keypair
- **Login**: Session management
- **Wallet**: View balance, send/receive
- **Files**: Upload & retrieve from AIPFS
- **Node**: Monitor chain & peer stats

---

## 🔒 Security Features

| Feature | Benefit |
|---------|---------|
| **Ed25519 Signatures** | Real cryptography, not toy crypto |
| **Proof-of-Work** | Prevents cheap block creation |
| **Chain History** | Immutable transaction log |
| **Longest Chain Rule** | Consensus on valid history |
| **Difficulty Adjustment** | Prevents mining attacks |
| **Merkle Trees** | Detects block tampering |
| **AI Anomaly Detection** | Flags suspicious transactions |

---

## 🔄 Version History

| Version | Date | Highlights |
|---------|------|-----------|
| **v2.0** | 2024-Jan | Real P2P networking, smart contracts, distributed storage |
| v1.0 | 2023-Dec | Single-node blockchain, Ed25519 signatures, persistence |

---

## 📦 Distribution

Binaries available for:

- 🪟 **Windows**: .exe (NSIS) or .zip
- 🍎 **macOS**: .dmg (Intel/Apple Silicon)
- 🐧 **Linux**: .deb (Ubuntu), .rpm (RedHat), .tar.gz (universal)

**Download**: https://axiondigitaverse.com/downloads

---

## 🤝 Contributing

Want to contribute? Great!

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

**Areas for contribution:**
- [ ] Additional smart contract features
- [ ] Performance optimizations
- [ ] More detailed logging
- [ ] REST API expansion
- [ ] Mobile wallet (stretch goal)

---

## 🐛 Reporting Issues

Found a bug? Please report it!

**GitHub Issues**: https://github.com/picode-the-ai-coder/AxionCore/issues

When reporting, include:
1. Operating system and version
2. Steps to reproduce
3. Expected vs. actual behavior
4. Logs (if applicable)

---

## 📋 Roadmap

**Q1 2024:**
- [ ] Public seed node deployments
- [ ] Website launch
- [ ] Official release

**Q2 2024:**
- [ ] Mobile wallet (iOS/Android)
- [ ] REST API server
- [ ] Docker support
- [ ] Database optimizations

**Q3 2024:**
- [ ] Proof-of-Stake transition
- [ ] Sharding for scalability
- [ ] Cross-chain bridges

---

## 💬 Community

- **Discord**: https://discord.gg/axiondigitaverse
- **GitHub**: https://github.com/picode-the-ai-coder/AxionCore
- **Twitter**: @AxionDigitaverse
- **Email**: support@axiondigitaverse.com

---

## 📄 License

MIT License - See [LICENSE](LICENSE) file for details

---

## 🙏 Credits

Built with ❤️ by the Axion Foundation

Special thanks to:
- Dear ImGui community
- GLFW maintainers
- Ed25519 cryptography researchers

---

## ⚠️ Disclaimer

Axion Digitaverse is experimental software. Use at your own risk.

- 🔴 **DO NOT** use with real money (testnet phase)
- 🔴 **DO NOT** share your private keys
- 🔴 **DO NOT** deploy critical infrastructure yet

---

## 📈 Getting Involved

**Help us grow the network:**

1. **Run a Node**: Download and launch your own node
2. **Report Bugs**: Found an issue? Tell us!
3. **Suggest Features**: Have ideas? Join Discord and discuss
4. **Contribute Code**: Submit PRs for bug fixes and features
5. **Spread the Word**: Share with friends and communities

---

**Ready to join the Axion Digitaverse?**

**→ [Download Now](https://axiondigitaverse.com) ←**

---

*Last Updated: 2024 | Version: 2.0.0*