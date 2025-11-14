#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/ActorComponent.h"
#include "PCGSettings.h"
#include "PCGElement.h"
#include "../Core/BiomeTypes.h"
#include "BiomeGenerator.generated.h"

class APCGActor;
class AIntersection;
class UPerformanceOptimizationSystem;

/**
 * Quality levels for biome generation
 */
UENUM(BlueprintType)
enum class EBiomeGenerationQuality : uint8
{
	Low = 0     UMETA(DisplayName = "Low Quality"),
	Medium = 1  UMETA(DisplayName = "Medium Quality"),
	High = 2    UMETA(DisplayName = "High Quality"),
	Ultra = 3   UMETA(DisplayName = "Ultra Quality")
};

/**
 * Platform types for quality presets
 */
UENUM(BlueprintType)
enum class EPlatformType : uint8
{
	LowEndPC = 0    UMETA(DisplayName = "Low-End PC"),
	MidRangePC = 1  UMETA(DisplayName = "Mid-Range PC"),
	HighEndPC = 2   UMETA(DisplayName = "High-End PC"),
	Console = 3     UMETA(DisplayName = "Console"),
	Mobile = 4      UMETA(DisplayName = "Mobile")
};

/**
 * Quality preset configuration
 */
USTRUCT(BlueprintType)
struct FQualityPreset
{
	GENERATED_BODY()

	/** Platform this preset is for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	EPlatformType Platform = EPlatformType::MidRangePC;

	/** Default quality level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	EBiomeGenerationQuality DefaultQuality = EBiomeGenerationQuality::Medium;

	/** Enable adaptive quality adjustment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	bool bEnableAdaptiveQuality = true;

	/** Maximum quality allowed (for adaptive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	EBiomeGenerationQuality MaxQuality = EBiomeGenerationQuality::High;

	/** Minimum quality allowed (for adaptive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
	EBiomeGenerationQuality MinQuality = EBiomeGenerationQuality::Low;
};

/**
 * Per-biome quality multiplier override
 */
USTRUCT(BlueprintType)
struct FBiomeQualityMultiplier
{
	GENERATED_BODY()

	/** Biome type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Quality")
	EBiomeType BiomeType = EBiomeType::Countryside;

	/** Quality multiplier override (0.1 to 2.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Quality", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float QualityMultiplier = 1.0f;

	/** Whether this override is enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Quality")
	bool bEnabled = false;
};

/**
 * Telemetry data for biome generation performance
 */
USTRUCT(BlueprintType)
struct FBiomeGenerationMetrics
{
	GENERATED_BODY()

	/** Total number of biomes generated */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 TotalBiomesGenerated = 0;

	/** Total number of PCG actors spawned */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 TotalPCGActorsSpawned = 0;

	/** Total number of path segments created */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 TotalPathSegments = 0;

	/** Total number of intersections created */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 TotalIntersections = 0;

	/** Average time to generate a biome (ms) */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float AverageBiomeGenerationTimeMs = 0.0f;

	/** Average time to generate a path segment (ms) */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float AveragePathGenerationTimeMs = 0.0f;

	/** Number of failed spawns */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 FailedSpawns = 0;

	/** Current generation quality level */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	EBiomeGenerationQuality CurrentQualityLevel = EBiomeGenerationQuality::High;

	/** Number of quality adjustments made */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 QualityAdjustments = 0;
};

/**
 * Base PCG settings used for generating biome-specific content
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API UBiomePCGSettings : public UPCGSettings
{
    GENERATED_BODY()

public:
    UBiomePCGSettings();

    /** Biome type these settings apply to */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    /** General generation parameters for this biome */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    FBiomeGenerationParams GenerationParams;

    // UPCGSettings interface
    virtual FPCGElementPtr CreateElement() const override;
};

/**
 * Manages procedural biome generation and transitions
 * Handles the creation of diverse environments for exploration
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BIKEADVENTURE_API UBiomeGenerator : public UActorComponent
{
	GENERATED_BODY()

public:
        UBiomeGenerator();

        /** Initialize the biome generation system */
        UFUNCTION(BlueprintCallable, Category = "Biome Generator")
        void Initialize();

        /** Generate a new biome section */
        UFUNCTION(BlueprintCallable, Category = "Biome Generator")
        void GenerateBiome(const FVector& Location, int32 BiomeType);

        /**
         * Determine the next biome based on current biome and history
         */
        UFUNCTION(BlueprintCallable, Category = "Biome Generator")
        EBiomeType GenerateNextBiome(EBiomeType CurrentBiome, bool bChooseLeftPath, const TArray<EBiomeType>& BiomeHistory);

        /**
         * Generate the path segment for the specified biome
         */
        UFUNCTION(BlueprintCallable, Category = "Biome Generator")
        TArray<APCGActor*> GeneratePathSegment(const FVector& Location, EBiomeType BiomeType, const FVector& Direction);

        /**
         * Spawn an intersection connecting to left and right biomes
         */
        UFUNCTION(BlueprintCallable, Category = "Biome Generator")
        AIntersection* GenerateIntersection(const FVector& Location, EBiomeType CurrentBiome, EBiomeType LeftBiome, EBiomeType RightBiome);

        /**
         * Set the random seed for deterministic biome generation
         */
        UFUNCTION(BlueprintCallable, Category = "Biome Generator")
        void SetGenerationSeed(int32 Seed);

        /**
         * Get PCG settings for a specific biome type
         */
        UFUNCTION(BlueprintCallable, Category = "Biome Generator")
        UBiomePCGSettings* GetBiomePCGSettings(EBiomeType BiomeType);

        /**
         * Set generation quality level (affects actor density and detail)
         */
        UFUNCTION(BlueprintCallable, Category = "Biome Generator")
        void SetGenerationQuality(EBiomeGenerationQuality Quality);

        /**
         * Get current generation quality level
         */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome Generator")
        EBiomeGenerationQuality GetGenerationQuality() const { return CurrentQualityLevel; }

        /**
         * Get generation metrics for telemetry
         */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome Generator")
        FBiomeGenerationMetrics GetGenerationMetrics() const { return GenerationMetrics; }

        /**
         * Reset generation metrics
         */
        UFUNCTION(BlueprintCallable, Category = "Biome Generator")
        void ResetMetrics();

        /**
         * Enable/disable adaptive quality based on performance
         */
        UFUNCTION(BlueprintCallable, Category = "Biome Generator")
        void SetAdaptiveQuality(bool bEnable);

        /**
         * Apply a quality preset for specific platform
         */
        UFUNCTION(BlueprintCallable, Category = "Biome Generator")
        void ApplyQualityPreset(EPlatformType Platform);

        /**
         * Get quality preset for platform
         */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome Generator")
        FQualityPreset GetQualityPreset(EPlatformType Platform) const;

        /**
         * Set biome-specific quality multiplier
         */
        UFUNCTION(BlueprintCallable, Category = "Biome Generator")
        void SetBiomeQualityMultiplier(EBiomeType BiomeType, float Multiplier, bool bEnable = true);

        /**
         * Get biome-specific quality multiplier
         */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome Generator")
        float GetBiomeQualityMultiplier(EBiomeType BiomeType) const;

        /**
         * Export metrics to JSON string for analytics
         */
        UFUNCTION(BlueprintCallable, Category = "Biome Generator")
        FString ExportMetricsToJSON() const;

        /**
         * Enable/disable debug visualization
         */
        UFUNCTION(BlueprintCallable, Category = "Biome Generator")
        void SetDebugVisualization(bool bEnable);

        /**
         * Get debug visualization state
         */
        UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome Generator")
        bool IsDebugVisualizationEnabled() const { return bShowDebugVisualization; }

protected:
	/** Whether the system has been initialized */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bInitialized = false;

	/** Current biome seed for procedural generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
        int32 BiomeSeed = 12345;

	/** Random stream for deterministic generation */
	FRandomStream RandomStream;

	/** PCG settings for each biome type */
	UPROPERTY()
	TMap<EBiomeType, UBiomePCGSettings*> BiomePCGSettingsMap;

	/** Current generation quality level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
	EBiomeGenerationQuality CurrentQualityLevel = EBiomeGenerationQuality::High;

	/** Whether to automatically adjust quality based on performance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
	bool bUseAdaptiveQuality = true;

	/** Generation metrics for telemetry */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
	FBiomeGenerationMetrics GenerationMetrics;

	/** Cumulative time spent on biome generation (for averaging) */
	float TotalBiomeGenerationTime = 0.0f;

	/** Cumulative time spent on path segment generation (for averaging) */
	float TotalPathGenerationTime = 0.0f;

	/** Quality presets for different platforms */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quality Presets")
	TMap<EPlatformType, FQualityPreset> QualityPresets;

	/** Biome-specific quality multipliers */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Quality")
	TMap<EBiomeType, FBiomeQualityMultiplier> BiomeQualityMultipliers;

	/** Current active platform type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quality Presets")
	EPlatformType CurrentPlatform = EPlatformType::MidRangePC;

	/** Maximum quality allowed by current preset */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quality Presets")
	EBiomeGenerationQuality MaxAllowedQuality = EBiomeGenerationQuality::Ultra;

	/** Minimum quality allowed by current preset */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quality Presets")
	EBiomeGenerationQuality MinAllowedQuality = EBiomeGenerationQuality::Low;

	/** Whether to show debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bShowDebugVisualization = false;

	/** Initialize PCG settings for all biome types */
	void InitializeBiomePCGSettings();

	/** Initialize default quality presets */
	void InitializeQualityPresets();

	/** Calculate actor count multiplier based on quality level */
	float GetQualityMultiplier() const;

	/** Calculate actor count multiplier for specific biome */
	float GetQualityMultiplierForBiome(EBiomeType BiomeType) const;

	/** Update adaptive quality based on current performance */
	void UpdateAdaptiveQuality();

	/** Record biome generation timing */
	void RecordBiomeGeneration(float GenerationTimeMs, bool bSuccess);

	/** Record path segment generation timing */
	void RecordPathGeneration(float GenerationTimeMs, int32 ActorsSpawned);

	/** Draw debug visualization for biome generation */
	void DrawDebugVisualization(const FVector& Location, EBiomeType BiomeType, int32 ActorCount) const;
};
