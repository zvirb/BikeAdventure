#!/bin/bash
# Enhanced build-all-platforms.sh for BikeAdventure UE5 Project
# Multi-platform build automation with comprehensive error handling and validation

set -euo pipefail  # Exit on error, undefined variables, pipe failures

# ==============================================
# BUILD CONFIGURATION
# ==============================================

PROJECT_NAME="BikeAdventure"
VERSION="${VERSION:-1.0.0}"
BUILD_NUMBER="${GITHUB_RUN_NUMBER:-$(date +%Y%m%d%H%M%S)}"
BUILD_CONFIG="${BUILD_CONFIG:-Development}"
UE5_ROOT="${UE5_ROOT:-/opt/UnrealEngine}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Colors and formatting
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'
BOLD='\033[1m'

# Build configuration
PLATFORMS=("Linux" "Win64")
PARALLEL_BUILDS="${PARALLEL_BUILDS:-false}"
SKIP_TESTS="${SKIP_TESTS:-false}"
CLEAN_BUILD="${CLEAN_BUILD:-false}"
VERBOSE="${VERBOSE:-false}"

# Logging
LOG_DIR="$PROJECT_ROOT/Builds/logs"
BUILD_LOG="$LOG_DIR/build-$(date +%Y%m%d-%H%M%S).log"
ERROR_LOG="$LOG_DIR/error-$(date +%Y%m%d-%H%M%S).log"

# ==============================================
# LOGGING AND UTILITY FUNCTIONS
# ==============================================

setup_logging() {
    mkdir -p "$LOG_DIR"
    exec 1> >(tee -a "$BUILD_LOG")
    exec 2> >(tee -a "$ERROR_LOG" >&2)
    
    log_info "Build logging initialized"
    log_info "Build log: $BUILD_LOG"
    log_info "Error log: $ERROR_LOG"
}

log_info() {
    echo -e "${CYAN}[INFO]${NC} $(date '+%H:%M:%S') $*" | tee -a "$BUILD_LOG"
}

log_warning() {
    echo -e "${YELLOW}[WARN]${NC} $(date '+%H:%M:%S') $*" | tee -a "$BUILD_LOG"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $(date '+%H:%M:%S') $*" | tee -a "$BUILD_LOG" "$ERROR_LOG"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $(date '+%H:%M:%S') $*" | tee -a "$BUILD_LOG"
}

log_header() {
    echo -e "\n${BOLD}${MAGENTA}==== $* ====${NC}\n" | tee -a "$BUILD_LOG"
}

# Spinner for long-running operations
spinner() {
    local pid=$1
    local delay=0.1
    local spinstr='|/-\'
    while [ "$(ps a | awk '{print $1}' | grep $pid)" ]; do
        local temp=${spinstr#?}
        printf " [%c]  " "$spinstr"
        local spinstr=$temp${spinstr%"$temp"}
        sleep $delay
        printf "\b\b\b\b\b\b"
    done
    printf "    \b\b\b\b"
}

# ==============================================
# VALIDATION FUNCTIONS
# ==============================================

validate_environment() {
    log_header "Environment Validation"
    
    # Check if we're in the correct directory
    if [ ! -f "$PROJECT_ROOT/$PROJECT_NAME.uproject" ]; then
        log_error "$PROJECT_NAME.uproject not found in $PROJECT_ROOT"
        log_error "Please run this script from the project root directory"
        exit 1
    fi
    log_success "Project file validated"
    
    # Check UE5 installation
    if [ ! -d "$UE5_ROOT" ]; then
        log_error "UE5 installation not found at $UE5_ROOT"
        log_error "Set UE5_ROOT environment variable or install UE5"
        exit 1
    fi
    log_success "UE5 installation found at $UE5_ROOT"
    
    # Check build tools
    local build_tools=("$UE5_ROOT/Engine/Build/BatchFiles/Linux/RunUAT.sh" 
                       "$UE5_ROOT/Engine/Binaries/Linux/UnrealEditor")
    
    for tool in "${build_tools[@]}"; do
        if [ ! -f "$tool" ]; then
            log_error "Build tool not found: $tool"
            exit 1
        fi
    done
    log_success "Build tools validated"
    
    # Check system requirements
    local required_space_gb=10
    local available_space_gb=$(df "$PROJECT_ROOT" | awk 'NR==2 {print int($4/1024/1024)}')
    
    if [ "$available_space_gb" -lt "$required_space_gb" ]; then
        log_warning "Low disk space: ${available_space_gb}GB available, ${required_space_gb}GB recommended"
    else
        log_success "Disk space check passed: ${available_space_gb}GB available"
    fi
    
    # Check memory
    local available_memory_gb=$(free -g | awk 'NR==2{print $7}')
    local required_memory_gb=8
    
    if [ "$available_memory_gb" -lt "$required_memory_gb" ]; then
        log_warning "Low available memory: ${available_memory_gb}GB available, ${required_memory_gb}GB recommended"
    else
        log_success "Memory check passed: ${available_memory_gb}GB available"
    fi
    
    # Validate project structure
    local required_dirs=("Source" "Content" "Config")
    for dir in "${required_dirs[@]}"; do
        if [ ! -d "$PROJECT_ROOT/$dir" ]; then
            log_error "Required directory not found: $dir"
            exit 1
        fi
    done
    log_success "Project structure validated"
    
    log_success "Environment validation completed"
}

check_git_status() {
    log_header "Git Status Check"
    
    if command -v git >/dev/null 2>&1 && [ -d "$PROJECT_ROOT/.git" ]; then
        local git_status=$(git status --porcelain 2>/dev/null | wc -l)
        local current_branch=$(git branch --show-current 2>/dev/null || echo "unknown")
        local commit_hash=$(git rev-parse --short HEAD 2>/dev/null || echo "unknown")
        
        log_info "Current branch: $current_branch"
        log_info "Commit hash: $commit_hash"
        
        if [ "$git_status" -gt 0 ]; then
            log_warning "Repository has uncommitted changes ($git_status files)"
        else
            log_success "Repository is clean"
        fi
    else
        log_warning "Not a git repository or git not available"
    fi
}

# ==============================================
# DEPENDENCY MANAGEMENT
# ==============================================

update_git_lfs() {
    log_header "Git LFS Management"
    
    if command -v git-lfs >/dev/null 2>&1; then
        log_info "Pulling latest LFS assets..."
        if git lfs pull 2>/dev/null; then
            log_success "Git LFS assets updated"
        else
            log_warning "Failed to update Git LFS assets (may not be critical)"
        fi
    else
        log_warning "Git LFS not installed - large assets may be missing"
    fi
}

clean_intermediate_files() {
    if [ "$CLEAN_BUILD" = "true" ]; then
        log_header "Cleaning Intermediate Files"
        
        local clean_dirs=("Binaries" "Intermediate" "DerivedDataCache" "Saved/Cooked")
        
        for dir in "${clean_dirs[@]}"; do
            if [ -d "$PROJECT_ROOT/$dir" ]; then
                log_info "Cleaning $dir..."
                rm -rf "$PROJECT_ROOT/$dir"
                log_success "Cleaned $dir"
            fi
        done
    fi
}

# ==============================================
# BUILD FUNCTIONS
# ==============================================

generate_project_files() {
    log_header "Generating Project Files"
    
    local ubt_path="$UE5_ROOT/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool"
    
    log_info "Generating project files for $PROJECT_NAME..."
    
    if [ "$VERBOSE" = "true" ]; then
        "$ubt_path" -projectfiles -project="$PROJECT_ROOT/$PROJECT_NAME.uproject" -game -rocket -progress
    else
        "$ubt_path" -projectfiles -project="$PROJECT_ROOT/$PROJECT_NAME.uproject" -game -rocket -progress >/dev/null 2>&1
    fi
    
    local exit_code=$?
    if [ $exit_code -eq 0 ]; then
        log_success "Project files generated successfully"
    else
        log_error "Failed to generate project files (exit code: $exit_code)"
        exit 1
    fi
}

build_platform() {
    local platform=$1
    local start_time=$(date +%s)
    
    log_header "Building $PROJECT_NAME for $platform"
    
    # Platform-specific configurations
    local build_script="$UE5_ROOT/Engine/Build/BatchFiles/Linux/RunUAT.sh"
    local archive_dir="$PROJECT_ROOT/Builds/$platform-$BUILD_CONFIG-$BUILD_NUMBER"
    
    if [ "$platform" = "Win64" ] && [ -f "$UE5_ROOT/Engine/Build/BatchFiles/RunUAT.bat" ]; then
        build_script="$UE5_ROOT/Engine/Build/BatchFiles/RunUAT.bat"
    fi
    
    # Ensure builds directory exists
    mkdir -p "$PROJECT_ROOT/Builds"
    
    log_info "Building for $platform with $BUILD_CONFIG configuration..."
    log_info "Archive directory: $archive_dir"
    
    # Build command with comprehensive options
    local build_cmd=(
        "$build_script" BuildCookRun
        -project="$PROJECT_ROOT/$PROJECT_NAME.uproject"
        -platform="$platform"
        -configuration="$BUILD_CONFIG"
        -clientconfig="$BUILD_CONFIG"
        -serverconfig="$BUILD_CONFIG"
        -build -cook -pak -stage
        -archive -archivedirectory="$archive_dir"
        -noP4 -nodebuginfo -utf8output
        -prereqs -distribution
    )
    
    # Add verbose output if requested
    if [ "$VERBOSE" = "true" ]; then
        build_cmd+=(-verbose)
    fi
    
    # Execute build
    log_info "Executing build command..."
    "${build_cmd[@]}" > "$LOG_DIR/build-$platform-$(date +%H%M%S).log" 2>&1 &
    local build_pid=$!
    
    # Show spinner for long builds
    if [ "$VERBOSE" != "true" ]; then
        spinner $build_pid
    fi
    
    # Wait for build to complete
    wait $build_pid
    local exit_code=$?
    
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    if [ $exit_code -eq 0 ]; then
        log_success "$platform build completed successfully in ${duration}s"
        
        # Verify build output
        if [ -d "$archive_dir" ]; then
            local build_size=$(du -sh "$archive_dir" | cut -f1)
            log_info "$platform build size: $build_size"
        else
            log_error "$platform build directory not found: $archive_dir"
            return 1
        fi
    else
        log_error "$platform build failed after ${duration}s (exit code: $exit_code)"
        log_error "Check build log: $LOG_DIR/build-$platform-*.log"
        return 1
    fi
    
    return 0
}

run_platform_tests() {
    local platform=$1
    
    if [ "$SKIP_TESTS" = "true" ]; then
        log_info "Skipping tests for $platform (SKIP_TESTS=true)"
        return 0
    fi
    
    log_header "Running Tests for $platform"
    
    local test_executable="$UE5_ROOT/Engine/Binaries/Linux/UnrealEditor"
    local test_output_dir="$PROJECT_ROOT/TestResults/$platform"
    
    if [ "$platform" = "Win64" ] && [ -f "$UE5_ROOT/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" ]; then
        test_executable="$UE5_ROOT/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
    fi
    
    mkdir -p "$test_output_dir"
    
    log_info "Running automated tests for $platform..."
    
    # Run tests with timeout
    timeout 600 "$test_executable" \
        "$PROJECT_ROOT/$PROJECT_NAME.uproject" \
        -ExecCmds="Automation RunTests BikeAdventure.Unit+BikeAdventure.Integration+BikeAdventure.Performance;Quit" \
        -unattended -nullrhi -nosplash \
        -ReportOutputDir="$test_output_dir" \
        > "$LOG_DIR/test-$platform-$(date +%H%M%S).log" 2>&1
        
    local exit_code=$?
    
    if [ $exit_code -eq 0 ]; then
        log_success "$platform tests passed"
        
        # Parse test results if available
        local test_report="$test_output_dir/index.html"
        if [ -f "$test_report" ]; then
            log_info "Test report available: $test_report"
        fi
    else
        log_warning "$platform tests failed or timed out (exit code: $exit_code)"
        log_warning "This may not prevent deployment, but should be investigated"
    fi
    
    return 0  # Don't fail build on test failures
}

# ==============================================
# PACKAGING AND VERIFICATION
# ==============================================

create_deployment_packages() {
    log_header "Creating Deployment Packages"
    
    cd "$PROJECT_ROOT/Builds" || exit 1
    
    for platform in "${PLATFORMS[@]}"; do
        local build_dir="$platform-$BUILD_CONFIG-$BUILD_NUMBER"
        local package_name="$PROJECT_NAME-$platform-v$VERSION-$BUILD_NUMBER"
        
        if [ -d "$build_dir" ]; then
            log_info "Packaging $platform build..."
            
            # Create compressed archive
            if command -v tar >/dev/null 2>&1; then
                tar -czf "$package_name.tar.gz" "$build_dir"
                local tar_exit=$?
                
                if [ $tar_exit -eq 0 ]; then
                    local package_size=$(du -sh "$package_name.tar.gz" | cut -f1)
                    log_success "Created $package_name.tar.gz ($package_size)"
                    
                    # Generate checksums
                    if command -v sha256sum >/dev/null 2>&1; then
                        sha256sum "$package_name.tar.gz" > "$package_name.sha256"
                        log_info "Generated checksum: $package_name.sha256"
                    fi
                else
                    log_error "Failed to create package for $platform"
                fi
            else
                log_error "tar command not available for packaging"
            fi
        else
            log_warning "Build directory not found for $platform: $build_dir"
        fi
    done
    
    # List all packages
    log_info "Generated packages:"
    ls -la *.tar.gz *.sha256 2>/dev/null || log_warning "No packages found"
}

verify_build_integrity() {
    log_header "Build Integrity Verification"
    
    local verification_passed=true
    
    for platform in "${PLATFORMS[@]}"; do
        local build_dir="$PROJECT_ROOT/Builds/$platform-$BUILD_CONFIG-$BUILD_NUMBER"
        
        log_info "Verifying $platform build integrity..."
        
        # Check if build directory exists
        if [ ! -d "$build_dir" ]; then
            log_error "$platform build directory not found"
            verification_passed=false
            continue
        fi
        
        # Check for essential files
        local essential_files=("$PROJECT_NAME.uproject")
        
        if [ "$platform" = "Linux" ]; then
            essential_files+=("LinuxNoEditor/$PROJECT_NAME/Binaries/Linux/$PROJECT_NAME")
        elif [ "$platform" = "Win64" ]; then
            essential_files+=("WindowsNoEditor/$PROJECT_NAME/Binaries/Win64/$PROJECT_NAME.exe")
        fi
        
        for file in "${essential_files[@]}"; do
            if [ ! -e "$build_dir/$file" ]; then
                log_error "Essential file missing in $platform build: $file"
                verification_passed=false
            fi
        done
        
        # Check build size (should be reasonable)
        local build_size_bytes=$(du -sb "$build_dir" | cut -f1)
        local build_size_mb=$((build_size_bytes / 1024 / 1024))
        
        if [ "$build_size_mb" -lt 10 ]; then
            log_warning "$platform build seems too small: ${build_size_mb}MB"
        elif [ "$build_size_mb" -gt 5000 ]; then
            log_warning "$platform build seems too large: ${build_size_mb}MB"
        else
            log_success "$platform build size OK: ${build_size_mb}MB"
        fi
    done
    
    if [ "$verification_passed" = "true" ]; then
        log_success "Build integrity verification passed"
    else
        log_error "Build integrity verification failed"
        exit 1
    fi
}

# ==============================================
# REPORTING
# ==============================================

generate_build_report() {
    log_header "Generating Build Report"
    
    local report_file="$PROJECT_ROOT/Builds/build-report-$BUILD_NUMBER.md"
    local json_report="$PROJECT_ROOT/Builds/build-report-$BUILD_NUMBER.json"
    
    # Generate Markdown report
    cat > "$report_file" << EOF
# BikeAdventure Build Report

## Build Information
- **Project**: $PROJECT_NAME
- **Version**: $VERSION
- **Build Number**: $BUILD_NUMBER
- **Configuration**: $BUILD_CONFIG
- **Build Date**: $(date -Iseconds)
- **Build Duration**: ${SECONDS}s

## Environment
- **UE5 Root**: $UE5_ROOT
- **Platform**: $(uname -s -m)
- **Builder**: $(whoami)@$(hostname)

## Platforms Built
EOF
    
    # Add platform status to report
    for platform in "${PLATFORMS[@]}"; do
        local build_dir="$PROJECT_ROOT/Builds/$platform-$BUILD_CONFIG-$BUILD_NUMBER"
        if [ -d "$build_dir" ]; then
            local build_size=$(du -sh "$build_dir" | cut -f1)
            echo "- **$platform**: ✅ Success ($build_size)" >> "$report_file"
        else
            echo "- **$platform**: ❌ Failed" >> "$report_file"
        fi
    done
    
    # Add packages section
    cat >> "$report_file" << EOF

## Generated Packages
EOF
    
    cd "$PROJECT_ROOT/Builds"
    for package in *.tar.gz; do
        if [ -f "$package" ]; then
            local size=$(du -sh "$package" | cut -f1)
            echo "- $package ($size)" >> "$report_file"
        fi
    done
    
    # Generate JSON report for automation
    cat > "$json_report" << EOF
{
  "project": "$PROJECT_NAME",
  "version": "$VERSION",
  "build_number": "$BUILD_NUMBER",
  "configuration": "$BUILD_CONFIG",
  "timestamp": "$(date -Iseconds)",
  "duration_seconds": $SECONDS,
  "platforms": [
EOF
    
    local first_platform=true
    for platform in "${PLATFORMS[@]}"; do
        local build_dir="$PROJECT_ROOT/Builds/$platform-$BUILD_CONFIG-$BUILD_NUMBER"
        
        if [ "$first_platform" = "false" ]; then
            echo "," >> "$json_report"
        fi
        first_platform=false
        
        if [ -d "$build_dir" ]; then
            local build_size_bytes=$(du -sb "$build_dir" | cut -f1)
            echo "    {\"name\": \"$platform\", \"status\": \"success\", \"size_bytes\": $build_size_bytes}" >> "$json_report"
        else
            echo "    {\"name\": \"$platform\", \"status\": \"failed\", \"size_bytes\": 0}" >> "$json_report"
        fi
    done
    
    cat >> "$json_report" << EOF
  ],
  "success": true
}
EOF
    
    log_success "Build report generated: $report_file"
    log_success "JSON report generated: $json_report"
}

# ==============================================
# MAIN EXECUTION
# ==============================================

print_banner() {
    cat << EOF
${BOLD}${CYAN}
╔═══════════════════════════════════════════════════════════════╗
║                     BikeAdventure Build                      ║
║                 Multi-Platform Automation                    ║
╚═══════════════════════════════════════════════════════════════╝
${NC}

Project: ${BOLD}$PROJECT_NAME${NC}
Version: ${BOLD}$VERSION${NC}
Build: ${BOLD}$BUILD_NUMBER${NC}
Configuration: ${BOLD}$BUILD_CONFIG${NC}
Platforms: ${BOLD}${PLATFORMS[*]}${NC}

EOF
}

parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            --platforms)
                IFS=',' read -r -a PLATFORMS <<< "$2"
                shift 2
                ;;
            --config)
                BUILD_CONFIG="$2"
                shift 2
                ;;
            --clean)
                CLEAN_BUILD="true"
                shift
                ;;
            --skip-tests)
                SKIP_TESTS="true"
                shift
                ;;
            --verbose)
                VERBOSE="true"
                shift
                ;;
            --parallel)
                PARALLEL_BUILDS="true"
                shift
                ;;
            --help)
                show_help
                exit 0
                ;;
            *)
                log_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
}

show_help() {
    cat << EOF
Usage: $0 [OPTIONS]

Options:
  --platforms PLATFORM1,PLATFORM2  Comma-separated list of platforms (default: Linux,Win64)
  --config CONFIG                   Build configuration (Development/Shipping/Test)
  --clean                          Clean intermediate files before building
  --skip-tests                     Skip automated tests
  --verbose                        Enable verbose output
  --parallel                       Build platforms in parallel (experimental)
  --help                           Show this help message

Environment Variables:
  UE5_ROOT                         Path to UE5 installation (default: /opt/UnrealEngine)
  BUILD_CONFIG                     Build configuration override
  VERSION                          Project version override
  SKIP_TESTS                       Set to 'true' to skip tests

Examples:
  $0                               # Build Linux and Win64 in Development mode
  $0 --platforms Linux --clean    # Build only Linux with clean build
  $0 --config Shipping --verbose  # Shipping build with verbose output
EOF
}

cleanup_on_exit() {
    local exit_code=$?
    
    if [ $exit_code -ne 0 ]; then
        log_error "Build process interrupted or failed"
        log_info "Check logs in: $LOG_DIR"
    fi
    
    exit $exit_code
}

main() {
    # Set up signal handlers
    trap cleanup_on_exit EXIT INT TERM
    
    # Parse command line arguments
    parse_arguments "$@"
    
    # Initialize
    print_banner
    setup_logging
    
    # Pre-build phase
    validate_environment
    check_git_status
    update_git_lfs
    clean_intermediate_files
    generate_project_files
    
    # Build phase
    local build_start_time=$(date +%s)
    local failed_platforms=()
    
    if [ "$PARALLEL_BUILDS" = "true" ]; then
        log_info "Starting parallel builds for ${PLATFORMS[*]}"
        
        declare -a build_pids
        for platform in "${PLATFORMS[@]}"; do
            (build_platform "$platform" && run_platform_tests "$platform") &
            build_pids+=($!)
        done
        
        # Wait for all builds
        for pid in "${build_pids[@]}"; do
            wait $pid || failed_platforms+=("unknown")
        done
    else
        for platform in "${PLATFORMS[@]}"; do
            if ! build_platform "$platform"; then
                failed_platforms+=("$platform")
            else
                run_platform_tests "$platform"
            fi
        done
    fi
    
    local build_end_time=$(date +%s)
    local total_build_time=$((build_end_time - build_start_time))
    
    # Post-build phase
    if [ ${#failed_platforms[@]} -eq 0 ]; then
        create_deployment_packages
        verify_build_integrity
        generate_build_report
        
        log_header "Build Summary"
        log_success "All platforms built successfully!"
        log_success "Total build time: ${total_build_time}s"
        log_success "Build artifacts available in: $PROJECT_ROOT/Builds"
        
        # Display package summary
        cd "$PROJECT_ROOT/Builds"
        log_info "Generated packages:"
        ls -la *.tar.gz 2>/dev/null | while read -r line; do
            log_info "  $line"
        done
    else
        log_error "Build failed for platforms: ${failed_platforms[*]}"
        log_error "Total build time: ${total_build_time}s"
        exit 1
    fi
}

# Execute main function with all arguments
main "$@"