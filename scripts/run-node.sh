#!/bin/bash
# ================================================================
#  Axion Digitaverse - Node Runner (Linux/macOS)
#  
#  Usage:
#    ./run-node.sh                         - Start on default port 9000
#    ./run-node.sh 9001                    - Start on port 9001
#    ./run-node.sh 9001 192.168.1.5:9000   - Start on 9001, connect to seed
# ================================================================

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default values
PORT=${1:-9000}
SEED_PEER=${2:-}
DATA_DIR="axion_data"

# Validate port
if ! [[ "$PORT" =~ ^[0-9]+$ ]] || [ "$PORT" -lt 1024 ] || [ "$PORT" -gt 65535 ]; then
    echo -e "${RED}Error: Port must be between 1024 and 65535${NC}"
    exit 1
fi

DATA_DIR="axion_data_${PORT}"

# Banner
cat << "EOF"

================================================================
  Axion Digitaverse - Node Startup
================================================================
EOF

echo "  Port       : $PORT"
echo "  Data Dir   : $DATA_DIR"
if [ -n "$SEED_PEER" ]; then
    echo "  Seed Peer  : $SEED_PEER"
else
    echo "  Seed Peer  : (none - will wait for connections)"
fi

echo "================================================================"
echo

# Create data directory if it doesn't exist
if [ ! -d "$DATA_DIR" ]; then
    mkdir -p "$DATA_DIR"
    echo -e "${GREEN}[+] Created data directory: $DATA_DIR${NC}"
fi

# Check if axion_node exists
if ! command -v axion_node &> /dev/null; then
    echo -e "${RED}Error: axion_node not found in PATH${NC}"
    echo "Make sure axion_node is installed and in your PATH"
    exit 1
fi

# Start the node
echo -e "${GREEN}[*] Starting Axion node...${NC}"
echo

if [ -n "$SEED_PEER" ]; then
    axion_node "$PORT" "$SEED_PEER"
else
    axion_node "$PORT"
fi

echo
echo -e "${YELLOW}[!] Node stopped${NC}"
