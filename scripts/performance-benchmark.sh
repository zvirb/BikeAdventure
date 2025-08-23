#!/bin/bash
# BikeAdventure Performance Benchmark Script
# Validates 60+ FPS performance targets and system requirements

PROJECT_ROOT="/home/marku/Documents/programming/BikeAdventure"
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}BikeAdventure Performance Validation${NC}"
echo "====================================="

# System Information
echo -e "\n${YELLOW}System Information${NC}"
echo "------------------"

# CPU Information
CPU_INFO=$(lscpu | grep "Model name" | cut -d ':' -f2 | xargs)
CPU_CORES=$(nproc)
echo -e "CPU: ${CYAN}$CPU_INFO${NC} (${CPU_CORES} cores)"

# Memory Information
TOTAL_RAM=$(free -h | grep "Mem:" | awk '{print $2}')
AVAILABLE_RAM=$(free -h | grep "Mem:" | awk '{print $7}')
echo -e "RAM: ${CYAN}$TOTAL_RAM${NC} total, ${CYAN}$AVAILABLE_RAM${NC} available"

# GPU Information
if command -v nvidia-smi &> /dev/null; then
    GPU_INFO=$(nvidia-smi --query-gpu=name --format=csv,noheader,nounits | head -1)
    GPU_MEMORY=$(nvidia-smi --query-gpu=memory.total --format=csv,noheader,nounits | head -1)
    echo -e "GPU: ${CYAN}$GPU_INFO${NC} (${GPU_MEMORY}MB VRAM)"
elif command -v lspci &> /dev/null && lspci | grep -i vga | grep -q -i amd; then
    GPU_INFO=$(lspci | grep -i vga | grep -i amd | cut -d ':' -f3 | xargs)
    echo -e "GPU: ${CYAN}$GPU_INFO${NC} (AMD)"
else
    echo -e "GPU: ${YELLOW}Integrated or Unknown${NC}"
fi

# Performance Requirements Check
echo -e "\n${YELLOW}Performance Requirements Check${NC}"
echo "------------------------------"

REQUIREMENTS_MET=0
TOTAL_REQUIREMENTS=4

# Check CPU cores (minimum 4 for 60+ FPS)
if [ "$CPU_CORES" -ge 4 ]; then
    echo -e "✅ CPU Cores: ${GREEN}$CPU_CORES >= 4${NC}"
    REQUIREMENTS_MET=$((REQUIREMENTS_MET + 1))
else
    echo -e "❌ CPU Cores: ${RED}$CPU_CORES < 4${NC}"
fi

# Check RAM (minimum 8GB for UE5)
RAM_GB=$(free -g | grep "Mem:" | awk '{print $2}')
if [ "$RAM_GB" -ge 8 ]; then
    echo -e "✅ RAM: ${GREEN}${RAM_GB}GB >= 8GB${NC}"
    REQUIREMENTS_MET=$((REQUIREMENTS_MET + 1))
else
    echo -e "❌ RAM: ${RED}${RAM_GB}GB < 8GB${NC}"
fi

# Check for dedicated GPU
if command -v nvidia-smi &> /dev/null || (command -v lspci &> /dev/null && lspci | grep -i vga | grep -q -i amd); then
    echo -e "✅ GPU: ${GREEN}Dedicated graphics detected${NC}"
    REQUIREMENTS_MET=$((REQUIREMENTS_MET + 1))
else
    echo -e "⚠️ GPU: ${YELLOW}Integrated graphics may impact performance${NC}"
fi

# Check for Vulkan support
if command -v vulkaninfo &> /dev/null && vulkaninfo | grep -q "Vulkan Instance"; then
    echo -e "✅ Vulkan: ${GREEN}Support detected${NC}"
    REQUIREMENTS_MET=$((REQUIREMENTS_MET + 1))
else
    echo -e "❌ Vulkan: ${RED}Not available or not configured${NC}"
fi

# Project-Specific Performance Metrics
echo -e "\n${YELLOW}BikeAdventure Performance Targets${NC}"
echo "----------------------------------"

# Memory Budget Check (4GB target)
AVAILABLE_RAM_GB=$(free -g | grep "Mem:" | awk '{print $7}')
if [ "$AVAILABLE_RAM_GB" -ge 4 ]; then
    echo -e "✅ Memory Budget: ${GREEN}${AVAILABLE_RAM_GB}GB available >= 4GB target${NC}"
else
    echo -e "⚠️ Memory Budget: ${YELLOW}${AVAILABLE_RAM_GB}GB available < 4GB target${NC}"
fi

# Performance Optimization Features
echo -e "\n${YELLOW}UE5 Performance Features (from Config)${NC}"
echo "--------------------------------------"

# Check DefaultEngine.ini for performance settings
if [ -f "$PROJECT_ROOT/Config/DefaultEngine.ini" ]; then
    if grep -q "DefaultGraphicsRHI=DefaultGraphicsRHI_DX12" "$PROJECT_ROOT/Config/DefaultEngine.ini"; then
        echo -e "✅ Graphics API: ${GREEN}DirectX 12 enabled${NC}"
    fi
    
    if grep -q "r.ReflectionMethod=1" "$PROJECT_ROOT/Config/DefaultEngine.ini"; then
        echo -e "✅ Reflections: ${GREEN}Lumen enabled${NC}"
    fi
    
    if grep -q "r.DynamicGlobalIlluminationMethod=1" "$PROJECT_ROOT/Config/DefaultEngine.ini"; then
        echo -e "✅ Global Illumination: ${GREEN}Lumen enabled${NC}"
    fi
    
    if grep -q "bEnableChaosVehicles=true" "$PROJECT_ROOT/Config/DefaultEngine.ini"; then
        echo -e "✅ Physics: ${GREEN}Chaos Physics enabled${NC}"
    fi
else
    echo -e "❌ Configuration: ${RED}DefaultEngine.ini not found${NC}"
fi

# Movement Component Performance Analysis
echo -e "\n${YELLOW}Movement Component Performance Analysis${NC}"
echo "---------------------------------------"

if [ -f "$PROJECT_ROOT/Source/BikeAdventure/Systems/BikeMovementComponent.cpp" ]; then
    # Check for performance-critical settings
    if grep -q "ForwardSpeed = 1200.0f" "$PROJECT_ROOT/Source/BikeAdventure/Systems/BikeMovementComponent.cpp"; then
        echo -e "✅ Forward Speed: ${GREEN}1200 cm/s (12 m/s) - optimal for exploration${NC}"
    fi
    
    if grep -q "MaxTurnRate = 45.0f" "$PROJECT_ROOT/Source/BikeAdventure/Systems/BikeMovementComponent.cpp"; then
        echo -e "✅ Turn Rate: ${GREEN}45°/sec - smooth, non-jarring${NC}"
    fi
    
    if grep -q "UpdateMovement.*DeltaTime" "$PROJECT_ROOT/Source/BikeAdventure/Systems/BikeMovementComponent.cpp"; then
        echo -e "✅ Frame Rate Independence: ${GREEN}DeltaTime-based calculations${NC}"
    fi
else
    echo -e "❌ Movement Component: ${RED}Not found${NC}"
fi

# Estimated Performance
echo -e "\n${YELLOW}Performance Estimation${NC}"
echo "----------------------"

PERFORMANCE_SCORE=$((REQUIREMENTS_MET * 25))

if [ "$PERFORMANCE_SCORE" -ge 75 ]; then
    echo -e "🚀 Performance Estimate: ${GREEN}60+ FPS Expected${NC}"
    echo -e "   ${GREEN}System meets or exceeds all requirements${NC}"
elif [ "$PERFORMANCE_SCORE" -ge 50 ]; then
    echo -e "⚡ Performance Estimate: ${YELLOW}45-60 FPS Expected${NC}"
    echo -e "   ${YELLOW}System meets most requirements${NC}"
else
    echo -e "⚠️ Performance Estimate: ${RED}30-45 FPS Expected${NC}"
    echo -e "   ${RED}Consider hardware upgrades for optimal experience${NC}"
fi

# Optimization Recommendations
echo -e "\n${YELLOW}Optimization Recommendations${NC}"
echo "-----------------------------"

if [ "$CPU_CORES" -lt 6 ]; then
    echo -e "• ${CYAN}CPU:${NC} Consider disabling heavy background processes during play"
fi

if [ "$RAM_GB" -lt 16 ]; then
    echo -e "• ${CYAN}RAM:${NC} Enable virtual memory/swap for memory headroom"
fi

if ! command -v nvidia-smi &> /dev/null; then
    echo -e "• ${CYAN}GPU:${NC} Use lower quality settings for integrated graphics"
fi

echo -e "\n• ${CYAN}UE5 Settings:${NC} Use 'Medium' quality preset for 60+ FPS target"
echo -e "• ${CYAN}Resolution:${NC} 1920x1080 recommended for stable performance"
echo -e "• ${CYAN}VSync:${NC} Enable to prevent screen tearing"

# Build Performance Tips
echo -e "\n${YELLOW}Build Performance Tips${NC}"
echo "----------------------"
echo -e "• Use ${CYAN}Development${NC} configuration for testing"
echo -e "• Use ${CYAN}Shipping${NC} configuration for final performance validation"
echo -e "• Enable ${CYAN}Unity Build${NC} for faster compilation"
echo -e "• Use ${CYAN}Incremental Builds${NC} during development"

echo -e "\n${CYAN}Performance validation complete!${NC}"
echo -e "Run this script after each major optimization to track improvements."