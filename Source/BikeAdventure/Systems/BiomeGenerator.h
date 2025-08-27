#pragma once

#include "CoreMinimal.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/BiomeTypes.h"
#include "BiomeGenerator.generated.h"

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
	void GenerateBiome(const FVector& Location, EBiomeType BiomeType);

	/** Generate a path segment */
	UFUNCTION(BlueprintCallable, Category = "Biome Generator")
	void GeneratePathSegment(const FVector& Location, EBiomeType BiomeType, const FVector& Direction);

	/** Generate an intersection */
	UFUNCTION(BlueprintCallable, Category = "Biome Generator")
	class AIntersection* GenerateIntersection(const FVector& Location, EBiomeType FromBiome, EBiomeType LeftBiome, EBiomeType RightBiome);

	/** Determine the next biome based on choice and history */
	UFUNCTION(BlueprintCallable, Category = "Biome Generator")
	EBiomeType GenerateNextBiome(EBiomeType CurrentBiome, bool bLeftChoice, const TArray<EBiomeType>& BiomeHistory);

protected:
	/** Whether the system has been initialized */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bInitialized = false;

	/** Current biome seed for procedural generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 BiomeSeed = 12345;
};