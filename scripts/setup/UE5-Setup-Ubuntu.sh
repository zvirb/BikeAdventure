#!/bin/bash
set -e

PROJECT_NAME="BikeAdventure"
UE5_VERSION="5.4"
INSTALL_DIR="$HOME/UnrealEngine5"
PROJECT_DIR="$HOME/Projects/$PROJECT_NAME"

echo "Setting up BikeAdventure development environment on Ubuntu..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Check if running as root
if [[ $EUID -eq 0 ]]; then
   echo -e "${RED}This script should not be run as root${NC}" 
   exit 1
fi

# Update system packages
echo -e "${YELLOW}Updating system packages...${NC}"
sudo apt update && sudo apt upgrade -y

# Install system dependencies
echo -e "${YELLOW}Installing system dependencies...${NC}"
sudo apt install -y \
    build-essential ninja-build git git-lfs \
    python3 python3-dev python3-pip \
    libvulkan1 vulkan-tools vulkan-utility-libraries-dev \
    libc6-dev libstdc++6 cmake clang-15 \
    curl wget unzip pciutils

# Install additional graphics libraries
sudo apt install -y \
    libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev \
    libgl1-mesa-dev libglu1-mesa-dev mesa-common-dev \
    libasound2-dev libpulse-dev

# Setup NVIDIA drivers if GPU detected
if lspci | grep -i nvidia > /dev/null; then
    echo -e "${CYAN}NVIDIA GPU detected, setting up drivers...${NC}"
    sudo add-apt-repository ppa:graphics-drivers/ppa -y
    sudo apt update && sudo apt install -y nvidia-driver-535
    echo -e "${YELLOW}NVIDIA drivers installed. Reboot may be required.${NC}"
fi

# Create project directories
echo -e "${YELLOW}Creating project directory structure...${NC}"
mkdir -p "$INSTALL_DIR"
mkdir -p "$PROJECT_DIR"/{Source,Content,Config,Documentation,Tests}

# Setup Git LFS
# echo -e "${YELLOW}Configuring Git LFS...${NC}"
# git lfs install

# Set environment variables
echo -e "${YELLOW}Setting environment variables...${NC}"
cat >> ~/.bashrc << EOF

# BikeAdventure Project Configuration
export UE5_ROOT="$INSTALL_DIR"
export BIKEADVENTURE_PROJECT_PATH="$PROJECT_DIR"
export UE5_PROJECT_NAME="$PROJECT_NAME"
export PATH="\$UE5_ROOT/Engine/Binaries/Linux:\$PATH"
EOF

# Create desktop entry for easy access
mkdir -p ~/.local/share/applications
cat > ~/.local/share/applications/bikeadventure-dev.desktop << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=BikeAdventure Development
Comment=Open BikeAdventure project directory
Exec=nautilus $PROJECT_DIR
Icon=folder
Terminal=false
Categories=Development;
EOF

echo -e "${GREEN}Environment setup complete!${NC}"
echo -e "${CYAN}Next steps:${NC}"
echo "1. Download UE5 Linux build and extract to: $INSTALL_DIR"
echo "2. Source your bashrc: source ~/.bashrc"
echo "3. Initialize git repository in project directory"
echo "4. Start development using UnifiedWorkflow agents"
echo -e "${YELLOW}Project directory: $PROJECT_DIR${NC}"
echo -e "${YELLOW}UE5 installation directory: $INSTALL_DIR${NC}"