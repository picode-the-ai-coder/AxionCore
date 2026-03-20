@echo off
REM Axion Bootnode Setup Script for Windows
REM Requires: Visual Studio Build Tools, CMake, Git

setlocal enabledelayedexpansion

echo.
echo ╔════════════════════════════════════════════════════════════════╗
echo ║          🌐 AXION BOOTNODE SETUP SCRIPT (WINDOWS) 🌐         ║
echo ╚════════════════════════════════════════════════════════════════╝
echo.

REM Check for admin privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [ERROR] This script requires administrator privileges
    echo Please run as Administrator
    pause
    exit /b 1
)

REM Check for required tools
echo [INFO] Checking requirements...

REM Check CMake
cmake --version >nul 2>&1
if %errorLevel% neq 0 (
    echo [ERROR] CMake is not installed
    echo Download from: https://cmake.org/download/
    pause
    exit /b 1
)
echo [OK] CMake found

REM Check Git
git --version >nul 2>&1
if %errorLevel% neq 0 (
    echo [ERROR] Git is not installed
    echo Download from: https://git-scm.com/download/win
    pause
    exit /b 1
)
echo [OK] Git found

REM Check Visual Studio
where cl.exe >nul 2>&1
if %errorLevel% neq 0 (
    echo [WARNING] MSVC compiler not found in PATH
    echo Please install Visual Studio Build Tools
    echo Or run from "Developer Command Prompt for VS"
)

echo.

REM Create directories
echo [INFO] Creating directories...
if not exist "data\chaindata" mkdir data\chaindata
if not exist "logs" mkdir logs
if not exist "metrics" mkdir metrics
if not exist "certs" mkdir certs
echo [OK] Directories created

echo.

REM Build bootnode
echo [INFO] Building bootnode...
if not exist "build" mkdir build
cd build

echo Running CMake...
cmake .. -G "Visual Studio 16 2019"
if %errorLevel% neq 0 (
    echo [ERROR] CMake generation failed
    pause
    exit /b 1
)

echo Building...
cmake --build . --config Release
if %errorLevel% neq 0 (
    echo [ERROR] Build failed
    pause
    exit /b 1
)

cd ..
echo [OK] Build completed

echo.

REM Check build result
if exist "bin\Release\axion-bootnode.exe" (
    echo [SUCCESS] Bootnode built: bin\Release\axion-bootnode.exe
) else if exist "bin\axion-bootnode.exe" (
    echo [SUCCESS] Bootnode built: bin\axion-bootnode.exe
) else (
    echo [ERROR] Build output not found
    pause
    exit /b 1
)

echo.

REM Firewall setup
echo [INFO] Windows Firewall Configuration
echo.
echo To allow bootnode through Windows Firewall, run these commands:
echo.
echo netsh advfirewall firewall add rule name="Axion P2P TCP" ^
echo    dir=in action=allow protocol=tcp localport=30303 enable=yes
echo.
echo netsh advfirewall firewall add rule name="Axion P2P UDP" ^
echo    dir=in action=allow protocol=udp localport=30303 enable=yes
echo.
echo netsh advfirewall firewall add rule name="Axion Discovery" ^
echo    dir=in action=allow protocol=udp localport=30304 enable=yes
echo.
echo netsh advfirewall firewall add rule name="Axion RPC" ^
echo    dir=in action=allow protocol=tcp localport=8545 enable=yes
echo.

echo.
echo ╔════════════════════════════════════════════════════════════════╗
echo ║           BOOTNODE SETUP COMPLETED SUCCESSFULLY                ║
echo ╚════════════════════════════════════════════════════════════════╝
echo.
echo Next steps:
echo.
echo 1. Update configuration:
echo    Edit: bootnode\bootnode.conf
echo    Set "external_ip" to your IP
echo.
echo 2. Run bootnode (test mode):
echo    bin\Release\axion-bootnode.exe --test-mode
echo.
echo 3. Setup as Windows Service (optional):
echo    Download NSSM from: https://nssm.cc/download
echo    nssm install AxionBootnode "%cd%\bin\Release\axion-bootnode.exe"
echo    nssm start AxionBootnode
echo.
echo 4. Configure Windows Firewall:
echo    Copy the firewall rules from above and run them in Command Prompt (as Admin)
echo.
echo Available commands (interactive mode):
echo    peer_count          - Show connected peers
echo    list_peers          - List all peers
echo    network_stats       - Show stats (JSON)
echo    add_peer [ip]       - Add a peer
echo    help                - Show all commands
echo    quit                - Exit
echo.

pause
