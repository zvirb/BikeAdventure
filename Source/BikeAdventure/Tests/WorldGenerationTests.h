#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"
#include "../Core/BiomeTypes.h"
#include "../Systems/BiomeGenerator.h"
#include "../Systems/WorldStreamingManager.h"
#include "../Systems/PathPersonalitySystem.h"
#include "../Gameplay/Intersection.h"

/**
 * Test suite for biome transition validation
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBiomeTransitionTest, "BikeAdventure.WorldGeneration.BiomeTransition", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)

/**
 * Test suite for biome generation consistency
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBiomeGenerationConsistencyTest, "BikeAdventure.WorldGeneration.BiomeConsistency", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)

/**
 * Test suite for world streaming performance
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWorldStreamingPerformanceTest, "BikeAdventure.WorldGeneration.StreamingPerformance", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::PerfFilter)

/**
 * Test suite for path personality system
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPathPersonalityTest, "BikeAdventure.WorldGeneration.PathPersonality", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)

/**
 * Test suite for memory budget validation
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemoryBudgetTest, "BikeAdventure.WorldGeneration.MemoryBudget", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::PerfFilter)

/**
 * Test suite for intersection generation
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntersectionGenerationTest, "BikeAdventure.WorldGeneration.IntersectionGeneration", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)

/**
 * Test suite for PCG integration
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPCGIntegrationTest, "BikeAdventure.WorldGeneration.PCGIntegration", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)

/**
 * Comprehensive test suite that runs multiple scenarios
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FComprehensiveWorldGenerationTest, "BikeAdventure.WorldGeneration.Comprehensive")

/**
 * Stress test for continuous world generation
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWorldGenerationStressTest, "BikeAdventure.WorldGeneration.StressTest", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::StressFilter)

/**
 * Utility class for world generation testing
 */
class BIKEADVENTURE_API FWorldGenerationTestUtils
{
public:
    /**
     * Create a test world with minimal setup
     */
    static UWorld* CreateTestWorld();

    /**
     * Create a test biome generator with default settings
     */
    static UBiomeGenerator* CreateTestBiomeGenerator(UWorld* World);

    /**
     * Create a test world streaming manager
     */
    static UWorldStreamingManager* CreateTestStreamingManager(UWorld* World);

    /**
     * Create a test path personality system
     */
    static UPathPersonalitySystem* CreateTestPathPersonalitySystem();

    /**
     * Generate a sequence of biome transitions for testing
     */
    static TArray<EBiomeType> GenerateBiomeSequence(UBiomeGenerator* Generator, EBiomeType StartBiome, int32 NumTransitions, bool bAlternateChoices = true);

    /**
     * Validate that biome transitions follow the rules
     */
    static bool ValidateBiomeTransitionRules(const TArray<EBiomeType>& BiomeSequence, int32 MaxConsecutive = 3);

    /**
     * Calculate memory usage estimation for a biome sequence
     */
    static int32 EstimateMemoryUsage(const TArray<EBiomeType>& BiomeSequence);

    /**
     * Validate intersection placement rules
     */
    static bool ValidateIntersectionPlacement(AIntersection* Intersection, EBiomeType BiomeType);

    /**
     * Generate test player choice history
     */
    static FPlayerChoiceHistory GenerateTestPlayerHistory(int32 NumChoices, float LeftBias = 0.5f);

    /**
     * Cleanup test objects
     */
    static void CleanupTestObjects(UWorld* World);
};

/**
 * Performance metrics collection for world generation tests
 */
USTRUCT()
struct BIKEADVENTURE_API FWorldGenerationTestMetrics
{
    GENERATED_BODY()

    FWorldGenerationTestMetrics()
    {
        BiomeGenerationTimeMs = 0.0f;
        IntersectionGenerationTimeMs = 0.0f;
        PCGGenerationTimeMs = 0.0f;
        MemoryUsageKB = 0;
        StreamingLoadTimeMs = 0.0f;
        StreamingUnloadTimeMs = 0.0f;
        TotalTestTimeMs = 0.0f;
        bPassedMemoryBudget = true;
        bPassedPerformanceTarget = true;
    }

    // Time taken to generate biomes
    float BiomeGenerationTimeMs;

    // Time taken to generate intersections
    float IntersectionGenerationTimeMs;

    // Time taken for PCG generation
    float PCGGenerationTimeMs;

    // Memory usage during test
    int32 MemoryUsageKB;

    // Streaming load time
    float StreamingLoadTimeMs;

    // Streaming unload time
    float StreamingUnloadTimeMs;

    // Total test execution time
    float TotalTestTimeMs;

    // Whether memory budget was respected
    bool bPassedMemoryBudget;

    // Whether performance targets were met
    bool bPassedPerformanceTarget;

    // Number of biomes generated
    int32 BiomesGenerated;

    // Number of intersections generated
    int32 IntersectionsGenerated;

    // Number of PCG actors created
    int32 PCGActorsCreated;
};

/**
 * Test scenario configuration
 */
USTRUCT()
struct BIKEADVENTURE_API FWorldGenerationTestScenario
{
    GENERATED_BODY()

    FWorldGenerationTestScenario()
    {
        ScenarioName = TEXT("DefaultScenario");
        StartBiome = EBiomeType::Countryside;
        NumTransitions = 10;
        bTestMemoryBudget = true;
        bTestPerformance = true;
        bTestStreamingOnly = false;
        ExpectedMaxMemoryKB = 4194304; // 4GB
        ExpectedMaxGenerationTimeMs = 1000.0f;
        PlayerChoicePattern = 0.5f; // Balanced choices
    }

    // Name of the test scenario
    FString ScenarioName;

    // Starting biome for the test
    EBiomeType StartBiome;

    // Number of biome transitions to test
    int32 NumTransitions;

    // Whether to test memory budget compliance
    bool bTestMemoryBudget;

    // Whether to test performance metrics
    bool bTestPerformance;

    // Whether to test only streaming without generation
    bool bTestStreamingOnly;

    // Expected maximum memory usage in KB
    int32 ExpectedMaxMemoryKB;

    // Expected maximum generation time in milliseconds
    float ExpectedMaxGenerationTimeMs;

    // Player choice pattern (0.0 = always left, 1.0 = always right, 0.5 = balanced)
    float PlayerChoicePattern;

    // Biomes to focus testing on (empty = test all)
    TArray<EBiomeType> BiomesToTest;

    // Personalities to test (empty = test all)
    TArray<EPathPersonality> PersonalitiesToTest;
};

/**
 * Test result validation class
 */
class BIKEADVENTURE_API FWorldGenerationTestValidator
{
public:
    /**
     * Validate biome transition rules
     */
    static bool ValidateBiomeTransitions(const TArray<EBiomeType>& Transitions, FString& OutErrorMessage);

    /**
     * Validate memory usage compliance
     */
    static bool ValidateMemoryUsage(int32 UsageKB, int32 BudgetKB, FString& OutErrorMessage);

    /**
     * Validate performance metrics
     */
    static bool ValidatePerformanceMetrics(const FWorldGenerationTestMetrics& Metrics, const FWorldGenerationTestScenario& Scenario, FString& OutErrorMessage);

    /**
     * Validate path personality assignments
     */
    static bool ValidatePathPersonalities(const TArray<EPathPersonality>& LeftPersonalities, const TArray<EPathPersonality>& RightPersonalities, FString& OutErrorMessage);

    /**
     * Validate intersection generation
     */
    static bool ValidateIntersectionGeneration(const TArray<AIntersection*>& Intersections, const TArray<EBiomeType>& BiomeSequence, FString& OutErrorMessage);

    /**
     * Validate PCG generation results
     */
    static bool ValidatePCGGeneration(const TArray<class APCGActor*>& PCGActors, const TArray<EBiomeType>& BiomeSequence, FString& OutErrorMessage);

    /**
     * Validate streaming behavior
     */
    static bool ValidateStreamingBehavior(UWorldStreamingManager* StreamingManager, const TArray<FVector>& PlayerPositions, FString& OutErrorMessage);
};

/**
 * Automated test execution manager
 */
class BIKEADVENTURE_API FWorldGenerationTestManager
{
public:
    /**
     * Run a complete test scenario
     */
    static FWorldGenerationTestMetrics RunTestScenario(const FWorldGenerationTestScenario& Scenario, FString& OutErrorMessage);

    /**
     * Run multiple test scenarios in sequence
     */
    static TArray<FWorldGenerationTestMetrics> RunTestSuite(const TArray<FWorldGenerationTestScenario>& Scenarios, FString& OutErrorMessage);

    /**
     * Generate comprehensive test report
     */
    static FString GenerateTestReport(const TArray<FWorldGenerationTestMetrics>& Results, const TArray<FWorldGenerationTestScenario>& Scenarios);

    /**
     * Create default test scenarios
     */
    static TArray<FWorldGenerationTestScenario> CreateDefaultTestScenarios();

private:
    /**
     * Run biome generation test
     */
    static bool RunBiomeGenerationTest(const FWorldGenerationTestScenario& Scenario, FWorldGenerationTestMetrics& OutMetrics, FString& OutErrorMessage);

    /**
     * Run streaming test
     */
    static bool RunStreamingTest(const FWorldGenerationTestScenario& Scenario, FWorldGenerationTestMetrics& OutMetrics, FString& OutErrorMessage);

    /**
     * Run path personality test
     */
    static bool RunPathPersonalityTest(const FWorldGenerationTestScenario& Scenario, FWorldGenerationTestMetrics& OutMetrics, FString& OutErrorMessage);

    /**
     * Run memory budget test
     */
    static bool RunMemoryBudgetTest(const FWorldGenerationTestScenario& Scenario, FWorldGenerationTestMetrics& OutMetrics, FString& OutErrorMessage);
};