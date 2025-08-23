#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Gameplay/Intersection.h"
#include "Core/BikeCharacter.h"

/**
 * Unit tests for Intersection system
 * Tests intersection detection, player arrival/departure, and choice handling
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntersectionBasicTest,
	"BikeAdventure.Gameplay.Intersection.BasicFunctionality",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntersectionBasicTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Create test intersection
	AIntersection* TestIntersection = TestWorld->SpawnActor<AIntersection>();
	if (!TestIntersection)
	{
		AddError(TEXT("Failed to spawn test intersection"));
		TestWorld->DestroyWorld(false);
		return false;
	}

	// Test initial state
	TestFalse(TEXT("Intersection should not have player initially"), TestIntersection->IsPlayerPresent());
	TestFalse(TEXT("Intersection should not be discovered initially"), TestIntersection->bDiscovered);

	// Test path direction calculations
	FVector LeftPath = TestIntersection->GetLeftPathDirection();
	FVector RightPath = TestIntersection->GetRightPathDirection();

	TestTrue(TEXT("Left path should be normalized"), FMath::IsNearlyEqual(LeftPath.Size(), 1.0f, 0.01f));
	TestTrue(TEXT("Right path should be normalized"), FMath::IsNearlyEqual(RightPath.Size(), 1.0f, 0.01f));

	// Test that left and right paths are different
	float DotProduct = FVector::DotProduct(LeftPath, RightPath);
	TestTrue(TEXT("Left and right paths should be different"), FMath::Abs(DotProduct) < 0.99f);

	TestWorld->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntersectionPlayerArrivalTest,
	"BikeAdventure.Gameplay.Intersection.PlayerArrival",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntersectionPlayerArrivalTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Create test intersection and bike character
	AIntersection* TestIntersection = TestWorld->SpawnActor<AIntersection>();
	ABikeCharacter* TestBike = TestWorld->SpawnActor<ABikeCharacter>();
	
	if (!TestIntersection || !TestBike)
	{
		AddError(TEXT("Failed to spawn test actors"));
		TestWorld->DestroyWorld(false);
		return false;
	}

	// Test player arrival
	TestIntersection->OnPlayerArrived(TestBike);

	TestTrue(TEXT("Intersection should have player present"), TestIntersection->IsPlayerPresent());
	TestTrue(TEXT("Intersection should be discovered"), TestIntersection->bDiscovered);
	// Note: Cannot directly test private member CurrentPlayer, but IsPlayerPresent() confirms the state

	// Test player departure
	TestIntersection->OnPlayerLeft(TestBike);

	TestFalse(TEXT("Intersection should not have player after departure"), TestIntersection->IsPlayerPresent());

	TestWorld->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntersectionChoiceHandlingTest,
	"BikeAdventure.Gameplay.Intersection.ChoiceHandling",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntersectionChoiceHandlingTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Create test intersection and bike character  
	AIntersection* TestIntersection = TestWorld->SpawnActor<AIntersection>();
	ABikeCharacter* TestBike = TestWorld->SpawnActor<ABikeCharacter>();
	
	if (!TestIntersection || !TestBike)
	{
		AddError(TEXT("Failed to spawn test actors"));
		TestWorld->DestroyWorld(false);
		return false;
	}

	// Player must arrive at intersection first
	TestIntersection->OnPlayerArrived(TestBike);

	// Test initial choice state
	TestFalse(TEXT("No choice should be made initially"), TestIntersection->bChoiceMade);

	// Record initial rotation
	FRotator InitialRotation = TestBike->GetActorRotation();

	// Test right choice
	TestIntersection->HandlePlayerChoice(true);

	TestTrue(TEXT("Choice should be recorded"), TestIntersection->bChoiceMade);
	TestTrue(TEXT("Should record right choice"), TestIntersection->bChoseRight);

	// Test that bike rotation changed
	FRotator NewRotation = TestBike->GetActorRotation();
	TestTrue(TEXT("Bike rotation should change after choice"), 
		!FMath::IsNearlyEqual(InitialRotation.Yaw, NewRotation.Yaw, 0.1f));

	TestWorld->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntersectionPathAngleTest,
	"BikeAdventure.Gameplay.Intersection.PathAngles",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FIntersectionPathAngleTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	AIntersection* TestIntersection = TestWorld->SpawnActor<AIntersection>();
	if (!TestIntersection)
	{
		AddError(TEXT("Failed to spawn test intersection"));
		TestWorld->DestroyWorld(false);
		return false;
	}

	// Test with default path angle (60 degrees)
	FVector LeftPath = TestIntersection->GetLeftPathDirection();
	FVector RightPath = TestIntersection->GetRightPathDirection();
	FVector Forward = TestIntersection->GetActorForwardVector();

	// Calculate angles
	float LeftAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Forward, LeftPath)));
	float RightAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Forward, RightPath)));

	float ExpectedAngle = TestIntersection->PathAngle / 2.0f; // 30 degrees from forward

	TestTrue(TEXT("Left path angle should match expected"), 
		FMath::Abs(LeftAngle - ExpectedAngle) < 1.0f);
	TestTrue(TEXT("Right path angle should match expected"), 
		FMath::Abs(RightAngle - ExpectedAngle) < 1.0f);

	// Test with custom path angle
	TestIntersection->PathAngle = 90.0f;
	LeftPath = TestIntersection->GetLeftPathDirection();
	RightPath = TestIntersection->GetRightPathDirection();

	LeftAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Forward, LeftPath)));
	RightAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Forward, RightPath)));

	ExpectedAngle = 45.0f; // 45 degrees from forward

	TestTrue(TEXT("Left path angle should update with custom angle"), 
		FMath::Abs(LeftAngle - ExpectedAngle) < 1.0f);
	TestTrue(TEXT("Right path angle should update with custom angle"), 
		FMath::Abs(RightAngle - ExpectedAngle) < 1.0f);

	TestWorld->DestroyWorld(false);
	return true;
}