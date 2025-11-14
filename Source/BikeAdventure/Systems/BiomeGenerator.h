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

	/** Initialize PCG settings for all biome types */
	void InitializeBiomePCGSettings();

	/** Calculate actor count multiplier based on quality level */
	float GetQualityMultiplier() const;

	/** Update adaptive quality based on current performance */
	void UpdateAdaptiveQuality();

	/** Record biome generation timing */
	void RecordBiomeGeneration(float GenerationTimeMs, bool bSuccess);

	/** Record path segment generation timing */
	void RecordPathGeneration(float GenerationTimeMs, int32 ActorsSpawned);
};
