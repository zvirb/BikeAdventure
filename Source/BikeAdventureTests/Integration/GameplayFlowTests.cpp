// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "Core/BikeMovementComponent.h"
#include "Gameplay/IntersectionDetector.h"
#include "Systems/BiomeGenerator.h"
#include "GameFramework/Actor.h"

// Complete gameplay flow test - Complex automation test with multiple sub-tests
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FGameplayFlowTest,
	"BikeAdventure.Integration.CompleteGameplayFlow")

void FGameplayFlowTest::GetTests(TArray<FString>& OutBeautifiedNames, 
								TArray<FString>& OutTestCommands) const
{
	OutBeautifiedNames.Add("Game Start to First Intersection");
	OutTestCommands.Add("GameStartToIntersection");
	
	OutBeautifiedNames.Add("Continuous Exploration Session");  
	OutTestCommands.Add("ContinuousExploration");
	
	OutBeautifiedNames.Add("Biome Transition During Gameplay");
	OutTestCommands.Add("BiomeTransitionGameplay");
}

bool FGameplayFlowTest::RunTest(const FString& Parameters)
{
	if (Parameters == "GameStartToIntersection")
	{
		return TestGameStartToIntersectionFlow();
	}
	else if (Parameters == "ContinuousExploration")  
	{
		return TestContinuousExplorationSession();
	}
	else if (Parameters == "BiomeTransitionGameplay")
	{
		return TestBiomeTransitionDuringGameplay();
	}
	
	return false;
}

bool FGameplayFlowTest::TestGameStartToIntersectionFlow()
{
	// Create test world
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull("Test world created", TestWorld);

	if (!TestWorld)
	{
		return false;
	}

	// Create player bike actor with all necessary components
	AActor* BikeActor = TestWorld->SpawnActor<AActor>();
	TestNotNull("Bike actor created", BikeActor);

	// Add movement component
	UBikeMovementComponent* BikeMovement = NewObject<UBikeMovementComponent>(BikeActor);
	TestNotNull("Bike movement component created", BikeMovement);
	BikeMovement->RegisterComponent();
	BikeActor->AddOwnedComponent(BikeMovement);

	// Add intersection detector
	UIntersectionDetector* IntersectionDetector = NewObject<UIntersectionDetector>(BikeActor);
	TestNotNull("Intersection detector created", IntersectionDetector);
	IntersectionDetector->RegisterComponent();
	BikeActor->AddOwnedComponent(IntersectionDetector);

	// Initialize components
	BikeMovement->BeginPlay();
	IntersectionDetector->BeginPlay();

	// Test initial game state
	TestFalse("Not at intersection initially", IntersectionDetector->IsAtIntersection());
	TestEqual("Initial velocity is zero", BikeMovement->GetVelocity(), FVector::ZeroVector);

	// Simulate gameplay - start moving forward
	BikeMovement->SetThrottle(1.0f);

	// Simulate game loop until we reach an intersection
	bool bReachedIntersection = false;
	float ElapsedTime = 0.0f;
	const float DeltaTime = 0.016f;
	const float MaxDuration = 160.0f; // ~10000 frames at 60fps
	bool bCheckedMovement = false;

	while (!bReachedIntersection && ElapsedTime < MaxDuration)
	{
		// Update movement
		BikeMovement->UpdateMovement(DeltaTime);
		
		// Check for intersections
		IntersectionDetector->TickComponent(DeltaTime, ELevelTick::LEVELTICK_All, nullptr);
		
		bReachedIntersection = IntersectionDetector->IsAtIntersection();
		ElapsedTime += DeltaTime;

		// Validate that bike is actually moving
		if (!bCheckedMovement && ElapsedTime >= 1.0f)
		{
			TestTrue("Bike is moving after 1 second", BikeMovement->GetVelocity().Size() > 0);
			bCheckedMovement = true;
		}
	}

	// Test that we can reach an intersection in reasonable time
	TestTrue("Reached intersection within reasonable time", ElapsedTime < MaxDuration);

	if (bReachedIntersection)
	{
		// Test intersection state
		TArray<FString> Choices = IntersectionDetector->GetAvailableChoices();
		TestTrue("Has choices at intersection", Choices.Num() >= 2);
		TestTrue("Has left turn option", Choices.Contains(TEXT("Turn Left")));
		TestTrue("Has right turn option", Choices.Contains(TEXT("Turn Right")));

		// Test making a choice
		IntersectionDetector->SelectPath(TEXT("Turn Left"));
		TestFalse("No longer at intersection after choice", IntersectionDetector->IsAtIntersection());
	}

	// Cleanup
	TestWorld->DestroyWorld(false);
	return true;
}

bool FGameplayFlowTest::TestContinuousExplorationSession()
{
	// Create test world
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull("Test world created", TestWorld);

	if (!TestWorld)
	{
		return false;
	}

	// Setup bike with all components
	AActor* BikeActor = TestWorld->SpawnActor<AActor>();
	UBikeMovementComponent* BikeMovement = NewObject<UBikeMovementComponent>(BikeActor);
	UIntersectionDetector* IntersectionDetector = NewObject<UIntersectionDetector>(BikeActor);
	
	BikeMovement->RegisterComponent();
	IntersectionDetector->RegisterComponent();
	BikeActor->AddOwnedComponent(BikeMovement);
	BikeActor->AddOwnedComponent(IntersectionDetector);
	
	BikeMovement->BeginPlay();
	IntersectionDetector->BeginPlay();

	// Simulate extended exploration session
	BikeMovement->SetThrottle(1.0f);
	
	int IntersectionsFound = 0;
	float ElapsedTime = 0.0f;
	const float DeltaTime = 0.016f;
	const float SessionDuration = 300.0f; // 5 minutes (18000 frames at 60fps)
	float NextValidationTime = 60.0f; // Every minute
	
	TArray<FString> ChoiceHistory;

	while (ElapsedTime < SessionDuration)
	{
		// Update movement
		BikeMovement->UpdateMovement(DeltaTime);
		IntersectionDetector->TickComponent(DeltaTime, ELevelTick::LEVELTICK_All, nullptr);

		// Handle intersections
		if (IntersectionDetector->IsAtIntersection())
		{
			IntersectionsFound++;
			
			TArray<FString> Choices = IntersectionDetector->GetAvailableChoices();
			TestTrue("Intersection has valid choices", Choices.Num() >= 2);

			// Make alternating choices for variety
			FString Choice = (IntersectionsFound % 2 == 0) ? TEXT("Turn Left") : TEXT("Turn Right");
			if (Choices.Contains(Choice))
			{
				IntersectionDetector->SelectPath(Choice);
				ChoiceHistory.Add(Choice);
			}
		}

		ElapsedTime += DeltaTime;

		// Periodic validation during session
		if (ElapsedTime >= NextValidationTime)
		{
			TestTrue("Bike still moving", BikeMovement->GetVelocity().Size() > 0);
			TestTrue("Speed within limits", BikeMovement->GetVelocity().Size() <= BikeMovement->GetMaxSpeed() + 1.0f);
			NextValidationTime += 60.0f;
		}
	}

	// Validate exploration session results
	TestTrue("Found multiple intersections", IntersectionsFound >= 3);
	TestTrue("Made multiple choices", ChoiceHistory.Num() >= 3);
	TestTrue("Session completed full duration", FMath::IsNearlyEqual(ElapsedTime, SessionDuration, 0.1f));

	// Test that we have variety in choices
	bool bHasLeftTurns = ChoiceHistory.Contains(TEXT("Turn Left"));
	bool bHasRightTurns = ChoiceHistory.Contains(TEXT("Turn Right"));
	TestTrue("Used different turn directions", bHasLeftTurns && bHasRightTurns);

	// Cleanup
	TestWorld->DestroyWorld(false);
	return true;
}

bool FGameplayFlowTest::TestBiomeTransitionDuringGameplay()
{
	// Create test world
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull("Test world created", TestWorld);

	if (!TestWorld)
	{
		return false;
	}

	// Create biome generator for world state
	UBiomeGenerator* BiomeGen = NewObject<UBiomeGenerator>();
	TestNotNull("Biome generator created", BiomeGen);

	// Setup bike
	AActor* BikeActor = TestWorld->SpawnActor<AActor>();
	UBikeMovementComponent* BikeMovement = NewObject<UBikeMovementComponent>(BikeActor);
	UIntersectionDetector* IntersectionDetector = NewObject<UIntersectionDetector>(BikeActor);
	
	BikeMovement->RegisterComponent();
	IntersectionDetector->RegisterComponent();
	BikeActor->AddOwnedComponent(BikeMovement);
	BikeActor->AddOwnedComponent(IntersectionDetector);
	
	BikeMovement->BeginPlay();
	IntersectionDetector->BeginPlay();

	// Simulate gameplay with biome transitions
	BikeMovement->SetThrottle(1.0f);
	
	EBiomeType CurrentBiome = EBiomeType::Forest;
	TArray<EBiomeType> BiomeHistory;
	BiomeHistory.Add(CurrentBiome);
	
	int IntersectionsFound = 0;
	float ElapsedTime = 0.0f;
	const float DeltaTime = 0.016f;
	const float MaxDuration = 160.0f; // ~10000 frames at 60fps

	while (IntersectionsFound < 5 && ElapsedTime < MaxDuration)
	{
		BikeMovement->UpdateMovement(DeltaTime);
		IntersectionDetector->TickComponent(DeltaTime, ELevelTick::LEVELTICK_All, nullptr);

		if (IntersectionDetector->IsAtIntersection())
		{
			IntersectionsFound++;
			
			// Make a choice
			TArray<FString> Choices = IntersectionDetector->GetAvailableChoices();
			if (Choices.Num() > 0)
			{
				IntersectionDetector->SelectPath(Choices[0]);
				
				// Generate new biome for this path
				EBiomeType NewBiome = BiomeGen->GenerateNextBiome(CurrentBiome, false);
				TestTrue("Generated valid biome", NewBiome != EBiomeType::None);
				
				BiomeHistory.Add(NewBiome);
				CurrentBiome = NewBiome;
			}
		}
		
		ElapsedTime += DeltaTime;
	}

	// Validate biome progression
	TestTrue("Traversed multiple biomes", BiomeHistory.Num() >= 5);
	
	// Check for biome variety
	TSet<EBiomeType> UniqueBiomes(BiomeHistory);
	TestTrue("Experienced multiple different biomes", UniqueBiomes.Num() >= 3);

	// Validate biome transition rules were followed
	for (int i = 1; i < BiomeHistory.Num(); i++)
	{
		bool bValidTransition = BiomeGen->IsValidTransition(BiomeHistory[i-1], BiomeHistory[i]);
		TestTrue(FString::Printf(TEXT("Valid biome transition %d->%d"), 
			(int32)BiomeHistory[i-1], (int32)BiomeHistory[i]), bValidTransition);
	}

	// Cleanup
	TestWorld->DestroyWorld(false);
	return true;
}