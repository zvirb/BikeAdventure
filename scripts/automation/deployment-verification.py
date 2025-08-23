#!/usr/bin/env python3
"""
BikeAdventure Deployment Verification System
Comprehensive validation of multi-platform builds with detailed reporting
"""

import os
import sys
import json
import subprocess
import hashlib
import time
import logging
from pathlib import Path
from datetime import datetime, timezone
from typing import Dict, List, Any, Optional, Tuple
import argparse
import tempfile
import shutil
import tarfile
import zipfile


class DeploymentValidator:
    """Comprehensive deployment validation system for BikeAdventure builds"""
    
    def __init__(self, build_path: str, platform: str, config: Dict[str, Any] = None):
        self.build_path = Path(build_path)
        self.platform = platform.lower()
        self.config = config or {}
        self.validation_results = {
            "validation_id": f"validation_{int(time.time())}",
            "timestamp": datetime.now(timezone.utc).isoformat(),
            "platform": platform,
            "build_path": str(build_path),
            "validation_version": "1.0.0",
            "results": {}
        }
        
        # Setup logging
        self.setup_logging()
        
        # Validation criteria
        self.essential_files = self.get_platform_essential_files()
        self.performance_targets = self.get_performance_targets()
        
        self.logger.info(f"Initialized deployment validator for {platform}")
        
    def setup_logging(self):
        """Configure logging for the validation process"""
        log_level = logging.DEBUG if self.config.get("verbose", False) else logging.INFO
        
        logging.basicConfig(
            level=log_level,
            format='%(asctime)s - %(levelname)s - %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )
        self.logger = logging.getLogger(__name__)
        
    def get_platform_essential_files(self) -> List[str]:
        """Get essential files that must be present for each platform"""
        platform_files = {
            "linux": [
                "BikeAdventure.uproject",
                "LinuxNoEditor/BikeAdventure/Binaries/Linux/BikeAdventure",
                "LinuxNoEditor/BikeAdventure/Content/Paks/BikeAdventure-LinuxNoEditor.pak",
                "LinuxNoEditor/BikeAdventure/Config/",
                "LinuxNoEditor/Engine/Binaries/Linux/",
            ],
            "windows": [
                "BikeAdventure.uproject", 
                "WindowsNoEditor/BikeAdventure/Binaries/Win64/BikeAdventure.exe",
                "WindowsNoEditor/BikeAdventure/Content/Paks/BikeAdventure-WindowsNoEditor.pak",
                "WindowsNoEditor/BikeAdventure/Config/",
                "WindowsNoEditor/Engine/Binaries/Win64/",
            ],
            "multiplatform": [
                "BikeAdventure.uproject",
                "Config/DefaultEngine.ini",
                "Content/",
            ]
        }
        
        return platform_files.get(self.platform, platform_files["multiplatform"])
        
    def get_performance_targets(self) -> Dict[str, Any]:
        """Performance targets for validation"""
        return {
            "max_build_size_mb": 5000,
            "min_build_size_mb": 50,
            "max_executable_size_mb": 500,
            "min_executable_size_mb": 10,
            "max_pak_files": 20,
            "required_directories": ["Config", "Content", "Binaries"],
            "max_loading_time_seconds": 30,
            "target_fps": 60,
            "min_fps": 30
        }
        
    def validate_build_structure(self) -> bool:
        """Validate the basic build directory structure"""
        self.logger.info("Validating build structure...")
        
        validation_result = {
            "test_name": "build_structure",
            "status": "UNKNOWN",
            "details": {},
            "errors": [],
            "warnings": []
        }
        
        try:
            # Check if build path exists
            if not self.build_path.exists():
                validation_result["status"] = "FAIL"
                validation_result["errors"].append(f"Build path does not exist: {self.build_path}")
                return False
                
            if not self.build_path.is_dir():
                validation_result["status"] = "FAIL" 
                validation_result["errors"].append(f"Build path is not a directory: {self.build_path}")
                return False
                
            # Check essential files
            missing_files = []
            present_files = []
            
            for file_path in self.essential_files:
                full_path = self.build_path / file_path
                
                if full_path.exists():
                    present_files.append(file_path)
                    if full_path.is_file():
                        file_size = full_path.stat().st_size
                        validation_result["details"][file_path] = {
                            "present": True,
                            "type": "file",
                            "size_bytes": file_size,
                            "size_mb": round(file_size / (1024 * 1024), 2)
                        }
                    else:
                        validation_result["details"][file_path] = {
                            "present": True,
                            "type": "directory"
                        }
                else:
                    missing_files.append(file_path)
                    validation_result["details"][file_path] = {"present": False}
                    
            # Evaluate results
            if missing_files:
                validation_result["status"] = "FAIL"
                validation_result["errors"].extend([f"Missing essential file: {f}" for f in missing_files])
            else:
                validation_result["status"] = "PASS"
                
            validation_result["details"]["summary"] = {
                "total_essential_files": len(self.essential_files),
                "present_files": len(present_files),
                "missing_files": len(missing_files)
            }
            
            self.logger.info(f"Build structure validation: {validation_result['status']}")
            
        except Exception as e:
            validation_result["status"] = "ERROR"
            validation_result["errors"].append(f"Exception during structure validation: {str(e)}")
            self.logger.error(f"Structure validation error: {e}")
            
        self.validation_results["results"]["build_structure"] = validation_result
        return validation_result["status"] == "PASS"
        
    def validate_executable_integrity(self) -> bool:
        """Validate game executable integrity and properties"""
        self.logger.info("Validating executable integrity...")
        
        validation_result = {
            "test_name": "executable_integrity",
            "status": "UNKNOWN",
            "details": {},
            "errors": [],
            "warnings": []
        }
        
        try:
            # Find the executable
            executable_path = self.find_game_executable()
            
            if not executable_path:
                validation_result["status"] = "FAIL"
                validation_result["errors"].append("Game executable not found")
                self.validation_results["results"]["executable_integrity"] = validation_result
                return False
                
            validation_result["details"]["executable_path"] = str(executable_path)
            
            # Check executable properties
            exe_stat = executable_path.stat()
            exe_size_mb = exe_stat.st_size / (1024 * 1024)
            
            validation_result["details"]["size_mb"] = round(exe_size_mb, 2)
            validation_result["details"]["size_bytes"] = exe_stat.st_size
            validation_result["details"]["modified_time"] = datetime.fromtimestamp(exe_stat.st_mtime).isoformat()
            
            # Check size constraints
            if exe_size_mb > self.performance_targets["max_executable_size_mb"]:
                validation_result["warnings"].append(f"Executable size ({exe_size_mb:.2f}MB) exceeds target ({self.performance_targets['max_executable_size_mb']}MB)")
            elif exe_size_mb < self.performance_targets["min_executable_size_mb"]:
                validation_result["errors"].append(f"Executable size ({exe_size_mb:.2f}MB) too small (minimum {self.performance_targets['min_executable_size_mb']}MB)")
                
            # Check file permissions (Linux only)
            if self.platform == "linux":
                if not os.access(executable_path, os.X_OK):
                    validation_result["errors"].append("Executable does not have execute permissions")
                else:
                    validation_result["details"]["executable_permissions"] = "OK"
                    
            # Try to get version info (if available)
            try:
                if self.platform == "windows":
                    # Windows version info extraction would go here
                    validation_result["details"]["version_info"] = "Not implemented for Windows"
                else:
                    # Linux binary info
                    result = subprocess.run(
                        ["file", str(executable_path)],
                        capture_output=True,
                        text=True,
                        timeout=10
                    )
                    if result.returncode == 0:
                        validation_result["details"]["file_type"] = result.stdout.strip()
                        
            except subprocess.TimeoutExpired:
                validation_result["warnings"].append("Version info check timed out")
            except Exception as e:
                validation_result["warnings"].append(f"Could not extract version info: {e}")
                
            # Calculate MD5 hash for integrity
            try:
                md5_hash = self.calculate_file_hash(executable_path)
                validation_result["details"]["md5_hash"] = md5_hash
            except Exception as e:
                validation_result["warnings"].append(f"Could not calculate hash: {e}")
                
            # Determine final status
            if validation_result["errors"]:
                validation_result["status"] = "FAIL"
            elif validation_result["warnings"]:
                validation_result["status"] = "PASS_WITH_WARNINGS"
            else:
                validation_result["status"] = "PASS"
                
            self.logger.info(f"Executable validation: {validation_result['status']}")
            
        except Exception as e:
            validation_result["status"] = "ERROR"
            validation_result["errors"].append(f"Exception during executable validation: {str(e)}")
            self.logger.error(f"Executable validation error: {e}")
            
        self.validation_results["results"]["executable_integrity"] = validation_result
        return validation_result["status"] in ["PASS", "PASS_WITH_WARNINGS"]
        
    def validate_content_packages(self) -> bool:
        """Validate content packages (PAK files)"""
        self.logger.info("Validating content packages...")
        
        validation_result = {
            "test_name": "content_packages", 
            "status": "UNKNOWN",
            "details": {},
            "errors": [],
            "warnings": []
        }
        
        try:
            # Find PAK files
            pak_files = list(self.build_path.rglob("*.pak"))
            validation_result["details"]["pak_count"] = len(pak_files)
            
            if not pak_files:
                validation_result["status"] = "FAIL"
                validation_result["errors"].append("No PAK files found")
                self.validation_results["results"]["content_packages"] = validation_result
                return False
                
            # Analyze each PAK file
            pak_details = {}
            total_pak_size = 0
            
            for pak_file in pak_files:
                pak_stat = pak_file.stat()
                pak_size_mb = pak_stat.st_size / (1024 * 1024)
                total_pak_size += pak_stat.st_size
                
                pak_info = {
                    "path": str(pak_file.relative_to(self.build_path)),
                    "size_mb": round(pak_size_mb, 2),
                    "size_bytes": pak_stat.st_size,
                    "modified_time": datetime.fromtimestamp(pak_stat.st_mtime).isoformat()
                }
                
                # Calculate hash for integrity
                try:
                    pak_info["md5_hash"] = self.calculate_file_hash(pak_file)
                except Exception as e:
                    pak_info["hash_error"] = str(e)
                    
                pak_details[pak_file.name] = pak_info
                
            validation_result["details"]["pak_files"] = pak_details
            validation_result["details"]["total_content_size_mb"] = round(total_pak_size / (1024 * 1024), 2)
            
            # Validate against targets
            if len(pak_files) > self.performance_targets["max_pak_files"]:
                validation_result["warnings"].append(f"Many PAK files ({len(pak_files)}) may impact loading performance")
                
            # Check for essential content PAK
            main_pak_found = any("BikeAdventure" in pak.name for pak in pak_files)
            if not main_pak_found:
                validation_result["errors"].append("Main game content PAK not found")
                
            # Determine status
            if validation_result["errors"]:
                validation_result["status"] = "FAIL"
            elif validation_result["warnings"]:
                validation_result["status"] = "PASS_WITH_WARNINGS"
            else:
                validation_result["status"] = "PASS"
                
            self.logger.info(f"Content packages validation: {validation_result['status']}")
            
        except Exception as e:
            validation_result["status"] = "ERROR"
            validation_result["errors"].append(f"Exception during content validation: {str(e)}")
            self.logger.error(f"Content validation error: {e}")
            
        self.validation_results["results"]["content_packages"] = validation_result
        return validation_result["status"] in ["PASS", "PASS_WITH_WARNINGS"]
        
    def validate_performance_characteristics(self) -> bool:
        """Validate performance-related characteristics"""
        self.logger.info("Validating performance characteristics...")
        
        validation_result = {
            "test_name": "performance_characteristics",
            "status": "UNKNOWN", 
            "details": {},
            "errors": [],
            "warnings": []
        }
        
        try:
            # Calculate total build size
            total_size = sum(f.stat().st_size for f in self.build_path.rglob('*') if f.is_file())
            total_size_mb = total_size / (1024 * 1024)
            
            validation_result["details"]["total_build_size_mb"] = round(total_size_mb, 2)
            validation_result["details"]["total_build_size_bytes"] = total_size
            
            # Check against size targets
            if total_size_mb > self.performance_targets["max_build_size_mb"]:
                validation_result["errors"].append(f"Build size ({total_size_mb:.2f}MB) exceeds maximum ({self.performance_targets['max_build_size_mb']}MB)")
            elif total_size_mb < self.performance_targets["min_build_size_mb"]:
                validation_result["warnings"].append(f"Build size ({total_size_mb:.2f}MB) seems small (minimum expected {self.performance_targets['min_build_size_mb']}MB)")
            else:
                validation_result["details"]["size_validation"] = "PASS"
                
            # Analyze file distribution
            file_types = {}
            large_files = []
            
            for file_path in self.build_path.rglob('*'):
                if file_path.is_file():
                    file_ext = file_path.suffix.lower() or "no_extension"
                    file_size = file_path.stat().st_size
                    
                    if file_ext not in file_types:
                        file_types[file_ext] = {"count": 0, "total_size": 0}
                        
                    file_types[file_ext]["count"] += 1
                    file_types[file_ext]["total_size"] += file_size
                    
                    # Track large files (>50MB)
                    if file_size > 50 * 1024 * 1024:
                        large_files.append({
                            "path": str(file_path.relative_to(self.build_path)),
                            "size_mb": round(file_size / (1024 * 1024), 2)
                        })
                        
            # Convert file_types sizes to MB
            for ext in file_types:
                file_types[ext]["total_size_mb"] = round(file_types[ext]["total_size"] / (1024 * 1024), 2)
                
            validation_result["details"]["file_distribution"] = file_types
            validation_result["details"]["large_files"] = large_files
            validation_result["details"]["large_files_count"] = len(large_files)
            
            # Check for required directories
            missing_dirs = []
            for req_dir in self.performance_targets["required_directories"]:
                if not (self.build_path / req_dir).exists():
                    missing_dirs.append(req_dir)
                    
            if missing_dirs:
                validation_result["errors"].extend([f"Missing required directory: {d}" for d in missing_dirs])
                
            # Performance predictions
            validation_result["details"]["performance_predictions"] = {
                "estimated_loading_time_seconds": min(30, max(5, total_size_mb / 100)),  # Rough estimate
                "memory_usage_estimate_mb": total_size_mb * 0.3,  # Rough estimate
                "disk_space_required_mb": total_size_mb * 1.1  # Include some overhead
            }
            
            # Determine status
            if validation_result["errors"]:
                validation_result["status"] = "FAIL"
            elif validation_result["warnings"]:
                validation_result["status"] = "PASS_WITH_WARNINGS"
            else:
                validation_result["status"] = "PASS"
                
            self.logger.info(f"Performance characteristics validation: {validation_result['status']}")
            
        except Exception as e:
            validation_result["status"] = "ERROR" 
            validation_result["errors"].append(f"Exception during performance validation: {str(e)}")
            self.logger.error(f"Performance validation error: {e}")
            
        self.validation_results["results"]["performance_characteristics"] = validation_result
        return validation_result["status"] in ["PASS", "PASS_WITH_WARNINGS"]
        
    def run_smoke_test(self) -> bool:
        """Run basic smoke test on the executable"""
        self.logger.info("Running smoke test...")
        
        validation_result = {
            "test_name": "smoke_test",
            "status": "UNKNOWN",
            "details": {},
            "errors": [],
            "warnings": []
        }
        
        try:
            executable_path = self.find_game_executable()
            
            if not executable_path:
                validation_result["status"] = "FAIL"
                validation_result["errors"].append("Cannot run smoke test - executable not found")
                self.validation_results["results"]["smoke_test"] = validation_result
                return False
                
            # Prepare smoke test command
            smoke_test_args = [str(executable_path)]
            
            # Add platform-specific arguments for automated testing
            if self.platform == "linux":
                smoke_test_args.extend([
                    "-nullrhi",           # No rendering
                    "-unattended",        # No user interaction
                    "-ExecCmds=quit",    # Quit immediately
                    "-log"               # Enable logging
                ])
            elif self.platform == "windows": 
                smoke_test_args.extend([
                    "-nullrhi",
                    "-unattended", 
                    "-ExecCmds=quit",
                    "-log"
                ])
                
            self.logger.info(f"Running smoke test command: {' '.join(smoke_test_args)}")
            
            # Execute smoke test with timeout
            start_time = time.time()
            
            try:
                result = subprocess.run(
                    smoke_test_args,
                    cwd=self.build_path,
                    capture_output=True,
                    text=True,
                    timeout=self.performance_targets["max_loading_time_seconds"]
                )
                
                execution_time = time.time() - start_time
                
                validation_result["details"]["execution_time_seconds"] = round(execution_time, 2)
                validation_result["details"]["return_code"] = result.returncode
                validation_result["details"]["stdout_length"] = len(result.stdout)
                validation_result["details"]["stderr_length"] = len(result.stderr)
                
                # Capture key output (truncated for size)
                if result.stdout:
                    validation_result["details"]["stdout_sample"] = result.stdout[:1000] + ("..." if len(result.stdout) > 1000 else "")
                if result.stderr:
                    validation_result["details"]["stderr_sample"] = result.stderr[:1000] + ("..." if len(result.stderr) > 1000 else "")
                    
                # Analyze results
                if result.returncode == 0:
                    validation_result["status"] = "PASS"
                    validation_result["details"]["test_result"] = "Executable started and quit successfully"
                    
                    # Check execution time
                    if execution_time > self.performance_targets["max_loading_time_seconds"] * 0.8:
                        validation_result["warnings"].append(f"Slow startup time: {execution_time:.2f}s")
                        
                else:
                    validation_result["status"] = "FAIL"
                    validation_result["errors"].append(f"Executable returned non-zero exit code: {result.returncode}")
                    
                    # Look for specific error patterns
                    error_text = result.stderr.lower()
                    if "missing" in error_text:
                        validation_result["errors"].append("Possible missing dependency detected")
                    if "crash" in error_text:
                        validation_result["errors"].append("Crash detected during startup")
                    if "vulkan" in error_text or "opengl" in error_text:
                        validation_result["warnings"].append("Graphics driver issue detected")
                        
            except subprocess.TimeoutExpired:
                execution_time = time.time() - start_time
                validation_result["status"] = "FAIL"
                validation_result["errors"].append(f"Smoke test timed out after {execution_time:.2f}s")
                validation_result["details"]["execution_time_seconds"] = round(execution_time, 2)
                validation_result["details"]["timeout"] = True
                
            self.logger.info(f"Smoke test: {validation_result['status']}")
            
        except Exception as e:
            validation_result["status"] = "ERROR"
            validation_result["errors"].append(f"Exception during smoke test: {str(e)}")
            self.logger.error(f"Smoke test error: {e}")
            
        self.validation_results["results"]["smoke_test"] = validation_result
        return validation_result["status"] == "PASS"
        
    def validate_configuration_files(self) -> bool:
        """Validate configuration files and settings"""
        self.logger.info("Validating configuration files...")
        
        validation_result = {
            "test_name": "configuration_files",
            "status": "UNKNOWN",
            "details": {},
            "errors": [],
            "warnings": []
        }
        
        try:
            config_files = list(self.build_path.rglob("*.ini"))
            validation_result["details"]["config_files_found"] = len(config_files)
            
            essential_configs = [
                "DefaultEngine.ini",
                "DefaultGame.ini", 
                "DefaultInput.ini"
            ]
            
            found_configs = {}
            missing_configs = []
            
            for config_file in config_files:
                config_name = config_file.name
                found_configs[config_name] = {
                    "path": str(config_file.relative_to(self.build_path)),
                    "size_bytes": config_file.stat().st_size
                }
                
            for essential_config in essential_configs:
                if essential_config not in found_configs:
                    missing_configs.append(essential_config)
                    
            validation_result["details"]["found_configs"] = found_configs
            validation_result["details"]["missing_essential_configs"] = missing_configs
            
            # Check for platform-specific optimizations in DefaultEngine.ini
            default_engine_path = None
            for config_file in config_files:
                if config_file.name == "DefaultEngine.ini":
                    default_engine_path = config_file
                    break
                    
            if default_engine_path:
                try:
                    with open(default_engine_path, 'r', encoding='utf-8') as f:
                        engine_config = f.read()
                        
                    # Check for performance optimizations
                    optimization_checks = {
                        "frame_rate_smoothing": "bSmoothFrameRate" in engine_config,
                        "texture_streaming": "r.TextureStreaming" in engine_config,
                        "occlusion_queries": "r.AllowOcclusionQueries" in engine_config,
                        "platform_specific": f"PlatformSettings_{self.platform.title()}" in engine_config
                    }
                    
                    validation_result["details"]["engine_optimizations"] = optimization_checks
                    
                    # Warnings for missing optimizations
                    for opt_name, found in optimization_checks.items():
                        if not found:
                            validation_result["warnings"].append(f"Missing optimization: {opt_name}")
                            
                except Exception as e:
                    validation_result["warnings"].append(f"Could not analyze DefaultEngine.ini: {e}")
                    
            # Check project file
            project_files = list(self.build_path.rglob("*.uproject"))
            if not project_files:
                validation_result["errors"].append("No .uproject file found")
            elif len(project_files) > 1:
                validation_result["warnings"].append(f"Multiple .uproject files found: {len(project_files)}")
            else:
                try:
                    with open(project_files[0], 'r', encoding='utf-8') as f:
                        project_data = json.load(f)
                        
                    validation_result["details"]["project_info"] = {
                        "engine_version": project_data.get("EngineAssociation", "Unknown"),
                        "plugins_count": len(project_data.get("Plugins", [])),
                        "modules_count": len(project_data.get("Modules", []))
                    }
                    
                except Exception as e:
                    validation_result["warnings"].append(f"Could not parse .uproject file: {e}")
                    
            # Determine status
            if validation_result["errors"]:
                validation_result["status"] = "FAIL"
            elif missing_configs or validation_result["warnings"]:
                validation_result["status"] = "PASS_WITH_WARNINGS"
            else:
                validation_result["status"] = "PASS"
                
            self.logger.info(f"Configuration validation: {validation_result['status']}")
            
        except Exception as e:
            validation_result["status"] = "ERROR"
            validation_result["errors"].append(f"Exception during configuration validation: {str(e)}")
            self.logger.error(f"Configuration validation error: {e}")
            
        self.validation_results["results"]["configuration_files"] = validation_result
        return validation_result["status"] in ["PASS", "PASS_WITH_WARNINGS"]
        
    def find_game_executable(self) -> Optional[Path]:
        """Find the main game executable"""
        possible_names = ["BikeAdventure", "BikeAdventure.exe"]
        
        for exe_name in possible_names:
            for exe_path in self.build_path.rglob(exe_name):
                if exe_path.is_file():
                    return exe_path
                    
        return None
        
    def calculate_file_hash(self, file_path: Path) -> str:
        """Calculate MD5 hash of a file"""
        hash_md5 = hashlib.md5()
        with open(file_path, "rb") as f:
            # Read in chunks to handle large files
            for chunk in iter(lambda: f.read(4096), b""):
                hash_md5.update(chunk)
        return hash_md5.hexdigest()
        
    def run_comprehensive_validation(self) -> bool:
        """Run all validation tests"""
        self.logger.info("Starting comprehensive deployment validation...")
        
        validation_start_time = time.time()
        
        # Define validation tests in order of priority
        validation_tests = [
            ("build_structure", self.validate_build_structure),
            ("executable_integrity", self.validate_executable_integrity), 
            ("content_packages", self.validate_content_packages),
            ("configuration_files", self.validate_configuration_files),
            ("performance_characteristics", self.validate_performance_characteristics),
            ("smoke_test", self.run_smoke_test)
        ]
        
        passed_tests = 0
        failed_tests = 0
        
        # Run each validation test
        for test_name, test_function in validation_tests:
            self.logger.info(f"Running {test_name} validation...")
            
            test_start_time = time.time()
            
            try:
                test_passed = test_function()
                test_duration = time.time() - test_start_time
                
                # Update test metadata
                if test_name in self.validation_results["results"]:
                    self.validation_results["results"][test_name]["duration_seconds"] = round(test_duration, 2)
                    
                if test_passed:
                    passed_tests += 1
                    self.logger.info(f"✅ {test_name} PASSED in {test_duration:.2f}s")
                else:
                    failed_tests += 1
                    self.logger.error(f"❌ {test_name} FAILED in {test_duration:.2f}s")
                    
            except Exception as e:
                failed_tests += 1
                self.logger.error(f"💥 {test_name} ERROR: {e}")
                
                # Record error in results
                if test_name not in self.validation_results["results"]:
                    self.validation_results["results"][test_name] = {
                        "test_name": test_name,
                        "status": "ERROR",
                        "errors": [str(e)]
                    }
                    
        total_validation_time = time.time() - validation_start_time
        
        # Generate summary
        self.validation_results["summary"] = {
            "total_tests": len(validation_tests),
            "passed_tests": passed_tests,
            "failed_tests": failed_tests,
            "success_rate": round((passed_tests / len(validation_tests)) * 100, 1),
            "total_validation_time_seconds": round(total_validation_time, 2),
            "overall_status": "PASS" if failed_tests == 0 else "FAIL"
        }
        
        self.logger.info(f"Validation complete: {passed_tests}/{len(validation_tests)} tests passed")
        
        return failed_tests == 0
        
    def generate_validation_report(self, output_path: Optional[Path] = None) -> Path:
        """Generate comprehensive validation report"""
        if output_path is None:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            output_path = Path(f"deployment_validation_{self.platform}_{timestamp}.json")
            
        # Add metadata
        self.validation_results["metadata"] = {
            "validator_version": "1.0.0",
            "python_version": sys.version,
            "platform": sys.platform,
            "validation_host": os.uname().nodename if hasattr(os, 'uname') else 'unknown',
            "report_generated": datetime.now(timezone.utc).isoformat()
        }
        
        # Write report
        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(self.validation_results, f, indent=2, ensure_ascii=False)
            
        self.logger.info(f"Validation report saved: {output_path}")
        return output_path
        
    def generate_markdown_report(self, output_path: Optional[Path] = None) -> Path:
        """Generate human-readable markdown report"""
        if output_path is None:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            output_path = Path(f"deployment_validation_{self.platform}_{timestamp}.md")
            
        summary = self.validation_results.get("summary", {})
        
        markdown_content = f"""# BikeAdventure Deployment Validation Report

## Summary
- **Platform**: {self.platform.title()}
- **Validation Date**: {self.validation_results['timestamp']}
- **Overall Status**: {summary.get('overall_status', 'UNKNOWN')} 
- **Success Rate**: {summary.get('success_rate', 0)}% ({summary.get('passed_tests', 0)}/{summary.get('total_tests', 0)} tests passed)
- **Total Time**: {summary.get('total_validation_time_seconds', 0)}s

## Test Results

"""
        
        for test_name, result in self.validation_results.get("results", {}).items():
            status_emoji = {
                "PASS": "✅", 
                "PASS_WITH_WARNINGS": "⚠️",
                "FAIL": "❌",
                "ERROR": "💥"
            }.get(result.get("status", "UNKNOWN"), "❓")
            
            markdown_content += f"### {status_emoji} {test_name.replace('_', ' ').title()}\n\n"
            markdown_content += f"**Status**: {result.get('status', 'UNKNOWN')}\n\n"
            
            if result.get("errors"):
                markdown_content += "**Errors**:\n"
                for error in result["errors"]:
                    markdown_content += f"- {error}\n"
                markdown_content += "\n"
                
            if result.get("warnings"):
                markdown_content += "**Warnings**:\n"
                for warning in result["warnings"]:
                    markdown_content += f"- {warning}\n"
                markdown_content += "\n"
                
            # Add key details
            details = result.get("details", {})
            if details:
                markdown_content += "**Key Details**:\n"
                for key, value in details.items():
                    if isinstance(value, (dict, list)):
                        continue  # Skip complex objects for readability
                    markdown_content += f"- {key.replace('_', ' ').title()}: {value}\n"
                markdown_content += "\n"
                
        markdown_content += f"""
## Validation Environment
- **Build Path**: {self.build_path}
- **Validator Version**: 1.0.0
- **Python Version**: {sys.version}
- **Host Platform**: {sys.platform}

---
*Report generated on {datetime.now().isoformat()}*
"""
        
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(markdown_content)
            
        self.logger.info(f"Markdown report saved: {output_path}")
        return output_path


def main():
    """Main entry point for deployment verification"""
    parser = argparse.ArgumentParser(
        description="BikeAdventure Deployment Verification System",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s /path/to/build Linux
  %(prog)s /path/to/build Windows --verbose
  %(prog)s /path/to/build MultiPlatform --output-dir ./reports
        """
    )
    
    parser.add_argument("build_path", help="Path to the build directory to validate")
    parser.add_argument("platform", choices=["Linux", "Windows", "MultiPlatform"],
                       help="Target platform for validation")
    
    parser.add_argument("--verbose", "-v", action="store_true",
                       help="Enable verbose logging")
    parser.add_argument("--output-dir", "-o", type=Path, default=Path("."),
                       help="Output directory for reports (default: current directory)")
    parser.add_argument("--json-only", action="store_true",
                       help="Generate only JSON report (skip markdown)")
    parser.add_argument("--fail-fast", action="store_true",
                       help="Stop validation on first failure")
    
    args = parser.parse_args()
    
    # Ensure output directory exists
    args.output_dir.mkdir(parents=True, exist_ok=True)
    
    # Create validator
    config = {
        "verbose": args.verbose,
        "fail_fast": args.fail_fast
    }
    
    validator = DeploymentValidator(args.build_path, args.platform, config)
    
    try:
        # Run validation
        validation_passed = validator.run_comprehensive_validation()
        
        # Generate reports
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        
        json_report = validator.generate_validation_report(
            args.output_dir / f"validation_{args.platform.lower()}_{timestamp}.json"
        )
        
        if not args.json_only:
            md_report = validator.generate_markdown_report(
                args.output_dir / f"validation_{args.platform.lower()}_{timestamp}.md"
            )
            print(f"Reports generated:")
            print(f"  JSON: {json_report}")
            print(f"  Markdown: {md_report}")
        else:
            print(f"JSON report generated: {json_report}")
            
        # Print summary
        summary = validator.validation_results.get("summary", {})
        print(f"\nValidation Summary:")
        print(f"  Platform: {args.platform}")
        print(f"  Status: {summary.get('overall_status', 'UNKNOWN')}")
        print(f"  Success Rate: {summary.get('success_rate', 0)}%")
        print(f"  Tests Passed: {summary.get('passed_tests', 0)}/{summary.get('total_tests', 0)}")
        
        # Exit with appropriate code
        sys.exit(0 if validation_passed else 1)
        
    except KeyboardInterrupt:
        print("\nValidation interrupted by user")
        sys.exit(130)
    except Exception as e:
        print(f"Fatal error during validation: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()