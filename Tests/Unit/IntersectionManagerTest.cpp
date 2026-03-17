#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "Systems/IntersectionManager.h"
#include "Gameplay/Intersection.h"

/**
 * Unit tests for IntersectionManager
 * Tests initialization and intersection registration/unregistration
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntersectionManagerInitializationTest,
	"BikeAdventure.Unit.Systems.IntersectionManager.Initialization",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntersectionManagerInitializationTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	UIntersectionManager* Manager = NewObject<UIntersectionManager>(TestWorld);
	if (!Manager)
	{
		AddError(TEXT("Failed to create IntersectionManager"));
		TestWorld->DestroyWorld(false);
		return false;
	}

	// Test initial state
	TestEqual(TEXT("Should have 0 intersections initially"), Manager->GetIntersectionCount(), 0);

	// Mocking registration to test if Initialize clears it
	AIntersection* TestIntersection = TestWorld->SpawnActor<AIntersection>();
	Manager->RegisterIntersection(TestIntersection);
	TestEqual(TEXT("Should have 1 intersection after registration"), Manager->GetIntersectionCount(), 1);

	// Initialize
	// The implementation only empties if !bInitialized
	Manager->Initialize();

	TestEqual(TEXT("Should have 0 intersections after Initialize()"), Manager->GetIntersectionCount(), 0);

	TestWorld->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntersectionManagerRegistrationTest,
	"BikeAdventure.Unit.Systems.IntersectionManager.Registration",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntersectionManagerRegistrationTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	UIntersectionManager* Manager = NewObject<UIntersectionManager>(TestWorld);
	Manager->Initialize();

	AIntersection* Intersection1 = TestWorld->SpawnActor<AIntersection>();
	AIntersection* Intersection2 = TestWorld->SpawnActor<AIntersection>();

	// Test Register
	Manager->RegisterIntersection(Intersection1);
	TestEqual(TEXT("Count should be 1 after registering 1"), Manager->GetIntersectionCount(), 1);

	// Registering same again
	Manager->RegisterIntersection(Intersection1);
	TestEqual(TEXT("Count should still be 1 after duplicate registration"), Manager->GetIntersectionCount(), 1);

	// Register null
	Manager->RegisterIntersection(nullptr);
	TestEqual(TEXT("Count should still be 1 after null registration"), Manager->GetIntersectionCount(), 1);

	// Register second
	Manager->RegisterIntersection(Intersection2);
	TestEqual(TEXT("Count should be 2 after registering 2"), Manager->GetIntersectionCount(), 2);

	// Test GetAllIntersections
	TArray<AIntersection*> All = Manager->GetAllIntersections();
	TestEqual(TEXT("GetAllIntersections should return 2 items"), All.Num(), 2);
	TestTrue(TEXT("List should contain Intersection1"), All.Contains(Intersection1));
	TestTrue(TEXT("List should contain Intersection2"), All.Contains(Intersection2));

	// Test Unregister
	Manager->UnregisterIntersection(Intersection1);
	TestEqual(TEXT("Count should be 1 after unregistering 1"), Manager->GetIntersectionCount(), 1);
	TestFalse(TEXT("List should no longer contain Intersection1"), Manager->GetAllIntersections().Contains(Intersection1));

	// Unregister null
	Manager->UnregisterIntersection(nullptr);
	TestEqual(TEXT("Count should still be 1 after null unregistration"), Manager->GetIntersectionCount(), 1);

	// Unregister not registered
	AIntersection* Intersection3 = TestWorld->SpawnActor<AIntersection>();
	Manager->UnregisterIntersection(Intersection3);
	TestEqual(TEXT("Count should still be 1 after unregistering non-registered actor"), Manager->GetIntersectionCount(), 1);

	TestWorld->DestroyWorld(false);
	return true;
}
