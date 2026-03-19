#!/bin/bash
# ================================================================
#  Axion Digitaverse - Seed Node Runner (Linux/macOS)
#  
#  A seed node that stays online and serves other nodes
#  Runs without GUI, using unlimited resources
#  
#  Usage:
#    ./seed-node.sh                    - Run on default port 9000
#    ./seed-node.sh 9000 cloud.example.com - Run with external hostname
# ================================================================

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

PORT=${1:-9000}
EXTERNAL_HOST=${2:-}
DATA_DIR="axion_seed_data"

# Validate
if ! [[ "$PORT" =~ ^[0-9]+$ ]] || [ "$PORT" -lt 1024 ] || [ "$PORT" -gt 65535 ]; then
    echo -e "${RED}Error: Port must be between 1024 and 65535${NC}"
    exit 1
fi

# Banner
cat << "EOF"

================================================================
  Axion Digitaverse - SEED NODE
================================================================
EOF

echo "  Purpose    : Bootstrap peer for the network"
echo "  Port       : $PORT"
echo "  Data Dir   : $DATA_DIR"
if [ -n "$EXTERNAL_HOST" ]; then
    echo "  Public URL : $EXTERNAL_HOST:$PORT"
fi
echo "================================================================"
echo

# Create data directory
mkdir -p "$DATA_DIR"

# Setup environment
export AXION_SEED_NODE=1
export AXION_DATA_DIR="$DATA_DIR"

echo -e "${GREEN}[*] Starting Seed Node...${NC}"
echo -e "${YELLOW}[!] This node will accept all connections (no peer limit)${NC}"
echo

# Run the seed node
# Note: In production, you might want to add ulimit and other tuning
ulimit -n 65536  # Increase file descriptor limit

axion_node "$PORT"

