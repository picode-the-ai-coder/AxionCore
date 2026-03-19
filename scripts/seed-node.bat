@echo off
REM ================================================================
REM  Axion Digitaverse - Seed Node Runner (Windows)
REM  
REM  A seed node that stays online and serves other nodes
REM ================================================================

setlocal enabledelayedexpansion

set PORT=%1
if "!PORT!"=="" set PORT=9000

set EXTERNAL_HOST=%2
set DATA_DIR=axion_seed_data

REM Validate port
for /f %%A in ('powershell -Command "if ('!PORT!' -match '^\d+$' -and [int]'!PORT!' -ge 1024 -and [int]'!PORT!' -le 65535) { 'valid' } else { 'invalid' }"') do set PORTCHECK=%%A
if not "!PORTCHECK!"=="valid" (
    echo Error: Port must be between 1024 and 65535
    exit /b 1
)

echo.
echo ================================================================
echo  Axion Digitaverse - SEED NODE
echo ================================================================
echo  Purpose    : Bootstrap peer for the network
echo  Port       : !PORT!
echo  Data Dir   : !DATA_DIR!
if not "!EXTERNAL_HOST!"=="" (
    echo  Public URL : !EXTERNAL_HOST!:!PORT!
)
echo ================================================================
echo.

if not exist "!DATA_DIR!" mkdir "!DATA_DIR!"

echo [*] Starting Seed Node...
echo [!] This node will accept all connections
echo.

set AXION_SEED_NODE=1
set AXION_DATA_DIR=!DATA_DIR!

axion_node !PORT!

echo.
echo [!] Seed node stopped
pause
