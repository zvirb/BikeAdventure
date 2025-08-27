#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PathNPCSpawner.generated.h"

class USplineComponent;

/**
 * Spawns NPCs along a spline path using procedural placement
 */
UCLASS()
class BIKEADVENTURE_API APathNPCSpawner : public AActor
{
    GENERATED_BODY()

public:
    APathNPCSpawner();

    /** Path spline along which NPCs are spawned */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC Spawner")
    USplineComponent* PathSpline;

    /** NPC class to spawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC Spawner")
    TSubclassOf<AActor> NPCClass;

    /** Number of NPCs to spawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC Spawner", meta=(ClampMin="0", ClampMax="100"))
    int32 NPCCount;

    /** Seed for procedural placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC Spawner")
    int32 RandomSeed;

    /** Spawned NPC references */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="NPC Spawner")
    TArray<AActor*> SpawnedNPCs;

    /** Generate NPCs along the path */
    UFUNCTION(BlueprintCallable, Category="NPC Spawner")
    void SpawnNPCsAlongPath();

protected:
    virtual void BeginPlay() override;
};

