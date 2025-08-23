#include "IntersectionManager.h"
#include "Gameplay/Intersection.h"
#include "Engine/Engine.h"

UIntersectionManager::UIntersectionManager()
{
	PrimaryComponentTick.bCanEverTick = false;
	bInitialized = false;
}

void UIntersectionManager::Initialize()
{
	if (!bInitialized)
	{
		RegisteredIntersections.Empty();
		bInitialized = true;
		UE_LOG(LogTemp, Log, TEXT("Intersection Manager initialized"));
	}
}

void UIntersectionManager::RegisterIntersection(AIntersection* Intersection)
{
	if (Intersection && !RegisteredIntersections.Contains(Intersection))
	{
		RegisteredIntersections.Add(Intersection);
		UE_LOG(LogTemp, Log, TEXT("Registered intersection: %s"), *Intersection->GetName());
	}
}

void UIntersectionManager::UnregisterIntersection(AIntersection* Intersection)
{
	if (Intersection)
	{
		RegisteredIntersections.Remove(Intersection);
		UE_LOG(LogTemp, Log, TEXT("Unregistered intersection: %s"), *Intersection->GetName());
	}
}