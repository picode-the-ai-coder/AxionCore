# 📋 GitHub Release Instructions

## How to Create the Official GitHub Release

### Step 1: Commit & Tag

```powershell
cd C:\Users\User\source\repos\AxionCore

# Commit all changes
git add .
git commit -m "Release v2.0.0 - Complete blockchain platform with P2P networking"

# Create tag
git tag v2.0.0

# Push to GitHub
git push origin master
git push origin v2.0.0
```

### Step 2: Create GitHub Release (Web UI)

1. **Go to GitHub Releases**:
   - https://github.com/picode-the-ai-coder/AxionCore/releases

2. **Click "Create a new release"**

3. **Fill in the form**:
   
   **Tag version**: `v2.0.0`
   
   **Release title**: `Axion Digitaverse v2.0.0 - Official Launch`
   
   **Description**: (Copy from RELEASE_NOTES.md)

4. **Upload Files**:
   - `AxionDigitaverse-2.0.0-Windows.zip`
   - `checksums.txt`
   - (Add macOS and Linux builds when ready)

5. **Publish Release**

### Step 3: Verify Release

```powershell
# Check it's live
Invoke-WebRequest -Uri "https://api.github.com/repos/picode-the-ai-coder/AxionCore/releases/latest" | ConvertFrom-Json | Select tag_name, name, published_at
```

---

## Automated Release with GitHub Actions (Future)

When ready, GitHub Actions will:
1. Detect tag push (v2.0.0)
2. Build on Windows, macOS, Linux
3. Create all packages
4. Upload to release automatically

See `.github/workflows/release.yml` for details.

---

## Upload Package Files

### From Your Local Build

```powershell
# Navigate to build directory
cd C:\Users\User\source\repos\AxionCore\build

# List files to upload
dir *.zip
dir *.txt

# Files to upload:
# - AxionDigitaverse-3.4.0-win32.zip
# - checksums.txt
```

### Upload to Release

1. Go to: https://github.com/picode-the-ai-coder/AxionCore/releases
2. Click "Edit" on v2.0.0 release
3. Under "Attachments", drag & drop files
4. Wait for upload to complete
5. Click "Save changes"

---

## Announcement Template

### Social Media Posts

**Discord (Announcements channel)**:
```
🚀 AXION DIGITAVERSE v2.0.0 IS LIVE! 🚀

We're thrilled to announce the official launch of Axion Digitaverse!

✅ Real cryptocurrency network
✅ Ed25519 cryptography
✅ P2P networking
✅ Smart contracts
✅ Cross-platform (Windows/macOS/Linux)

Download now: https://axiondigitaverse.com
GitHub: https://github.com/picode-the-ai-coder/AxionCore
Docs: https://docs.axiondigitaverse.com

Join us! 🎉
```

**Twitter/X**:
```
🚀 Axion Digitaverse v2.0.0 is officially LIVE!

A complete blockchain platform with:
• Ed25519 signatures
• P2P networking
• Smart contracts
• Cross-platform support

Download: https://axiondigitaverse.com
GitHub: https://github.com/picode-the-ai-coder/AxionCore

#blockchain #cryptocurrency #axion
```

**Reddit (r/cryptocurrency, r/blockchain)**:
```
[Release] Axion Digitaverse v2.0.0 - Complete Blockchain Platform

Hi everyone! We're excited to announce the launch of Axion Digitaverse v2.0.0.

Axion is a complete, production-ready blockchain platform built from scratch in C++:

**Key Features:**
• Real Ed25519 elliptic curve cryptography (not toy crypto!)
• Proof-of-Work consensus with dynamic difficulty
• Real TCP P2P networking (16-peer mesh)
• Smart contracts (Axion Virtual Machine)
• Distributed file storage (AIPFS)
• Beautiful desktop GUI + CLI node
• Cross-platform (Windows/macOS/Linux)

**Get Started:**
- Download: https://axiondigitaverse.com
- GitHub: https://github.com/picode-the-ai-coder/AxionCore
- Docs: https://docs.axiondigitaverse.com

**Networks Stats:**
- Block time: ~15 seconds
- Supply: 1 billion AXC
- Halving: Every 210k blocks
- TPS: 100-1000 tx/sec

This is testnet phase software - not for use with real money yet.

We'd love community feedback and contributions!

Questions? Join our Discord: https://discord.gg/axiondigitaverse
```

---

## Post-Release Tasks

### Day 1 (Release Day)

- [ ] GitHub release created
- [ ] Downloads working
- [ ] Website updated
- [ ] Announcements posted
- [ ] Support team ready
- [ ] Monitor downloads

### Day 2-7

- [ ] Respond to community feedback
- [ ] Fix any critical bugs
- [ ] Update documentation as needed
- [ ] Track downloads/stars
- [ ] Welcome new community members

### Week 2

- [ ] Deploy seed nodes (if not done)
- [ ] Reach 100+ downloads
- [ ] 10+ community nodes online
- [ ] Growing Discord community

### Month 1

- [ ] 1,000+ downloads
- [ ] 100+ community nodes
- [ ] Mature network operation
- [ ] Regular updates as needed

---

## Important URLs

```
GitHub Releases:     https://github.com/picode-the-ai-coder/AxionCore/releases
Website:             https://axiondigitaverse.com
Discord:             https://discord.gg/axiondigitaverse
GitHub Issues:       https://github.com/picode-the-ai-coder/AxionCore/issues
Documentation:       https://docs.axiondigitaverse.com
```

---

## Troubleshooting Release

### If upload fails

```powershell
# Try uploading via GitHub CLI
gh release upload v2.0.0 AxionDigitaverse-2.0.0-Windows.zip
gh release upload v2.0.0 checksums.txt
```

### If tag not found

```powershell
# Verify tag exists
git tag

# If not, create it
git tag v2.0.0
git push origin v2.0.0
```

### If release shows as draft

```
1. Go to release page
2. Click "Edit"
3. Uncheck "This is a pre-release"
4. Click "Publish release"
```

---

**Release checklist complete!** ✅

Your blockchain is now ready for global distribution.
