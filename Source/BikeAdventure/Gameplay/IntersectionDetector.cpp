// Copyright Epic Games, Inc. All Rights Reserved.

#include "IntersectionDetector.h"
#include "GameFramework/Actor.h"

UIntersectionDetector::UIntersectionDetector()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UIntersectionDetector::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize available choices
	AvailableChoices.Add(TEXT("Turn Left"));
	AvailableChoices.Add(TEXT("Turn Right"));
}

void UIntersectionDetector::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	CheckForIntersection();
}

void UIntersectionDetector::CheckForIntersection()
{
	if (!GetOwner())
	{
		return;
	}

	FVector CurrentLocation = GetOwner()->GetActorLocation();
	
	// Simple intersection detection based on distance traveled
	float DistanceFromLastIntersection = FVector::Dist(CurrentLocation, LastIntersectionLocation);
	
	if (DistanceFromLastIntersection >= MinimumIntersectionDistance)
	{
		// Probability-based intersection detection (10% chance per check when eligible)
		if (FMath::RandRange(0.0f, 1.0f) <= 0.1f)
		{
			if (!bAtIntersection)
			{
				bAtIntersection = true;
				LastIntersectionLocation = CurrentLocation;
				GenerateChoices();
				
				// Broadcast event
				OnIntersectionReached.Broadcast();
			}
		}
	}
}

void UIntersectionDetector::GenerateChoices()
{
	// For now, always provide left/right choices
	// In a full implementation, this could be more sophisticated
	AvailableChoices.Empty();
	AvailableChoices.Add(TEXT("Turn Left"));
	AvailableChoices.Add(TEXT("Turn Right"));
	
	// Occasionally add a straight option
	if (FMath::RandRange(0.0f, 1.0f) <= 0.3f)
	{
		AvailableChoices.Add(TEXT("Continue Straight"));
	}
}

void UIntersectionDetector::SelectPath(const FString& Choice)
{
	if (bAtIntersection && AvailableChoices.Contains(Choice))
	{
		// Mark intersection as resolved
		bAtIntersection = false;
		
		// In a full implementation, this would trigger world generation
		// for the selected path
	}
}