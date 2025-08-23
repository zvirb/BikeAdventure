// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformTime.h"
#include "Stats/Stats.h"
#include "Core/BikeMovementComponent.h"
#include "Gameplay/IntersectionDetector.h"
#include "Systems/BiomeGenerator.h"
#include "GameFramework/Actor.h"

// Frame rate performance test
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFrameRatePerformanceTest,
	"BikeAdventure.Performance.FrameRate",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFrameRatePerformanceTest::RunTest(const FString& Parameters)
{
	// Create test world
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull("Test world created", TestWorld);

	if (!TestWorld)
	{
		return false;
	}

	// Setup test scenario with multiple actors
	TArray<AActor*> TestActors;
	for (int i = 0; i < 10; i++) // Create multiple bike actors for load testing
	{
		AActor* BikeActor = TestWorld->SpawnActor<AActor>();
		UBikeMovementComponent* BikeMovement = NewObject<UBikeMovementComponent>(BikeActor);
		UIntersectionDetector* IntersectionDetector = NewObject<UIntersectionDetector>(BikeActor);
		
		BikeMovement->RegisterComponent();
		IntersectionDetector->RegisterComponent();
		BikeActor->AddOwnedComponent(BikeMovement);
		BikeActor->AddOwnedComponent(IntersectionDetector);
		
		BikeMovement->BeginPlay();
		IntersectionDetector->BeginPlay();
		BikeMovement->SetThrottle(1.0f);
		
		TestActors.Add(BikeActor);
	}

	// Setup performance monitoring
	const float TestDuration = 5.0f; // 5 second test
	TArray<float> FrameTimes;
	double StartTime = FPlatformTime::Seconds();
	
	// Performance test loop
	while ((FPlatformTime::Seconds() - StartTime) < TestDuration)
	{
		double FrameStartTime = FPlatformTime::Seconds();
		
		// Simulate frame work
		TestWorld->Tick(LEVELTICK_All, 0.016f);
		
		// Update all bike actors
		for (AActor* Actor : TestActors)
		{
			if (UBikeMovementComponent* BikeMovement = Actor->FindComponentByClass<UBikeMovementComponent>())
			{
				BikeMovement->UpdateMovement(0.016f);
			}
			if (UIntersectionDetector* IntersectionDetector = Actor->FindComponentByClass<UIntersectionDetector>())
			{
				IntersectionDetector->TickComponent(0.016f, ELevelTick::LEVELTICK_All, nullptr);
			}
		}
		
		double FrameTime = FPlatformTime::Seconds() - FrameStartTime;
		FrameTimes.Add(static_cast<float>(FrameTime));
	}

	// Calculate performance metrics
	float TotalFrameTime = 0.0f;
	for (float FrameTime : FrameTimes)
	{
		TotalFrameTime += FrameTime;
	}
	float AverageFrameTime = TotalFrameTime / FrameTimes.Num();
	float AverageFPS = 1.0f / AverageFrameTime;
	
	// Calculate percentile performance (1% low)
	FrameTimes.Sort();
	int OnePercentIndex = FMath::Max(1, FrameTimes.Num() / 100);
	float OnePercentLowFrameTime = FrameTimes[FrameTimes.Num() - OnePercentIndex];
	float OnePercentLowFPS = 1.0f / OnePercentLowFrameTime;

	// Log performance results
	UE_LOG(LogTemp, Warning, TEXT("Performance Test Results:"));
	UE_LOG(LogTemp, Warning, TEXT("Average FPS: %.2f"), AverageFPS);
	UE_LOG(LogTemp, Warning, TEXT("1%% Low FPS: %.2f"), OnePercentLowFPS);
	UE_LOG(LogTemp, Warning, TEXT("Total Frames: %d"), FrameTimes.Num());

	// Validate performance targets
	TestTrue("Average FPS meets 60 FPS minimum", AverageFPS >= 60.0f);
	TestTrue("1% Low FPS meets 45 FPS minimum", OnePercentLowFPS >= 45.0f);

	// Additional performance validation
	TestTrue("Frame time consistency", (OnePercentLowFrameTime / AverageFrameTime) < 2.0f); // 1% low shouldn't be more than 2x average

	// Cleanup
	TestWorld->DestroyWorld(false);

	return true;
}

// Memory usage performance test
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemoryUsagePerformanceTest,
	"BikeAdventure.Performance.MemoryUsage", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMemoryUsagePerformanceTest::RunTest(const FString& Parameters)
{
	// Get initial memory statistics
	FPlatformMemoryStats InitialStats = FPlatformMemory::GetStats();
	uint64 InitialUsedPhysical = InitialStats.UsedPhysical;

	// Create test world
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull("Test world created", TestWorld);

	if (!TestWorld)
	{
		return false;
	}

	// Create memory-intensive test scenario
	TArray<UObject*> CreatedObjects;

	// Create many biome generators (memory intensive)
	for (int i = 0; i < 100; i++)
	{
		UBiomeGenerator* BiomeGen = NewObject<UBiomeGenerator>();
		CreatedObjects.Add(BiomeGen);
		
		// Generate biomes to use memory
		EBiomeType CurrentBiome = EBiomeType::Forest;
		for (int j = 0; j < 50; j++)
		{
			CurrentBiome = BiomeGen->GenerateNextBiome(CurrentBiome, false);
		}
	}

	// Create many bike actors
	TArray<AActor*> BikeActors;
	for (int i = 0; i < 50; i++)
	{
		AActor* BikeActor = TestWorld->SpawnActor<AActor>();
		UBikeMovementComponent* BikeMovement = NewObject<UBikeMovementComponent>(BikeActor);
		UIntersectionDetector* IntersectionDetector = NewObject<UIntersectionDetector>(BikeActor);
		
		BikeMovement->RegisterComponent();
		IntersectionDetector->RegisterComponent();
		BikeActor->AddOwnedComponent(BikeMovement);
		BikeActor->AddOwnedComponent(IntersectionDetector);
		
		BikeActors.Add(BikeActor);
	}

	// Run simulation for a while to accumulate memory usage
	for (int Frame = 0; Frame < 300; Frame++) // 5 seconds at 60fps
	{
		TestWorld->Tick(LEVELTICK_All, 0.016f);
		
		for (AActor* Actor : BikeActors)
		{
			if (UBikeMovementComponent* BikeMovement = Actor->FindComponentByClass<UBikeMovementComponent>())
			{
				BikeMovement->SetThrottle(1.0f);
				BikeMovement->UpdateMovement(0.016f);
			}
			if (UIntersectionDetector* IntersectionDetector = Actor->FindComponentByClass<UIntersectionDetector>())
			{
				IntersectionDetector->TickComponent(0.016f, ELevelTick::LEVELTICK_All, nullptr);
			}
		}
	}

	// Measure peak memory usage
	FPlatformMemoryStats PeakStats = FPlatformMemory::GetStats();
	uint64 PeakUsedPhysical = PeakStats.UsedPhysical;
	
	// Calculate memory usage in GB
	float MemoryUsageGB = static_cast<float>(PeakUsedPhysical - InitialUsedPhysical) / (1024.0f * 1024.0f * 1024.0f);
	float TotalMemoryGB = static_cast<float>(PeakUsedPhysical) / (1024.0f * 1024.0f * 1024.0f);

	// Log memory results
	UE_LOG(LogTemp, Warning, TEXT("Memory Test Results:"));
	UE_LOG(LogTemp, Warning, TEXT("Test Memory Usage: %.3f GB"), MemoryUsageGB);
	UE_LOG(LogTemp, Warning, TEXT("Total Memory Usage: %.3f GB"), TotalMemoryGB);
	UE_LOG(LogTemp, Warning, TEXT("Available Physical Memory: %.3f GB"), static_cast<float>(PeakStats.AvailablePhysical) / (1024.0f * 1024.0f * 1024.0f));

	// Validate memory budget
	TestTrue("Total memory usage within 4GB budget", TotalMemoryGB <= 4.0f);
	
	// Test that we're not using excessive memory for our test objects
	TestTrue("Test scenario memory usage reasonable", MemoryUsageGB < 1.0f); // Should be less than 1GB for test scenario

	// Test memory cleanup
	CreatedObjects.Empty();
	BikeActors.Empty();
	
	// Force garbage collection
	GEngine->ForceGarbageCollection(true);
	
	FPlatformMemoryStats PostCleanupStats = FPlatformMemory::GetStats();
	uint64 PostCleanupUsed = PostCleanupStats.UsedPhysical;
	
	// Memory should be released (though not necessarily all due to engine overhead)
	TestTrue("Memory released after cleanup", PostCleanupUsed < PeakUsedPhysical);

	// Cleanup
	TestWorld->DestroyWorld(false);

	return true;
}

// Loading time performance test
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLoadingTimePerformanceTest,
	"BikeAdventure.Performance.LoadingTime",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FLoadingTimePerformanceTest::RunTest(const FString& Parameters)
{
	// Test world creation time (simulates level loading)
	double LoadStartTime = FPlatformTime::Seconds();
	
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull("Test world created", TestWorld);

	if (!TestWorld)
	{
		return false;
	}

	// Simulate loading game objects
	TArray<AActor*> LoadedActors;
	for (int i = 0; i < 20; i++)
	{
		AActor* Actor = TestWorld->SpawnActor<AActor>();
		UBikeMovementComponent* BikeMovement = NewObject<UBikeMovementComponent>(Actor);
		UIntersectionDetector* IntersectionDetector = NewObject<UIntersectionDetector>(Actor);
		
		BikeMovement->RegisterComponent();
		IntersectionDetector->RegisterComponent();
		Actor->AddOwnedComponent(BikeMovement);
		Actor->AddOwnedComponent(IntersectionDetector);
		
		LoadedActors.Add(Actor);
	}

	// Simulate biome generation (part of world loading)
	TArray<UBiomeGenerator*> BiomeGenerators;
	for (int i = 0; i < 10; i++)
	{
		UBiomeGenerator* BiomeGen = NewObject<UBiomeGenerator>();
		BiomeGenerators.Add(BiomeGen);
		
		// Pre-generate some biome sequences
		EBiomeType CurrentBiome = static_cast<EBiomeType>(i % 6 + 1); // Cycle through biome types
		for (int j = 0; j < 10; j++)
		{
			CurrentBiome = BiomeGen->GenerateNextBiome(CurrentBiome, false);
		}
	}

	double LoadEndTime = FPlatformTime::Seconds();
	float LoadingTime = static_cast<float>(LoadEndTime - LoadStartTime);

	// Log loading performance
	UE_LOG(LogTemp, Warning, TEXT("Loading Performance Results:"));
	UE_LOG(LogTemp, Warning, TEXT("Loading Time: %.3f seconds"), LoadingTime);
	UE_LOG(LogTemp, Warning, TEXT("Objects Loaded: %d"), LoadedActors.Num() + BiomeGenerators.Num());

	// Validate loading time target
	TestTrue("Loading time meets 3 second target", LoadingTime <= 3.0f);
	
	// Additional validation for reasonable loading performance
	TestTrue("Loading time is reasonable", LoadingTime >= 0.001f); // Should take some measurable time
	TestTrue("Loading created expected objects", LoadedActors.Num() == 20 && BiomeGenerators.Num() == 10);

	// Test that loaded objects are functional
	if (LoadedActors.Num() > 0)
	{
		UBikeMovementComponent* BikeMovement = LoadedActors[0]->FindComponentByClass<UBikeMovementComponent>();
		TestNotNull("Loaded bike has movement component", BikeMovement);
		
		if (BikeMovement)
		{
			BikeMovement->SetThrottle(1.0f);
			BikeMovement->UpdateMovement(0.016f);
			TestTrue("Loaded bike can move", BikeMovement->GetVelocity().Size() > 0);
		}
	}

	// Cleanup
	TestWorld->DestroyWorld(false);

	return true;
}