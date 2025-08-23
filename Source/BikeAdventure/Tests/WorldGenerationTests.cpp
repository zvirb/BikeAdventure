#include "WorldGenerationTests.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

// Biome Transition Test Implementation
bool FBiomeTransitionTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = FWorldGenerationTestUtils::CreateTestWorld();
    if (!TestWorld)
    {
        AddError(TEXT("Failed to create test world"));
        return false;
    }

    UBiomeGenerator* BiomeGenerator = FWorldGenerationTestUtils::CreateTestBiomeGenerator(TestWorld);
    if (!BiomeGenerator)
    {
        AddError(TEXT("Failed to create biome generator"));
        FWorldGenerationTestUtils::CleanupTestObjects(TestWorld);
        return false;
    }

    // Test all biome types
    TArray<EBiomeType> AllBiomes = {
        EBiomeType::Forest, EBiomeType::Beach, EBiomeType::Desert,
        EBiomeType::Urban, EBiomeType::Countryside, EBiomeType::Mountains,
        EBiomeType::Wetlands
    };

    bool bTestPassed = true;
    FString ErrorMessage;

    // Test transitions from each biome
    for (EBiomeType StartBiome : AllBiomes)
    {
        TArray<EBiomeType> BiomeSequence = FWorldGenerationTestUtils::GenerateBiomeSequence(
            BiomeGenerator, StartBiome, 20, true);

        if (!FWorldGenerationTestUtils::ValidateBiomeTransitionRules(BiomeSequence, 3))
        {
            AddError(FString::Printf(TEXT("Biome transition rules violated starting from %s"),
                *UBiomeUtilities::GetBiomeName(StartBiome)));
            bTestPassed = false;
        }

        // Test for valid transitions
        for (int32 i = 1; i < BiomeSequence.Num(); i++)
        {
            if (!UBiomeUtilities::CanBiomesTransition(BiomeSequence[i-1], BiomeSequence[i]))
            {
                AddError(FString::Printf(TEXT("Invalid biome transition from %s to %s"),
                    *UBiomeUtilities::GetBiomeName(BiomeSequence[i-1]),
                    *UBiomeUtilities::GetBiomeName(BiomeSequence[i])));
                bTestPassed = false;
            }
        }
    }

    FWorldGenerationTestUtils::CleanupTestObjects(TestWorld);
    return bTestPassed;
}

// Biome Generation Consistency Test Implementation
bool FBiomeGenerationConsistencyTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = FWorldGenerationTestUtils::CreateTestWorld();
    UBiomeGenerator* BiomeGenerator = FWorldGenerationTestUtils::CreateTestBiomeGenerator(TestWorld);
    
    if (!TestWorld || !BiomeGenerator)
    {
        AddError(TEXT("Failed to create test environment"));
        return false;
    }

    bool bTestPassed = true;

    // Test consistent generation with same seed
    int32 TestSeed = 12345;
    BiomeGenerator->SetGenerationSeed(TestSeed);

    TArray<EBiomeType> FirstRun = FWorldGenerationTestUtils::GenerateBiomeSequence(
        BiomeGenerator, EBiomeType::Forest, 15, true);

    // Reset with same seed
    BiomeGenerator->SetGenerationSeed(TestSeed);

    TArray<EBiomeType> SecondRun = FWorldGenerationTestUtils::GenerateBiomeSequence(
        BiomeGenerator, EBiomeType::Forest, 15, true);

    // Compare results
    if (FirstRun.Num() != SecondRun.Num())
    {
        AddError(TEXT("Inconsistent sequence length with same seed"));
        bTestPassed = false;
    }
    else
    {
        for (int32 i = 0; i < FirstRun.Num(); i++)
        {
            if (FirstRun[i] != SecondRun[i])
            {
                AddError(FString::Printf(TEXT("Inconsistent biome at index %d: %s vs %s"), i,
                    *UBiomeUtilities::GetBiomeName(FirstRun[i]),
                    *UBiomeUtilities::GetBiomeName(SecondRun[i])));
                bTestPassed = false;
                break;
            }
        }
    }

    // Test different seeds produce different results
    BiomeGenerator->SetGenerationSeed(54321);
    TArray<EBiomeType> ThirdRun = FWorldGenerationTestUtils::GenerateBiomeSequence(
        BiomeGenerator, EBiomeType::Forest, 15, true);

    int32 DifferentCount = 0;
    for (int32 i = 0; i < FMath::Min(FirstRun.Num(), ThirdRun.Num()); i++)
    {
        if (FirstRun[i] != ThirdRun[i])
        {
            DifferentCount++;
        }
    }

    if (DifferentCount < 3)
    {
        AddError(FString::Printf(TEXT("Different seeds produced too similar results (%d differences)"), DifferentCount));
        bTestPassed = false;
    }

    FWorldGenerationTestUtils::CleanupTestObjects(TestWorld);
    return bTestPassed;
}

// World Streaming Performance Test Implementation
bool FWorldStreamingPerformanceTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = FWorldGenerationTestUtils::CreateTestWorld();
    UWorldStreamingManager* StreamingManager = FWorldGenerationTestUtils::CreateTestStreamingManager(TestWorld);
    
    if (!TestWorld || !StreamingManager)
    {
        AddError(TEXT("Failed to create streaming test environment"));
        return false;
    }

    bool bTestPassed = true;
    const float MaxAllowedLoadTime = 100.0f; // 100ms max load time per section
    const int32 MaxMemoryBudgetKB = 4194304; // 4GB

    // Test streaming in multiple sections
    TArray<FVector> TestPositions = {
        FVector(0, 0, 0),
        FVector(200000, 0, 0),      // 2km away
        FVector(400000, 0, 0),      // 4km away
        FVector(200000, 200000, 0), // Diagonal
        FVector(-200000, 0, 0),     // Negative direction
    };

    for (const FVector& Position : TestPositions)
    {
        double StartTime = FPlatformTime::Seconds();
        
        bool bStreamSuccess = StreamingManager->StreamInBiomeSection(Position, EBiomeType::Forest);
        
        double EndTime = FPlatformTime::Seconds();
        float LoadTimeMs = (EndTime - StartTime) * 1000.0f;

        if (!bStreamSuccess)
        {
            AddError(FString::Printf(TEXT("Failed to stream in section at position %s"), *Position.ToString()));
            bTestPassed = false;
        }

        if (LoadTimeMs > MaxAllowedLoadTime)
        {
            AddError(FString::Printf(TEXT("Streaming load time exceeded limit: %.2fms > %.2fms"), LoadTimeMs, MaxAllowedLoadTime));
            bTestPassed = false;
        }

        // Check memory usage
        int32 CurrentMemory = StreamingManager->GetTotalMemoryUsageKB();
        if (CurrentMemory > MaxMemoryBudgetKB)
        {
            AddError(FString::Printf(TEXT("Memory budget exceeded: %dKB > %dKB"), CurrentMemory, MaxMemoryBudgetKB));
            bTestPassed = false;
        }
    }

    // Test cleanup performance
    double CleanupStartTime = FPlatformTime::Seconds();
    StreamingManager->CleanupDistantSections(FVector::ZeroVector, true);
    double CleanupEndTime = FPlatformTime::Seconds();
    
    float CleanupTimeMs = (CleanupEndTime - CleanupStartTime) * 1000.0f;
    if (CleanupTimeMs > 50.0f) // 50ms max cleanup time
    {
        AddError(FString::Printf(TEXT("Cleanup time exceeded limit: %.2fms > 50.0ms"), CleanupTimeMs));
        bTestPassed = false;
    }

    FWorldGenerationTestUtils::CleanupTestObjects(TestWorld);
    return bTestPassed;
}

// Path Personality Test Implementation
bool FPathPersonalityTest::RunTest(const FString& Parameters)
{
    UPathPersonalitySystem* PathSystem = FWorldGenerationTestUtils::CreateTestPathPersonalitySystem();
    if (!PathSystem)
    {
        AddError(TEXT("Failed to create path personality system"));
        return false;
    }

    bool bTestPassed = true;

    // Test personality generation for different biomes
    TArray<EBiomeType> TestBiomes = {
        EBiomeType::Forest, EBiomeType::Urban, EBiomeType::Mountains, EBiomeType::Beach
    };

    FPlayerChoiceHistory TestHistory = FWorldGenerationTestUtils::GenerateTestPlayerHistory(10, 0.6f);

    for (EBiomeType CurrentBiome : TestBiomes)
    {
        for (EBiomeType LeftBiome : TestBiomes)
        {
            for (EBiomeType RightBiome : TestBiomes)
            {
                FPathHints Hints = PathSystem->GeneratePathHintsForIntersection(
                    CurrentBiome, LeftBiome, RightBiome, TestHistory);

                // Validate hint values are within expected ranges
                if (Hints.LeftPathChallengeFactor < 0.0f || Hints.LeftPathChallengeFactor > 1.0f)
                {
                    AddError(FString::Printf(TEXT("Left path challenge factor out of range: %.2f"), Hints.LeftPathChallengeFactor));
                    bTestPassed = false;
                }

                if (Hints.RightPathSceneryFactor < 0.0f || Hints.RightPathSceneryFactor > 1.0f)
                {
                    AddError(FString::Printf(TEXT("Right path scenery factor out of range: %.2f"), Hints.RightPathSceneryFactor));
                    bTestPassed = false;
                }

                if (Hints.HintSubtlety < 0.0f || Hints.HintSubtlety > 1.0f)
                {
                    AddError(FString::Printf(TEXT("Hint subtlety out of range: %.2f"), Hints.HintSubtlety));
                    bTestPassed = false;
                }

                // Validate personalities are valid
                if (Hints.LeftPathPersonality == EPathPersonality::None)
                {
                    AddError(TEXT("Left path personality not assigned"));
                    bTestPassed = false;
                }

                if (Hints.RightPathPersonality == EPathPersonality::None)
                {
                    AddError(TEXT("Right path personality not assigned"));
                    bTestPassed = false;
                }
            }
        }
    }

    return bTestPassed;
}

// Memory Budget Test Implementation
bool FMemoryBudgetTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = FWorldGenerationTestUtils::CreateTestWorld();
    UWorldStreamingManager* StreamingManager = FWorldGenerationTestUtils::CreateTestStreamingManager(TestWorld);
    
    if (!TestWorld || !StreamingManager)
    {
        AddError(TEXT("Failed to create test environment"));
        return false;
    }

    bool bTestPassed = true;
    const int32 MemoryBudgetKB = 1024000; // 1GB test budget
    
    // Stream in many sections to test memory management
    TArray<FVector> ManyPositions;
    for (int32 X = -5; X <= 5; X++)
    {
        for (int32 Y = -5; Y <= 5; Y++)
        {
            ManyPositions.Add(FVector(X * 200000, Y * 200000, 0));
        }
    }

    int32 MaxMemoryUsed = 0;
    int32 SectionsStreamed = 0;

    for (const FVector& Position : ManyPositions)
    {
        if (StreamingManager->StreamInBiomeSection(Position, EBiomeType::Forest))
        {
            SectionsStreamed++;
            int32 CurrentMemory = StreamingManager->GetTotalMemoryUsageKB();
            MaxMemoryUsed = FMath::Max(MaxMemoryUsed, CurrentMemory);
            
            if (CurrentMemory > MemoryBudgetKB && StreamingManager->IsWithinMemoryBudget())
            {
                AddError(FString::Printf(TEXT("Memory budget check failed: reported within budget but using %dKB > %dKB"), 
                    CurrentMemory, MemoryBudgetKB));
                bTestPassed = false;
                break;
            }
        }
    }

    AddInfo(FString::Printf(TEXT("Streamed %d sections, max memory used: %dKB"), SectionsStreamed, MaxMemoryUsed));

    // Test cleanup reduces memory usage
    int32 MemoryBeforeCleanup = StreamingManager->GetTotalMemoryUsageKB();
    StreamingManager->CleanupDistantSections(FVector::ZeroVector, true);
    int32 MemoryAfterCleanup = StreamingManager->GetTotalMemoryUsageKB();

    if (MemoryAfterCleanup >= MemoryBeforeCleanup)
    {
        AddError(FString::Printf(TEXT("Cleanup did not reduce memory usage: %dKB -> %dKB"), 
            MemoryBeforeCleanup, MemoryAfterCleanup));
        bTestPassed = false;
    }

    FWorldGenerationTestUtils::CleanupTestObjects(TestWorld);
    return bTestPassed;
}

// Intersection Generation Test Implementation
bool FIntersectionGenerationTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = FWorldGenerationTestUtils::CreateTestWorld();
    UBiomeGenerator* BiomeGenerator = FWorldGenerationTestUtils::CreateTestBiomeGenerator(TestWorld);
    
    if (!TestWorld || !BiomeGenerator)
    {
        AddError(TEXT("Failed to create test environment"));
        return false;
    }

    bool bTestPassed = true;

    // Test intersection generation for different biome combinations
    TArray<EBiomeType> TestBiomes = {
        EBiomeType::Forest, EBiomeType::Urban, EBiomeType::Beach, EBiomeType::Mountains
    };

    for (EBiomeType CurrentBiome : TestBiomes)
    {
        for (EBiomeType LeftBiome : TestBiomes)
        {
            for (EBiomeType RightBiome : TestBiomes)
            {
                FVector TestLocation(FMath::RandRange(-1000, 1000), FMath::RandRange(-1000, 1000), 0);
                
                AIntersection* Intersection = BiomeGenerator->GenerateIntersection(
                    TestLocation, CurrentBiome, LeftBiome, RightBiome);

                if (!Intersection)
                {
                    AddError(FString::Printf(TEXT("Failed to generate intersection for %s biome"), 
                        *UBiomeUtilities::GetBiomeName(CurrentBiome)));
                    bTestPassed = false;
                    continue;
                }

                // Validate intersection properties
                if (!FWorldGenerationTestUtils::ValidateIntersectionPlacement(Intersection, CurrentBiome))
                {
                    AddError(FString::Printf(TEXT("Invalid intersection placement for %s biome"), 
                        *UBiomeUtilities::GetBiomeName(CurrentBiome)));
                    bTestPassed = false;
                }

                // Validate path biomes are set correctly
                if (Intersection->GetLeftPathBiome() != LeftBiome)
                {
                    AddError(TEXT("Left path biome not set correctly"));
                    bTestPassed = false;
                }

                if (Intersection->GetRightPathBiome() != RightBiome)
                {
                    AddError(TEXT("Right path biome not set correctly"));
                    bTestPassed = false;
                }

                // Cleanup
                if (IsValid(Intersection))
                {
                    Intersection->Destroy();
                }
            }
        }
    }

    FWorldGenerationTestUtils::CleanupTestObjects(TestWorld);
    return bTestPassed;
}

// PCG Integration Test Implementation
bool FPCGIntegrationTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = FWorldGenerationTestUtils::CreateTestWorld();
    UBiomeGenerator* BiomeGenerator = FWorldGenerationTestUtils::CreateTestBiomeGenerator(TestWorld);
    
    if (!TestWorld || !BiomeGenerator)
    {
        AddError(TEXT("Failed to create test environment"));
        return false;
    }

    bool bTestPassed = true;

    // Test PCG settings for each biome
    TArray<EBiomeType> TestBiomes = {
        EBiomeType::Forest, EBiomeType::Desert, EBiomeType::Beach,
        EBiomeType::Urban, EBiomeType::Mountains, EBiomeType::Wetlands
    };

    for (EBiomeType BiomeType : TestBiomes)
    {
        UBiomePCGSettings* Settings = BiomeGenerator->GetBiomePCGSettings(BiomeType);
        
        if (!Settings)
        {
            AddError(FString::Printf(TEXT("No PCG settings found for %s biome"), 
                *UBiomeUtilities::GetBiomeName(BiomeType)));
            bTestPassed = false;
            continue;
        }

        if (Settings->BiomeType != BiomeType)
        {
            AddError(FString::Printf(TEXT("PCG settings biome type mismatch for %s"), 
                *UBiomeUtilities::GetBiomeName(BiomeType)));
            bTestPassed = false;
        }

        // Test generation parameters are valid
        FBiomeGenerationParams Params = Settings->GenerationParams;
        
        if (Params.VegetationDensity < 0.0f || Params.VegetationDensity > 1.0f)
        {
            AddError(FString::Printf(TEXT("Invalid vegetation density for %s: %.2f"), 
                *UBiomeUtilities::GetBiomeName(BiomeType), Params.VegetationDensity));
            bTestPassed = false;
        }

        if (Params.PathWidth < 100.0f || Params.PathWidth > 1000.0f)
        {
            AddError(FString::Printf(TEXT("Invalid path width for %s: %.2f"), 
                *UBiomeUtilities::GetBiomeName(BiomeType), Params.PathWidth));
            bTestPassed = false;
        }
    }

    FWorldGenerationTestUtils::CleanupTestObjects(TestWorld);
    return bTestPassed;
}

// Comprehensive World Generation Test Implementation
void FComprehensiveWorldGenerationTest::GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
    OutBeautifiedNames.Add(TEXT("Full World Generation Flow"));
    OutTestCommands.Add(TEXT("FullFlow"));
    
    OutBeautifiedNames.Add(TEXT("Memory Stress Test"));
    OutTestCommands.Add(TEXT("MemoryStress"));
    
    OutBeautifiedNames.Add(TEXT("Performance Benchmark"));
    OutTestCommands.Add(TEXT("PerformanceBenchmark"));
}

bool FComprehensiveWorldGenerationTest::RunTest(const FString& Parameters)
{
    if (Parameters == TEXT("FullFlow"))
    {
        return RunFullFlowTest();
    }
    else if (Parameters == TEXT("MemoryStress"))
    {
        return RunMemoryStressTest();
    }
    else if (Parameters == TEXT("PerformanceBenchmark"))
    {
        return RunPerformanceBenchmark();
    }
    
    return false;
}

bool FComprehensiveWorldGenerationTest::RunFullFlowTest()
{
    // Create comprehensive test scenario
    FWorldGenerationTestScenario Scenario;
    Scenario.ScenarioName = TEXT("FullFlowTest");
    Scenario.NumTransitions = 50;
    Scenario.bTestMemoryBudget = true;
    Scenario.bTestPerformance = true;

    FString ErrorMessage;
    FWorldGenerationTestMetrics Metrics = FWorldGenerationTestManager::RunTestScenario(Scenario, ErrorMessage);

    if (!ErrorMessage.IsEmpty())
    {
        AddError(ErrorMessage);
        return false;
    }

    // Validate results
    if (Metrics.BiomesGenerated < Scenario.NumTransitions)
    {
        AddError(FString::Printf(TEXT("Not enough biomes generated: %d < %d"), 
            Metrics.BiomesGenerated, Scenario.NumTransitions));
        return false;
    }

    if (!Metrics.bPassedMemoryBudget)
    {
        AddError(TEXT("Memory budget test failed"));
        return false;
    }

    if (!Metrics.bPassedPerformanceTarget)
    {
        AddError(TEXT("Performance target test failed"));
        return false;
    }

    AddInfo(FString::Printf(TEXT("Full flow test completed successfully in %.2fms"), Metrics.TotalTestTimeMs));
    return true;
}

bool FComprehensiveWorldGenerationTest::RunMemoryStressTest()
{
    FWorldGenerationTestScenario Scenario;
    Scenario.ScenarioName = TEXT("MemoryStressTest");
    Scenario.NumTransitions = 200; // Large number for stress testing
    Scenario.ExpectedMaxMemoryKB = 2097152; // 2GB limit for stress test

    FString ErrorMessage;
    FWorldGenerationTestMetrics Metrics = FWorldGenerationTestManager::RunTestScenario(Scenario, ErrorMessage);

    if (!ErrorMessage.IsEmpty())
    {
        AddError(ErrorMessage);
        return false;
    }

    AddInfo(FString::Printf(TEXT("Memory stress test used maximum %dKB"), Metrics.MemoryUsageKB));
    return Metrics.bPassedMemoryBudget;
}

bool FComprehensiveWorldGenerationTest::RunPerformanceBenchmark()
{
    TArray<FWorldGenerationTestScenario> Scenarios = FWorldGenerationTestManager::CreateDefaultTestScenarios();
    FString ErrorMessage;
    TArray<FWorldGenerationTestMetrics> Results = FWorldGenerationTestManager::RunTestSuite(Scenarios, ErrorMessage);

    if (!ErrorMessage.IsEmpty())
    {
        AddError(ErrorMessage);
        return false;
    }

    // Generate performance report
    FString Report = FWorldGenerationTestManager::GenerateTestReport(Results, Scenarios);
    AddInfo(Report);

    // Check if all scenarios passed performance targets
    for (const FWorldGenerationTestMetrics& Result : Results)
    {
        if (!Result.bPassedPerformanceTarget)
        {
            AddError(TEXT("One or more performance benchmarks failed"));
            return false;
        }
    }

    return true;
}

// World Generation Stress Test Implementation
bool FWorldGenerationStressTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = FWorldGenerationTestUtils::CreateTestWorld();
    UBiomeGenerator* BiomeGenerator = FWorldGenerationTestUtils::CreateTestBiomeGenerator(TestWorld);
    UWorldStreamingManager* StreamingManager = FWorldGenerationTestUtils::CreateTestStreamingManager(TestWorld);
    
    if (!TestWorld || !BiomeGenerator || !StreamingManager)
    {
        AddError(TEXT("Failed to create stress test environment"));
        return false;
    }

    bool bTestPassed = true;
    const int32 StressTestIterations = 1000;
    const float MaxAllowedAverageTimeMs = 10.0f;

    double TotalTime = 0.0;
    int32 SuccessfulIterations = 0;

    for (int32 i = 0; i < StressTestIterations; i++)
    {
        double IterationStart = FPlatformTime::Seconds();

        // Generate random biome transition
        EBiomeType CurrentBiome = (EBiomeType)FMath::RandRange(0, 6);
        bool bLeftChoice = FMath::RandBool();
        TArray<EBiomeType> History;
        
        EBiomeType NextBiome = BiomeGenerator->GenerateNextBiome(CurrentBiome, bLeftChoice, History);
        
        // Generate random position for streaming
        FVector Position(FMath::RandRange(-500000, 500000), FMath::RandRange(-500000, 500000), 0);
        
        bool bStreamSuccess = StreamingManager->StreamInBiomeSection(Position, NextBiome);
        
        double IterationEnd = FPlatformTime::Seconds();
        
        if (bStreamSuccess && NextBiome != EBiomeType::None)
        {
            SuccessfulIterations++;
            TotalTime += (IterationEnd - IterationStart);
        }

        // Periodic cleanup to prevent memory buildup
        if (i % 100 == 99)
        {
            StreamingManager->CleanupDistantSections(Position, true);
        }
    }

    float AverageTimeMs = (TotalTime / SuccessfulIterations) * 1000.0f;
    
    if (AverageTimeMs > MaxAllowedAverageTimeMs)
    {
        AddError(FString::Printf(TEXT("Stress test average time exceeded limit: %.2fms > %.2fms"), 
            AverageTimeMs, MaxAllowedAverageTimeMs));
        bTestPassed = false;
    }

    float SuccessRate = (float)SuccessfulIterations / StressTestIterations;
    if (SuccessRate < 0.95f)
    {
        AddError(FString::Printf(TEXT("Stress test success rate too low: %.1f%%"), SuccessRate * 100.0f));
        bTestPassed = false;
    }

    AddInfo(FString::Printf(TEXT("Stress test completed: %d/%d successful (%.1f%%), average time: %.2fms"), 
        SuccessfulIterations, StressTestIterations, SuccessRate * 100.0f, AverageTimeMs));

    FWorldGenerationTestUtils::CleanupTestObjects(TestWorld);
    return bTestPassed;
}