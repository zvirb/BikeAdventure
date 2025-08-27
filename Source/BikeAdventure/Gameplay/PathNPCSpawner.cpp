#include "PathNPCSpawner.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"

APathNPCSpawner::APathNPCSpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    PathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PathSpline"));
    RootComponent = PathSpline;

    NPCCount = 5;
    RandomSeed = 12345;
}

void APathNPCSpawner::BeginPlay()
{
    Super::BeginPlay();

    SpawnNPCsAlongPath();
}

void APathNPCSpawner::SpawnNPCsAlongPath()
{
    if (!PathSpline || !NPCClass)
    {
        return;
    }

    SpawnedNPCs.Empty();

    FRandomStream Random(RandomSeed);
    float SplineLength = PathSpline->GetSplineLength();

    for (int32 i = 0; i < NPCCount; i++)
    {
        float Distance = Random.FRandRange(0.0f, SplineLength);
        FVector Location = PathSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
        FRotator Rotation = PathSpline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

        if (AActor* Spawned = GetWorld()->SpawnActor<AActor>(NPCClass, Location, Rotation))
        {
            SpawnedNPCs.Add(Spawned);
        }
    }
}

