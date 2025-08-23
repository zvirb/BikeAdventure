#!/usr/bin/env python3
"""
BikeAdventure Deployment Monitoring System
Real-time monitoring and reporting for multi-platform deployments
"""

import os
import sys
import json
import time
import logging
import subprocess
import threading
import queue
from datetime import datetime, timezone
from pathlib import Path
from typing import Dict, List, Any, Optional, Callable
import argparse
import psutil
import signal


class DeploymentMonitor:
    """Comprehensive deployment monitoring system"""
    
    def __init__(self, config: Dict[str, Any] = None):
        self.config = config or {}
        self.monitoring_active = False
        self.monitor_threads = []
        self.metrics_queue = queue.Queue()
        self.alerts_queue = queue.Queue()
        
        # Monitoring data
        self.metrics_data = {
            "system_metrics": {},
            "build_metrics": {},
            "deployment_metrics": {},
            "performance_metrics": {},
            "error_metrics": {}
        }
        
        # Alert thresholds
        self.alert_thresholds = self.config.get("alert_thresholds", {
            "cpu_usage_percent": 90,
            "memory_usage_percent": 85,
            "disk_usage_percent": 90,
            "build_time_minutes": 60,
            "error_count": 5,
            "failed_builds": 3
        })
        
        self.setup_logging()
        
    def setup_logging(self):
        """Setup logging configuration"""
        log_level = logging.DEBUG if self.config.get("verbose", False) else logging.INFO
        
        # Create logs directory
        log_dir = Path("logs/deployment-monitoring")
        log_dir.mkdir(parents=True, exist_ok=True)
        
        # Setup logging
        logging.basicConfig(
            level=log_level,
            format='%(asctime)s - %(levelname)s - [%(name)s] %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S',
            handlers=[
                logging.FileHandler(log_dir / f"deployment-monitor-{datetime.now().strftime('%Y%m%d')}.log"),
                logging.StreamHandler(sys.stdout)
            ]
        )
        
        self.logger = logging.getLogger(__name__)
        
    def start_monitoring(self):
        """Start all monitoring threads"""
        self.logger.info("Starting deployment monitoring system...")
        self.monitoring_active = True
        
        # Start monitoring threads
        monitor_functions = [
            ("system_monitor", self.monitor_system_resources),
            ("build_monitor", self.monitor_build_processes),
            ("deployment_monitor", self.monitor_deployment_status),
            ("performance_monitor", self.monitor_performance_metrics),
            ("error_monitor", self.monitor_error_conditions),
            ("metrics_processor", self.process_metrics),
            ("alert_processor", self.process_alerts)
        ]
        
        for thread_name, monitor_func in monitor_functions:
            thread = threading.Thread(target=monitor_func, name=thread_name, daemon=True)
            thread.start()
            self.monitor_threads.append(thread)
            self.logger.info(f"Started {thread_name} thread")
            
        self.logger.info(f"Deployment monitoring started with {len(self.monitor_threads)} threads")
        
    def stop_monitoring(self):
        """Stop all monitoring threads"""
        self.logger.info("Stopping deployment monitoring system...")
        self.monitoring_active = False
        
        # Wait for threads to finish
        for thread in self.monitor_threads:
            thread.join(timeout=5)
            
        self.logger.info("Deployment monitoring stopped")
        
    def monitor_system_resources(self):
        """Monitor system resource usage"""
        self.logger.info("System resource monitoring started")
        
        while self.monitoring_active:
            try:
                # CPU usage
                cpu_percent = psutil.cpu_percent(interval=1)
                
                # Memory usage
                memory = psutil.virtual_memory()
                memory_percent = memory.percent
                memory_used_gb = memory.used / (1024**3)
                memory_total_gb = memory.total / (1024**3)
                
                # Disk usage
                disk = psutil.disk_usage('/')
                disk_percent = disk.percent
                disk_used_gb = disk.used / (1024**3)
                disk_free_gb = disk.free / (1024**3)
                
                # Network I/O
                network = psutil.net_io_counters()
                
                # System load
                load_avg = os.getloadavg() if hasattr(os, 'getloadavg') else [0, 0, 0]
                
                system_metrics = {
                    "timestamp": datetime.now(timezone.utc).isoformat(),
                    "cpu": {
                        "usage_percent": cpu_percent,
                        "load_avg_1m": load_avg[0],
                        "load_avg_5m": load_avg[1], 
                        "load_avg_15m": load_avg[2]
                    },
                    "memory": {
                        "usage_percent": memory_percent,
                        "used_gb": round(memory_used_gb, 2),
                        "total_gb": round(memory_total_gb, 2),
                        "available_gb": round(memory.available / (1024**3), 2)
                    },
                    "disk": {
                        "usage_percent": disk_percent,
                        "used_gb": round(disk_used_gb, 2),
                        "free_gb": round(disk_free_gb, 2),
                        "total_gb": round(disk.total / (1024**3), 2)
                    },
                    "network": {
                        "bytes_sent": network.bytes_sent,
                        "bytes_recv": network.bytes_recv,
                        "packets_sent": network.packets_sent,
                        "packets_recv": network.packets_recv
                    }
                }
                
                # Queue metrics
                self.metrics_queue.put(("system", system_metrics))
                
                # Check for alerts
                if cpu_percent > self.alert_thresholds["cpu_usage_percent"]:
                    self.alerts_queue.put({
                        "type": "system_alert",
                        "severity": "warning",
                        "message": f"High CPU usage: {cpu_percent}%"
                    })
                    
                if memory_percent > self.alert_thresholds["memory_usage_percent"]:
                    self.alerts_queue.put({
                        "type": "system_alert", 
                        "severity": "warning",
                        "message": f"High memory usage: {memory_percent}%"
                    })
                    
                if disk_percent > self.alert_thresholds["disk_usage_percent"]:
                    self.alerts_queue.put({
                        "type": "system_alert",
                        "severity": "critical",
                        "message": f"Low disk space: {disk_percent}% used"
                    })
                    
                time.sleep(10)  # Monitor every 10 seconds
                
            except Exception as e:
                self.logger.error(f"Error in system resource monitoring: {e}")
                time.sleep(30)  # Wait longer on error
                
    def monitor_build_processes(self):
        """Monitor active build processes"""
        self.logger.info("Build process monitoring started")
        
        build_processes = {}
        
        while self.monitoring_active:
            try:
                current_processes = {}
                
                # Find UE5 and build-related processes
                for proc in psutil.process_iter(['pid', 'name', 'cpu_percent', 'memory_percent', 'create_time']):
                    try:
                        proc_info = proc.info
                        proc_name = proc_info['name'].lower()
                        
                        # Check if it's a build-related process
                        build_keywords = ['unrealbuildtool', 'unrealheadertool', 'unrealcooker', 'msbuild', 'clang', 'gcc']
                        
                        if any(keyword in proc_name for keyword in build_keywords):
                            current_processes[proc_info['pid']] = {
                                "name": proc_info['name'],
                                "cpu_percent": proc_info['cpu_percent'],
                                "memory_percent": proc_info['memory_percent'],
                                "create_time": proc_info['create_time'],
                                "duration": time.time() - proc_info['create_time']
                            }
                            
                    except (psutil.NoSuchProcess, psutil.AccessDenied):
                        continue
                        
                # Detect new builds
                new_builds = set(current_processes.keys()) - set(build_processes.keys())
                completed_builds = set(build_processes.keys()) - set(current_processes.keys())
                
                for pid in new_builds:
                    self.logger.info(f"New build process detected: {current_processes[pid]['name']} (PID: {pid})")
                    
                for pid in completed_builds:
                    duration = time.time() - build_processes[pid]['create_time']
                    self.logger.info(f"Build process completed: {build_processes[pid]['name']} (Duration: {duration:.1f}s)")
                    
                    # Check for long builds
                    duration_minutes = duration / 60
                    if duration_minutes > self.alert_thresholds["build_time_minutes"]:
                        self.alerts_queue.put({
                            "type": "build_alert",
                            "severity": "warning",
                            "message": f"Long build detected: {duration_minutes:.1f} minutes"
                        })
                        
                # Queue build metrics
                build_metrics = {
                    "timestamp": datetime.now(timezone.utc).isoformat(),
                    "active_builds": len(current_processes),
                    "processes": current_processes
                }
                
                self.metrics_queue.put(("build", build_metrics))
                
                build_processes = current_processes
                time.sleep(15)  # Monitor every 15 seconds
                
            except Exception as e:
                self.logger.error(f"Error in build process monitoring: {e}")
                time.sleep(30)
                
    def monitor_deployment_status(self):
        """Monitor deployment status and progress"""
        self.logger.info("Deployment status monitoring started")
        
        while self.monitoring_active:
            try:
                # Check for deployment artifacts
                builds_dir = Path("Builds")
                deployment_status = {
                    "timestamp": datetime.now(timezone.utc).isoformat(),
                    "builds_directory_exists": builds_dir.exists(),
                    "total_builds": 0,
                    "recent_builds": [],
                    "total_size_mb": 0
                }
                
                if builds_dir.exists():
                    # Count build artifacts
                    build_files = list(builds_dir.rglob("*.tar.gz")) + list(builds_dir.rglob("*.zip"))
                    deployment_status["total_builds"] = len(build_files)
                    
                    # Calculate total size
                    total_size = 0
                    recent_builds = []
                    
                    for build_file in build_files:
                        file_stat = build_file.stat()
                        total_size += file_stat.st_size
                        
                        # Track recent builds (last 24 hours)
                        if time.time() - file_stat.st_mtime < 86400:  # 24 hours
                            recent_builds.append({
                                "name": build_file.name,
                                "size_mb": round(file_stat.st_size / (1024**2), 2),
                                "modified": datetime.fromtimestamp(file_stat.st_mtime).isoformat()
                            })
                            
                    deployment_status["total_size_mb"] = round(total_size / (1024**2), 2)
                    deployment_status["recent_builds"] = sorted(recent_builds, key=lambda x: x["modified"], reverse=True)
                    
                    # Check GitHub Actions status (if available)
                    github_status = self.check_github_actions_status()
                    if github_status:
                        deployment_status["github_actions"] = github_status
                        
                # Queue deployment metrics
                self.metrics_queue.put(("deployment", deployment_status))
                
                time.sleep(30)  # Monitor every 30 seconds
                
            except Exception as e:
                self.logger.error(f"Error in deployment status monitoring: {e}")
                time.sleep(60)
                
    def check_github_actions_status(self) -> Optional[Dict[str, Any]]:
        """Check GitHub Actions workflow status"""
        try:
            # This would use GitHub API to check workflow status
            # For now, return a placeholder
            return {
                "status": "unknown",
                "last_run": None,
                "message": "GitHub API integration not implemented"
            }
        except Exception as e:
            self.logger.debug(f"Could not check GitHub Actions status: {e}")
            return None
            
    def monitor_performance_metrics(self):
        """Monitor performance-related metrics"""
        self.logger.info("Performance metrics monitoring started")
        
        while self.monitoring_active:
            try:
                # Monitor UE5 Editor performance (if running)
                ue5_processes = []
                for proc in psutil.process_iter(['pid', 'name', 'cpu_percent', 'memory_info']):
                    try:
                        if 'unrealeditor' in proc.info['name'].lower():
                            ue5_processes.append({
                                "pid": proc.info['pid'],
                                "name": proc.info['name'],
                                "cpu_percent": proc.info['cpu_percent'],
                                "memory_mb": round(proc.info['memory_info'].rss / (1024**2), 2)
                            })
                    except (psutil.NoSuchProcess, psutil.AccessDenied):
                        continue
                        
                # GPU usage (if nvidia-smi available)
                gpu_info = self.get_gpu_info()
                
                performance_metrics = {
                    "timestamp": datetime.now(timezone.utc).isoformat(),
                    "ue5_processes": ue5_processes,
                    "gpu_info": gpu_info,
                    "system_temperature": self.get_system_temperature()
                }
                
                self.metrics_queue.put(("performance", performance_metrics))
                
                time.sleep(20)  # Monitor every 20 seconds
                
            except Exception as e:
                self.logger.error(f"Error in performance monitoring: {e}")
                time.sleep(30)
                
    def get_gpu_info(self) -> Optional[Dict[str, Any]]:
        """Get GPU information if available"""
        try:
            result = subprocess.run(['nvidia-smi', '--query-gpu=utilization.gpu,memory.used,memory.total,temperature.gpu', '--format=csv,noheader,nounits'], 
                                  capture_output=True, text=True, timeout=10)
            
            if result.returncode == 0:
                values = result.stdout.strip().split(', ')
                return {
                    "utilization_percent": int(values[0]),
                    "memory_used_mb": int(values[1]),
                    "memory_total_mb": int(values[2]),
                    "temperature_celsius": int(values[3])
                }
        except (subprocess.TimeoutExpired, subprocess.CalledProcessError, FileNotFoundError, ValueError):
            pass
            
        return None
        
    def get_system_temperature(self) -> Optional[Dict[str, Any]]:
        """Get system temperature information"""
        try:
            if hasattr(psutil, 'sensors_temperatures'):
                temps = psutil.sensors_temperatures()
                if temps:
                    cpu_temps = temps.get('coretemp', [])
                    if cpu_temps:
                        return {
                            "cpu_temperature": cpu_temps[0].current,
                            "cpu_temp_high": cpu_temps[0].high,
                            "cpu_temp_critical": cpu_temps[0].critical
                        }
        except Exception:
            pass
            
        return None
        
    def monitor_error_conditions(self):
        """Monitor for error conditions and failures"""
        self.logger.info("Error condition monitoring started")
        
        error_counts = {"build_errors": 0, "system_errors": 0, "deployment_errors": 0}
        
        while self.monitoring_active:
            try:
                # Check log files for errors
                log_patterns = [
                    ("build_errors", ["error:", "failed:", "exception:"]),
                    ("system_errors", ["segfault", "kernel panic", "out of memory"]),
                    ("deployment_errors", ["deploy failed", "artifact missing", "validation failed"])
                ]
                
                current_errors = {}
                
                # Scan recent log files
                logs_dir = Path("logs")
                if logs_dir.exists():
                    for log_file in logs_dir.rglob("*.log"):
                        if time.time() - log_file.stat().st_mtime < 3600:  # Last hour
                            try:
                                with open(log_file, 'r', encoding='utf-8', errors='ignore') as f:
                                    content = f.read().lower()
                                    
                                    for error_type, patterns in log_patterns:
                                        error_count = sum(content.count(pattern) for pattern in patterns)
                                        if error_count > 0:
                                            current_errors[error_type] = current_errors.get(error_type, 0) + error_count
                                            
                            except Exception:
                                continue
                                
                # Check for error threshold breaches
                for error_type, count in current_errors.items():
                    if count > self.alert_thresholds.get("error_count", 5):
                        self.alerts_queue.put({
                            "type": "error_alert",
                            "severity": "critical",
                            "message": f"High error count detected: {error_type} = {count}"
                        })
                        
                error_metrics = {
                    "timestamp": datetime.now(timezone.utc).isoformat(),
                    "error_counts": current_errors,
                    "total_errors": sum(current_errors.values())
                }
                
                self.metrics_queue.put(("errors", error_metrics))
                
                time.sleep(60)  # Monitor every minute
                
            except Exception as e:
                self.logger.error(f"Error in error condition monitoring: {e}")
                time.sleep(60)
                
    def process_metrics(self):
        """Process and store collected metrics"""
        self.logger.info("Metrics processor started")
        
        while self.monitoring_active:
            try:
                # Process metrics from queue
                metrics_to_process = []
                
                while not self.metrics_queue.empty():
                    try:
                        metric_type, metric_data = self.metrics_queue.get_nowait()
                        metrics_to_process.append((metric_type, metric_data))
                    except queue.Empty:
                        break
                        
                # Store metrics
                for metric_type, metric_data in metrics_to_process:
                    if metric_type not in self.metrics_data:
                        self.metrics_data[metric_type] = []
                        
                    # Keep only recent metrics (last 24 hours)
                    cutoff_time = time.time() - 86400  # 24 hours
                    self.metrics_data[metric_type] = [
                        m for m in self.metrics_data[metric_type] 
                        if datetime.fromisoformat(m.get("timestamp", "1970-01-01")).timestamp() > cutoff_time
                    ]
                    
                    # Add new metric
                    self.metrics_data[metric_type].append(metric_data)
                    
                # Periodically save metrics to file
                if metrics_to_process:  # Only save if we have new data
                    self.save_metrics_snapshot()
                    
                time.sleep(5)  # Process metrics every 5 seconds
                
            except Exception as e:
                self.logger.error(f"Error in metrics processing: {e}")
                time.sleep(10)
                
    def process_alerts(self):
        """Process and handle alerts"""
        self.logger.info("Alert processor started")
        
        while self.monitoring_active:
            try:
                alerts_to_process = []
                
                while not self.alerts_queue.empty():
                    try:
                        alert = self.alerts_queue.get_nowait()
                        alert["timestamp"] = datetime.now(timezone.utc).isoformat()
                        alerts_to_process.append(alert)
                    except queue.Empty:
                        break
                        
                # Process alerts
                for alert in alerts_to_process:
                    self.handle_alert(alert)
                    
                time.sleep(1)  # Process alerts quickly
                
            except Exception as e:
                self.logger.error(f"Error in alert processing: {e}")
                time.sleep(10)
                
    def handle_alert(self, alert: Dict[str, Any]):
        """Handle a single alert"""
        severity = alert.get("severity", "info")
        message = alert.get("message", "Unknown alert")
        
        if severity == "critical":
            self.logger.critical(f"🚨 CRITICAL ALERT: {message}")
        elif severity == "warning":
            self.logger.warning(f"⚠️  WARNING: {message}")
        else:
            self.logger.info(f"ℹ️  INFO: {message}")
            
        # Save alert to alerts log
        alerts_file = Path("logs/deployment-monitoring/alerts.jsonl")
        alerts_file.parent.mkdir(parents=True, exist_ok=True)
        
        with open(alerts_file, 'a', encoding='utf-8') as f:
            f.write(json.dumps(alert) + '\n')
            
    def save_metrics_snapshot(self):
        """Save current metrics snapshot to file"""
        try:
            metrics_dir = Path("logs/deployment-monitoring/metrics")
            metrics_dir.mkdir(parents=True, exist_ok=True)
            
            snapshot_file = metrics_dir / f"metrics-{datetime.now().strftime('%Y%m%d-%H%M%S')}.json"
            
            with open(snapshot_file, 'w', encoding='utf-8') as f:
                json.dump(self.metrics_data, f, indent=2, default=str)
                
            # Keep only recent snapshots (last 7 days)
            cutoff_time = time.time() - (7 * 86400)
            for old_file in metrics_dir.glob("metrics-*.json"):
                if old_file.stat().st_mtime < cutoff_time:
                    old_file.unlink()
                    
        except Exception as e:
            self.logger.error(f"Error saving metrics snapshot: {e}")
            
    def generate_monitoring_report(self) -> Dict[str, Any]:
        """Generate comprehensive monitoring report"""
        report = {
            "report_info": {
                "generated_at": datetime.now(timezone.utc).isoformat(),
                "monitoring_duration": "N/A",
                "report_version": "1.0.0"
            },
            "system_summary": self.get_system_summary(),
            "build_summary": self.get_build_summary(),
            "deployment_summary": self.get_deployment_summary(),
            "performance_summary": self.get_performance_summary(),
            "alerts_summary": self.get_alerts_summary(),
            "recommendations": self.get_recommendations()
        }
        
        return report
        
    def get_system_summary(self) -> Dict[str, Any]:
        """Get system metrics summary"""
        system_metrics = self.metrics_data.get("system", [])
        if not system_metrics:
            return {"status": "no_data"}
            
        latest = system_metrics[-1]
        return {
            "current_cpu_usage": latest["cpu"]["usage_percent"],
            "current_memory_usage": latest["memory"]["usage_percent"],
            "current_disk_usage": latest["disk"]["usage_percent"],
            "load_average": latest["cpu"]["load_avg_1m"],
            "available_memory_gb": latest["memory"]["available_gb"],
            "free_disk_gb": latest["disk"]["free_gb"]
        }
        
    def get_build_summary(self) -> Dict[str, Any]:
        """Get build metrics summary"""
        build_metrics = self.metrics_data.get("build", [])
        if not build_metrics:
            return {"status": "no_data"}
            
        total_builds = sum(m.get("active_builds", 0) for m in build_metrics)
        return {
            "total_monitored_builds": total_builds,
            "current_active_builds": build_metrics[-1].get("active_builds", 0) if build_metrics else 0
        }
        
    def get_deployment_summary(self) -> Dict[str, Any]:
        """Get deployment metrics summary"""
        deployment_metrics = self.metrics_data.get("deployment", [])
        if not deployment_metrics:
            return {"status": "no_data"}
            
        latest = deployment_metrics[-1]
        return {
            "total_builds": latest.get("total_builds", 0),
            "total_size_mb": latest.get("total_size_mb", 0),
            "recent_builds_count": len(latest.get("recent_builds", []))
        }
        
    def get_performance_summary(self) -> Dict[str, Any]:
        """Get performance metrics summary"""
        performance_metrics = self.metrics_data.get("performance", [])
        if not performance_metrics:
            return {"status": "no_data"}
            
        latest = performance_metrics[-1]
        return {
            "ue5_processes_running": len(latest.get("ue5_processes", [])),
            "gpu_available": latest.get("gpu_info") is not None,
            "gpu_utilization": latest.get("gpu_info", {}).get("utilization_percent") if latest.get("gpu_info") else None
        }
        
    def get_alerts_summary(self) -> Dict[str, Any]:
        """Get alerts summary"""
        alerts_file = Path("logs/deployment-monitoring/alerts.jsonl")
        if not alerts_file.exists():
            return {"total_alerts": 0}
            
        try:
            alert_counts = {"critical": 0, "warning": 0, "info": 0}
            total_alerts = 0
            
            with open(alerts_file, 'r', encoding='utf-8') as f:
                for line in f:
                    try:
                        alert = json.loads(line.strip())
                        severity = alert.get("severity", "info")
                        alert_counts[severity] = alert_counts.get(severity, 0) + 1
                        total_alerts += 1
                    except json.JSONDecodeError:
                        continue
                        
            return {
                "total_alerts": total_alerts,
                "alert_counts": alert_counts
            }
            
        except Exception as e:
            self.logger.error(f"Error reading alerts: {e}")
            return {"total_alerts": 0, "error": str(e)}
            
    def get_recommendations(self) -> List[str]:
        """Generate recommendations based on monitoring data"""
        recommendations = []
        
        # System recommendations
        system_summary = self.get_system_summary()
        if isinstance(system_summary, dict) and "current_cpu_usage" in system_summary:
            if system_summary["current_cpu_usage"] > 80:
                recommendations.append("Consider upgrading CPU or reducing concurrent builds")
            if system_summary["current_memory_usage"] > 80:
                recommendations.append("Monitor memory usage during builds - may need more RAM")
            if system_summary["free_disk_gb"] < 10:
                recommendations.append("Low disk space - clean up build artifacts or add storage")
                
        # Build recommendations
        build_summary = self.get_build_summary()
        if isinstance(build_summary, dict):
            if build_summary.get("current_active_builds", 0) > 3:
                recommendations.append("Multiple concurrent builds detected - may impact performance")
                
        # Performance recommendations
        perf_summary = self.get_performance_summary()
        if isinstance(perf_summary, dict):
            if not perf_summary.get("gpu_available", False):
                recommendations.append("No GPU detected - rendering may be slower")
                
        if not recommendations:
            recommendations.append("System appears to be running optimally")
            
        return recommendations


def main():
    """Main entry point for deployment monitoring"""
    parser = argparse.ArgumentParser(
        description="BikeAdventure Deployment Monitoring System",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    
    parser.add_argument("--duration", "-d", type=int, default=0,
                       help="Monitoring duration in minutes (0 = run indefinitely)")
    parser.add_argument("--interval", "-i", type=int, default=10,
                       help="Metrics collection interval in seconds")
    parser.add_argument("--verbose", "-v", action="store_true",
                       help="Enable verbose logging")
    parser.add_argument("--report-only", action="store_true",
                       help="Generate report from existing data and exit")
    parser.add_argument("--output-dir", "-o", type=Path, default=Path("."),
                       help="Output directory for reports")
    
    args = parser.parse_args()
    
    # Configuration
    config = {
        "verbose": args.verbose,
        "collection_interval": args.interval,
        "monitoring_duration": args.duration
    }
    
    monitor = DeploymentMonitor(config)
    
    if args.report_only:
        # Generate report from existing data
        print("Generating monitoring report from existing data...")
        report = monitor.generate_monitoring_report()
        
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        report_file = args.output_dir / f"deployment-monitoring-report-{timestamp}.json"
        
        with open(report_file, 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, default=str)
            
        print(f"Monitoring report saved: {report_file}")
        return
    
    # Setup signal handlers for graceful shutdown
    def signal_handler(signum, frame):
        print("\nReceived shutdown signal...")
        monitor.stop_monitoring()
        
        # Generate final report
        print("Generating final monitoring report...")
        report = monitor.generate_monitoring_report()
        
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        report_file = args.output_dir / f"deployment-monitoring-final-{timestamp}.json"
        
        with open(report_file, 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, default=str)
            
        print(f"Final monitoring report saved: {report_file}")
        sys.exit(0)
        
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    try:
        # Start monitoring
        monitor.start_monitoring()
        
        print(f"Deployment monitoring active...")
        if args.duration > 0:
            print(f"Will run for {args.duration} minutes")
        else:
            print("Running indefinitely - press Ctrl+C to stop")
            
        # Run for specified duration or indefinitely
        if args.duration > 0:
            time.sleep(args.duration * 60)
            monitor.stop_monitoring()
        else:
            # Run indefinitely
            while True:
                time.sleep(60)
                
    except KeyboardInterrupt:
        print("\nMonitoring interrupted by user")
    except Exception as e:
        print(f"Fatal error in monitoring: {e}")
        sys.exit(1)
    finally:
        if monitor.monitoring_active:
            monitor.stop_monitoring()


if __name__ == "__main__":
    main()