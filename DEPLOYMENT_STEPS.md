# 🚀 Axion Digitaverse - Step-by-Step Deployment Guide

**Complete walkthrough from code to live network**

---

## Phase 1: Prepare Your Development Environment

### 1.1 Install Build Tools

**Windows:**
```powershell
# Install Visual Studio 2022 Community (free)
# - Download from visualstudio.microsoft.com
# - Select "Desktop development with C++"
# - Install CMake (included in VS, or brew install cmake)

# Install Ninja
choco install ninja -y
```

**macOS:**
```bash
brew install cmake ninja git
xcode-select --install
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt update
sudo apt install -y cmake ninja-build git build-essential libgl1-mesa-dev
```

### 1.2 Clone Repository

```bash
git clone https://github.com/picode-the-ai-coder/AxionCore.git
cd AxionCore
```

---

## Phase 2: Build on Your Local Machine

### 2.1 Create Build Directory

```bash
mkdir build
cd build
```

### 2.2 Configure CMake

**Windows (Visual Studio):**
```cmd
cmake .. -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release
```

**macOS/Linux (Ninja):**
```bash
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
```

### 2.3 Build

```bash
cmake --build . --config Release
```

**Output:**
```
bin/
├── AxionDigitaverse.exe/app    (GUI)
├── axion_node.exe              (CLI)
└── scripts/                     (Helper scripts)
```

### 2.4 Test Locally

```bash
# GUI
./bin/AxionDigitaverse

# CLI Node (default port 9000)
./bin/axion_node

# CLI Node (custom port)
./bin/axion_node 9001

# Headless with seed peer
./bin/axion_node 9001 127.0.0.1:9000
```

---

## Phase 3: Create Distribution Packages

### 3.1 Generate Packages

```bash
# Windows (NSIS installer)
cpack -G NSIS

# Windows (ZIP archive)
cpack -G ZIP

# macOS (DMG)
cpack -G DragNDrop

# macOS (ZIP)
cpack -G ZIP

# Linux (DEB)
cpack -G DEB

# Linux (RPM)
cpack -G RPM

# Linux (TGZ - universal)
cpack -G TGZ

# Generate all at once
cpack
```

### 3.2 Create Checksums

```bash
# Windows (PowerShell)
Get-FileHash AxionDigitaverse-*.exe | Format-List | Out-File checksums.txt

# macOS/Linux
sha256sum *.exe *.deb *.rpm *.dmg > checksums.txt
```

**Output files:**
```
build/
├── AxionDigitaverse-2.0.0-Windows.exe      (NSIS installer)
├── AxionDigitaverse-2.0.0-Windows.zip      (ZIP)
├── AxionDigitaverse-2.0.0-Darwin.dmg       (macOS DMG)
├── AxionDigitaverse-2.0.0-Darwin.tar.gz    (macOS TAR)
├── AxionDigitaverse-2.0.0-Linux.deb        (Debian)
├── AxionDigitaverse-2.0.0-Linux.rpm        (RedHat)
├── AxionDigitaverse-2.0.0-Linux.tar.gz     (Universal)
└── checksums.txt                           (SHA256 hashes)
```

---

## Phase 4: Set Up GitHub Release

### 4.1 Create Git Tag

```bash
git add .
git commit -m "Release v2.0.0"
git tag v2.0.0
git push origin master
git push origin v2.0.0
```

### 4.2 GitHub Actions (Automated)

Once you push the tag, GitHub Actions automatically:
1. Builds on Windows, macOS, Linux
2. Creates packages for all platforms
3. Uploads to GitHub Release

**Files appear in:**
https://github.com/picode-the-ai-coder/AxionCore/releases/tag/v2.0.0

### 4.3 Manual Release (Alternative)

If GitHub Actions fails:

1. Go to https://github.com/picode-the-ai-coder/AxionCore/releases
2. Click "Create a new release"
3. Tag: `v2.0.0`
4. Title: `Axion Digitaverse v2.0.0`
5. Upload files:
   - All `.exe`, `.deb`, `.rpm`, `.dmg`, `.tar.gz` files
   - `checksums.txt`
6. Publish release

---

## Phase 5: Deploy Seed Nodes (3 minimum)

### 5.1 Seed Node #1 - US East (Digital Ocean)

```bash
# 1. Create account at digitalocean.com

# 2. Create Droplet:
# - Image: Ubuntu 22.04 LTS
# - Size: 2 GB RAM / 50 GB SSD
# - Region: New York (US East)
# - Price: ~$12/month

# 3. SSH into droplet
ssh root@your-droplet-ip

# 4. Setup dependencies
sudo apt update
sudo apt install -y build-essential cmake ninja-build git libgl1-mesa-dev
git clone https://github.com/picode-the-ai-coder/AxionCore.git
cd AxionCore && mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# 5. Create systemd service
sudo tee /etc/systemd/system/axion.service > /dev/null <<'EOF'
[Unit]
Description=Axion Digitaverse Seed Node
After=network.target

[Service]
Type=simple
User=root
WorkingDirectory=/root/AxionCore/build
ExecStart=/root/AxionCore/build/bin/axion_node 9000
Restart=always
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

# 6. Enable and start
sudo systemctl daemon-reload
sudo systemctl enable axion
sudo systemctl start axion

# 7. Check status
sudo systemctl status axion
sudo journalctl -u axion -f

# 8. Update DNS record:
# In your domain registrar, add A record:
# seed-1.axiondigitaverse.com → droplet-ip-address
```

### 5.2 Seed Node #2 - EU Central (Digital Ocean)

Repeat 5.1 but:
- Region: Frankfurt (EU Central)
- Hostname: `seed-2.axiondigitaverse.com`

### 5.3 Seed Node #3 - Asia Pacific (Digital Ocean)

Repeat 5.1 but:
- Region: Singapore (Asia Pacific)
- Hostname: `seed-3.axiondigitaverse.com`

### 5.4 Verify Seed Nodes

```bash
# Test connectivity
ping seed-1.axiondigitaverse.com
ping seed-2.axiondigitaverse.com
ping seed-3.axiondigitaverse.com

# Test node port
netstat -tuln | grep 9000
nmap -p 9000 seed-1.axiondigitaverse.com

# Check logs
ssh root@seed-1-ip "journalctl -u axion -n 100"
```

---

## Phase 6: Set Up Website

### 6.1 Deploy Static Website

**Option A: GitHub Pages (Free)**

```bash
# In your repository root:
mkdir docs
cp website/* docs/

git add docs/
git commit -m "Add website"
git push origin master

# Enable in GitHub:
# Settings → Pages → Source: main/docs folder
# Wait 1-2 minutes for deployment
```

**Option B: Netlify (Free)**

```bash
# 1. Sign up at netlify.com
# 2. Connect your GitHub repo
# 3. Build command: (none)
# 4. Publish directory: website/
# 5. Deploy
# Site appears at: https://your-site.netlify.app
# Custom domain: https://axiondigitaverse.com
```

**Option C: Custom Server**

```bash
# Upload to your server via FTP/SFTP
scp -r website/ user@yourserver.com:/var/www/axiondigitaverse.com/

# Configure nginx
sudo tee /etc/nginx/sites-available/axion > /dev/null <<'EOF'
server {
    listen 80;
    server_name axiondigitaverse.com www.axiondigitaverse.com;
    root /var/www/axiondigitaverse.com;
    index index.html;
    
    location / {
        try_files $uri $uri/ =404;
    }
}
EOF

sudo systemctl restart nginx
```

### 6.2 Set Up HTTPS

```bash
# Using Let's Encrypt (free)
sudo apt install certbot python3-certbot-nginx
sudo certbot --nginx -d axiondigitaverse.com
# Renews automatically
```

### 6.3 Update Website

Update `website/index.html` with:
- GitHub download links
- Seed node list
- Network status
- Documentation links

---

## Phase 7: Update Network Configuration

### 7.1 Update DNS Seeds in Code

Edit `network.h`:

```cpp
static const std::vector<std::string> DNS_SEEDS = {
    "seed-1.axiondigitaverse.com:9000",
    "seed-2.axiondigitaverse.com:9000",
    "seed-3.axiondigitaverse.com:9000",
};
```

### 7.2 Rebuild & Redeploy

```bash
cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cpack
```

---

## Phase 8: Launch!

### 8.1 Final Checklist

- [ ] Website live and accessible
- [ ] Downloads working
- [ ] All 3 seed nodes online
- [ ] DNS resolving correctly
- [ ] Documentation published
- [ ] Social media accounts ready

### 8.2 Announcement

Post on:
- [ ] GitHub Releases
- [ ] Twitter/X
- [ ] Discord
- [ ] Reddit (r/cryptocurrency, r/blockchain)
- [ ] Hacker News (if appropriate)
- [ ] Blog/Medium

**Template:**
```
🎉 Axion Digitaverse v2.0 is LIVE!

Complete blockchain platform with:
✅ Real Ed25519 cryptography
✅ P2P networking (TCP mesh)
✅ Smart contracts (AVM)
✅ Distributed storage (AIPFS)
✅ Desktop GUI + CLI node
✅ Cross-platform (Windows/macOS/Linux)

Download: https://axiondigitaverse.com
Docs: https://docs.axiondigitaverse.com
GitHub: https://github.com/picode-the-ai-coder/AxionCore

Join us! 🚀
```

### 8.3 Monitor First 24 Hours

- [ ] Check download numbers
- [ ] Monitor GitHub stars
- [ ] Track seed node connections
- [ ] Watch for issues/bugs
- [ ] Respond to community questions
- [ ] Fix any critical bugs immediately

---

## Phase 9: Ongoing Operations

### 9.1 Daily Monitoring

```bash
# SSH into each seed node
ssh root@seed-1-ip

# Check if running
systemctl status axion

# View recent logs
journalctl -u axion -n 50 -f

# Check network
netstat -an | grep 9000
```

### 9.2 Weekly Tasks

- [ ] Monitor disk usage on seed nodes
- [ ] Check GitHub issues & respond
- [ ] Review logs for errors
- [ ] Verify DNS resolution

### 9.3 Monthly Tasks

- [ ] Update dependencies
- [ ] Security audit
- [ ] Performance analysis
- [ ] Community feedback review

---

## Phase 10: Growth

### 10.1 Add More Seed Nodes

As network grows, add more regional nodes:

```
seed-4.axiondigitaverse.com (South America)
seed-5.axiondigitaverse.com (Middle East)
seed-6.axiondigitaverse.com (Australia)
```

### 10.2 Public API Endpoint

Create REST API service:

```bash
# Expose on seed-1 only
curl https://api.axiondigitaverse.com/stats
# Returns: { chain_height, peer_count, tps, ... }
```

### 10.3 Status Page

Set up uptime monitoring:

```
https://status.axiondigitaverse.com
├── Seed 1: Online ✅
├── Seed 2: Online ✅
├── Seed 3: Online ✅
├── Website: Online ✅
└── API: Online ✅
```

---

## Troubleshooting

### Build fails

```bash
# Clean rebuild
rm -rf build
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Seed node won't start

```bash
# Check logs
sudo journalctl -u axion -n 100

# Check port conflict
netstat -tuln | grep 9000

# Try different port
sudo systemctl stop axion
# Edit /etc/systemd/system/axion.service
# Change port 9000 → 9001
sudo systemctl start axion
```

### DNS not resolving

```bash
# Check DNS propagation
nslookup seed-1.axiondigitaverse.com
dig seed-1.axiondigitaverse.com

# Wait 24 hours for propagation
# Or contact your DNS provider
```

---

## Success Metrics

Track these after launch:

| Metric | Target | Check |
|--------|--------|-------|
| Seed nodes online | 3/3 | ✅ |
| Downloads/week | 100+ | — |
| GitHub stars | 100+ | — |
| Network nodes | 10+ | — |
| Chain height | Growing | — |
| Uptime | 99%+ | — |

---

**Congratulations! Your blockchain is live!** 🎉

For questions: support@axiondigitaverse.com
