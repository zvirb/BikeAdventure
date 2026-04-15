// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "Gameplay/PathNPCSpawner.h"
#include "Components/SplineComponent.h"

/**
 * Unit tests for PathNPCSpawner
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPathNPCSpawnerNullSplineTest,
    "BikeAdventure.Unit.Gameplay.PathNPCSpawner.NullSpline",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPathNPCSpawnerNullSplineTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
    TestNotNull("Test world created", TestWorld);

    if (!TestWorld)
    {
        return false;
    }

    APathNPCSpawner* Spawner = TestWorld->SpawnActor<APathNPCSpawner>();
    TestNotNull("Spawner created", Spawner);

    if (Spawner)
    {
        // Explicitly nullify PathSpline
        Spawner->PathSpline = nullptr;
        Spawner->NPCClass = AActor::StaticClass();
        Spawner->NPCCount = 5;

        Spawner->SpawnNPCsAlongPath();

        TestEqual("SpawnedNPCs should be empty when PathSpline is null", Spawner->SpawnedNPCs.Num(), 0);
    }

    TestWorld->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPathNPCSpawnerNullNPCClassTest,
    "BikeAdventure.Unit.Gameplay.PathNPCSpawner.NullNPCClass",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPathNPCSpawnerNullNPCClassTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
    TestNotNull("Test world created", TestWorld);

    if (!TestWorld)
    {
        return false;
    }

    APathNPCSpawner* Spawner = TestWorld->SpawnActor<APathNPCSpawner>();
    TestNotNull("Spawner created", Spawner);

    if (Spawner)
    {
        // PathSpline is created in constructor, but NPCClass is null by default
        Spawner->NPCClass = nullptr;
        Spawner->NPCCount = 5;

        Spawner->SpawnNPCsAlongPath();

        TestEqual("SpawnedNPCs should be empty when NPCClass is null", Spawner->SpawnedNPCs.Num(), 0);
    }

    TestWorld->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPathNPCSpawnerSuccessfulSpawnTest,
    "BikeAdventure.Unit.Gameplay.PathNPCSpawner.SuccessfulSpawn",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPathNPCSpawnerSuccessfulSpawnTest::RunTest(const FString& Parameters)
{
    UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
    TestNotNull("Test world created", TestWorld);

    if (!TestWorld)
    {
        return false;
    }

    APathNPCSpawner* Spawner = TestWorld->SpawnActor<APathNPCSpawner>();
    TestNotNull("Spawner created", Spawner);

    if (Spawner)
    {
        // Setup valid spawner
        Spawner->NPCClass = AActor::StaticClass();
        Spawner->NPCCount = 3;

        // Add some points to the spline to give it length
        if (Spawner->PathSpline)
        {
            Spawner->PathSpline->ClearSplinePoints();
            Spawner->PathSpline->AddSplinePoint(FVector(0, 0, 0), ESplineCoordinateSpace::Local);
            Spawner->PathSpline->AddSplinePoint(FVector(1000, 0, 0), ESplineCoordinateSpace::Local);
            Spawner->PathSpline->UpdateSpline();
        }

        Spawner->SpawnNPCsAlongPath();

        TestEqual("Should have spawned the requested number of NPCs", Spawner->SpawnedNPCs.Num(), 3);

        for (AActor* NPC : Spawner->SpawnedNPCs)
        {
            TestNotNull("Spawned NPC should not be null", NPC);
        }
    }

    TestWorld->DestroyWorld(false);
    return true;
}
