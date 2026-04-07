// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "Systems/DiscoverySystem.h"
#include "Core/BikeCharacter.h"
#include "Core/BiomeTypes.h"

/**
 * Unit tests for DiscoverySystem
 * Tests discovery initialization, triggering, and state tracking
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDiscoverySystemInitializationTest,
	"BikeAdventure.Unit.Systems.Discovery.Initialization",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDiscoverySystemInitializationTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull("Test world created", TestWorld);

	if (!TestWorld)
	{
		return false;
	}

	UDiscoverySystem* DiscoverySystem = NewObject<UDiscoverySystem>(TestWorld);
	TestNotNull("DiscoverySystem created", DiscoverySystem);

	if (!DiscoverySystem)
	{
		TestWorld->DestroyWorld(false);
		return false;
	}

	// Test initial state before initialization
	TestEqual(TEXT("Should have 0 discoveries before initialization"), DiscoverySystem->GetTotalDiscoveriesAvailable(), 0);

	// Initialize the system
	DiscoverySystem->Initialize();

	// Test state after initialization
	int32 TotalAvailable = DiscoverySystem->GetTotalDiscoveriesAvailable();
	TestTrue(TEXT("Should have discoveries available after initialization"), TotalAvailable > 0);
	TestEqual(TEXT("Should have 0 discoveries found initially"), DiscoverySystem->GetTotalDiscoveriesFound(), 0);

	TestWorld->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDiscoverySystemTriggerTest,
	"BikeAdventure.Unit.Systems.Discovery.TriggerDiscovery",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDiscoverySystemTriggerTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull("Test world created", TestWorld);

	if (!TestWorld)
	{
		return false;
	}

	UDiscoverySystem* DiscoverySystem = NewObject<UDiscoverySystem>(TestWorld);
	DiscoverySystem->Initialize();

	// Get a valid discovery name from the default list (e.g., "Deer Crossing")
	FString DiscoveryName = TEXT("Deer Crossing");

	// Trigger discovery
	bool bSuccess = DiscoverySystem->TriggerDiscovery(DiscoveryName);
	TestTrue(TEXT("TriggerDiscovery should return true for valid new discovery"), bSuccess);
	TestEqual(TEXT("Found count should be 1"), DiscoverySystem->GetTotalDiscoveriesFound(), 1);

	// Trigger same discovery again
	bool bSuccessRepeat = DiscoverySystem->TriggerDiscovery(DiscoveryName);
	TestFalse(TEXT("TriggerDiscovery should return false for already discovered item"), bSuccessRepeat);
	TestEqual(TEXT("Found count should still be 1"), DiscoverySystem->GetTotalDiscoveriesFound(), 1);

	// Trigger invalid discovery (Address the untested failure path issue)
	bool bSuccessInvalid = DiscoverySystem->TriggerDiscovery(TEXT("Non-existent Discovery"));
	TestFalse(TEXT("TriggerDiscovery should return false for invalid discovery name"), bSuccessInvalid);
	TestEqual(TEXT("Found count should still be 1"), DiscoverySystem->GetTotalDiscoveriesFound(), 1);

	TestWorld->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDiscoverySystemBiomeTriggerTest,
	"BikeAdventure.Unit.Systems.Discovery.BiomeTrigger",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDiscoverySystemBiomeTriggerTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull("Test world created", TestWorld);

	if (!TestWorld)
	{
		return false;
	}

	UDiscoverySystem* DiscoverySystem = NewObject<UDiscoverySystem>(TestWorld);
	DiscoverySystem->Initialize();

	// HandleBiomeEntered has a 30% random chance.
	// To test it reliably in a unit test, we will loop until we get at least one discovery or a reasonable limit.
	int32 InitialFound = DiscoverySystem->GetTotalDiscoveriesFound();
	bool bFoundSomething = false;

	for (int32 i = 0; i < 100; ++i)
	{
		DiscoverySystem->HandleBiomeEntered(EBiomeType::Forest);
		if (DiscoverySystem->GetTotalDiscoveriesFound() > InitialFound)
		{
			bFoundSomething = true;
			break;
		}
	}

	TestTrue(TEXT("Should eventually trigger a discovery for a valid biome"), bFoundSomething);

	// Verify it was a forest discovery
	TArray<FDiscoveryData> Discovered = DiscoverySystem->GetDiscoveredItems();
	if (Discovered.Num() > 0)
	{
		TestEqual(TEXT("Discovered item should be from Forest biome"), Discovered[0].RequiredBiome, EBiomeType::Forest);
	}

	TestWorld->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDiscoverySystemGettersTest,
	"BikeAdventure.Unit.Systems.Discovery.Getters",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDiscoverySystemGettersTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull("Test world created", TestWorld);

	if (!TestWorld)
	{
		return false;
	}

	UDiscoverySystem* DiscoverySystem = NewObject<UDiscoverySystem>(TestWorld);
	DiscoverySystem->Initialize();

	// Discover a few items
	DiscoverySystem->TriggerDiscovery(TEXT("Deer Crossing"));
	DiscoverySystem->TriggerDiscovery(TEXT("Breaching Whale"));

	TArray<FDiscoveryData> DiscoveredItems = DiscoverySystem->GetDiscoveredItems();

	TestEqual(TEXT("GetDiscoveredItems should return 2 items"), DiscoveredItems.Num(), 2);

	bool bFoundDeer = false;
	bool bFoundWhale = false;

	for (const FDiscoveryData& Data : DiscoveredItems)
	{
		if (Data.Name == TEXT("Deer Crossing")) bFoundDeer = true;
		if (Data.Name == TEXT("Breaching Whale")) bFoundWhale = true;
	}

	TestTrue(TEXT("Should have found Deer Crossing in list"), bFoundDeer);
	TestTrue(TEXT("Should have found Breaching Whale in list"), bFoundWhale);

	TestWorld->DestroyWorld(false);
	return true;
}
