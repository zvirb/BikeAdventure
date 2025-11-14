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
        RandomStream = FRandomStream(BiomeSeed);

        // Initialize PCG settings for all biome types
        InitializeBiomePCGSettings();
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
        if (!GetWorld())
        {
                UE_LOG(LogTemp, Warning, TEXT("Cannot generate biome: World is null"));
                return;
        }

        EBiomeType Biome = static_cast<EBiomeType>(BiomeType);
        UBiomePCGSettings* Settings = GetBiomePCGSettings(Biome);

        if (!Settings)
        {
                UE_LOG(LogTemp, Warning, TEXT("No PCG settings found for biome type %d"), BiomeType);
                return;
        }

        // Spawn PCG actor for this biome section
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        APCGActor* PCGActor = GetWorld()->SpawnActor<APCGActor>(Location, FRotator::ZeroRotator, SpawnParams);
        if (PCGActor)
        {
                // Get the PCG component and configure it with our biome settings
                if (UPCGComponent* PCGComponent = PCGActor->GetPCGComponent())
                {
                        // Note: In a full implementation with PCG graph assets, you would set the graph here
                        // For now, we configure the component with basic parameters
                        PCGComponent->Seed = BiomeSeed;

                        // The PCG generation would be triggered when a graph is assigned
                        // PCGComponent->GenerateLocal(true);

                        UE_LOG(LogTemp, Log, TEXT("Generated biome %s at location %s with PCG actor"),
                               *UBiomeUtilities::GetBiomeName(Biome), *Location.ToString());
                }
                else
                {
                        UE_LOG(LogTemp, Warning, TEXT("PCG Actor spawned but has no PCG Component"));
                }
        }
        else
        {
                UE_LOG(LogTemp, Warning, TEXT("Failed to spawn PCG actor for biome %d at location %s"),
                       BiomeType, *Location.ToString());
        }
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

        if (!GetWorld())
        {
                UE_LOG(LogTemp, Warning, TEXT("Cannot generate path segment: World is null"));
                return SpawnedActors;
        }

        UBiomePCGSettings* Settings = GetBiomePCGSettings(BiomeType);
        if (!Settings)
        {
                UE_LOG(LogTemp, Warning, TEXT("No PCG settings found for biome type %s"),
                       *UBiomeUtilities::GetBiomeName(BiomeType));
                return SpawnedActors;
        }

        FBiomeGenerationParams Params = Settings->GenerationParams;

        // Calculate segment length based on biome parameters
        float SegmentLength = Params.PathWidth * 50.0f; // Approximately 50 path widths per segment
        int32 NumActors = FMath::Max(1, FMath::RoundToInt(SegmentLength / 10000.0f)); // One actor per 10km

        // Normalize direction
        FVector NormalizedDirection = Direction.GetSafeNormal();
        if (NormalizedDirection.IsNearlyZero())
        {
                NormalizedDirection = FVector::ForwardVector;
        }

        // Spawn PCG actors along the path segment
        for (int32 i = 0; i < NumActors; i++)
        {
                float Offset = (SegmentLength / NumActors) * i;
                FVector SpawnLocation = Location + (NormalizedDirection * Offset);

                // Add some randomness perpendicular to the path for natural variation
                FVector PerpendicularOffset = FVector::CrossProduct(NormalizedDirection, FVector::UpVector);
                PerpendicularOffset *= RandomStream.FRandRange(-Params.PathWidth * 0.5f, Params.PathWidth * 0.5f);
                SpawnLocation += PerpendicularOffset;

                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

                APCGActor* PCGActor = GetWorld()->SpawnActor<APCGActor>(SpawnLocation, FRotator::ZeroRotator, SpawnParams);
                if (PCGActor)
                {
                        // Configure PCG component with biome settings
                        if (UPCGComponent* PCGComponent = PCGActor->GetPCGComponent())
                        {
                                // Set biome-specific generation parameters
                                PCGComponent->Seed = RandomStream.GetCurrentSeed() + i;

                                // Note: In a full implementation with PCG graph assets, you would:
                                // 1. Load or create a PCG graph for this biome type
                                // 2. Assign it to the component
                                // 3. Trigger generation with PCGComponent->GenerateLocal(true)
                        }

                        SpawnedActors.Add(PCGActor);
                }
        }

        UE_LOG(LogTemp, Log, TEXT("Generated path segment for %s biome at %s with %d PCG actors"),
               *UBiomeUtilities::GetBiomeName(BiomeType), *Location.ToString(), SpawnedActors.Num());

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

void UBiomeGenerator::SetGenerationSeed(int32 Seed)
{
        BiomeSeed = Seed;
        RandomStream = FRandomStream(Seed);

        UE_LOG(LogTemp, Log, TEXT("Biome generation seed set to %d"), Seed);
}

UBiomePCGSettings* UBiomeGenerator::GetBiomePCGSettings(EBiomeType BiomeType)
{
        UBiomePCGSettings** FoundSettings = BiomePCGSettingsMap.Find(BiomeType);

        if (FoundSettings && *FoundSettings)
        {
                return *FoundSettings;
        }

        // Create default settings if not found
        UBiomePCGSettings* DefaultSettings = NewObject<UBiomePCGSettings>(this);
        DefaultSettings->BiomeType = BiomeType;
        DefaultSettings->GenerationParams = UBiomeUtilities::GetDefaultBiomeParams(BiomeType);

        BiomePCGSettingsMap.Add(BiomeType, DefaultSettings);

        return DefaultSettings;
}

void UBiomeGenerator::InitializeBiomePCGSettings()
{
        // Initialize PCG settings for all biome types
        TArray<EBiomeType> AllBiomes = {
                EBiomeType::Forest,
                EBiomeType::Beach,
                EBiomeType::Desert,
                EBiomeType::Urban,
                EBiomeType::Countryside,
                EBiomeType::Mountains,
                EBiomeType::Wetlands
        };

        for (EBiomeType BiomeType : AllBiomes)
        {
                UBiomePCGSettings* Settings = NewObject<UBiomePCGSettings>(this);
                Settings->BiomeType = BiomeType;
                Settings->GenerationParams = UBiomeUtilities::GetDefaultBiomeParams(BiomeType);

                BiomePCGSettingsMap.Add(BiomeType, Settings);
        }
}
