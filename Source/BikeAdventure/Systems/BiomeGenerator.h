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

	/** Initialize PCG settings for all biome types */
	void InitializeBiomePCGSettings();
};
