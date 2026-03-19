# 🚀 Axion Digitaverse - Deployment & Distribution Guide

**Version 2.0** | Last Updated: 2024

---

## 📋 Table of Contents

1. [Overview](#overview)
2. [System Architecture](#system-architecture)
3. [Building from Source](#building-from-source)
4. [Distribution Methods](#distribution-methods)
5. [Website Setup](#website-setup)
6. [Running Seed Nodes](#running-seed-nodes)
7. [Network Bootstrapping](#network-bootstrapping)
8. [Monitoring & Operations](#monitoring--operations)

---

## Overview

Axion Digitaverse is distributed via:

- **Desktop GUI** (`AxionDigitaverse.exe/app`) - For users to join and manage wallets
- **CLI Headless Node** (`axion_node`) - For server operators and miners
- **Website** - Central hub for downloads and information
- **GitHub Releases** - Version management
- **Package Managers** - (Optional) apt, brew, chocolatey

---

## System Architecture

```
Internet Users
    ↓
https://axiondigitaverse.com (Static Website)
    ├─→ Download GUI for Windows/macOS/Linux
    ├─→ Download CLI Node
    ├─→ View Documentation
    └─→ See Network Status
    
Community Operators (DNS Seeds)
    ├─→ seed-1.axiondigitaverse.com:9000
    ├─→ seed-2.axiondigitaverse.com:9000
    └─→ seed-3.axiondigitaverse.com:9000
    
User Nodes
    ├─→ Port 9000 (default)
    ├─→ Port 9001
    └─→ Custom ports
    
Blockchain Network
    └─→ P2P mesh (16 max connections per node)
```

---

## Building from Source

### Prerequisites

```bash
# Windows
- Visual Studio 2022+ Community Edition (free)
- CMake 3.20+
- Git

# macOS
brew install cmake
brew install git

# Ubuntu/Debian
sudo apt-get install cmake git build-essential libgl1-mesa-dev
```

### Build Steps

```bash
# Clone repository
git clone https://github.com/picode-the-ai-coder/AxionCore.git
cd AxionCore

# Create build directory
mkdir build && cd build

# Configure (Windows with Visual Studio)
cmake .. -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release

# OR Configure (Linux/macOS with Ninja)
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release

# Package
# Windows (NSIS installer)
cpack -G NSIS

# Linux (DEB/RPM)
cpack -G DEB
cpack -G RPM

# macOS (DMG)
cpack -G DragNDrop

# Universal ZIP
cpack -G ZIP
```

### Output Files

```
build/
├── bin/
│   ├── AxionDigitaverse.exe/app  (GUI)
│   ├── axion_node.exe            (CLI Node)
│   └── run-node.bat/sh           (Helper scripts)
├── AxionDigitaverse-2.0.0-Windows.exe    (NSIS installer)
├── AxionDigitaverse-2.0.0-Linux.deb      (Debian package)
├── AxionDigitaverse-2.0.0-Linux.rpm      (RedHat package)
├── AxionDigitaverse-2.0.0-Darwin.dmg     (macOS image)
└── AxionDigitaverse-2.0.0-Linux.tar.gz   (Universal archive)
```

---

## Distribution Methods

### Method 1: GitHub Releases (Recommended)

```bash
# On your development machine, after successful build:

# 1. Create GitHub Release
git tag v2.0.0
git push origin v2.0.0

# 2. Upload to GitHub Release (via web UI or CLI)
# https://github.com/picode-the-ai-coder/AxionCore/releases

# Files to upload:
# - AxionDigitaverse-2.0.0-Windows.exe
# - AxionDigitaverse-2.0.0-Linux.deb
# - AxionDigitaverse-2.0.0-Linux.rpm
# - AxionDigitaverse-2.0.0-Darwin.dmg
# - AxionDigitaverse-2.0.0-Windows.zip
# - AxionDigitaverse-2.0.0-Linux.tar.gz
```

### Method 2: Website Downloads

See **Website Setup** section below.

### Method 3: Package Managers

#### Windows - Chocolatey

Create `axion.nuspec`:

```xml
<?xml version="1.0" encoding="utf-8"?>
<package>
  <metadata>
    <id>axiondigitaverse</id>
    <version>2.0.0</version>
    <title>Axion Digitaverse</title>
    <authors>Axion Foundation</authors>
    <description>Full-featured blockchain node and wallet</description>
    <projectUrl>https://axiondigitaverse.com</projectUrl>
    <licenseUrl>https://github.com/picode-the-ai-coder/AxionCore/blob/master/LICENSE</licenseUrl>
    <tags>blockchain cryptocurrency axion</tags>
  </metadata>
  <files>
    <file src="AxionDigitaverse.exe" target="bin" />
    <file src="axion_node.exe" target="bin" />
  </files>
</package>
```

```bash
# Build and push
choco pack
choco push axiondigitaverse.2.0.0.nupkg
```

#### macOS - Homebrew

Create `axion-digitaverse.rb`:

```ruby
class AxionDigitaverse < Formula
  desc "Full-featured blockchain platform"
  homepage "https://axiondigitaverse.com"
  url "https://github.com/picode-the-ai-coder/AxionCore/releases/download/v2.0.0/AxionDigitaverse-2.0.0-Darwin.dmg"
  sha256 "YOUR_SHA256_HASH_HERE"
  
  def install
    app.install "AxionDigitaverse.app"
    bin.install "axion_node"
  end
end
```

#### Linux - APT

```bash
# Create Ubuntu PPA (requires Launchpad account)
# See: https://help.launchpad.net/Packaging/PPA
```

---

## Website Setup

### Simple Website Structure (HTML/CSS/JavaScript)

Create `index.html`:

```html
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Axion Digitaverse - Decentralized Blockchain Network</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <header>
        <nav>
            <div class="logo">🌐 Axion Digitaverse</div>
            <ul>
                <li><a href="#download">Download</a></li>
                <li><a href="#docs">Docs</a></li>
                <li><a href="#network">Network Status</a></li>
                <li><a href="https://github.com/picode-the-ai-coder/AxionCore">GitHub</a></li>
            </ul>
        </nav>
    </header>

    <main>
        <!-- Hero Section -->
        <section class="hero">
            <h1>Axion Digitaverse v2.0</h1>
            <p>A Complete Blockchain Platform with P2P Networking, Smart Contracts, and Real Cryptography</p>
            <button class="cta" onclick="document.getElementById('download').scrollIntoView()">
                Download Now
            </button>
        </section>

        <!-- Download Section -->
        <section id="download" class="downloads">
            <h2>Download Axion Node</h2>
            <p>Choose your operating system</p>
            
            <div class="download-grid">
                <!-- Windows -->
                <div class="card">
                    <h3>🪟 Windows</h3>
                    <p>Windows 10/11 (64-bit)</p>
                    <a href="https://github.com/picode-the-ai-coder/AxionCore/releases/download/v2.0.0/AxionDigitaverse-2.0.0-Windows.exe"
                       class="btn">
                        Download Installer
                    </a>
                    <small>100 MB | SHA256: ...</small>
                </div>

                <!-- macOS -->
                <div class="card">
                    <h3>🍎 macOS</h3>
                    <p>macOS 12+ (Intel/Apple Silicon)</p>
                    <a href="https://github.com/picode-the-ai-coder/AxionCore/releases/download/v2.0.0/AxionDigitaverse-2.0.0-Darwin.dmg"
                       class="btn">
                        Download DMG
                    </a>
                    <small>95 MB | SHA256: ...</small>
                </div>

                <!-- Linux -->
                <div class="card">
                    <h3>🐧 Linux</h3>
                    <p>Ubuntu 20.04+ / Debian 11+</p>
                    <a href="https://github.com/picode-the-ai-coder/AxionCore/releases/download/v2.0.0/AxionDigitaverse-2.0.0-Linux.deb"
                       class="btn">
                        Download DEB
                    </a>
                    <small>110 MB | SHA256: ...</small>
                </div>
            </div>

            <hr>

            <h3>Alternative Downloads</h3>
            <ul>
                <li><a href="https://github.com/picode-the-ai-coder/AxionCore/releases/download/v2.0.0/AxionDigitaverse-2.0.0-Linux.rpm">Linux RPM</a></li>
                <li><a href="https://github.com/picode-the-ai-coder/AxionCore/releases/download/v2.0.0/AxionDigitaverse-2.0.0-Windows.zip">Windows ZIP</a></li>
                <li><a href="https://github.com/picode-the-ai-coder/AxionCore/releases">All Releases</a></li>
            </ul>
        </section>

        <!-- Features Section -->
        <section class="features">
            <h2>Platform Features</h2>
            <div class="feature-grid">
                <div class="feature">
                    <h4>✓ Real Cryptography</h4>
                    <p>Ed25519 elliptic curve digital signatures</p>
                </div>
                <div class="feature">
                    <h4>✓ Proof-of-Work Consensus</h4>
                    <p>Dynamic difficulty adjustment, 15s block time</p>
                </div>
                <div class="feature">
                    <h4>✓ P2P Networking</h4>
                    <p>TCP-based peer mesh, automatic peering</p>
                </div>
                <div class="feature">
                    <h4>✓ Smart Contracts</h4>
                    <p>Axion Virtual Machine with gas metering</p>
                </div>
                <div class="feature">
                    <h4>✓ Distributed Storage</h4>
                    <p>Content-addressed file storage (AIPFS)</p>
                </div>
                <div class="feature">
                    <h4>✓ Desktop GUI</h4>
                    <p>Beautiful ImGui interface, cross-platform</p>
                </div>
            </div>
        </section>

        <!-- Network Status Section -->
        <section id="network" class="network-status">
            <h2>Network Status</h2>
            <div class="status-grid">
                <div class="stat">
                    <div class="value" id="stat-height">Loading...</div>
                    <div class="label">Chain Height</div>
                </div>
                <div class="stat">
                    <div class="value" id="stat-peers">Loading...</div>
                    <div class="label">Active Peers</div>
                </div>
                <div class="stat">
                    <div class="value" id="stat-supply">Loading...</div>
                    <div class="label">Circulating Supply</div>
                </div>
                <div class="stat">
                    <div class="value" id="stat-tps">Loading...</div>
                    <div class="label">Transactions/sec</div>
                </div>
            </div>
        </section>

        <!-- Documentation Section -->
        <section id="docs" class="docs">
            <h2>Documentation</h2>
            <div class="doc-links">
                <a href="/docs/getting-started.md" class="doc-link">
                    <strong>Getting Started</strong>
                    <p>Install and run your first node in 5 minutes</p>
                </a>
                <a href="/docs/node-setup.md" class="doc-link">
                    <strong>Node Setup Guide</strong>
                    <p>Complete guide to configuring your node</p>
                </a>
                <a href="/docs/api.md" class="doc-link">
                    <strong>Node API</strong>
                    <p>REST API endpoints and JSON-RPC methods</p>
                </a>
                <a href="/docs/architecture.md" class="doc-link">
                    <strong>Architecture</strong>
                    <p>Deep dive into the system design</p>
                </a>
            </div>
        </section>

        <!-- Seed Nodes Section -->
        <section class="seed-nodes">
            <h2>Public Seed Nodes</h2>
            <p>Use these to bootstrap your node onto the network</p>
            <table>
                <tr>
                    <th>Host</th>
                    <th>Port</th>
                    <th>Location</th>
                    <th>Operator</th>
                </tr>
                <tr>
                    <td>seed-1.axiondigitaverse.com</td>
                    <td>9000</td>
                    <td>US East</td>
                    <td>Axion Foundation</td>
                </tr>
                <tr>
                    <td>seed-2.axiondigitaverse.com</td>
                    <td>9000</td>
                    <td>EU Central</td>
                    <td>Axion Foundation</td>
                </tr>
                <tr>
                    <td>seed-3.axiondigitaverse.com</td>
                    <td>9000</td>
                    <td>Asia Pacific</td>
                    <td>Axion Foundation</td>
                </tr>
            </table>
        </section>
    </main>

    <footer>
        <p>&copy; 2024 Axion Foundation. Licensed under MIT.</p>
        <p>
            <a href="https://github.com/picode-the-ai-coder/AxionCore">GitHub</a> •
            <a href="/docs">Docs</a> •
            <a href="mailto:support@axiondigitaverse.com">Support</a>
        </p>
    </footer>

    <script>
        // Fetch network stats from a public node
        async function updateNetworkStats() {
            try {
                const response = await fetch('https://seed-1.axiondigitaverse.com:9001/api/stats');
                const data = await response.json();
                document.getElementById('stat-height').textContent = data.chain_height;
                document.getElementById('stat-peers').textContent = data.peer_count;
                document.getElementById('stat-supply').textContent = data.circulating_supply + ' AXC';
                document.getElementById('stat-tps').textContent = data.transactions_per_sec.toFixed(1);
            } catch (e) {
                console.error('Failed to fetch stats:', e);
            }
        }

        updateNetworkStats();
        setInterval(updateNetworkStats, 30000);  // Refresh every 30s
    </script>
</body>
</html>
```

---

## Running Seed Nodes

Seed nodes are critical infrastructure for network bootstrapping.

### Cloud Deployment Options

#### Option 1: Digital Ocean ($5-20/month)

```bash
# 1. Create Ubuntu 22.04 LTS droplet
# 2. SSH into droplet
ssh root@your-droplet-ip

# 3. Install dependencies
sudo apt-get update
sudo apt-get install -y cmake git build-essential libgl1-mesa-dev

# 4. Build from source
git clone https://github.com/picode-the-ai-coder/AxionCore.git
cd AxionCore
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# 5. Set up systemd service
sudo tee /etc/systemd/system/axion-seed.service > /dev/null << 'EOF'
[Unit]
Description=Axion Digitaverse Seed Node
After=network.target

[Service]
Type=simple
User=axion
WorkingDirectory=/home/axion/axion-node
ExecStart=/home/axion/axion-node/build/bin/axion_node 9000
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

# 6. Start service
sudo systemctl enable axion-seed
sudo systemctl start axion-seed

# 7. Check logs
sudo journalctl -u axion-seed -f
```

#### Option 2: AWS EC2 (Free tier available)

```bash
# Launch t3.micro instance with Ubuntu 22.04
# Set security group to allow port 9000 inbound

# Follow same steps as Digital Ocean above
```

#### Option 3: Linode ($5-10/month)

Similar to Digital Ocean.

---

## Network Bootstrapping

### DNS Seed Configuration

When users download and run a node, it should:

1. Check DNS for seed nodes
2. Connect to at least one seed
3. Request peer list
4. Begin sync

Update `network.h`:

```cpp
static const std::vector<std::string> DNS_SEEDS = {
    "seed-1.axiondigitaverse.com:9000",
    "seed-2.axiondigitaverse.com:9000",
    "seed-3.axiondigitaverse.com:9000",
};
```

### Initial Sync

```cpp
// In AxionNode::start_network()
bool start_network() {
    if (!net.start()) return false;
    
    // Connect to seed nodes
    for (const auto& seed : DNS_SEEDS) {
        net.add_seed(seed);
        net.connect_to(seed);
    }
    
    // Wait for sync
    std::cout << "[Node] Syncing from peers...\n";
    return true;
}
```

---

## Monitoring & Operations

### Health Check Endpoint

Add to `main.cpp`:

```cpp
// Simple HTTP server on port 9001 for monitoring
void run_http_metrics_server(const AxionNode& node, int port) {
    // Returns JSON with:
    // - chain_height
    // - peer_count
    // - mempool_size
    // - circulating_supply
    // - difficulty
}
```

### Automated Restarts

systemd (Linux):

```ini
[Service]
Restart=always
RestartSec=10
```

Windows Task Scheduler:

```xml
<Task>
  <Triggers>
    <BootTrigger />
  </Triggers>
  <Actions>
    <Exec>
      <Command>C:\Program Files\AxionDigitaverse\bin\axion_node.exe</Command>
      <Arguments>9000</Arguments>
    </Exec>
  </Actions>
</Task>
```

### Monitoring Tools

```bash
# Monitor seed node metrics
watch 'curl -s http://localhost:9001/api/stats | jq'

# Check peer connections
netstat -tuln | grep 9000
```

---

## Summary

| Step | Action |
|------|--------|
| 1 | Build & test locally |
| 2 | Create GitHub Release with binaries |
| 3 | Deploy website to GitHub Pages or Netlify |
| 4 | Launch 3+ seed nodes on cloud servers |
| 5 | Update DNS/domain records |
| 6 | Monitor network health |
| 7 | Release updates via GitHub |

Your system is now ready for global distribution! 🚀
