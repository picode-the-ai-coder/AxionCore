#!/bin/bash

# Axion Bootnode Setup and Installation Script
# Supports Linux and macOS

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Functions
print_header() {
    echo -e "${BLUE}"
    echo "╔════════════════════════════════════════════════════════════════╗"
    echo "║          🌐 AXION BOOTNODE SETUP SCRIPT 🌐                   ║"
    echo "╚════════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

check_requirements() {
    print_info "Checking requirements..."
    
    # Check for required tools
    if ! command -v cmake &> /dev/null; then
        print_error "CMake is not installed"
        echo "Install with: sudo apt install cmake"
        return 1
    fi
    print_success "CMake found"
    
    if ! command -v g++ &> /dev/null; then
        print_error "G++ is not installed"
        echo "Install with: sudo apt install build-essential"
        return 1
    fi
    print_success "G++ found"
    
    if ! command -v git &> /dev/null; then
        print_error "Git is not installed"
        echo "Install with: sudo apt install git"
        return 1
    fi
    print_success "Git found"
    
    return 0
}

create_directories() {
    print_info "Creating directories..."
    
    mkdir -p data/chaindata
    mkdir -p logs
    mkdir -p metrics
    mkdir -p certs
    
    print_success "Directories created"
}

build_bootnode() {
    print_info "Building bootnode..."
    
    if [ ! -d "build" ]; then
        mkdir build
    fi
    
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build .
    cd ..
    
    if [ -f "build/bin/axion-bootnode" ]; then
        print_success "Bootnode built successfully"
        return 0
    else
        print_error "Build failed"
        return 1
    fi
}

install_service() {
    print_info "Installing as system service..."
    
    # Get current directory
    BOOTNODE_DIR=$(pwd)
    
    # Create service file
    SERVICE_FILE="/tmp/axion-bootnode.service"
    cat > "$SERVICE_FILE" << EOF
[Unit]
Description=Axion Network Bootnode
After=network.target

[Service]
Type=simple
User=$USER
WorkingDirectory=$BOOTNODE_DIR
ExecStart=$BOOTNODE_DIR/build/bin/axion-bootnode
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF
    
    echo "Service file created. To install (requires sudo):"
    echo "  sudo cp $SERVICE_FILE /etc/systemd/system/"
    echo "  sudo systemctl daemon-reload"
    echo "  sudo systemctl enable axion-bootnode"
    echo "  sudo systemctl start axion-bootnode"
}

configure_firewall() {
    print_info "Checking firewall configuration..."
    
    if command -v ufw &> /dev/null; then
        echo "UFW found. Recommended rules:"
        echo "  sudo ufw allow 30303/tcp"
        echo "  sudo ufw allow 30303/udp"
        echo "  sudo ufw allow 30304/udp"
        echo "  sudo ufw allow 8545/tcp"
    else
        print_warning "UFW not found. Please configure firewall manually for ports: 30303, 30304, 8545, 8546"
    fi
}

setup_test_mode() {
    print_info "Setting up test mode..."
    
    echo "To run bootnode in test mode:"
    echo "  ./build/bin/axion-bootnode --test-mode"
    echo ""
    echo "This will:"
    echo "  • Add 4 demo peers"
    echo "  • Start all services"
    echo "  • Enable interactive commands"
}

print_summary() {
    echo ""
    echo -e "${GREEN}╔════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║           BOOTNODE SETUP COMPLETED SUCCESSFULLY                ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo "Next steps:"
    echo ""
    echo "1. Update configuration:"
    echo "   nano bootnode.conf"
    echo "   - Set 'external_ip' to your IP (curl ifconfig.me)"
    echo ""
    echo "2. Run bootnode:"
    echo "   ./build/bin/axion-bootnode --test-mode"
    echo ""
    echo "3. Configure firewall (Linux):"
    echo "   sudo ufw allow 30303/tcp"
    echo "   sudo ufw allow 30303/udp"
    echo "   sudo ufw allow 30304/udp"
    echo ""
    echo "4. Setup as system service:"
    echo "   sudo cp /tmp/axion-bootnode.service /etc/systemd/system/"
    echo "   sudo systemctl enable axion-bootnode"
    echo "   sudo systemctl start axion-bootnode"
    echo ""
    echo "Available commands (interactive mode):"
    echo "   peer_count          - Show connected peers"
    echo "   list_peers          - List all peers"
    echo "   network_stats       - Show stats (JSON)"
    echo "   add_peer <ip>       - Add a peer"
    echo "   help                - Show all commands"
    echo "   quit                - Exit"
    echo ""
}

main() {
    print_header
    echo ""
    
    # Check requirements
    if ! check_requirements; then
        print_error "Requirements not met. Please install missing packages."
        return 1
    fi
    
    echo ""
    
    # Create directories
    create_directories
    
    echo ""
    
    # Build
    if ! build_bootnode; then
        print_error "Build failed"
        return 1
    fi
    
    echo ""
    
    # Install service
    install_service
    
    echo ""
    
    # Configure firewall
    configure_firewall
    
    echo ""
    
    # Test mode info
    setup_test_mode
    
    echo ""
    
    # Summary
    print_summary
}

main "$@"
