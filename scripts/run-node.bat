@echo off
REM ================================================================
REM  Axion Digitaverse - Node Runner (Windows)
REM  
REM  Usage:
REM    run-node.bat                    - Start on default port 9000
REM    run-node.bat 9001               - Start on port 9001
REM    run-node.bat 9001 192.168.1.5:9000  - Start on 9001, connect to seed
REM ================================================================

setlocal enabledelayedexpansion

REM Default values
set PORT=9000
set SEED_PEER=
set DATA_DIR=axion_data

REM Parse arguments
if not "%1"=="" set PORT=%1
if not "%2"=="" set SEED_PEER=%2

REM Validate port is numeric
for /f %%A in ('powershell -Command "if ('!PORT!' -match '^\d+$' -and [int]'!PORT!' -ge 1024 -and [int]'!PORT!' -le 65535) { 'valid' } else { 'invalid' }"') do set PORTCHECK=%%A
if not "!PORTCHECK!"=="valid" (
    echo Error: Port must be between 1024 and 65535
    exit /b 1
)

set DATA_DIR=axion_data_!PORT!

echo.
echo ================================================================
echo  Axion Digitaverse - Node Startup
echo ================================================================
echo  Port       : !PORT!
echo  Data Dir   : !DATA_DIR!
if not "!SEED_PEER!"=="" (
    echo  Seed Peer  : !SEED_PEER!
) else (
    echo  Seed Peer  : (none - will wait for connections)
)
echo ================================================================
echo.

REM Create data directory if it doesn't exist
if not exist "!DATA_DIR!" (
    mkdir "!DATA_DIR!"
    echo [+] Created data directory: !DATA_DIR!
)

REM Start the node
echo [*] Starting Axion node...
echo.

if not "!SEED_PEER!"=="" (
    axion_node !PORT! !SEED_PEER!
) else (
    axion_node !PORT!
)

echo.
echo [!] Node stopped
pause
