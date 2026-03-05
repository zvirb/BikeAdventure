#include "DiscoverySystem.h"
#include "Engine/World.h"
#include "../Core/BikeCharacter.h"
#include "GameFramework/PlayerController.h"

UDiscoverySystem::UDiscoverySystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDiscoverySystem::BeginPlay()
{
	Super::BeginPlay();
}

void UDiscoverySystem::Initialize()
{
	LoadDefaultDiscoveries();
	UE_LOG(LogTemp, Log, TEXT("Discovery System Initialized with %d items"), AvailableDiscoveries.Num());
}

void UDiscoverySystem::LoadDefaultDiscoveries()
{
	if (AvailableDiscoveries.Num() > 0) return;

	// Add some default wildlife and elements
	FDiscoveryData Deer;
	Deer.Name = TEXT("Deer Crossing");
	Deer.Description = TEXT("Saw deer crossing the meadow path.");
	Deer.RequiredBiome = EBiomeType::Forest;
	AvailableDiscoveries.Add(Deer);

	FDiscoveryData Whale;
	Whale.Name = TEXT("Breaching Whale");
	Whale.Description = TEXT("Spotted a whale off the coast.");
	Whale.RequiredBiome = EBiomeType::Beach;
	AvailableDiscoveries.Add(Whale);

	FDiscoveryData Rain;
	Rain.Name = TEXT("Sudden Rain");
	Rain.Description = TEXT("Caught in a sudden rain shower.");
	Rain.RequiredBiome = EBiomeType::Wetlands;
	AvailableDiscoveries.Add(Rain);

	FDiscoveryData DesertOasis;
	DesertOasis.Name = TEXT("Desert Oasis");
	DesertOasis.Description = TEXT("Stumbled upon a hidden palm oasis in the sand.");
	DesertOasis.RequiredBiome = EBiomeType::Desert;
	AvailableDiscoveries.Add(DesertOasis);

	FDiscoveryData UrbanMusic;
	UrbanMusic.Name = TEXT("Street Musicians");
	UrbanMusic.Description = TEXT("Listened to music on a city corner.");
	UrbanMusic.RequiredBiome = EBiomeType::Urban;
	AvailableDiscoveries.Add(UrbanMusic);

	FDiscoveryData MountainPeak;
	MountainPeak.Name = TEXT("Snowy Peak");
	MountainPeak.Description = TEXT("Reached the snow-covered mountain peaks.");
	MountainPeak.RequiredBiome = EBiomeType::Mountains;
	AvailableDiscoveries.Add(MountainPeak);

	FDiscoveryData CountrysideFarm;
	CountrysideFarm.Name = TEXT("Rural Farm");
	CountrysideFarm.Description = TEXT("Passed a quiet countryside farm.");
	CountrysideFarm.RequiredBiome = EBiomeType::Countryside;
	AvailableDiscoveries.Add(CountrysideFarm);
}

void UDiscoverySystem::HandleBiomeEntered(EBiomeType BiomeType)
{
	// Randomly trigger a discovery for the entered biome
	FRandomStream Random(FMath::Rand());
	if (Random.FRand() < 0.3f) // 30% chance to find something
	{
		for (FDiscoveryData& Data : AvailableDiscoveries)
		{
			if (!Data.bIsDiscovered && Data.RequiredBiome == BiomeType)
			{
				TriggerDiscovery(Data.Name);
				break;
			}
		}
	}
}

bool UDiscoverySystem::TriggerDiscovery(const FString& DiscoveryName)
{
	for (FDiscoveryData& Data : AvailableDiscoveries)
	{
		if (Data.Name == DiscoveryName && !Data.bIsDiscovered)
		{
			Data.bIsDiscovered = true;
			
			// Notify character via public method
			UWorld* World = GetWorld();
			if (World)
			{
				APlayerController* PC = World->GetFirstPlayerController();
				if (PC)
				{
					ABikeCharacter* BikeChar = Cast<ABikeCharacter>(PC->GetPawn());
					if (BikeChar)
					{
						BikeChar->TriggerDiscoveryEvent(Data.Name, Data.Description);
					}
				}
			}
			
			UE_LOG(LogTemp, Log, TEXT("New Discovery: %s"), *DiscoveryName);
			return true;
		}
	}
	return false;
}

int32 UDiscoverySystem::GetTotalDiscoveriesFound() const
{
	int32 Count = 0;
	for (const FDiscoveryData& Data : AvailableDiscoveries)
	{
		if (Data.bIsDiscovered) Count++;
	}
	return Count;
}

int32 UDiscoverySystem::GetTotalDiscoveriesAvailable() const
{
	return AvailableDiscoveries.Num();
}

TArray<FDiscoveryData> UDiscoverySystem::GetDiscoveredItems() const
{
	TArray<FDiscoveryData> FoundItems;
	for (const FDiscoveryData& Data : AvailableDiscoveries)
	{
		if (Data.bIsDiscovered)
		{
			FoundItems.Add(Data);
		}
	}
	return FoundItems;
}
