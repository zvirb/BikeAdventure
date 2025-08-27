#include "BiomeGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PCGComponent.h"
#include "PCGActor.h"
#include "../Core/BiomeTypes.h"
#include "../Gameplay/Intersection.h"
#include "AdvancedBiomePCGSettings.h"

UBiomePCGSettings::UBiomePCGSettings()
{
    BiomeType = EBiomeType::None;
    GenerationParams = UBiomeUtilities::GetDefaultBiomeParams(EBiomeType::Countryside);
}

FPCGElementPtr UBiomePCGSettings::CreateElement() const
{
    return MakeShared<FAdvancedBiomeGenerationElement>();
}

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

EBiomeType UBiomeGenerator::GenerateNextBiome(EBiomeType CurrentBiome, bool bChooseLeftPath, const TArray<EBiomeType>& BiomeHistory)
{
        EBiomeType NextBiome = UBiomeUtilities::GetRandomValidTransition(CurrentBiome, BiomeHistory);

        UE_LOG(LogTemp, Log, TEXT("Transitioning from %s to %s via %s path"),
               *UBiomeUtilities::GetBiomeName(CurrentBiome),
               *UBiomeUtilities::GetBiomeName(NextBiome),
               bChooseLeftPath ? TEXT("left") : TEXT("right"));

        return NextBiome;
}

TArray<APCGActor*> UBiomeGenerator::GeneratePathSegment(const FVector& Location, EBiomeType BiomeType, const FVector& Direction)
{
        TArray<APCGActor*> SpawnedActors;

        UE_LOG(LogTemp, Log, TEXT("Generating path segment for %s biome at %s"),
               *UBiomeUtilities::GetBiomeName(BiomeType), *Location.ToString());

        // In a full implementation, PCG actors would be spawned here using biome-specific settings
        // For now we simply log and return an empty array

        return SpawnedActors;
}

AIntersection* UBiomeGenerator::GenerateIntersection(const FVector& Location, EBiomeType CurrentBiome, EBiomeType LeftBiome, EBiomeType RightBiome)
{
        if (!GetWorld())
        {
                return nullptr;
        }

        FBiomeTransitionRules Rules = UBiomeUtilities::GetDefaultTransitionRules(CurrentBiome);
        EIntersectionType Type = EIntersectionType::YFork;
        if (Rules.PreferredIntersectionTypes.Num() > 0)
        {
                int32 Index = FMath::RandRange(0, Rules.PreferredIntersectionTypes.Num() - 1);
                Type = Rules.PreferredIntersectionTypes[Index];
        }

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AIntersection* Intersection = GetWorld()->SpawnActor<AIntersection>(Location, FRotator::ZeroRotator, Params);
        if (Intersection)
        {
                Intersection->SetIntersectionType(Type);
                Intersection->SetPathBiomes(LeftBiome, RightBiome);
        }

        UE_LOG(LogTemp, Log, TEXT("Generated intersection at %s leading to %s and %s"),
               *Location.ToString(),
               *UBiomeUtilities::GetBiomeName(LeftBiome),
               *UBiomeUtilities::GetBiomeName(RightBiome));

        return Intersection;
}
