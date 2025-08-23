#include "BiomeGenerator.h"
#include "Engine/Engine.h"

UBiomeGenerator::UBiomeGenerator()
{
	PrimaryComponentTick.bCanEverTick = false;
	bInitialized = false;
	BiomeSeed = 12345;
}

void UBiomeGenerator::Initialize()
{
	if (!bInitialized)
	{
		bInitialized = true;
		UE_LOG(LogTemp, Log, TEXT("Biome Generator initialized with seed: %d"), BiomeSeed);
	}
}

void UBiomeGenerator::GenerateBiome(const FVector& Location, int32 BiomeType)
{
	// Placeholder for biome generation logic
	// This will be expanded in future streams
	UE_LOG(LogTemp, Log, TEXT("Generated biome type %d at location %s"), BiomeType, *Location.ToString());
}