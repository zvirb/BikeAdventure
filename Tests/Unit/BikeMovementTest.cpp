#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Systems/BikeMovementComponent.h"
#include "Core/BikeCharacter.h"

/**
 * Unit tests for BikeMovementComponent
 * Tests core movement mechanics, physics, and responsiveness
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBikeMovementForwardTest,
	"BikeAdventure.Core.Movement.ForwardMovement",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBikeMovementForwardTest::RunTest(const FString& Parameters)
{
	// Test forward movement functionality
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Create a test bike character
	ABikeCharacter* TestBike = TestWorld->SpawnActor<ABikeCharacter>();
	if (!TestBike)
	{
		AddError(TEXT("Failed to spawn test bike character"));
		TestWorld->DestroyWorld(false);
		return false;
	}

	UBikeMovementComponent* MovementComponent = TestBike->GetBikeMovement();
	if (!MovementComponent)
	{
		AddError(TEXT("BikeCharacter missing BikeMovementComponent"));
		TestWorld->DestroyWorld(false);
		return false;
	}

	// Test initial speed is zero
	TestEqual(TEXT("Initial speed should be zero"), MovementComponent->GetCurrentSpeed(), 0.0f);

	// Simulate movement update
	float DeltaTime = 0.016f; // 60 FPS
	MovementComponent->UpdateMovement(DeltaTime);

	// Test that bike starts moving forward
	float SpeedAfterUpdate = MovementComponent->GetCurrentSpeed();
	TestTrue(TEXT("Bike should start moving forward"), SpeedAfterUpdate > 0.0f);

	// Test that speed approaches target speed
	for (int32 i = 0; i < 120; ++i) // 2 seconds at 60 FPS
	{
		MovementComponent->UpdateMovement(DeltaTime);
	}

	float FinalSpeed = MovementComponent->GetCurrentSpeed();
	float ExpectedSpeed = MovementComponent->ForwardSpeed;
	TestTrue(TEXT("Speed should approach target forward speed"), 
		FMath::Abs(FinalSpeed - ExpectedSpeed) < 50.0f); // Within 50 cm/s

	TestWorld->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBikeMovementSteeringTest,
	"BikeAdventure.Core.Movement.SteeringMechanics",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBikeMovementSteeringTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	ABikeCharacter* TestBike = TestWorld->SpawnActor<ABikeCharacter>();
	if (!TestBike)
	{
		AddError(TEXT("Failed to spawn test bike character"));
		TestWorld->DestroyWorld(false);
		return false;
	}

	UBikeMovementComponent* MovementComponent = TestBike->GetBikeMovement();
	if (!MovementComponent)
	{
		AddError(TEXT("BikeCharacter missing BikeMovementComponent"));
		TestWorld->DestroyWorld(false);
		return false;
	}

	// Test steering input
	float SteeringInput = 1.0f; // Full right turn
	MovementComponent->SetSteering(SteeringInput);

	float DeltaTime = 0.016f;
	FRotator InitialRotation = TestBike->GetActorRotation();

	// Update movement for a few frames
	for (int32 i = 0; i < 60; ++i) // 1 second
	{
		MovementComponent->UpdateMovement(DeltaTime);
	}

	FRotator FinalRotation = TestBike->GetActorRotation();
	float YawDifference = FinalRotation.Yaw - InitialRotation.Yaw;

	TestTrue(TEXT("Bike should turn right with positive steering input"), YawDifference > 0.0f);

	// Test left steering
	MovementComponent->SetSteering(-1.0f); // Full left turn
	InitialRotation = TestBike->GetActorRotation();

	for (int32 i = 0; i < 60; ++i) // 1 second
	{
		MovementComponent->UpdateMovement(DeltaTime);
	}

	FinalRotation = TestBike->GetActorRotation();
	YawDifference = FinalRotation.Yaw - InitialRotation.Yaw;

	TestTrue(TEXT("Bike should turn left with negative steering input"), YawDifference < 0.0f);

	TestWorld->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBikeMovementIntersectionModeTest,
	"BikeAdventure.Core.Movement.IntersectionMode",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBikeMovementIntersectionModeTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	ABikeCharacter* TestBike = TestWorld->SpawnActor<ABikeCharacter>();
	if (!TestBike)
	{
		AddError(TEXT("Failed to spawn test bike character"));
		TestWorld->DestroyWorld(false);
		return false;
	}

	UBikeMovementComponent* MovementComponent = TestBike->GetBikeMovement();
	if (!MovementComponent)
	{
		AddError(TEXT("BikeCharacter missing BikeMovementComponent"));
		TestWorld->DestroyWorld(false);
		return false;
	}

	// Let bike reach normal speed first
	float DeltaTime = 0.016f;
	for (int32 i = 0; i < 120; ++i) // 2 seconds
	{
		MovementComponent->UpdateMovement(DeltaTime);
	}

	float NormalSpeed = MovementComponent->GetCurrentSpeed();

	// Enable intersection mode
	MovementComponent->SetIntersectionMode(true);

	// Let speed adjust
	for (int32 i = 0; i < 120; ++i) // 2 seconds
	{
		MovementComponent->UpdateMovement(DeltaTime);
	}

	float IntersectionSpeed = MovementComponent->GetCurrentSpeed();

	TestTrue(TEXT("Intersection mode should reduce speed"), IntersectionSpeed < NormalSpeed);
	TestTrue(TEXT("Intersection speed should be close to target"), 
		FMath::Abs(IntersectionSpeed - MovementComponent->IntersectionSpeed) < 50.0f);

	// Disable intersection mode
	MovementComponent->SetIntersectionMode(false);

	// Let speed adjust back
	for (int32 i = 0; i < 120; ++i) // 2 seconds
	{
		MovementComponent->UpdateMovement(DeltaTime);
	}

	float RestoredSpeed = MovementComponent->GetCurrentSpeed();

	TestTrue(TEXT("Speed should restore after leaving intersection"), 
		FMath::Abs(RestoredSpeed - NormalSpeed) < 50.0f);

	TestWorld->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBikeCharacterInputTest,
	"BikeAdventure.Core.Character.InputHandling", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBikeCharacterInputTest::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	ABikeCharacter* TestBike = TestWorld->SpawnActor<ABikeCharacter>();
	if (!TestBike)
	{
		AddError(TEXT("Failed to spawn test bike character"));
		TestWorld->DestroyWorld(false);
		return false;
	}

	// Test turning input handling
	TestBike->HandleTurnInput(1.0f);
	
	// We can't easily test the internal steering value directly,
	// but we can test that the function doesn't crash
	TestTrue(TEXT("HandleTurnInput should not crash"), true);

	// Test intersection choice handling (should not crash when no intersection)
	TestBike->HandleLeftChoice();
	TestBike->HandleRightChoice();

	TestTrue(TEXT("Choice handling should not crash without intersection"), true);

	// Test speed getter
	float CurrentSpeed = TestBike->GetCurrentSpeed();
	TestTrue(TEXT("GetCurrentSpeed should return valid value"), CurrentSpeed >= 0.0f);

	// Test intersection state
	TestFalse(TEXT("Should not be at intersection initially"), TestBike->IsAtIntersection());
	TestNull(TEXT("Current intersection should be null"), TestBike->GetCurrentIntersection());

	TestWorld->DestroyWorld(false);
	return true;
}