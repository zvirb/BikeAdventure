#include "BiomeGenerator.h"
#include "Engine/Engine.h"
#include "Core/BiomeTypes.h"

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

void UBiomeGenerator::GenerateBiome(const FVector& Location, EBiomeType BiomeType)
{
	// Placeholder for biome generation logic
	// This will be expanded in future streams
	FString BiomeName = UBiomeUtilities::GetBiomeName(BiomeType);
	UE_LOG(LogTemp, Log, TEXT("Generated biome type %s at location %s"), *BiomeName, *Location.ToString());
}

void UBiomeGenerator::GeneratePathSegment(const FVector& Location, EBiomeType BiomeType, const FVector& Direction)
{
	// Placeholder
	UE_LOG(LogTemp, Log, TEXT("Generated path segment for %s at %s"), *UBiomeUtilities::GetBiomeName(BiomeType), *Location.ToString());
}

AIntersection* UBiomeGenerator::GenerateIntersection(const FVector& Location, EBiomeType FromBiome, EBiomeType LeftBiome, EBiomeType RightBiome)
{
	// Placeholder
	UE_LOG(LogTemp, Log, TEXT("Generated intersection at %s from %s to %s and %s"), *Location.ToString(), *UBiomeUtilities::GetBiomeName(FromBiome), *UBiomeUtilities::GetBiomeName(LeftBiome), *UBiomeUtilities::GetBiomeName(RightBiome));
	return nullptr;
}

EBiomeType UBiomeGenerator::GenerateNextBiome(EBiomeType CurrentBiome, bool bLeftChoice, const TArray<EBiomeType>& BiomeHistory)
{
	// Placeholder
	return UBiomeUtilities::GetRandomValidTransition(CurrentBiome, BiomeHistory);
}