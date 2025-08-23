// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "Core/BikeMovementComponent.h"
#include "GameFramework/Actor.h"

// Basic bike movement test
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBikeMovementBasicTest, 
	"BikeAdventure.Unit.Movement.BasicMovement", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBikeMovementBasicTest::RunTest(const FString& Parameters)
{
	// Create test world
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull("Test world created", TestWorld);

	if (!TestWorld)
	{
		return false;
	}

	// Create test actor with bike movement component
	AActor* TestActor = TestWorld->SpawnActor<AActor>();
	TestNotNull("Test actor created", TestActor);

	UBikeMovementComponent* BikeMovement = NewObject<UBikeMovementComponent>(TestActor);
	TestNotNull("Bike movement component created", BikeMovement);

	if (!TestActor || !BikeMovement)
	{
		return false;
	}

	// Initialize component
	BikeMovement->RegisterComponent();
	TestActor->AddOwnedComponent(BikeMovement);

	// Test initial state
	TestEqual("Initial velocity is zero", BikeMovement->GetVelocity(), FVector::ZeroVector);
	TestEqual("Initial angular velocity is zero", BikeMovement->GetAngularVelocity(), FVector::ZeroVector);

	// Test throttle input
	BikeMovement->SetThrottle(1.0f);
	BikeMovement->UpdateMovement(0.016f); // 60fps frame time

	TestTrue("Bike moves forward with throttle", BikeMovement->GetVelocity().X > 0);
	TestTrue("Forward velocity is reasonable", BikeMovement->GetVelocity().Size() < BikeMovement->GetMaxSpeed());

	// Test steering
	BikeMovement->SetSteering(1.0f);
	BikeMovement->UpdateMovement(0.016f);

	TestTrue("Bike turns with steering input", FMath::Abs(BikeMovement->GetAngularVelocity().Z) > 0);

	// Test speed limits
	for (int i = 0; i < 100; i++) // Simulate extended acceleration
	{
		BikeMovement->UpdateMovement(0.016f);
	}

	TestTrue("Speed respects maximum limit", BikeMovement->GetVelocity().Size() <= BikeMovement->GetMaxSpeed() + 1.0f); // Small tolerance

	// Test throttle clamping
	BikeMovement->SetThrottle(2.0f); // Over maximum
	BikeMovement->UpdateMovement(0.016f);
	// Should not crash or cause issues

	BikeMovement->SetThrottle(-1.0f); // Under minimum
	BikeMovement->UpdateMovement(0.016f);
	// Should not crash or cause issues

	// Test steering clamping
	BikeMovement->SetSteering(5.0f);
	BikeMovement->UpdateMovement(0.016f);
	TestTrue("Steering input is clamped", FMath::Abs(BikeMovement->GetAngularVelocity().Z) <= 90.0f + 1.0f);

	// Cleanup
	TestWorld->DestroyWorld(false);

	return true;
}

// Bike physics accuracy test
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBikeMovementPhysicsTest,
	"BikeAdventure.Unit.Movement.PhysicsAccuracy",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBikeMovementPhysicsTest::RunTest(const FString& Parameters)
{
	// Create test world
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull("Test world created", TestWorld);

	if (!TestWorld)
	{
		return false;
	}

	AActor* TestActor = TestWorld->SpawnActor<AActor>();
	UBikeMovementComponent* BikeMovement = NewObject<UBikeMovementComponent>(TestActor);
	BikeMovement->RegisterComponent();
	TestActor->AddOwnedComponent(BikeMovement);

	// Test friction behavior
	BikeMovement->SetThrottle(1.0f);
	BikeMovement->UpdateMovement(0.016f);
	float InitialSpeed = BikeMovement->GetVelocity().Size();

	BikeMovement->SetThrottle(0.0f); // No more throttle
	BikeMovement->UpdateMovement(0.016f);
	float SpeedAfterFriction = BikeMovement->GetVelocity().Size();

	TestTrue("Friction reduces speed", SpeedAfterFriction < InitialSpeed);

	// Test that bike eventually stops without throttle
	for (int i = 0; i < 1000; i++)
	{
		BikeMovement->UpdateMovement(0.016f);
	}

	TestTrue("Bike eventually stops due to friction", BikeMovement->GetVelocity().Size() < 1.0f);

	// Test turning physics
	FVector InitialPosition = TestActor->GetActorLocation();
	FRotator InitialRotation = TestActor->GetActorRotation();

	BikeMovement->SetThrottle(1.0f);
	BikeMovement->SetSteering(1.0f);

	for (int i = 0; i < 60; i++) // 1 second at 60fps
	{
		BikeMovement->UpdateMovement(0.016f);
	}

	TestTrue("Actor moved forward", TestActor->GetActorLocation().X > InitialPosition.X);
	TestTrue("Actor rotated", !TestActor->GetActorRotation().Equals(InitialRotation, 1.0f));

	// Cleanup
	TestWorld->DestroyWorld(false);

	return true;
}