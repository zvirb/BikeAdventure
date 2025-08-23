#!/bin/bash
# BikeAdventure Project Setup Validation Script
# Validates that all components of Stream 1 are correctly implemented

PROJECT_ROOT="/home/marku/Documents/programming/BikeAdventure"
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}BikeAdventure Stream 1 Validation${NC}"
echo "======================================"

# Function to check file existence and report
check_file() {
    if [ -f "$1" ]; then
        echo -e "✅ ${GREEN}$2${NC}"
        return 0
    else
        echo -e "❌ ${RED}Missing: $2${NC}"
        return 1
    fi
}

# Function to check directory existence
check_directory() {
    if [ -d "$1" ]; then
        echo -e "✅ ${GREEN}$2${NC}"
        return 0
    else
        echo -e "❌ ${RED}Missing: $2${NC}"
        return 1
    fi
}

# Validation counters
TOTAL_CHECKS=0
PASSED_CHECKS=0

echo -e "\n${YELLOW}1. Core UE5 Project Files${NC}"
echo "-------------------------"

check_file "$PROJECT_ROOT/BikeAdventure.uproject" "UE5 Project File"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_file "$PROJECT_ROOT/Source/BikeAdventure/BikeAdventure.h" "Main Module Header"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_file "$PROJECT_ROOT/Source/BikeAdventure/BikeAdventure.cpp" "Main Module Implementation"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_file "$PROJECT_ROOT/Source/BikeAdventure/BikeAdventure.Build.cs" "Build Configuration"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

echo -e "\n${YELLOW}2. Core C++ Classes${NC}"
echo "------------------"

check_file "$PROJECT_ROOT/Source/BikeAdventure/Core/BikeAdventureGameMode.h" "Game Mode Header"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_file "$PROJECT_ROOT/Source/BikeAdventure/Core/BikeAdventureGameMode.cpp" "Game Mode Implementation"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_file "$PROJECT_ROOT/Source/BikeAdventure/Core/BikeCharacter.h" "Bike Character Header"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_file "$PROJECT_ROOT/Source/BikeAdventure/Core/BikeCharacter.cpp" "Bike Character Implementation"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

echo -e "\n${YELLOW}3. Systems Components${NC}"
echo "--------------------"

check_file "$PROJECT_ROOT/Source/BikeAdventure/Systems/BikeMovementComponent.h" "Movement Component Header"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_file "$PROJECT_ROOT/Source/BikeAdventure/Systems/BikeMovementComponent.cpp" "Movement Component Implementation"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_file "$PROJECT_ROOT/Source/BikeAdventure/Systems/IntersectionManager.h" "Intersection Manager Header"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_file "$PROJECT_ROOT/Source/BikeAdventure/Systems/IntersectionManager.cpp" "Intersection Manager Implementation"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_file "$PROJECT_ROOT/Source/BikeAdventure/Systems/BiomeGenerator.h" "Biome Generator Header"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_file "$PROJECT_ROOT/Source/BikeAdventure/Systems/BiomeGenerator.cpp" "Biome Generator Implementation"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

echo -e "\n${YELLOW}4. Gameplay Classes${NC}"
echo "------------------"

check_file "$PROJECT_ROOT/Source/BikeAdventure/Gameplay/Intersection.h" "Intersection Actor Header"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_file "$PROJECT_ROOT/Source/BikeAdventure/Gameplay/Intersection.cpp" "Intersection Actor Implementation"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

echo -e "\n${YELLOW}5. Configuration Files${NC}"
echo "---------------------"

check_file "$PROJECT_ROOT/Config/DefaultEngine.ini" "Engine Configuration"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_file "$PROJECT_ROOT/Config/DefaultGame.ini" "Game Configuration"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_file "$PROJECT_ROOT/Config/DefaultInput.ini" "Input Configuration"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

echo -e "\n${YELLOW}6. Build Configuration${NC}"
echo "---------------------"

check_file "$PROJECT_ROOT/Source/BikeAdventure.Target.cs" "Game Build Target"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_file "$PROJECT_ROOT/Source/BikeAdventureEditor.Target.cs" "Editor Build Target"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

echo -e "\n${YELLOW}7. Unit Tests${NC}"
echo "-------------"

check_file "$PROJECT_ROOT/Tests/Unit/BikeMovementTest.cpp" "Bike Movement Tests"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_file "$PROJECT_ROOT/Tests/Unit/IntersectionTest.cpp" "Intersection Tests"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

echo -e "\n${YELLOW}8. Directory Structure${NC}"
echo "---------------------"

check_directory "$PROJECT_ROOT/Source" "Source Directory"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_directory "$PROJECT_ROOT/Content" "Content Directory"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_directory "$PROJECT_ROOT/Config" "Config Directory"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

check_directory "$PROJECT_ROOT/Tests" "Tests Directory"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
[ $? -eq 0 ] && PASSED_CHECKS=$((PASSED_CHECKS + 1))

# Summary
echo -e "\n${CYAN}Validation Summary${NC}"
echo "=================="
echo -e "Total Checks: ${YELLOW}$TOTAL_CHECKS${NC}"
echo -e "Passed: ${GREEN}$PASSED_CHECKS${NC}"
echo -e "Failed: ${RED}$((TOTAL_CHECKS - PASSED_CHECKS))${NC}"

PERCENTAGE=$(( (PASSED_CHECKS * 100) / TOTAL_CHECKS ))
echo -e "Completion: ${CYAN}$PERCENTAGE%${NC}"

if [ $PASSED_CHECKS -eq $TOTAL_CHECKS ]; then
    echo -e "\n🎉 ${GREEN}All Stream 1 components implemented successfully!${NC}"
    echo -e "${GREEN}Ready for UE5 compilation and testing.${NC}"
    exit 0
else
    echo -e "\n⚠️ ${YELLOW}Some components are missing. Check the failed items above.${NC}"
    exit 1
fi