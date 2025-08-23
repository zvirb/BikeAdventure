// Copyright Epic Games, Inc. All Rights Reserved.

#include "BikeMovementComponent.h"
#include "GameFramework/Actor.h"

UBikeMovementComponent::UBikeMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBikeMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBikeMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateMovement(DeltaTime);
}

void UBikeMovementComponent::SetThrottle(float Value)
{
	ThrottleInput = FMath::Clamp(Value, 0.0f, 1.0f);
}

void UBikeMovementComponent::SetSteering(float Value)
{
	SteeringInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

void UBikeMovementComponent::UpdateMovement(float DeltaTime)
{
	if (!GetOwner())
	{
		return;
	}

	// Update forward velocity based on throttle
	float ForwardAccel = ThrottleInput * Acceleration * DeltaTime;
	FVector ForwardDirection = GetOwner()->GetActorForwardVector();
	
	// Add acceleration
	Velocity += ForwardDirection * ForwardAccel;
	
	// Apply friction
	Velocity *= FMath::Pow(Friction, DeltaTime);
	
	// Clamp to max speed
	if (Velocity.Size() > MaxSpeed)
	{
		Velocity = Velocity.GetSafeNormal() * MaxSpeed;
	}

	// Update angular velocity based on steering
	AngularVelocity.Z = SteeringInput * TurnRate;

	// Apply movement to actor
	FVector NewLocation = GetOwner()->GetActorLocation() + (Velocity * DeltaTime);
	FRotator NewRotation = GetOwner()->GetActorRotation() + FRotator(0, AngularVelocity.Z * DeltaTime, 0);
	
	GetOwner()->SetActorLocationAndRotation(NewLocation, NewRotation);
}