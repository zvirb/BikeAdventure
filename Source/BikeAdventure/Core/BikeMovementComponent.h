// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BikeMovementComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BIKEADVENTURE_API UBikeMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBikeMovementComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Movement functions
	UFUNCTION(BlueprintCallable, Category = "Bike Movement")
	void SetThrottle(float Value);

	UFUNCTION(BlueprintCallable, Category = "Bike Movement")
	void SetSteering(float Value);

	UFUNCTION(BlueprintCallable, Category = "Bike Movement")
	void UpdateMovement(float DeltaTime);

	// Getters
	UFUNCTION(BlueprintPure, Category = "Bike Movement")
	FVector GetVelocity() const { return Velocity; }

	UFUNCTION(BlueprintPure, Category = "Bike Movement")
	FVector GetAngularVelocity() const { return AngularVelocity; }

	UFUNCTION(BlueprintPure, Category = "Bike Movement")
	float GetMaxSpeed() const { return MaxSpeed; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MaxSpeed = 1200.0f; // cm/s

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float Acceleration = 600.0f; // cm/s²

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float TurnRate = 90.0f; // degrees/s

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float Friction = 0.95f;

private:
	FVector Velocity = FVector::ZeroVector;
	FVector AngularVelocity = FVector::ZeroVector;
	float ThrottleInput = 0.0f;
	float SteeringInput = 0.0f;
};