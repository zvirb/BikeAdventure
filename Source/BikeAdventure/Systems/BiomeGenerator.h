#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/ActorComponent.h"
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
	void GenerateBiome(const FVector& Location, int32 BiomeType);

protected:
	/** Whether the system has been initialized */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bInitialized = false;

	/** Current biome seed for procedural generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 BiomeSeed = 12345;
};