# Stream 5: Platform & Deployment Context Package
**Target Agent**: deployment_agent  
**Token Limit**: 4000 tokens  
**Success Criteria**: Multi-platform CI/CD pipeline with automated builds

## Objective
Establish comprehensive deployment infrastructure for BikeAdventure UE5 project supporting Windows PC and Ubuntu Linux with automated CI/CD, asset management, and cross-platform optimization.

## Platform Architecture

### 1. Multi-Platform Build System
```yaml
# GitHub Actions CI/CD Pipeline
name: BikeAdventure Multi-Platform Build
on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main]

jobs:
  build-linux:
    runs-on: ubuntu-latest
    container:
      image: ghcr.io/epicgames/unreal-engine:5.4
    steps:
      - name: Checkout with LFS
        uses: actions/checkout@v3
        with:
          lfs: true
          
      - name: Setup UE5 Environment
        run: |
          export UE5_ROOT=/opt/UnrealEngine
          export PATH="$UE5_ROOT/Engine/Binaries/Linux:$PATH"
          
      - name: Generate Project Files
        run: |
          $UE5_ROOT/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.exe \
            -projectfiles -project="BikeAdventure.uproject" \
            -game -rocket -progress
            
      - name: Build Game
        run: |
          $UE5_ROOT/Engine/Build/BatchFiles/Linux/RunUAT.sh BuildCookRun \
            -project="BikeAdventure.uproject" \
            -platform=Linux \
            -configuration=Development \
            -build -cook -pak -stage \
            -archive="Builds/Linux-Development"
            
      - name: Run Automated Tests
        run: |
          $UE5_ROOT/Engine/Binaries/Linux/UnrealEditor \
            "BikeAdventure.uproject" \
            -ExecCmds="Automation RunTests BikeAdventure.Unit+BikeAdventure.Integration;Quit" \
            -unattended -nullrhi -ReportOutputDir="TestResults"
            
      - name: Upload Build Artifacts
        uses: actions/upload-artifact@v3
        with:
          name: linux-build
          path: Builds/Linux-Development/
          
  build-windows:
    runs-on: windows-latest
    steps:
      - name: Checkout with LFS
        uses: actions/checkout@v3
        with:
          lfs: true
          
      - name: Setup UE5 Build Tools
        run: |
          # Use pre-installed UE5 on GitHub runner
          $env:UE5_ROOT = "C:\Program Files\Epic Games\UE_5.4"
          $env:PATH += ";$env:UE5_ROOT\Engine\Binaries\Win64"
          
      - name: Build Windows Version
        run: |
          & "$env:UE5_ROOT\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun `
            -project="BikeAdventure.uproject" `
            -platform=Win64 `
            -configuration=Development `
            -build -cook -pak -stage `
            -archive="Builds\Windows-Development"
            
      - name: Upload Windows Build
        uses: actions/upload-artifact@v3
        with:
          name: windows-build
          path: Builds\Windows-Development\
```

### 2. Git LFS Configuration for UE5 Assets
```bash
# .gitattributes for UE5 asset management
*.uasset filter=lfs diff=lfs merge=lfs -text
*.umap filter=lfs diff=lfs merge=lfs -text
*.upk filter=lfs diff=lfs merge=lfs -text
*.udk filter=lfs diff=lfs merge=lfs -text
*.fbx filter=lfs diff=lfs merge=lfs -text
*.3ds filter=lfs diff=lfs merge=lfs -text
*.psd filter=lfs diff=lfs merge=lfs -text
*.png filter=lfs diff=lfs merge=lfs -text
*.mp3 filter=lfs diff=lfs merge=lfs -text
*.wav filter=lfs diff=lfs merge=lfs -text
*.xcf filter=lfs diff=lfs merge=lfs -text
*.jpg filter=lfs diff=lfs merge=lfs -text
*.jpeg filter=lfs diff=lfs merge=lfs -text
*.mp4 filter=lfs diff=lfs merge=lfs -text
*.mov filter=lfs diff=lfs merge=lfs -text
```

### 3. Cross-Platform Build Script
```bash
#!/bin/bash
# scripts/automation/build-all-platforms.sh (Enhanced)

PROJECT_NAME="BikeAdventure"
VERSION="1.0.0"
BUILD_NUMBER="${GITHUB_RUN_NUMBER:-local}"
UE5_ROOT="${UE5_ROOT:-/opt/UnrealEngine}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Build configuration
BUILD_CONFIG="Development"
PLATFORMS=("Linux" "Win64")

# Function to validate UE5 installation
validate_ue5_installation() {
    if [ ! -d "$UE5_ROOT" ]; then
        echo -e "${RED}UE5 installation not found at $UE5_ROOT${NC}"
        exit 1
    fi
    
    if [ ! -f "$UE5_ROOT/Engine/Build/BatchFiles/Linux/RunUAT.sh" ]; then
        echo -e "${RED}UE5 build tools not found${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}UE5 installation validated${NC}"
}

# Function to build for specific platform
build_platform() {
    local platform=$1
    local start_time=$(date +%s)
    
    echo -e "${YELLOW}Building $PROJECT_NAME for $platform...${NC}"
    
    # Platform-specific build commands
    if [ "$platform" == "Linux" ]; then
        BUILD_SCRIPT="$UE5_ROOT/Engine/Build/BatchFiles/Linux/RunUAT.sh"
    else
        BUILD_SCRIPT="$UE5_ROOT/Engine/Build/BatchFiles/RunUAT.bat"
    fi
    
    # Execute build
    $BUILD_SCRIPT BuildCookRun \
        -project="$(pwd)/$PROJECT_NAME.uproject" \
        -platform="$platform" \
        -configuration="$BUILD_CONFIG" \
        -build -cook -pak -stage -archive \
        -archivedirectory="$(pwd)/Builds/$platform-$VERSION-$BUILD_NUMBER" \
        -noP4 -nodebuginfo
    
    local exit_code=$?
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    if [ $exit_code -eq 0 ]; then
        echo -e "${GREEN}✓ $platform build completed in ${duration}s${NC}"
    else
        echo -e "${RED}✗ $platform build failed after ${duration}s${NC}"
        exit 1
    fi
}

# Function to run platform-specific tests
run_platform_tests() {
    local platform=$1
    
    echo -e "${YELLOW}Running tests for $platform...${NC}"
    
    if [ "$platform" == "Linux" ]; then
        TEST_EXECUTABLE="$UE5_ROOT/Engine/Binaries/Linux/UnrealEditor"
    else
        TEST_EXECUTABLE="$UE5_ROOT/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
    fi
    
    $TEST_EXECUTABLE \
        "$(pwd)/$PROJECT_NAME.uproject" \
        -ExecCmds="Automation RunTests BikeAdventure.Unit+BikeAdventure.Integration+BikeAdventure.Performance;Quit" \
        -unattended -nullrhi \
        -ReportOutputDir="TestResults/$platform"
        
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ $platform tests passed${NC}"
    else
        echo -e "${RED}✗ $platform tests failed${NC}"
        exit 1
    fi
}

# Main execution
validate_ue5_installation

# Create build directories
mkdir -p Builds TestResults

# Build for all platforms
for platform in "${PLATFORMS[@]}"; do
    build_platform "$platform"
    run_platform_tests "$platform"
done

# Create deployment packages
echo -e "${YELLOW}Creating deployment packages...${NC}"
cd Builds

for platform in "${PLATFORMS[@]}"; do
    package_name="$PROJECT_NAME-$platform-$VERSION-$BUILD_NUMBER"
    if [ -d "$package_name" ]; then
        tar -czf "$package_name.tar.gz" "$package_name"
        echo -e "${GREEN}✓ Created $package_name.tar.gz${NC}"
    fi
done

echo -e "${GREEN}Multi-platform build complete!${NC}"
ls -la *.tar.gz 2>/dev/null || echo "No packages created"
```

### 4. Performance Optimization Configuration
```cpp
// Config/DefaultEngine.ini optimizations
[/Script/Engine.Engine]
bUseFixedFrameRate=False
FixedFrameRate=60.000000
bSmoothFrameRate=True
SmoothedFrameRateRange=(LowerBound=(Type="ERangeBoundTypes::Inclusive",Value=30),UpperBound=(Type="ERangeBoundTypes::Exclusive",Value=60))

[/Script/Engine.RendererSettings]
r.Mobile.DisableVertexFog=True
r.Shadow.CSM.MaxCascades=1
r.AllowOcclusionQueries=True
r.MinScreenRadiusForLights=0.03
r.MinScreenRadiusForDepthPrepass=0.03

[/Script/Engine.StreamingSettings]
s.MinBulkDataSizeForAsyncLoading=131072
s.AsyncLoadingThreadEnabled=True
s.EventDrivenLoaderEnabled=True

// Linux-specific optimizations
[PlatformSettings_Linux]
AudioCallbackBufferFrameSize=1024
AudioNumBuffersToEnqueue=4
bRenderOffScreen=False
```

### 5. Asset Packaging and Distribution
```yaml
# Asset pipeline configuration
asset_pipeline:
  texture_compression:
    - format: "DXT"
      platforms: ["Windows"]
      quality: "High"
    - format: "ASTC"  
      platforms: ["Linux"]
      quality: "Medium"
      
  audio_compression:
    - format: "OGG Vorbis"
      quality: 0.7
      sample_rate: 44100
      
  mesh_optimization:
    - auto_lod: true
      lod_levels: 4
      triangle_reduction: [1.0, 0.75, 0.5, 0.25]
      
  packaging_settings:
    exclude_editor_content: true
    compress_packages: true
    encrypt_ini_files: false
    include_prerequisites: true
```

### 6. Deployment Verification Scripts
```python
#!/usr/bin/env python3
# scripts/automation/deployment-verification.py

import os
import subprocess
import json
import sys
from pathlib import Path

class DeploymentValidator:
    def __init__(self, build_path, platform):
        self.build_path = Path(build_path)
        self.platform = platform
        self.validation_results = {}
        
    def validate_executable(self):
        """Verify game executable exists and is functional"""
        if self.platform == "Linux":
            executable = self.build_path / "BikeAdventure" / "Binaries" / "Linux" / "BikeAdventure"
        else:
            executable = self.build_path / "BikeAdventure" / "Binaries" / "Win64" / "BikeAdventure.exe"
            
        if not executable.exists():
            self.validation_results["executable"] = {"status": "FAIL", "error": "Executable not found"}
            return False
            
        # Check if executable is not corrupted
        try:
            if self.platform == "Linux":
                result = subprocess.run([str(executable), "-version"], 
                                      capture_output=True, timeout=10)
            else:
                result = subprocess.run([str(executable), "-version"], 
                                      capture_output=True, timeout=10, shell=True)
                                      
            if result.returncode == 0:
                self.validation_results["executable"] = {"status": "PASS", "version": result.stdout.decode()}
                return True
        except Exception as e:
            self.validation_results["executable"] = {"status": "FAIL", "error": str(e)}
            
        return False
        
    def validate_assets(self):
        """Verify critical game assets are present"""
        critical_assets = [
            "Content/Core/GameModes/BP_BikeAdventureGameMode.uasset",
            "Content/Core/Characters/BP_BikeCharacter.uasset", 
            "Content/Gameplay/TestLevels/TestLevel.umap"
        ]
        
        missing_assets = []
        for asset in critical_assets:
            asset_path = self.build_path / "BikeAdventure" / "Content" / asset
            if not asset_path.exists():
                missing_assets.append(asset)
                
        if missing_assets:
            self.validation_results["assets"] = {
                "status": "FAIL", 
                "missing": missing_assets
            }
            return False
        else:
            self.validation_results["assets"] = {"status": "PASS"}
            return True
            
    def validate_performance(self):
        """Run basic performance validation"""
        # Launch game with performance profiling
        if self.platform == "Linux":
            executable = self.build_path / "BikeAdventure" / "Binaries" / "Linux" / "BikeAdventure"
            cmd = [str(executable), "-benchmark", "-ExecCmds=quit"]
        else:
            executable = self.build_path / "BikeAdventure" / "Binaries" / "Win64" / "BikeAdventure.exe" 
            cmd = [str(executable), "-benchmark", "-ExecCmds=quit"]
            
        try:
            result = subprocess.run(cmd, capture_output=True, timeout=30)
            # Parse benchmark results (simplified)
            if "FPS: 60" in result.stdout.decode():
                self.validation_results["performance"] = {"status": "PASS", "fps": "60+"}
                return True
        except Exception as e:
            self.validation_results["performance"] = {"status": "FAIL", "error": str(e)}
            
        return False
        
    def generate_report(self):
        """Generate validation report"""
        report = {
            "platform": self.platform,
            "build_path": str(self.build_path),
            "validation_timestamp": subprocess.check_output(["date", "-Iseconds"]).decode().strip(),
            "results": self.validation_results,
            "overall_status": "PASS" if all(r.get("status") == "PASS" for r in self.validation_results.values()) else "FAIL"
        }
        
        report_file = f"deployment_validation_{self.platform.lower()}.json"
        with open(report_file, 'w') as f:
            json.dump(report, f, indent=2)
            
        return report

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: deployment-verification.py <build_path> <platform>")
        sys.exit(1)
        
    validator = DeploymentValidator(sys.argv[1], sys.argv[2])
    
    # Run all validations
    validator.validate_executable()
    validator.validate_assets()  
    validator.validate_performance()
    
    # Generate and display report
    report = validator.generate_report()
    print(f"Deployment validation: {report['overall_status']}")
    
    if report['overall_status'] == "FAIL":
        sys.exit(1)
```

### 7. Success Criteria Validation
- ✅ Multi-platform builds (Linux + Windows) compile successfully
- ✅ Automated CI/CD pipeline executes without errors
- ✅ Git LFS manages UE5 assets efficiently
- ✅ Cross-platform tests pass on both platforms
- ✅ Performance targets met on both platforms (60+ FPS)
- ✅ Asset packaging optimizes for platform-specific formats
- ✅ Deployment verification confirms functional builds
- ✅ Build artifacts are properly versioned and archived

### 8. Integration Dependencies
- **Requires Stream 1**: UE5 project structure for building
- **Requires Stream 4**: Test suite for validation
- **Provides to All**: Deployment infrastructure and automated builds
- **Coordinates with Stream 6**: Asset pipeline for content delivery