#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../Core/BiomeTypes.h"
#include "PerformanceOptimizationSystem.generated.h"

class UStaticMeshComponent;
class UNiagaraComponent;
class APCGActor;

/**
 * LOD (Level of Detail) configuration for biome elements
 */
USTRUCT(BlueprintType)
struct BIKEADVENTURE_API FBiomeLODConfig
{
    GENERATED_BODY()

    FBiomeLODConfig()
    {
        LOD0Distance = 1000.0f;
        LOD1Distance = 3000.0f;
        LOD2Distance = 6000.0f;
        CullingDistance = 10000.0f;
        bEnableLOD = true;
        bEnableDistanceCulling = true;
        ParticleLODMultiplier = 1.0f;
        AudioLODMultiplier = 1.0f;
    }

    // Distance for highest detail LOD (0-1000m)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Distances", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float LOD0Distance;

    // Distance for medium detail LOD (1000-3000m)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Distances", meta = (ClampMin = "1000.0", ClampMax = "5000.0"))
    float LOD1Distance;

    // Distance for low detail LOD (3000-6000m)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Distances", meta = (ClampMin = "2000.0", ClampMax = "8000.0"))
    float LOD2Distance;

    // Distance beyond which objects are culled completely
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Distances", meta = (ClampMin = "5000.0", ClampMax = "15000.0"))
    float CullingDistance;

    // Whether LOD system is enabled for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableLOD;

    // Whether distance culling is enabled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableDistanceCulling;

    // Multiplier for particle system LOD (0.0 = no particles, 1.0 = full particles)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects LOD", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ParticleLODMultiplier;

    // Multiplier for audio LOD (0.0 = no audio, 1.0 = full audio)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects LOD", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AudioLODMultiplier;

    // Mesh LOD settings for different distances
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh LOD")
    TArray<int32> MeshLODLevels;
};

/**
 * Performance metrics for monitoring system performance
 */
USTRUCT(BlueprintType)
struct BIKEADVENTURE_API FPerformanceMetrics
{
    GENERATED_BODY()

    FPerformanceMetrics()
    {
        FrameTimeMs = 0.0f;
        MemoryUsageMB = 0.0f;
        DrawCalls = 0;
        VisibleObjects = 0;
        ActiveParticleSystems = 0;
        StreamingSectionsLoaded = 0;
        LODLevel = 0;
        bWithinPerformanceTarget = true;
        CPUUsagePercent = 0.0f;
        GPUUsagePercent = 0.0f;
    }

    // Current frame time in milliseconds
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMs;

    // Current memory usage in megabytes
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    // Number of draw calls this frame
    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 DrawCalls;

    // Number of visible objects
    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 VisibleObjects;

    // Number of active particle systems
    UPROPERTY(BlueprintReadOnly, Category = "Effects")
    int32 ActiveParticleSystems;

    // Number of streaming sections currently loaded
    UPROPERTY(BlueprintReadOnly, Category = "Streaming")
    int32 StreamingSectionsLoaded;

    // Current average LOD level (0 = highest detail, 3 = lowest)
    UPROPERTY(BlueprintReadOnly, Category = "LOD")
    int32 LODLevel;

    // Whether we're within performance targets
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bWithinPerformanceTarget;

    // CPU usage percentage
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUUsagePercent;

    // GPU usage percentage
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUUsagePercent;
};

/**
 * Performance optimization settings
 */
USTRUCT(BlueprintType)
struct BIKEADVENTURE_API FPerformanceOptimizationSettings
{
    GENERATED_BODY()

    FPerformanceOptimizationSettings()
    {
        TargetFrameRate = 60.0f;
        MaxMemoryBudgetMB = 4096.0f;
        AdaptiveLODBias = 1.0f;
        bEnableAdaptiveOptimization = true;
        bEnableAggressiveOptimization = false;
        ParticleOptimizationLevel = 1;
        ShadowOptimizationLevel = 1;
        TextureOptimizationLevel = 1;
        CullingOptimizationLevel = 1;
    }

    // Target frame rate for optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets", meta = (ClampMin = "30.0", ClampMax = "120.0"))
    float TargetFrameRate;

    // Maximum memory budget in megabytes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets", meta = (ClampMin = "1024.0", ClampMax = "8192.0"))
    float MaxMemoryBudgetMB;

    // Adaptive LOD bias multiplier (higher = more aggressive LOD)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Settings", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float AdaptiveLODBias;

    // Whether adaptive optimization is enabled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Settings")
    bool bEnableAdaptiveOptimization;

    // Whether to use aggressive optimization when needed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Settings")
    bool bEnableAggressiveOptimization;

    // Particle optimization level (0 = off, 1 = moderate, 2 = aggressive)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Levels", meta = (ClampMin = "0", ClampMax = "2"))
    int32 ParticleOptimizationLevel;

    // Shadow optimization level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Levels", meta = (ClampMin = "0", ClampMax = "2"))
    int32 ShadowOptimizationLevel;

    // Texture optimization level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Levels", meta = (ClampMin = "0", ClampMax = "2"))
    int32 TextureOptimizationLevel;

    // Culling optimization level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Levels", meta = (ClampMin = "0", ClampMax = "2"))
    int32 CullingOptimizationLevel;
};

/**
 * Main performance optimization system
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API UPerformanceOptimizationSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Update the performance optimization system
     * Should be called every frame from game mode or similar
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void UpdateOptimization(const FVector& PlayerLocation, const FVector& PlayerVelocity = FVector::ZeroVector);

    /**
     * Get current performance metrics
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Optimization")
    FPerformanceMetrics GetCurrentMetrics() const;

    /**
     * Set optimization settings
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void SetOptimizationSettings(const FPerformanceOptimizationSettings& NewSettings);

    /**
     * Get current optimization settings
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Optimization")
    FPerformanceOptimizationSettings GetOptimizationSettings() const;

    /**
     * Enable or disable adaptive optimization
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void SetAdaptiveOptimizationEnabled(bool bEnabled);

    /**
     * Set LOD configuration for a specific biome
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void SetBiomeLODConfig(EBiomeType BiomeType, const FBiomeLODConfig& LODConfig);

    /**
     * Get LOD configuration for a specific biome
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance Optimization")
    FBiomeLODConfig GetBiomeLODConfig(EBiomeType BiomeType) const;

    /**
     * Optimize all objects within a specific distance from the player
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizeObjectsInRadius(const FVector& Center, float Radius);

    /**
     * Force apply specific LOD level to all objects
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void ForceApplyLODLevel(int32 LODLevel);

    /**
     * Get memory usage breakdown
     */
    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    TMap<FString, float> GetMemoryUsageBreakdown();

protected:
    // Performance optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerformanceOptimizationSettings OptimizationSettings;

    // LOD configurations for different biomes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    TMap<EBiomeType, FBiomeLODConfig> BiomeLODConfigs;

    // Current performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FPerformanceMetrics CurrentMetrics;

    // Objects currently being tracked for optimization
    UPROPERTY()
    TArray<TWeakObjectPtr<UStaticMeshComponent>> TrackedMeshComponents;

    UPROPERTY()
    TArray<TWeakObjectPtr<UNiagaraComponent>> TrackedParticleSystems;

    UPROPERTY()
    TArray<TWeakObjectPtr<APCGActor>> TrackedPCGActors;

    // Performance history for adaptive optimization
    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    TArray<float> MemoryUsageHistory;

    // Timer for periodic optimization updates
    UPROPERTY()
    float OptimizationUpdateTimer;

    // Current adaptive LOD bias
    UPROPERTY()
    float CurrentLODBias;

private:
    /**
     * Update performance metrics
     */
    void UpdatePerformanceMetrics();

    /**
     * Apply adaptive optimizations based on current performance
     */
    void ApplyAdaptiveOptimizations();

    /**
     * Update LOD for tracked components
     */
    void UpdateComponentLODs(const FVector& PlayerLocation);

    /**
     * Optimize particle systems based on distance and performance
     */
    void OptimizeParticleSystems(const FVector& PlayerLocation);

    /**
     * Optimize PCG actors
     */
    void OptimizePCGActors(const FVector& PlayerLocation);

    /**
     * Calculate LOD level for a given distance and biome
     */
    int32 CalculateLODLevel(float Distance, EBiomeType BiomeType) const;

    /**
     * Apply LOD to a static mesh component
     */
    void ApplyMeshLOD(UStaticMeshComponent* MeshComponent, int32 LODLevel);

    /**
     * Apply optimization to particle system
     */
    void ApplyParticleOptimization(UNiagaraComponent* ParticleSystem, float Distance, int32 OptimizationLevel);

    /**
     * Register component for optimization tracking
     */
    void RegisterComponentForOptimization(UStaticMeshComponent* MeshComponent);
    void RegisterParticleSystemForOptimization(UNiagaraComponent* ParticleSystem);
    void RegisterPCGActorForOptimization(APCGActor* PCGActor);

    /**
     * Clean up invalid/destroyed tracked objects
     */
    void CleanupTrackedObjects();

    /**
     * Initialize default LOD configurations for all biomes
     */
    void InitializeDefaultLODConfigs();

    /**
     * Calculate adaptive LOD bias based on performance
     */
    void CalculateAdaptiveLODBias();

    /**
     * Apply emergency optimizations when performance is very poor
     */
    void ApplyEmergencyOptimizations();

public:
    // Delegates for performance events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceTargetMissed, const FPerformanceMetrics&, Metrics);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLODLevelChanged, int32, OldLevel, int32, NewLevel);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryBudgetExceeded, float, MemoryUsageMB);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAdaptiveOptimizationApplied, int32, OptimizationLevel, FString, OptimizationType);

    UPROPERTY(BlueprintAssignable, Category = "Performance Events")
    FOnPerformanceTargetMissed OnPerformanceTargetMissedEvent;

    UPROPERTY(BlueprintAssignable, Category = "Performance Events")
    FOnLODLevelChanged OnLODLevelChangedEvent;

    UPROPERTY(BlueprintAssignable, Category = "Performance Events")
    FOnMemoryBudgetExceeded OnMemoryBudgetExceededEvent;

    UPROPERTY(BlueprintAssignable, Category = "Performance Events")
    FOnAdaptiveOptimizationApplied OnAdaptiveOptimizationAppliedEvent;
};

/**
 * Component for automatic performance optimization on actors
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class BIKEADVENTURE_API UAutoOptimizationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAutoOptimizationComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    /**
     * Enable automatic optimization for this actor
     */
    UFUNCTION(BlueprintCallable, Category = "Auto Optimization")
    void EnableAutoOptimization(bool bEnable = true);

    /**
     * Set biome type for optimization context
     */
    UFUNCTION(BlueprintCallable, Category = "Auto Optimization")
    void SetBiomeType(EBiomeType BiomeType);

    /**
     * Set optimization priority (higher priority = optimized less aggressively)
     */
    UFUNCTION(BlueprintCallable, Category = "Auto Optimization")
    void SetOptimizationPriority(int32 Priority);

protected:
    // Whether auto optimization is enabled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Optimization")
    bool bAutoOptimizationEnabled;

    // Biome type for optimization context
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Optimization")
    EBiomeType BiomeType;

    // Optimization priority (0 = highest, 10 = lowest)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Optimization", meta = (ClampMin = "0", ClampMax = "10"))
    int32 OptimizationPriority;

    // Whether this component has been registered with the optimization system
    UPROPERTY()
    bool bRegisteredWithOptimizationSystem;

private:
    /**
     * Register all relevant components with the optimization system
     */
    void RegisterWithOptimizationSystem();

    /**
     * Unregister from optimization system
     */
    void UnregisterFromOptimizationSystem();
};