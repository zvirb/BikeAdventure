#!/bin/bash
# Stream 5 Deployment Infrastructure Validation Script
# Validates all components of the deployment system

set -euo pipefail

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

print_header() {
    echo -e "${BLUE}"
    cat << 'EOF'
╔════════════════════════════════════════════════════════════╗
║           BikeAdventure Deployment Infrastructure          ║
║                    Validation Script                      ║
╚════════════════════════════════════════════════════════════╝
EOF
    echo -e "${NC}\n"
}

validate_file() {
    local file_path="$1"
    local description="$2"
    
    if [ -f "$file_path" ]; then
        echo -e "✅ ${GREEN}$description${NC} - Found: $file_path"
        return 0
    else
        echo -e "❌ ${RED}$description${NC} - Missing: $file_path"
        return 1
    fi
}

validate_directory() {
    local dir_path="$1"
    local description="$2"
    
    if [ -d "$dir_path" ]; then
        echo -e "✅ ${GREEN}$description${NC} - Found: $dir_path"
        return 0
    else
        echo -e "❌ ${RED}$description${NC} - Missing: $dir_path"
        return 1
    fi
}

validate_executable() {
    local file_path="$1"
    local description="$2"
    
    if [ -f "$file_path" ] && [ -x "$file_path" ]; then
        echo -e "✅ ${GREEN}$description${NC} - Executable: $file_path"
        return 0
    else
        echo -e "❌ ${RED}$description${NC} - Not executable: $file_path"
        return 1
    fi
}

check_python_dependencies() {
    echo -e "${BLUE}Checking Python Dependencies:${NC}"
    
    local deps=("psutil" "pathlib")
    local all_present=true
    
    for dep in "${deps[@]}"; do
        if python3 -c "import $dep" 2>/dev/null; then
            echo -e "✅ ${GREEN}Python module: $dep${NC}"
        else
            echo -e "❌ ${RED}Python module: $dep (install with: pip3 install $dep)${NC}"
            all_present=false
        fi
    done
    
    return $([[ "$all_present" == "true" ]])
}

validate_deployment_infrastructure() {
    echo -e "${BLUE}=== Validating CI/CD Infrastructure ===${NC}"
    local ci_score=0
    local ci_total=0
    
    ((ci_total++))
    if validate_file "$PROJECT_ROOT/.github/workflows/ci-cd-multiplatform.yml" "GitHub Actions CI/CD Workflow"; then
        ((ci_score++))
    fi
    
    ((ci_total++))
    if validate_file "$PROJECT_ROOT/.gitattributes" "Git LFS Configuration"; then
        ((ci_score++))
    fi
    
    echo -e "\n${BLUE}=== Validating Build Automation ===${NC}"
    local build_score=0
    local build_total=0
    
    ((build_total++))
    if validate_executable "$PROJECT_ROOT/scripts/automation/build-all-platforms.sh" "Enhanced Build Script"; then
        ((build_score++))
    fi
    
    echo -e "\n${BLUE}=== Validating Engine Configurations ===${NC}"
    local config_score=0
    local config_total=0
    
    ((config_total++))
    if validate_file "$PROJECT_ROOT/config/engine/DefaultEngine-Linux.ini" "Linux Engine Configuration"; then
        ((config_score++))
    fi
    
    ((config_total++))
    if validate_file "$PROJECT_ROOT/config/engine/DefaultEngine-Windows.ini" "Windows Engine Configuration"; then
        ((config_score++))
    fi
    
    ((config_total++))
    if validate_file "$PROJECT_ROOT/config/engine/Performance-Optimization.ini" "Performance Optimization Config"; then
        ((config_score++))
    fi
    
    echo -e "\n${BLUE}=== Validating Asset Management ===${NC}"
    local asset_score=0
    local asset_total=0
    
    ((asset_total++))
    if validate_file "$PROJECT_ROOT/config/engine/AssetPackaging.ini" "Asset Packaging Configuration"; then
        ((asset_score++))
    fi
    
    ((asset_total++))
    if validate_executable "$PROJECT_ROOT/scripts/automation/optimize-assets.sh" "Asset Optimization Script"; then
        ((asset_score++))
    fi
    
    echo -e "\n${BLUE}=== Validating Verification Systems ===${NC}"
    local verify_score=0
    local verify_total=0
    
    ((verify_total++))
    if validate_executable "$PROJECT_ROOT/scripts/automation/deployment-verification.py" "Deployment Verification Script"; then
        ((verify_score++))
        
        # Test Python script syntax
        if python3 -m py_compile "$PROJECT_ROOT/scripts/automation/deployment-verification.py" 2>/dev/null; then
            echo -e "✅ ${GREEN}Deployment verification script syntax valid${NC}"
        else
            echo -e "⚠️ ${YELLOW}Deployment verification script has syntax issues${NC}"
        fi
    fi
    
    echo -e "\n${BLUE}=== Validating Monitoring Systems ===${NC}"
    local monitor_score=0
    local monitor_total=0
    
    ((monitor_total++))
    if validate_executable "$PROJECT_ROOT/scripts/automation/deployment-monitor.py" "Deployment Monitor Script"; then
        ((monitor_score++))
        
        # Test Python script syntax
        if python3 -m py_compile "$PROJECT_ROOT/scripts/automation/deployment-monitor.py" 2>/dev/null; then
            echo -e "✅ ${GREEN}Deployment monitor script syntax valid${NC}"
        else
            echo -e "⚠️ ${YELLOW}Deployment monitor script has syntax issues${NC}"
        fi
    fi
    
    echo -e "\n${BLUE}=== Checking Dependencies ===${NC}"
    local deps_ok=true
    if ! check_python_dependencies; then
        deps_ok=false
    fi
    
    # Calculate total scores
    local total_score=$((ci_score + build_score + config_score + asset_score + verify_score + monitor_score))
    local total_possible=$((ci_total + build_total + config_total + asset_total + verify_total + monitor_total))
    local success_rate=$((total_score * 100 / total_possible))
    
    echo -e "\n${BLUE}=== Validation Summary ===${NC}"
    echo "CI/CD Infrastructure: $ci_score/$ci_total"
    echo "Build Automation: $build_score/$build_total"  
    echo "Engine Configurations: $config_score/$config_total"
    echo "Asset Management: $asset_score/$asset_total"
    echo "Verification Systems: $verify_score/$verify_total"
    echo "Monitoring Systems: $monitor_score/$monitor_total"
    echo "Dependencies: $($deps_ok && echo "✅ OK" || echo "❌ Missing")"
    
    echo -e "\n${BLUE}=== Overall Results ===${NC}"
    echo "Components: $total_score/$total_possible ($success_rate%)"
    
    if [ $success_rate -eq 100 ] && [ "$deps_ok" == "true" ]; then
        echo -e "🎉 ${GREEN}ALL DEPLOYMENT INFRASTRUCTURE COMPONENTS VALIDATED SUCCESSFULLY!${NC}"
        echo -e "${GREEN}The BikeAdventure deployment system is ready for use.${NC}"
        return 0
    elif [ $success_rate -ge 80 ]; then
        echo -e "⚠️ ${YELLOW}Deployment infrastructure mostly complete ($success_rate%).${NC}"
        echo -e "${YELLOW}Address missing components before production deployment.${NC}"
        return 1
    else
        echo -e "❌ ${RED}Deployment infrastructure incomplete ($success_rate%).${NC}"
        echo -e "${RED}Significant setup required before deployment.${NC}"
        return 1
    fi
}

test_basic_functionality() {
    echo -e "\n${BLUE}=== Testing Basic Functionality ===${NC}"
    
    # Test deployment verification script help
    if [ -x "$PROJECT_ROOT/scripts/automation/deployment-verification.py" ]; then
        echo "Testing deployment verification script..."
        if python3 "$PROJECT_ROOT/scripts/automation/deployment-verification.py" --help >/dev/null 2>&1; then
            echo -e "✅ ${GREEN}Deployment verification script help works${NC}"
        else
            echo -e "❌ ${RED}Deployment verification script help failed${NC}"
        fi
    fi
    
    # Test deployment monitor script help
    if [ -x "$PROJECT_ROOT/scripts/automation/deployment-monitor.py" ]; then
        echo "Testing deployment monitor script..."
        if python3 "$PROJECT_ROOT/scripts/automation/deployment-monitor.py" --help >/dev/null 2>&1; then
            echo -e "✅ ${GREEN}Deployment monitor script help works${NC}"
        else
            echo -e "❌ ${RED}Deployment monitor script help failed${NC}"
        fi
    fi
    
    # Test build script help
    if [ -x "$PROJECT_ROOT/scripts/automation/build-all-platforms.sh" ]; then
        echo "Testing build script..."
        if "$PROJECT_ROOT/scripts/automation/build-all-platforms.sh" --help >/dev/null 2>&1; then
            echo -e "✅ ${GREEN}Build script help works${NC}"
        else
            echo -e "❌ ${RED}Build script help failed${NC}"
        fi
    fi
    
    # Test asset optimization script help
    if [ -x "$PROJECT_ROOT/scripts/automation/optimize-assets.sh" ]; then
        echo "Testing asset optimization script..."
        if "$PROJECT_ROOT/scripts/automation/optimize-assets.sh" --help >/dev/null 2>&1; then
            echo -e "✅ ${GREEN}Asset optimization script help works${NC}"
        else
            echo -e "❌ ${RED}Asset optimization script help failed${NC}"
        fi
    fi
}

generate_validation_report() {
    echo -e "\n${BLUE}=== Generating Validation Report ===${NC}"
    
    local report_file="$PROJECT_ROOT/deployment-infrastructure-validation-$(date +%Y%m%d-%H%M%S).json"
    
    cat > "$report_file" << EOF
{
  "validation_report": {
    "timestamp": "$(date -Iseconds)",
    "project_root": "$PROJECT_ROOT",
    "validator_version": "1.0.0",
    "components_validated": [
      {
        "category": "CI/CD Infrastructure",
        "components": [
          {"name": "GitHub Actions Workflow", "status": "$([ -f "$PROJECT_ROOT/.github/workflows/ci-cd-multiplatform.yml" ] && echo "present" || echo "missing")"},
          {"name": "Git LFS Configuration", "status": "$([ -f "$PROJECT_ROOT/.gitattributes" ] && echo "present" || echo "missing")"}
        ]
      },
      {
        "category": "Build Automation",
        "components": [
          {"name": "Enhanced Build Script", "status": "$([ -x "$PROJECT_ROOT/scripts/automation/build-all-platforms.sh" ] && echo "executable" || echo "missing")"}
        ]
      },
      {
        "category": "Engine Configurations",
        "components": [
          {"name": "Linux Engine Config", "status": "$([ -f "$PROJECT_ROOT/config/engine/DefaultEngine-Linux.ini" ] && echo "present" || echo "missing")"},
          {"name": "Windows Engine Config", "status": "$([ -f "$PROJECT_ROOT/config/engine/DefaultEngine-Windows.ini" ] && echo "present" || echo "missing")"},
          {"name": "Performance Optimization", "status": "$([ -f "$PROJECT_ROOT/config/engine/Performance-Optimization.ini" ] && echo "present" || echo "missing")"}
        ]
      },
      {
        "category": "Asset Management",
        "components": [
          {"name": "Asset Packaging Config", "status": "$([ -f "$PROJECT_ROOT/config/engine/AssetPackaging.ini" ] && echo "present" || echo "missing")"},
          {"name": "Asset Optimization Script", "status": "$([ -x "$PROJECT_ROOT/scripts/automation/optimize-assets.sh" ] && echo "executable" || echo "missing")"}
        ]
      },
      {
        "category": "Verification Systems",
        "components": [
          {"name": "Deployment Verification", "status": "$([ -x "$PROJECT_ROOT/scripts/automation/deployment-verification.py" ] && echo "executable" || echo "missing")"}
        ]
      },
      {
        "category": "Monitoring Systems", 
        "components": [
          {"name": "Deployment Monitor", "status": "$([ -x "$PROJECT_ROOT/scripts/automation/deployment-monitor.py" ] && echo "executable" || echo "missing")"}
        ]
      }
    ],
    "next_steps": [
      "Configure GitHub Actions with proper secrets and environment variables",
      "Initialize Git LFS in repository for asset management",
      "Set up UE5 installation paths in build environment",
      "Test deployment pipeline with sample builds",
      "Configure monitoring alerts and thresholds"
    ]
  }
}
EOF
    
    echo -e "📋 Validation report saved: ${BLUE}$report_file${NC}"
}

main() {
    print_header
    
    echo "Project Root: $PROJECT_ROOT"
    echo "Validation Date: $(date)"
    echo ""
    
    if validate_deployment_infrastructure; then
        echo -e "\n${GREEN}🚀 DEPLOYMENT INFRASTRUCTURE VALIDATION PASSED${NC}"
        test_basic_functionality
        generate_validation_report
        
        echo -e "\n${GREEN}Ready to proceed with BikeAdventure deployment!${NC}"
        exit 0
    else
        echo -e "\n${YELLOW}⚠️ DEPLOYMENT INFRASTRUCTURE NEEDS ATTENTION${NC}"
        generate_validation_report
        
        echo -e "\n${YELLOW}Please address missing components before deployment.${NC}"
        exit 1
    fi
}

main "$@"