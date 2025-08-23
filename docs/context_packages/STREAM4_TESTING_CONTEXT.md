# Stream 4: Testing & Quality Assurance Context Package
**Target Agent**: testing_agent  
**Token Limit**: 4000 tokens  
**Success Criteria**: Comprehensive automated testing framework with 80%+ coverage

## Objective
Establish robust testing infrastructure for BikeAdventure UE5 project, ensuring quality, performance, and cross-platform compatibility through automated testing frameworks.

## Testing Architecture

### 1. UE5 Automation Framework Setup
```cpp
// Core test module configuration
// File: Source/BikeAdventureTests/BikeAdventureTests.Build.cs
using UnrealBuildTool;

public class BikeAdventureTests : ModuleRules
{
    public BikeAdventureTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core", "CoreUObject", "Engine", "UnrealEd",
            "AutomationController", "AutomationWorker", "Gauntlet",
            "BikeAdventure" // Main game module
        });
        
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate", "SlateCore", "ToolMenus", "EditorStyle",
            "EditorWidgets", "DesktopWidgets"
        });
    }
}
```

### 2. Unit Testing Framework
```cpp
// Core movement system tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBikeMovementTest,
    "BikeAdventure.Unit.Movement.BasicMovement",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBikeMovementTest::RunTest(const FString& Parameters)
{
    // Create test world
    UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
    UBikeMovementComponent* BikeMovement = NewObject<UBikeMovementComponent>();
    
    // Test forward movement
    BikeMovement->SetThrottle(1.0f);
    BikeMovement->UpdateMovement(0.016f); // 60fps frame
    TestTrue("Bike moves forward", BikeMovement->GetVelocity().X > 0);
    
    // Test turning mechanics
    BikeMovement->SetSteering(1.0f);
    BikeMovement->UpdateMovement(0.016f);
    TestTrue("Bike turns right", BikeMovement->GetAngularVelocity().Z > 0);
    
    // Test speed limits
    TestTrue("Speed within limits", 
             BikeMovement->GetVelocity().Size() <= BikeMovement->GetMaxSpeed());
    
    return true;
}

// Intersection detection tests
IMPLEMENT_AUTOMATION_TEST(FIntersectionDetectionTest,
    "BikeAdventure.Unit.Intersection.Detection",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntersectionDetectionTest::RunTest(const FString& Parameters)
{
    // Test intersection trigger volumes
    // Test choice presentation timing
    // Test path selection mechanics
    return true;
}

// World generation tests
IMPLEMENT_AUTOMATION_TEST(FBiomeGenerationTest,
    "BikeAdventure.Unit.WorldGen.BiomeTransitions",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBiomeGenerationTest::RunTest(const FString& Parameters)
{
    UBiomeGenerator* BiomeGen = NewObject<UBiomeGenerator>();
    
    // Test biome transition rules
    EBiomeType CurrentBiome = EBiomeType::Forest;
    EBiomeType NextBiome = BiomeGen->GenerateNextBiome(CurrentBiome, true);
    TestTrue("Valid biome transition", NextBiome != EBiomeType::None);
    
    // Test no repetition rule
    TArray<EBiomeType> BiomeSequence;
    EBiomeType TestBiome = EBiomeType::Desert;
    for (int i = 0; i < 5; i++)
    {
        BiomeSequence.Add(BiomeGen->GenerateNextBiome(TestBiome, false));
    }
    
    // Check no more than 3 consecutive same biomes
    int SameCount = 0;
    for (int i = 1; i < BiomeSequence.Num(); i++)
    {
        if (BiomeSequence[i] == BiomeSequence[i-1])
        {
            SameCount++;
            TestTrue("Max same biome rule", SameCount <= 3);
        }
        else
        {
            SameCount = 0;
        }
    }
    
    return true;
}
```

### 3. Integration Testing
```cpp
// Complete gameplay flow test
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FGameplayFlowTest,
    "BikeAdventure.Integration.CompleteGameplayFlow")

void FGameplayFlowTest::GetTests(TArray<FString>& OutBeautifiedNames, 
                                TArray<FString>& OutTestCommands) const
{
    OutBeautifiedNames.Add("Game Start to First Intersection");
    OutTestCommands.Add("GameStartToIntersection");
    
    OutBeautifiedNames.Add("Continuous Exploration");  
    OutTestCommands.Add("ContinuousExploration");
    
    OutBeautifiedNames.Add("Save Load System");
    OutTestCommands.Add("SaveLoadSystem");
}

bool FGameplayFlowTest::RunTest(const FString& Parameters)
{
    if (Parameters == "GameStartToIntersection")
    {
        // Test complete flow from game start to first choice
        return TestGameStartFlow();
    }
    else if (Parameters == "ContinuousExploration")  
    {
        // Test extended exploration session
        return TestContinuousExploration();
    }
    else if (Parameters == "SaveLoadSystem")
    {
        // Test save/load functionality
        return TestSaveLoadSystem();
    }
    
    return false;
}
```

### 4. Performance Testing
```cpp
// Performance validation framework
IMPLEMENT_AUTOMATION_TEST(FPerformanceTest,
    "BikeAdventure.Performance.FrameRate",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPerformanceTest::RunTest(const FString& Parameters)
{
    // Setup performance monitoring
    float TestDuration = 10.0f; // 10 second test
    TArray<float> FrameTimes;
    float StartTime = FPlatformTime::Seconds();
    
    while ((FPlatformTime::Seconds() - StartTime) < TestDuration)
    {
        float FrameStartTime = FPlatformTime::Seconds();
        
        // Simulate frame work
        GEngine->Tick(0.016f, false);
        
        float FrameTime = FPlatformTime::Seconds() - FrameStartTime;
        FrameTimes.Add(FrameTime);
    }
    
    // Calculate performance metrics
    float AverageFrameTime = 0.0f;
    for (float FrameTime : FrameTimes)
    {
        AverageFrameTime += FrameTime;
    }
    AverageFrameTime /= FrameTimes.Num();
    
    float AverageFPS = 1.0f / AverageFrameTime;
    
    // Validate performance targets
    TestTrue("60 FPS minimum", AverageFPS >= 60.0f);
    
    // Calculate 1% low FPS
    FrameTimes.Sort();
    int OnePercentIndex = FMath::Max(1, FrameTimes.Num() / 100);
    float OnePercentLowFrameTime = FrameTimes[FrameTimes.Num() - OnePercentIndex];
    float OnePercentLowFPS = 1.0f / OnePercentLowFrameTime;
    
    TestTrue("45 FPS 1% low minimum", OnePercentLowFPS >= 45.0f);
    
    return true;
}

// Memory usage testing
IMPLEMENT_AUTOMATION_TEST(FMemoryUsageTest,
    "BikeAdventure.Performance.MemoryUsage", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMemoryUsageTest::RunTest(const FString& Parameters)
{
    // Get initial memory usage
    FPlatformMemoryStats InitialStats = FPlatformMemory::GetStats();
    
    // Run memory-intensive operations
    // Generate multiple biomes, load assets, etc.
    
    // Measure peak memory usage  
    FPlatformMemoryStats PeakStats = FPlatformMemory::GetStats();
    
    // Convert to GB and validate budget
    float MemoryUsageGB = PeakStats.UsedPhysical / (1024.0f * 1024.0f * 1024.0f);
    TestTrue("Memory within 4GB budget", MemoryUsageGB <= 4.0f);
    
    return true;
}
```

### 5. Gauntlet Integration Testing
```python
# Gauntlet test suite for automated gameplay validation
# File: Scripts/GauntletTests/BikeAdventureTest.py

import gauntlet
import json
import time

class BikeAdventureGauntletTest(gauntlet.UnrealTest):
    """Complete gameplay validation using Gauntlet framework"""
    
    def __init__(self, context):
        super(BikeAdventureGauntletTest, self).__init__(context)
        self.test_duration = 300  # 5 minutes
        self.required_fps = 60
        self.memory_limit_gb = 4.0
        
    def setup(self):
        """Initialize test environment"""
        self.context.load_map("BikeAdventure_TestMap")
        self.context.spawn_player_at_start()
        
    def run(self):
        """Execute comprehensive gameplay test"""
        start_time = time.time()
        
        # Test basic movement
        self.context.simulate_input("Throttle", 1.0, duration=2.0)
        player_speed = self.context.get_player_speed()
        assert player_speed > 0, f"Player not moving: speed={player_speed}"
        
        # Test intersection navigation
        intersection_count = 0
        while time.time() - start_time < self.test_duration:
            if self.context.is_at_intersection():
                # Alternate between left and right choices
                choice = "TurnLeft" if intersection_count % 2 == 0 else "TurnRight"
                self.context.simulate_input(choice, 1.0)
                intersection_count += 1
                
                # Validate biome changes
                current_biome = self.context.get_current_biome()
                assert current_biome is not None, "Invalid biome detected"
                
            # Monitor performance continuously
            fps = self.context.get_current_fps()
            memory_gb = self.context.get_memory_usage_gb()
            
            assert fps >= self.required_fps, f"FPS too low: {fps}"
            assert memory_gb <= self.memory_limit_gb, f"Memory exceeded: {memory_gb}GB"
            
            time.sleep(0.1)  # 100ms polling
            
        # Validate test results
        assert intersection_count >= 10, f"Insufficient intersections: {intersection_count}"
        
        # Generate test report
        self.generate_test_report({
            "test_duration": time.time() - start_time,
            "intersections_navigated": intersection_count,
            "final_fps": self.context.get_current_fps(),
            "peak_memory_gb": self.context.get_peak_memory_usage_gb()
        })
        
    def cleanup(self):
        """Clean up test environment"""
        self.context.reset_game_state()
        
    def generate_test_report(self, data):
        """Generate JSON test report"""
        with open("bike_adventure_test_report.json", "w") as f:
            json.dump(data, f, indent=2)
```

### 6. Cross-Platform Testing
```bash
# CI/CD integration for automated cross-platform testing
# File: .github/workflows/cross-platform-test.yml

name: Cross-Platform Testing
on: [push, pull_request]

jobs:
  test-ubuntu:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Run Ubuntu Tests
        run: |
          ./scripts/automation/run-all-tests.sh linux
  
  test-windows:
    runs-on: windows-latest  
    steps:
      - uses: actions/checkout@v3
      - name: Run Windows Tests
        run: |
          ./scripts/automation/run-all-tests.ps1 windows
```

### 7. Test Quality Gates
```yaml
# Quality gate definitions
quality_gates:
  build_requirements:
    - compile_success: true
    - warning_count_max: 10
    - error_count: 0
    
  test_requirements:
    - unit_test_coverage_min: 80
    - integration_test_pass_rate_min: 95
    - performance_test_pass_rate: 100
    
  performance_targets:
    - fps_minimum: 60
    - fps_1_percent_low_minimum: 45  
    - memory_usage_max_gb: 4.0
    - loading_time_max_seconds: 3.0
```

### 8. Success Validation Checklist
- ✅ All unit tests pass (80%+ coverage)
- ✅ Integration tests validate complete gameplay flow  
- ✅ Performance tests meet 60+ FPS requirements
- ✅ Memory usage stays within 4GB budget
- ✅ Cross-platform tests pass on Windows and Linux
- ✅ Gauntlet automation runs successfully for 5+ minutes
- ✅ Quality gates meet defined thresholds
- ✅ Test reports generated automatically

### 9. Integration Points
- **Validates Stream 1**: Core UE5 foundation and movement systems
- **Validates Stream 2**: World generation and biome transitions  
- **Validates Stream 5**: Cross-platform deployment and performance
- **Provides to All**: Continuous quality assurance and regression detection