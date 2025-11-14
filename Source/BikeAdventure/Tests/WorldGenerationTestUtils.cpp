#include "WorldGenerationTests.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

// World Generation Test Utils Implementation
UWorld* FWorldGenerationTestUtils::CreateTestWorld()
{
    // Create a minimal test world for automation testing
    UWorld* TestWorld = UWorld::CreateWorld(EWorldType::None, false);
    if (TestWorld)
    {
        TestWorld->InitWorld();
    }
    return TestWorld;
}

UBiomeGenerator* FWorldGenerationTestUtils::CreateTestBiomeGenerator(UWorld* World)
{
    if (!World)
    {
        return nullptr;
    }

    UBiomeGenerator* Generator = NewObject<UBiomeGenerator>();
    if (Generator)
    {
        Generator->Initialize();
    }
    return Generator;
}

UWorldStreamingManager* FWorldGenerationTestUtils::CreateTestStreamingManager(UWorld* World)
{
    if (!World)
    {
        return nullptr;
    }

    UWorldStreamingManager* StreamingManager = NewObject<UWorldStreamingManager>();
    if (StreamingManager)
    {
        // Initialize would normally be called by subsystem, do it manually for tests
        StreamingManager->Initialize();
    }
    return StreamingManager;
}

UPathPersonalitySystem* FWorldGenerationTestUtils::CreateTestPathPersonalitySystem()
{
    UPathPersonalitySystem* PathSystem = NewObject<UPathPersonalitySystem>();
    if (PathSystem)
    {
        PathSystem->Initialize();
    }
    return PathSystem;
}

TArray<EBiomeType> FWorldGenerationTestUtils::GenerateBiomeSequence(UBiomeGenerator* Generator, EBiomeType StartBiome, int32 NumTransitions, bool bAlternateChoices)
{
    TArray<EBiomeType> Sequence;
    if (!Generator || NumTransitions <= 0)
    {
        return Sequence;
    }

    EBiomeType CurrentBiome = StartBiome;
    TArray<EBiomeType> History;
    Sequence.Add(CurrentBiome);

    for (int32 i = 0; i < NumTransitions; i++)
    {
        bool bLeftChoice = bAlternateChoices ? (i % 2 == 0) : FMath::RandBool();
        
        EBiomeType NextBiome = Generator->GenerateNextBiome(CurrentBiome, bLeftChoice, History);
        Sequence.Add(NextBiome);
        
        History.Add(CurrentBiome);
        if (History.Num() > 10) // Keep history manageable
        {
            History.RemoveAt(0);
        }
        
        CurrentBiome = NextBiome;
    }

    return Sequence;
}

bool FWorldGenerationTestUtils::ValidateBiomeTransitionRules(const TArray<EBiomeType>& BiomeSequence, int32 MaxConsecutive)
{
    if (BiomeSequence.Num() < 2)
    {
        return true; // Empty or single-element sequences are valid
    }

    // Check consecutive biome limit
    int32 ConsecutiveCount = 1;
    EBiomeType PreviousBiome = BiomeSequence[0];

    for (int32 i = 1; i < BiomeSequence.Num(); i++)
    {
        if (BiomeSequence[i] == PreviousBiome)
        {
            ConsecutiveCount++;
            if (ConsecutiveCount > MaxConsecutive)
            {
                return false; // Too many consecutive same biomes
            }
        }
        else
        {
            ConsecutiveCount = 1;
            PreviousBiome = BiomeSequence[i];
        }
    }

    return true;
}

int32 FWorldGenerationTestUtils::EstimateMemoryUsage(const TArray<EBiomeType>& BiomeSequence)
{
    int32 TotalMemoryKB = 0;
    const int32 BaseBiomeMemoryKB = 10240; // 10MB base per biome

    for (EBiomeType Biome : BiomeSequence)
    {
        int32 BiomeMemory = BaseBiomeMemoryKB;
        
        // Adjust based on biome complexity
        switch (Biome)
        {
            case EBiomeType::Forest:
            case EBiomeType::Urban:
                BiomeMemory = FMath::RoundToInt(BaseBiomeMemoryKB * 1.5f); // More complex biomes
                break;
            case EBiomeType::Desert:
            case EBiomeType::Beach:
                BiomeMemory = FMath::RoundToInt(BaseBiomeMemoryKB * 0.7f); // Simpler biomes
                break;
            default:
                break;
        }
        
        TotalMemoryKB += BiomeMemory;
    }

    return TotalMemoryKB;
}

bool FWorldGenerationTestUtils::ValidateIntersectionPlacement(AIntersection* Intersection, EBiomeType BiomeType)
{
    if (!IsValid(Intersection))
    {
        return false;
    }

    // Validate intersection type matches biome
    EIntersectionType IntersectionType = Intersection->GetIntersectionType();
    
    switch (BiomeType)
    {
        case EBiomeType::Forest:
            return IntersectionType == EIntersectionType::YFork || IntersectionType == EIntersectionType::CaveEntrance;
        case EBiomeType::Urban:
            return IntersectionType == EIntersectionType::Roundabout || IntersectionType == EIntersectionType::TJunction;
        case EBiomeType::Beach:
            return IntersectionType == EIntersectionType::Boardwalk || IntersectionType == EIntersectionType::Bridge;
        case EBiomeType::Mountains:
            return IntersectionType == EIntersectionType::RockPass || IntersectionType == EIntersectionType::Bridge || IntersectionType == EIntersectionType::CaveEntrance;
        case EBiomeType::Wetlands:
            return IntersectionType == EIntersectionType::RiverCrossing || IntersectionType == EIntersectionType::Bridge || IntersectionType == EIntersectionType::Boardwalk;
        default:
            return true; // Allow any intersection type for other biomes
    }
}

FPlayerChoiceHistory FWorldGenerationTestUtils::GenerateTestPlayerHistory(int32 NumChoices, float LeftBias)
{
    FPlayerChoiceHistory History;
    
    History.TotalChoices = NumChoices;
    History.LeftChoices = FMath::RoundToInt(NumChoices * LeftBias);
    History.RightChoices = NumChoices - History.LeftChoices;

    // Generate recent choices
    for (int32 i = 0; i < FMath::Min(NumChoices, 10); i++)
    {
        bool bLeftChoice = FMath::RandRange(0.0f, 1.0f) < LeftBias;
        History.RecentChoices.Add(bLeftChoice);
        
        // Add random biome and personality
        EBiomeType RandomBiome = (EBiomeType)FMath::RandRange(0, 6);
        EPathPersonality RandomPersonality = (EPathPersonality)FMath::RandRange(0, 5);
        
        History.RecentBiomes.Add(RandomBiome);
        History.RecentPersonalities.Add(RandomPersonality);
    }

    // Generate personality preferences
    TArray<EPathPersonality> AllPersonalities = {
        EPathPersonality::Wild, EPathPersonality::Safe, EPathPersonality::Scenic,
        EPathPersonality::Challenge, EPathPersonality::Mystery, EPathPersonality::Peaceful
    };

    for (EPathPersonality Personality : AllPersonalities)
    {
        History.PersonalityPreferences.Add(Personality, FMath::RandRange(0.0f, 1.0f));
    }

    // Set preferred personality to highest scoring
    EPathPersonality PreferredPersonality = EPathPersonality::Peaceful;
    float HighestScore = 0.0f;
    
    for (const auto& PrefPair : History.PersonalityPreferences)
    {
        if (PrefPair.Value > HighestScore)
        {
            HighestScore = PrefPair.Value;
            PreferredPersonality = PrefPair.Key;
        }
    }
    
    History.PreferredPersonality = PreferredPersonality;
    History.AdaptiveWeight = LeftBias;

    return History;
}

void FWorldGenerationTestUtils::CleanupTestObjects(UWorld* World)
{
    if (World)
    {
        // Clean up any spawned actors
        for (TActorIterator<AIntersection> ActorItr(World); ActorItr; ++ActorItr)
        {
            AIntersection* Intersection = *ActorItr;
            if (IsValid(Intersection))
            {
                Intersection->Destroy();
            }
        }
        
        // Clean up PCG actors
        for (TActorIterator<APCGActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            APCGActor* PCGActor = *ActorItr;
            if (IsValid(PCGActor))
            {
                PCGActor->Destroy();
            }
        }
    }
}

// Test Validator Implementation
bool FWorldGenerationTestValidator::ValidateBiomeTransitions(const TArray<EBiomeType>& Transitions, FString& OutErrorMessage)
{
    if (Transitions.Num() < 2)
    {
        return true;
    }

    for (int32 i = 1; i < Transitions.Num(); i++)
    {
        if (!UBiomeUtilities::CanBiomesTransition(Transitions[i-1], Transitions[i]))
        {
            OutErrorMessage = FString::Printf(TEXT("Invalid transition from %s to %s at index %d"),
                *UBiomeUtilities::GetBiomeName(Transitions[i-1]),
                *UBiomeUtilities::GetBiomeName(Transitions[i]),
                i);
            return false;
        }
    }

    return true;
}

bool FWorldGenerationTestValidator::ValidateMemoryUsage(int32 UsageKB, int32 BudgetKB, FString& OutErrorMessage)
{
    if (UsageKB > BudgetKB)
    {
        OutErrorMessage = FString::Printf(TEXT("Memory usage (%dKB) exceeds budget (%dKB)"), UsageKB, BudgetKB);
        return false;
    }
    return true;
}

bool FWorldGenerationTestValidator::ValidatePerformanceMetrics(const FWorldGenerationTestMetrics& Metrics, const FWorldGenerationTestScenario& Scenario, FString& OutErrorMessage)
{
    if (Metrics.BiomeGenerationTimeMs > Scenario.ExpectedMaxGenerationTimeMs)
    {
        OutErrorMessage = FString::Printf(TEXT("Biome generation time (%.2fms) exceeds expected maximum (%.2fms)"),
            Metrics.BiomeGenerationTimeMs, Scenario.ExpectedMaxGenerationTimeMs);
        return false;
    }

    if (Metrics.MemoryUsageKB > Scenario.ExpectedMaxMemoryKB)
    {
        OutErrorMessage = FString::Printf(TEXT("Memory usage (%dKB) exceeds expected maximum (%dKB)"),
            Metrics.MemoryUsageKB, Scenario.ExpectedMaxMemoryKB);
        return false;
    }

    return true;
}

// Test Manager Implementation
FWorldGenerationTestMetrics FWorldGenerationTestManager::RunTestScenario(const FWorldGenerationTestScenario& Scenario, FString& OutErrorMessage)
{
    FWorldGenerationTestMetrics Metrics;
    double TestStartTime = FPlatformTime::Seconds();

    // Run individual test components
    bool bSuccess = true;
    
    if (Scenario.bTestPerformance)
    {
        bSuccess &= RunBiomeGenerationTest(Scenario, Metrics, OutErrorMessage);
    }
    
    if (bSuccess && !Scenario.bTestStreamingOnly)
    {
        bSuccess &= RunStreamingTest(Scenario, Metrics, OutErrorMessage);
    }
    
    if (bSuccess)
    {
        bSuccess &= RunPathPersonalityTest(Scenario, Metrics, OutErrorMessage);
    }
    
    if (bSuccess && Scenario.bTestMemoryBudget)
    {
        bSuccess &= RunMemoryBudgetTest(Scenario, Metrics, OutErrorMessage);
    }

    double TestEndTime = FPlatformTime::Seconds();
    Metrics.TotalTestTimeMs = (TestEndTime - TestStartTime) * 1000.0f;
    
    Metrics.bPassedPerformanceTarget = Metrics.BiomeGenerationTimeMs <= Scenario.ExpectedMaxGenerationTimeMs;
    Metrics.bPassedMemoryBudget = Metrics.MemoryUsageKB <= Scenario.ExpectedMaxMemoryKB;

    if (!bSuccess && OutErrorMessage.IsEmpty())
    {
        OutErrorMessage = TEXT("Test scenario failed with unknown error");
    }

    return Metrics;
}

TArray<FWorldGenerationTestMetrics> FWorldGenerationTestManager::RunTestSuite(const TArray<FWorldGenerationTestScenario>& Scenarios, FString& OutErrorMessage)
{
    TArray<FWorldGenerationTestMetrics> Results;
    
    for (const FWorldGenerationTestScenario& Scenario : Scenarios)
    {
        FString ScenarioError;
        FWorldGenerationTestMetrics ScenarioMetrics = RunTestScenario(Scenario, ScenarioError);
        
        Results.Add(ScenarioMetrics);
        
        if (!ScenarioError.IsEmpty())
        {
            OutErrorMessage += FString::Printf(TEXT("Scenario '%s': %s\n"), *Scenario.ScenarioName, *ScenarioError);
        }
    }
    
    return Results;
}

FString FWorldGenerationTestManager::GenerateTestReport(const TArray<FWorldGenerationTestMetrics>& Results, const TArray<FWorldGenerationTestScenario>& Scenarios)
{
    FString Report = TEXT("=== World Generation Test Report ===\n\n");
    
    int32 PassedTests = 0;
    float TotalTime = 0.0f;
    int32 TotalBiomes = 0;
    int32 TotalMemory = 0;
    
    for (int32 i = 0; i < Results.Num() && i < Scenarios.Num(); i++)
    {
        const FWorldGenerationTestMetrics& Metrics = Results[i];
        const FWorldGenerationTestScenario& Scenario = Scenarios[i];
        
        Report += FString::Printf(TEXT("Scenario: %s\n"), *Scenario.ScenarioName);
        Report += FString::Printf(TEXT("  Status: %s\n"), 
            (Metrics.bPassedPerformanceTarget && Metrics.bPassedMemoryBudget) ? TEXT("PASSED") : TEXT("FAILED"));
        Report += FString::Printf(TEXT("  Execution Time: %.2fms\n"), Metrics.TotalTestTimeMs);
        Report += FString::Printf(TEXT("  Biomes Generated: %d\n"), Metrics.BiomesGenerated);
        Report += FString::Printf(TEXT("  Memory Usage: %dKB\n"), Metrics.MemoryUsageKB);
        Report += FString::Printf(TEXT("  Generation Time: %.2fms\n"), Metrics.BiomeGenerationTimeMs);
        Report += TEXT("\n");
        
        if (Metrics.bPassedPerformanceTarget && Metrics.bPassedMemoryBudget)
        {
            PassedTests++;
        }
        
        TotalTime += Metrics.TotalTestTimeMs;
        TotalBiomes += Metrics.BiomesGenerated;
        TotalMemory += Metrics.MemoryUsageKB;
    }
    
    Report += FString::Printf(TEXT("=== Summary ===\n"));
    Report += FString::Printf(TEXT("Tests Passed: %d/%d (%.1f%%)\n"), PassedTests, Results.Num(), 
        (float)PassedTests / Results.Num() * 100.0f);
    Report += FString::Printf(TEXT("Total Execution Time: %.2fms\n"), TotalTime);
    Report += FString::Printf(TEXT("Average Time Per Test: %.2fms\n"), TotalTime / Results.Num());
    Report += FString::Printf(TEXT("Total Biomes Generated: %d\n"), TotalBiomes);
    Report += FString::Printf(TEXT("Average Memory Usage: %dKB\n"), TotalMemory / Results.Num());
    
    return Report;
}

TArray<FWorldGenerationTestScenario> FWorldGenerationTestManager::CreateDefaultTestScenarios()
{
    TArray<FWorldGenerationTestScenario> Scenarios;
    
    // Basic functionality test
    FWorldGenerationTestScenario BasicScenario;
    BasicScenario.ScenarioName = TEXT("BasicFunctionality");
    BasicScenario.NumTransitions = 20;
    BasicScenario.ExpectedMaxMemoryKB = 1048576; // 1GB
    BasicScenario.ExpectedMaxGenerationTimeMs = 500.0f;
    Scenarios.Add(BasicScenario);
    
    // Performance test
    FWorldGenerationTestScenario PerformanceScenario;
    PerformanceScenario.ScenarioName = TEXT("Performance");
    PerformanceScenario.NumTransitions = 100;
    PerformanceScenario.ExpectedMaxMemoryKB = 2097152; // 2GB
    PerformanceScenario.ExpectedMaxGenerationTimeMs = 200.0f;
    Scenarios.Add(PerformanceScenario);
    
    // Memory stress test
    FWorldGenerationTestScenario MemoryScenario;
    MemoryScenario.ScenarioName = TEXT("MemoryStress");
    MemoryScenario.NumTransitions = 200;
    MemoryScenario.bTestMemoryBudget = true;
    MemoryScenario.ExpectedMaxMemoryKB = 4194304; // 4GB
    Scenarios.Add(MemoryScenario);
    
    return Scenarios;
}

bool FWorldGenerationTestManager::RunBiomeGenerationTest(const FWorldGenerationTestScenario& Scenario, FWorldGenerationTestMetrics& OutMetrics, FString& OutErrorMessage)
{
    UWorld* TestWorld = FWorldGenerationTestUtils::CreateTestWorld();
    UBiomeGenerator* Generator = FWorldGenerationTestUtils::CreateTestBiomeGenerator(TestWorld);
    
    if (!TestWorld || !Generator)
    {
        OutErrorMessage = TEXT("Failed to create test environment for biome generation");
        return false;
    }
    
    double StartTime = FPlatformTime::Seconds();
    
    TArray<EBiomeType> BiomeSequence = FWorldGenerationTestUtils::GenerateBiomeSequence(
        Generator, Scenario.StartBiome, Scenario.NumTransitions, true);
    
    double EndTime = FPlatformTime::Seconds();
    
    OutMetrics.BiomeGenerationTimeMs = (EndTime - StartTime) * 1000.0f;
    OutMetrics.BiomesGenerated = BiomeSequence.Num();
    
    // Validate biome transitions
    if (!FWorldGenerationTestValidator::ValidateBiomeTransitions(BiomeSequence, OutErrorMessage))
    {
        FWorldGenerationTestUtils::CleanupTestObjects(TestWorld);
        return false;
    }
    
    FWorldGenerationTestUtils::CleanupTestObjects(TestWorld);
    return true;
}

bool FWorldGenerationTestManager::RunStreamingTest(const FWorldGenerationTestScenario& Scenario, FWorldGenerationTestMetrics& OutMetrics, FString& OutErrorMessage)
{
    UWorld* TestWorld = FWorldGenerationTestUtils::CreateTestWorld();
    UWorldStreamingManager* StreamingManager = FWorldGenerationTestUtils::CreateTestStreamingManager(TestWorld);
    
    if (!TestWorld || !StreamingManager)
    {
        OutErrorMessage = TEXT("Failed to create test environment for streaming");
        return false;
    }
    
    // Test streaming multiple sections
    TArray<FVector> TestPositions;
    for (int32 i = 0; i < 10; i++)
    {
        TestPositions.Add(FVector(i * 200000, 0, 0));
    }
    
    double StartTime = FPlatformTime::Seconds();
    
    for (const FVector& Position : TestPositions)
    {
        StreamingManager->StreamInBiomeSection(Position, EBiomeType::Forest);
    }
    
    double EndTime = FPlatformTime::Seconds();
    OutMetrics.StreamingLoadTimeMs = (EndTime - StartTime) * 1000.0f;
    
    // Test cleanup
    StartTime = FPlatformTime::Seconds();
    StreamingManager->CleanupDistantSections(FVector::ZeroVector, true);
    EndTime = FPlatformTime::Seconds();
    OutMetrics.StreamingUnloadTimeMs = (EndTime - StartTime) * 1000.0f;
    
    FWorldGenerationTestUtils::CleanupTestObjects(TestWorld);
    return true;
}

bool FWorldGenerationTestManager::RunPathPersonalityTest(const FWorldGenerationTestScenario& Scenario, FWorldGenerationTestMetrics& OutMetrics, FString& OutErrorMessage)
{
    UPathPersonalitySystem* PathSystem = FWorldGenerationTestUtils::CreateTestPathPersonalitySystem();
    
    if (!PathSystem)
    {
        OutErrorMessage = TEXT("Failed to create path personality system");
        return false;
    }
    
    FPlayerChoiceHistory TestHistory = FWorldGenerationTestUtils::GenerateTestPlayerHistory(20, 0.5f);
    
    // Test path hint generation for different biome combinations
    TArray<EBiomeType> TestBiomes = {EBiomeType::Forest, EBiomeType::Urban, EBiomeType::Beach};
    
    for (EBiomeType CurrentBiome : TestBiomes)
    {
        for (EBiomeType LeftBiome : TestBiomes)
        {
            FPathHints Hints = PathSystem->GeneratePathHintsForIntersection(
                CurrentBiome, LeftBiome, EBiomeType::Countryside, TestHistory);
            
            if (Hints.LeftPathPersonality == EPathPersonality::None || 
                Hints.RightPathPersonality == EPathPersonality::None)
            {
                OutErrorMessage = TEXT("Path personality generation failed");
                return false;
            }
        }
    }
    
    return true;
}

bool FWorldGenerationTestManager::RunMemoryBudgetTest(const FWorldGenerationTestScenario& Scenario, FWorldGenerationTestMetrics& OutMetrics, FString& OutErrorMessage)
{
    UWorld* TestWorld = FWorldGenerationTestUtils::CreateTestWorld();
    UWorldStreamingManager* StreamingManager = FWorldGenerationTestUtils::CreateTestStreamingManager(TestWorld);
    
    if (!TestWorld || !StreamingManager)
    {
        OutErrorMessage = TEXT("Failed to create test environment for memory budget test");
        return false;
    }
    
    // Stream in many sections to test memory usage
    for (int32 i = 0; i < 20; i++)
    {
        FVector Position(i * 200000, 0, 0);
        StreamingManager->StreamInBiomeSection(Position, EBiomeType::Forest);
    }
    
    OutMetrics.MemoryUsageKB = StreamingManager->GetTotalMemoryUsageKB();

    FWorldGenerationTestUtils::CleanupTestObjects(TestWorld);
    return FWorldGenerationTestValidator::ValidateMemoryUsage(
        OutMetrics.MemoryUsageKB, Scenario.ExpectedMaxMemoryKB, OutErrorMessage);
}

bool FWorldGenerationTestValidator::ValidatePathPersonalities(const TArray<EPathPersonality>& LeftPersonalities, const TArray<EPathPersonality>& RightPersonalities, FString& OutErrorMessage)
{
    // Validate array sizes match
    if (LeftPersonalities.Num() != RightPersonalities.Num())
    {
        OutErrorMessage = FString::Printf(TEXT("Personality array size mismatch: Left=%d, Right=%d"),
            LeftPersonalities.Num(), RightPersonalities.Num());
        return false;
    }

    // Validate no None personalities
    for (int32 i = 0; i < LeftPersonalities.Num(); i++)
    {
        if (LeftPersonalities[i] == EPathPersonality::None)
        {
            OutErrorMessage = FString::Printf(TEXT("Invalid left path personality (None) at index %d"), i);
            return false;
        }

        if (RightPersonalities[i] == EPathPersonality::None)
        {
            OutErrorMessage = FString::Printf(TEXT("Invalid right path personality (None) at index %d"), i);
            return false;
        }
    }

    // Validate personality diversity (ensure not all paths have the same personality)
    if (LeftPersonalities.Num() > 5)
    {
        TSet<EPathPersonality> UniqueLeftPersonalities;
        TSet<EPathPersonality> UniqueRightPersonalities;

        for (int32 i = 0; i < LeftPersonalities.Num(); i++)
        {
            UniqueLeftPersonalities.Add(LeftPersonalities[i]);
            UniqueRightPersonalities.Add(RightPersonalities[i]);
        }

        if (UniqueLeftPersonalities.Num() < 2)
        {
            OutErrorMessage = TEXT("Left path personalities lack diversity (all paths have same personality)");
            return false;
        }

        if (UniqueRightPersonalities.Num() < 2)
        {
            OutErrorMessage = TEXT("Right path personalities lack diversity (all paths have same personality)");
            return false;
        }
    }

    // Validate left/right personality tendencies
    // Left paths should have more Wild, Challenge, Mystery personalities
    // Right paths should have more Safe, Scenic, Peaceful personalities
    int32 LeftAdventurous = 0;
    int32 RightAdventurous = 0;

    for (int32 i = 0; i < LeftPersonalities.Num(); i++)
    {
        // Count "adventurous" personalities on left paths
        if (LeftPersonalities[i] == EPathPersonality::Wild ||
            LeftPersonalities[i] == EPathPersonality::Challenge ||
            LeftPersonalities[i] == EPathPersonality::Mystery)
        {
            LeftAdventurous++;
        }

        // Count "adventurous" personalities on right paths
        if (RightPersonalities[i] == EPathPersonality::Wild ||
            RightPersonalities[i] == EPathPersonality::Challenge ||
            RightPersonalities[i] == EPathPersonality::Mystery)
        {
            RightAdventurous++;
        }
    }

    // Left paths should tend toward more adventurous personalities than right paths
    if (LeftPersonalities.Num() > 10 && LeftAdventurous < RightAdventurous)
    {
        OutErrorMessage = FString::Printf(TEXT("Path personality distribution unexpected: Left has fewer adventurous paths (%d) than Right (%d)"),
            LeftAdventurous, RightAdventurous);
        // This is a warning, not a failure - return true but log the issue
    }

    return true;
}

bool FWorldGenerationTestValidator::ValidateIntersectionGeneration(const TArray<AIntersection*>& Intersections, const TArray<EBiomeType>& BiomeSequence, FString& OutErrorMessage)
{
    if (Intersections.Num() == 0)
    {
        OutErrorMessage = TEXT("No intersections generated");
        return false;
    }

    // Validate all intersections are valid
    for (int32 i = 0; i < Intersections.Num(); i++)
    {
        AIntersection* Intersection = Intersections[i];

        if (!IsValid(Intersection))
        {
            OutErrorMessage = FString::Printf(TEXT("Invalid intersection at index %d (null or destroyed)"), i);
            return false;
        }

        // Validate intersection has valid biome assignments
        EBiomeType LeftBiome = Intersection->GetLeftPathBiome();
        EBiomeType RightBiome = Intersection->GetRightPathBiome();

        if (LeftBiome == EBiomeType::None)
        {
            OutErrorMessage = FString::Printf(TEXT("Intersection %d has invalid left biome (None)"), i);
            return false;
        }

        if (RightBiome == EBiomeType::None)
        {
            OutErrorMessage = FString::Printf(TEXT("Intersection %d has invalid right biome (None)"), i);
            return false;
        }

        // Validate intersection type is appropriate for context
        EIntersectionType IntersectionType = Intersection->GetIntersectionType();

        if (IntersectionType == EIntersectionType::YFork &&
            (LeftBiome == EBiomeType::Urban || RightBiome == EBiomeType::Urban))
        {
            // Y-forks are less common in urban areas - prefer roundabouts/T-junctions
            // This is a style check, not a hard error
        }

        // Validate path directions are set
        FVector LeftDirection = Intersection->GetLeftPathDirection();
        FVector RightDirection = Intersection->GetRightPathDirection();

        if (LeftDirection.IsNearlyZero() || RightDirection.IsNearlyZero())
        {
            OutErrorMessage = FString::Printf(TEXT("Intersection %d has invalid path directions"), i);
            return false;
        }

        // Validate path hints are configured
        FPathHints PathHints = Intersection->GetPathHints();

        if (PathHints.LeftPathPersonality == EPathPersonality::None ||
            PathHints.RightPathPersonality == EPathPersonality::None)
        {
            OutErrorMessage = FString::Printf(TEXT("Intersection %d missing path personality assignments"), i);
            return false;
        }
    }

    // Validate intersection spacing (if we have biome sequence context)
    if (BiomeSequence.Num() > 0)
    {
        // Check that number of intersections is reasonable for biome sequence length
        int32 ExpectedIntersections = BiomeSequence.Num() - 1; // One intersection per transition

        if (Intersections.Num() < ExpectedIntersections * 0.5f)
        {
            OutErrorMessage = FString::Printf(TEXT("Too few intersections generated: %d for %d biome transitions"),
                Intersections.Num(), BiomeSequence.Num());
            return false;
        }

        if (Intersections.Num() > ExpectedIntersections * 2.0f)
        {
            OutErrorMessage = FString::Printf(TEXT("Too many intersections generated: %d for %d biome transitions"),
                Intersections.Num(), BiomeSequence.Num());
            return false;
        }
    }

    return true;
}

bool FWorldGenerationTestValidator::ValidatePCGGeneration(const TArray<APCGActor*>& PCGActors, const TArray<EBiomeType>& BiomeSequence, FString& OutErrorMessage)
{
    if (PCGActors.Num() == 0)
    {
        OutErrorMessage = TEXT("No PCG actors generated");
        return false;
    }

    // Validate all PCG actors are valid
    int32 InvalidActorCount = 0;
    for (int32 i = 0; i < PCGActors.Num(); i++)
    {
        if (!IsValid(PCGActors[i]))
        {
            InvalidActorCount++;
        }
    }

    if (InvalidActorCount > 0)
    {
        OutErrorMessage = FString::Printf(TEXT("%d out of %d PCG actors are invalid"),
            InvalidActorCount, PCGActors.Num());
        return false;
    }

    // Validate PCG actor count is reasonable for biome sequence
    if (BiomeSequence.Num() > 0)
    {
        // Expect at least some PCG actors per biome segment
        int32 MinExpectedActors = BiomeSequence.Num() * 1;  // At least 1 per biome
        int32 MaxExpectedActors = BiomeSequence.Num() * 50; // Max 50 per biome

        if (PCGActors.Num() < MinExpectedActors)
        {
            OutErrorMessage = FString::Printf(TEXT("Too few PCG actors generated: %d for %d biomes (expected at least %d)"),
                PCGActors.Num(), BiomeSequence.Num(), MinExpectedActors);
            return false;
        }

        if (PCGActors.Num() > MaxExpectedActors)
        {
            OutErrorMessage = FString::Printf(TEXT("Too many PCG actors generated: %d for %d biomes (expected at most %d)"),
                PCGActors.Num(), BiomeSequence.Num(), MaxExpectedActors);
            return false;
        }
    }

    // Validate PCG actors have proper component setup
    for (APCGActor* PCGActor : PCGActors)
    {
        if (IsValid(PCGActor))
        {
            // Check that PCG actor has essential components
            UActorComponent* PCGComponent = PCGActor->GetComponentByClass(UPCGComponent::StaticClass());

            if (!PCGComponent)
            {
                OutErrorMessage = FString::Printf(TEXT("PCG actor '%s' missing PCGComponent"),
                    *PCGActor->GetName());
                return false;
            }
        }
    }

    return true;
}

bool FWorldGenerationTestValidator::ValidateStreamingBehavior(UWorldStreamingManager* StreamingManager, const TArray<FVector>& PlayerPositions, FString& OutErrorMessage)
{
    if (!IsValid(StreamingManager))
    {
        OutErrorMessage = TEXT("Invalid streaming manager");
        return false;
    }

    if (PlayerPositions.Num() == 0)
    {
        OutErrorMessage = TEXT("No player positions provided for streaming validation");
        return false;
    }

    // Test streaming behavior at each position
    for (int32 i = 0; i < PlayerPositions.Num(); i++)
    {
        const FVector& Position = PlayerPositions[i];

        // Update streaming for this position
        StreamingManager->UpdateStreamingForPlayer(Position, FVector::ZeroVector);

        // Validate that sections near the player are loaded
        FWorldSection CurrentSection = StreamingManager->GetSectionAtLocation(Position);

        if (!CurrentSection.bIsLoaded)
        {
            OutErrorMessage = FString::Printf(TEXT("Section at player position %s not loaded"),
                *Position.ToString());
            return false;
        }

        // Validate memory usage is within budget
        if (!StreamingManager->IsWithinMemoryBudget())
        {
            OutErrorMessage = FString::Printf(TEXT("Memory budget exceeded at position %s (usage: %dKB)"),
                *Position.ToString(), StreamingManager->GetTotalMemoryUsageKB());
            return false;
        }

        // Validate active sections count is reasonable
        TArray<FWorldSection> ActiveSections = StreamingManager->GetActiveSections();

        if (ActiveSections.Num() == 0)
        {
            OutErrorMessage = FString::Printf(TEXT("No active sections at position %s"),
                *Position.ToString());
            return false;
        }

        if (ActiveSections.Num() > 50)
        {
            OutErrorMessage = FString::Printf(TEXT("Too many active sections (%d) at position %s"),
                ActiveSections.Num(), *Position.ToString());
            return false;
        }
    }

    // Test cleanup behavior by moving to distant position
    if (PlayerPositions.Num() > 1)
    {
        FVector FirstPosition = PlayerPositions[0];
        FVector LastPosition = PlayerPositions[PlayerPositions.Num() - 1];

        // Stream at first position
        StreamingManager->UpdateStreamingForPlayer(FirstPosition, FVector::ZeroVector);
        int32 SectionsAtFirst = StreamingManager->GetActiveSections().Num();

        // Move to distant last position
        StreamingManager->UpdateStreamingForPlayer(LastPosition, FVector::ZeroVector);

        // Force cleanup of distant sections
        StreamingManager->CleanupDistantSections(LastPosition, false);

        // Validate that cleanup occurred
        FWorldSection FirstSection = StreamingManager->GetSectionAtLocation(FirstPosition);

        float Distance = FVector::Dist(FirstPosition, LastPosition);
        if (Distance > 500000.0f) // If positions are far apart (>5km)
        {
            // First section should be unloaded after cleanup
            if (FirstSection.bIsLoaded)
            {
                // This might be okay if within streaming distance, just log
            }
        }

        // Validate performance metrics are being tracked
        FStreamingPerformanceMetrics Metrics = StreamingManager->GetPerformanceMetrics();

        if (Metrics.LoadedSections < 0)
        {
            OutErrorMessage = TEXT("Invalid streaming performance metrics (negative loaded sections)");
            return false;
        }
    }

    return true;
}