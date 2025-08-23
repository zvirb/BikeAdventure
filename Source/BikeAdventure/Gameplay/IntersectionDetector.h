// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "IntersectionDetector.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIntersectionReached);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BIKEADVENTURE_API UIntersectionDetector : public UActorComponent
{
	GENERATED_BODY()

public:	
	UIntersectionDetector();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Check if player is at an intersection
	UFUNCTION(BlueprintCallable, Category = "Intersection")
	bool IsAtIntersection() const { return bAtIntersection; }

	// Get available choices at current intersection
	UFUNCTION(BlueprintCallable, Category = "Intersection")
	TArray<FString> GetAvailableChoices() const { return AvailableChoices; }

	// Select a path at intersection
	UFUNCTION(BlueprintCallable, Category = "Intersection")
	void SelectPath(const FString& Choice);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Intersection")
	FOnIntersectionReached OnIntersectionReached;

protected:
	// Detection parameters
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection")
	float DetectionRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection")
	float MinimumIntersectionDistance = 500.0f; // Minimum distance between intersections

private:
	bool bAtIntersection = false;
	TArray<FString> AvailableChoices;
	FVector LastIntersectionLocation = FVector::ZeroVector;

	void CheckForIntersection();
	void GenerateChoices();
};