#!/usr/bin/env python3
"""
BikeAdventure Gauntlet Test Suite
Comprehensive automated gameplay validation using Gauntlet framework
"""

import json
import time
import sys
import os
from typing import Dict, List, Any, Optional

# Mock Gauntlet framework for standalone testing
class MockGauntletContext:
    def __init__(self):
        self.current_fps = 60.0
        self.memory_usage_gb = 1.5
        self.peak_memory_gb = 2.0
        self.player_speed = 0.0
        self.at_intersection = False
        self.current_biome = "Forest"
        self.intersection_count = 0
        
    def load_map(self, map_name: str):
        print(f"Loading map: {map_name}")
        time.sleep(0.1)  # Simulate loading time
        
    def spawn_player_at_start(self):
        print("Spawning player at start position")
        self.player_speed = 0.0
        
    def simulate_input(self, input_name: str, value: float, duration: float = 0.1):
        print(f"Simulating input: {input_name} = {value} for {duration}s")
        if input_name == "Throttle":
            self.player_speed = value * 1200.0  # Max speed
        elif input_name in ["TurnLeft", "TurnRight"]:
            if self.at_intersection:
                self.at_intersection = False
                self.intersection_count += 1
                
    def get_player_speed(self) -> float:
        return self.player_speed
        
    def is_at_intersection(self) -> bool:
        # Simulate intersection discovery every 3 seconds for demo
        return (int(time.time()) % 3) == 0
        
    def get_current_biome(self) -> str:
        biomes = ["Forest", "Desert", "Mountains", "Plains", "Coast", "Swamp"]
        return biomes[self.intersection_count % len(biomes)]
        
    def get_current_fps(self) -> float:
        # Simulate some FPS variation
        return self.current_fps + (time.time() % 10 - 5) * 2
        
    def get_memory_usage_gb(self) -> float:
        # Simulate gradual memory increase
        return self.memory_usage_gb + (time.time() % 100) * 0.01
        
    def get_peak_memory_usage_gb(self) -> float:
        return max(self.peak_memory_gb, self.get_memory_usage_gb())
        
    def reset_game_state(self):
        print("Resetting game state")
        self.__init__()

class BikeAdventureGauntletTest:
    """Complete gameplay validation using Gauntlet framework"""
    
    def __init__(self, context: Optional[MockGauntletContext] = None):
        self.context = context or MockGauntletContext()
        self.test_duration = 10  # 10 seconds for demo
        self.required_fps = 60
        self.memory_limit_gb = 4.0
        self.test_results = {}
        self.start_time = 0
        self.errors = []
        
    def setup(self):
        """Initialize test environment"""
        print("Setting up BikeAdventure Gauntlet test...")
        try:
            self.context.load_map("BikeAdventure_TestMap")
            self.context.spawn_player_at_start()
            print("Test setup completed successfully")
        except Exception as e:
            self.errors.append(f"Setup failed: {str(e)}")
            raise
        
    def run(self):
        """Execute comprehensive gameplay test"""
        print(f"Starting {self.test_duration}s gameplay test...")
        self.start_time = time.time()
        
        try:
            # Test basic movement
            self._test_basic_movement()
            
            # Test intersection navigation
            intersection_count = self._test_intersection_navigation()
            
            # Validate test results
            self._validate_test_results(intersection_count)
            
            # Generate test report
            self._generate_test_report(intersection_count)
            
        except Exception as e:
            self.errors.append(f"Test execution failed: {str(e)}")
            raise
            
    def _test_basic_movement(self):
        """Test basic bike movement functionality"""
        print("Testing basic movement...")
        
        self.context.simulate_input("Throttle", 1.0, duration=2.0)
        time.sleep(2.1)  # Wait for input to take effect
        
        player_speed = self.context.get_player_speed()
        if player_speed <= 0:
            raise AssertionError(f"Player not moving: speed={player_speed}")
        
        print(f"✓ Movement test passed (speed: {player_speed})")
        
    def _test_intersection_navigation(self) -> int:
        """Test intersection detection and navigation"""
        print("Testing intersection navigation...")
        
        intersection_count = 0
        performance_samples = []
        
        while time.time() - self.start_time < self.test_duration:
            # Check for intersection
            if self.context.is_at_intersection():
                print(f"Reached intersection #{intersection_count + 1}")
                
                # Alternate between left and right choices
                choice = "TurnLeft" if intersection_count % 2 == 0 else "TurnRight"
                self.context.simulate_input(choice, 1.0)
                intersection_count += 1
                
                # Validate biome changes
                current_biome = self.context.get_current_biome()
                if current_biome is None:
                    raise AssertionError("Invalid biome detected")
                
                print(f"  Current biome: {current_biome}")
                
            # Monitor performance continuously
            fps = self.context.get_current_fps()
            memory_gb = self.context.get_memory_usage_gb()
            
            performance_samples.append({
                'fps': fps,
                'memory_gb': memory_gb,
                'timestamp': time.time() - self.start_time
            })
            
            if fps < self.required_fps:
                self.errors.append(f"FPS too low: {fps} at {time.time() - self.start_time:.1f}s")
                
            if memory_gb > self.memory_limit_gb:
                raise AssertionError(f"Memory exceeded: {memory_gb}GB")
                
            time.sleep(0.1)  # 100ms polling
        
        self.test_results['performance_samples'] = performance_samples
        return intersection_count
        
    def _validate_test_results(self, intersection_count: int):
        """Validate that test results meet requirements"""
        print("Validating test results...")
        
        if intersection_count < 2:
            raise AssertionError(f"Insufficient intersections: {intersection_count}")
            
        # Validate performance metrics
        if 'performance_samples' in self.test_results:
            samples = self.test_results['performance_samples']
            avg_fps = sum(s['fps'] for s in samples) / len(samples)
            min_fps = min(s['fps'] for s in samples)
            max_memory = max(s['memory_gb'] for s in samples)
            
            if avg_fps < self.required_fps:
                self.errors.append(f"Average FPS below target: {avg_fps:.1f}")
                
            if min_fps < self.required_fps * 0.75:  # 75% of target for 1% low
                self.errors.append(f"Minimum FPS too low: {min_fps:.1f}")
                
            print(f"✓ Performance validation completed")
            print(f"  Average FPS: {avg_fps:.1f}")
            print(f"  Minimum FPS: {min_fps:.1f}")
            print(f"  Peak Memory: {max_memory:.2f}GB")
        
        print(f"✓ Test validation passed ({intersection_count} intersections)")
        
    def cleanup(self):
        """Clean up test environment"""
        print("Cleaning up test environment...")
        try:
            self.context.reset_game_state()
            print("✓ Cleanup completed")
        except Exception as e:
            self.errors.append(f"Cleanup failed: {str(e)}")
        
    def _generate_test_report(self, intersection_count: int):
        """Generate comprehensive JSON test report"""
        end_time = time.time()
        test_duration = end_time - self.start_time
        
        # Calculate performance metrics
        performance_data = {}
        if 'performance_samples' in self.test_results:
            samples = self.test_results['performance_samples']
            performance_data = {
                'avg_fps': sum(s['fps'] for s in samples) / len(samples),
                'min_fps': min(s['fps'] for s in samples),
                'max_fps': max(s['fps'] for s in samples),
                'avg_memory_gb': sum(s['memory_gb'] for s in samples) / len(samples),
                'peak_memory_gb': max(s['memory_gb'] for s in samples),
                'sample_count': len(samples)
            }
        
        report_data = {
            'test_info': {
                'test_name': 'BikeAdventure Gauntlet Test',
                'version': '1.0',
                'timestamp': time.strftime('%Y-%m-%d %H:%M:%S'),
                'duration_seconds': test_duration,
                'target_duration': self.test_duration
            },
            'gameplay_metrics': {
                'intersections_navigated': intersection_count,
                'final_biome': self.context.get_current_biome(),
                'total_distance_simulated': intersection_count * 500  # Approximate
            },
            'performance_metrics': performance_data,
            'quality_gates': {
                'min_intersections': {'target': 10, 'actual': intersection_count, 'passed': intersection_count >= 10},
                'fps_target': {'target': self.required_fps, 'actual': performance_data.get('avg_fps', 0), 'passed': performance_data.get('avg_fps', 0) >= self.required_fps},
                'memory_limit': {'target': self.memory_limit_gb, 'actual': performance_data.get('peak_memory_gb', 0), 'passed': performance_data.get('peak_memory_gb', 0) <= self.memory_limit_gb}
            },
            'errors': self.errors,
            'overall_result': 'PASSED' if len(self.errors) == 0 else 'FAILED'
        }
        
        # Save report to file
        report_filename = f"bike_adventure_gauntlet_report_{int(time.time())}.json"
        report_path = os.path.join(os.path.dirname(__file__), report_filename)
        
        try:
            with open(report_path, 'w') as f:
                json.dump(report_data, f, indent=2)
            print(f"✓ Test report saved to: {report_path}")
        except Exception as e:
            print(f"Warning: Failed to save report: {e}")
        
        # Print summary
        print("\n" + "="*60)
        print("GAUNTLET TEST SUMMARY")
        print("="*60)
        print(f"Result: {report_data['overall_result']}")
        print(f"Duration: {test_duration:.1f}s / {self.test_duration}s")
        print(f"Intersections: {intersection_count}")
        if performance_data:
            print(f"Average FPS: {performance_data['avg_fps']:.1f}")
            print(f"Peak Memory: {performance_data['peak_memory_gb']:.2f}GB")
        if self.errors:
            print(f"Errors: {len(self.errors)}")
            for error in self.errors[:5]:  # Show first 5 errors
                print(f"  - {error}")
        print("="*60)

def main():
    """Main entry point for Gauntlet test execution"""
    test = BikeAdventureGauntletTest()
    
    try:
        test.setup()
        test.run()
        test.cleanup()
        
        return 0 if len(test.errors) == 0 else 1
        
    except Exception as e:
        print(f"FATAL ERROR: {e}")
        test.cleanup()
        return 2

if __name__ == "__main__":
    sys.exit(main())