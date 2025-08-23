#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/Level.h"
#include "../Core/BiomeTypes.h"
#include "WorldStreamingManager.generated.h"

class ULevelStreamingDynamic;
class UBiomeGenerator;
class AIntersection;

/**
 * Structure representing a streaming world section
 */
USTRUCT(BlueprintType)
struct BIKEADVENTURE_API FWorldSection
{
    GENERATED_BODY()

    FWorldSection()
    {
        SectionCoordinates = FIntVector::ZeroValue;
        BiomeType = EBiomeType::None;
        bIsLoaded = false;
        bIsVisible = false;
        LastAccessTime = 0.0f;
        MemoryUsageKB = 0;
        bHasIntersection = false;
    }

    // Grid coordinates of this section
    UPROPERTY(BlueprintReadOnly, Category = "Section Info")
    FIntVector SectionCoordinates;

    // Biome type for this section
    UPROPERTY(BlueprintReadOnly, Category = "Section Info")
    EBiomeType BiomeType;

    // World position of section center
    UPROPERTY(BlueprintReadOnly, Category = "Section Info")
    FVector WorldPosition;

    // Section bounds in world space
    UPROPERTY(BlueprintReadOnly, Category = "Section Info")
    FBox WorldBounds;

    // Whether this section is currently loaded
    UPROPERTY(BlueprintReadOnly, Category = "Section State")
    bool bIsLoaded;

    // Whether this section is currently visible
    UPROPERTY(BlueprintReadOnly, Category = "Section State")
    bool bIsVisible;

    // Last time this section was accessed by the player
    UPROPERTY(BlueprintReadOnly, Category = "Section State")
    float LastAccessTime;

    // Memory usage of this section in KB
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MemoryUsageKB;

    // Whether this section contains an intersection
    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    bool bHasIntersection;

    // Reference to the streaming level
    UPROPERTY()
    ULevelStreamingDynamic* StreamingLevel;

    // Active PCG actors in this section
    UPROPERTY()
    TArray<class APCGActor*> PCGActors;

    // Intersection actor if present
    UPROPERTY()
    AIntersection* IntersectionActor;
};

/**
 * Performance metrics for world streaming
 */
USTRUCT(BlueprintType)
struct BIKEADVENTURE_API FStreamingPerformanceMetrics
{
    GENERATED_BODY()

    FStreamingPerformanceMetrics()
    {
        TotalMemoryUsageKB = 0;
        ActiveSections = 0;
        LoadedSections = 0;
        StreamingLoadTime = 0.0f;
        StreamingUnloadTime = 0.0f;
        FrameTimeImpactMs = 0.0f;
        bWithinMemoryBudget = true;
    }

    // Total memory usage of all loaded sections
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 TotalMemoryUsageKB;

    // Number of currently active (visible) sections
    UPROPERTY(BlueprintReadOnly, Category = "Sections")
    int32 ActiveSections;

    // Number of currently loaded sections
    UPROPERTY(BlueprintReadOnly, Category = "Sections")
    int32 LoadedSections;

    // Average loading time for sections
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float StreamingLoadTime;

    // Average unloading time for sections
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float StreamingUnloadTime;

    // Frame time impact of streaming operations
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeImpactMs;

    // Whether we're within the memory budget
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    bool bWithinMemoryBudget;
};

/**
 * World streaming manager for efficient memory usage and seamless exploration
 * Manages dynamic loading/unloading of world sections based on player position
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API UWorldStreamingManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Stream in a new biome section based on player movement
     * @param PlayerLocation - Current player world location
     * @param BiomeType - Type of biome to stream in
     * @param Direction - Direction from player for section placement
     * @return Success of streaming operation
     */
    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    bool StreamInBiomeSection(const FVector& PlayerLocation, EBiomeType BiomeType, const FVector& Direction = FVector::ZeroVector);

    /**
     * Cleanup distant sections to manage memory usage
     * @param PlayerLocation - Current player world location
     * @param ForceCleanup - Force cleanup regardless of distance
     */
    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void CleanupDistantSections(const FVector& PlayerLocation, bool bForceCleanup = false);

    /**
     * Update streaming based on current player position
     * @param PlayerLocation - Current player world location
     * @param PlayerVelocity - Player movement velocity for prediction
     */
    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void UpdateStreamingForPlayer(const FVector& PlayerLocation, const FVector& PlayerVelocity = FVector::ZeroVector);

    /**
     * Get the world section at the given world coordinates
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Streaming")
    FWorldSection GetSectionAtLocation(const FVector& WorldLocation);

    /**
     * Get all currently active world sections
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Streaming")
    TArray<FWorldSection> GetActiveSections();

    /**
     * Get current streaming performance metrics
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    FStreamingPerformanceMetrics GetPerformanceMetrics();

    /**
     * Preload sections in the specified direction for smoother experience
     */
    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void PreloadSections(const FVector& PlayerLocation, const FVector& MovementDirection, int32 PreloadDistance = 2);

    /**
     * Force unload a specific section
     */
    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void ForceUnloadSection(const FIntVector& SectionCoordinates);

    /**
     * Get memory usage statistics
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    int32 GetTotalMemoryUsageKB();

    /**
     * Check if we're within the memory budget
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    bool IsWithinMemoryBudget();

protected:
    // Maximum streaming distance in Unreal units (5km default)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings", meta = (ClampMin = "1000.0", ClampMax = "10000.0"))
    float MaxStreamingDistanceCm;

    // Maximum number of active sections (3x3 grid around player)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings", meta = (ClampMin = "4", ClampMax = "25"))
    int32 MaxActiveSections;

    // Size of each world section in Unreal units
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings", meta = (ClampMin = "1000.0", ClampMax = "10000.0"))
    float SectionSizeCm;

    // Maximum memory budget in KB (4GB = 4194304 KB)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings", meta = (ClampMin = "1000000", ClampMax = "8388608"))
    int32 MaxMemoryBudgetKB;

    // Time threshold for unloading unused sections (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings", meta = (ClampMin = "10.0", ClampMax = "300.0"))
    float UnloadTimeThreshold;

    // Enable predictive loading based on player velocity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    bool bEnablePredictiveLoading;

    // Distance multiplier for predictive loading
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings", meta = (ClampMin = "1.0", ClampMax = "5.0"))
    float PredictiveLoadingMultiplier;

    // Reference to the biome generator
    UPROPERTY()
    UBiomeGenerator* BiomeGenerator;

    // Active streaming sections mapped by coordinates
    UPROPERTY()
    TMap<FIntVector, FWorldSection> ActiveSections;

    // Sections pending loading
    UPROPERTY()
    TArray<FIntVector> PendingLoadSections;

    // Sections pending unloading  
    UPROPERTY()
    TArray<FIntVector> PendingUnloadSections;

    // Performance metrics tracking
    UPROPERTY()
    FStreamingPerformanceMetrics PerformanceMetrics;

    // Last player position for tracking movement
    UPROPERTY()
    FVector LastPlayerPosition;

private:
    /**
     * Convert world position to section coordinates
     */
    FIntVector WorldToSectionCoordinates(const FVector& WorldLocation);

    /**
     * Convert section coordinates to world position
     */
    FVector SectionCoordinatesToWorld(const FIntVector& SectionCoordinates);

    /**
     * Create a new world section at the specified coordinates
     */
    FWorldSection CreateWorldSection(const FIntVector& SectionCoordinates, EBiomeType BiomeType);

    /**
     * Load a world section asynchronously
     */
    void LoadSection(const FIntVector& SectionCoordinates);

    /**
     * Unload a world section and clean up resources
     */
    void UnloadSection(const FIntVector& SectionCoordinates);

    /**
     * Update performance metrics
     */
    void UpdatePerformanceMetrics();

    /**
     * Calculate memory usage for a section
     */
    int32 CalculateSectionMemoryUsage(const FWorldSection& Section);

    /**
     * Get sections within streaming distance of player
     */
    TArray<FIntVector> GetSectionsInRange(const FVector& PlayerLocation);

    /**
     * Determine biome type for a new section based on context
     */
    EBiomeType DetermineSectionBiome(const FIntVector& SectionCoordinates, const FVector& PlayerLocation);

    /**
     * Handle section loading completion
     */
    UFUNCTION()
    void OnSectionLoadCompleted(ULevelStreamingDynamic* StreamingLevel);

    /**
     * Handle section unloading completion
     */
    UFUNCTION()
    void OnSectionUnloadCompleted(ULevelStreamingDynamic* StreamingLevel);

public:
    // Delegates for streaming events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSectionLoaded, const FIntVector&, SectionCoordinates, EBiomeType, BiomeType);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSectionUnloaded, const FIntVector&, SectionCoordinates, EBiomeType, BiomeType);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryBudgetExceeded, int32, CurrentUsageKB);

    UPROPERTY(BlueprintAssignable, Category = "Streaming Events")
    FOnSectionLoaded OnSectionLoadedEvent;

    UPROPERTY(BlueprintAssignable, Category = "Streaming Events")  
    FOnSectionUnloaded OnSectionUnloadedEvent;

    UPROPERTY(BlueprintAssignable, Category = "Streaming Events")
    FOnMemoryBudgetExceeded OnMemoryBudgetExceededEvent;
};