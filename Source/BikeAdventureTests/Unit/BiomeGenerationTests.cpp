// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Systems/BiomeGenerator.h"

// Basic biome generation test
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBiomeGenerationBasicTest,
	"BikeAdventure.Unit.WorldGen.BiomeGeneration",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBiomeGenerationBasicTest::RunTest(const FString& Parameters)
{
	// Create biome generator
	UBiomeGenerator* BiomeGen = NewObject<UBiomeGenerator>();
	TestNotNull("Biome generator created", BiomeGen);

	if (!BiomeGen)
	{
		return false;
	}

	// Test basic biome generation
	EBiomeType CurrentBiome = EBiomeType::Forest;
	EBiomeType NextBiome = BiomeGen->GenerateNextBiome(CurrentBiome, false);
	
	TestTrue("Generated valid biome", NextBiome != EBiomeType::None);

	// Test that forest can transition to expected biomes
	TArray<EBiomeType> ValidForestTransitions = {
		EBiomeType::Plains, EBiomeType::Mountains, EBiomeType::Swamp, EBiomeType::Coast
	};

	bool bValidTransition = ValidForestTransitions.Contains(NextBiome);
	TestTrue("Forest transitions to valid biome", bValidTransition);

	// Test transition validation function
	TestTrue("Forest to Plains is valid", BiomeGen->IsValidTransition(EBiomeType::Forest, EBiomeType::Plains));
	TestFalse("Forest to Desert is invalid", BiomeGen->IsValidTransition(EBiomeType::Forest, EBiomeType::Desert));

	// Test probability calculation
	float Probability = BiomeGen->GetTransitionProbability(EBiomeType::Forest, EBiomeType::Plains);
	TestTrue("Probability is reasonable", Probability > 0.0f && Probability <= 1.0f);

	float InvalidProbability = BiomeGen->GetTransitionProbability(EBiomeType::Forest, EBiomeType::Desert);
	TestEqual("Invalid transition has zero probability", InvalidProbability, 0.0f);

	return true;
}

// Biome transition rules test
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBiomeTransitionRulesTest,
	"BikeAdventure.Unit.WorldGen.BiomeTransitionRules",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBiomeTransitionRulesTest::RunTest(const FString& Parameters)
{
	UBiomeGenerator* BiomeGen = NewObject<UBiomeGenerator>();
	TestNotNull("Biome generator created", BiomeGen);

	if (!BiomeGen)
	{
		return false;
	}

	// Test no repetition rule by generating sequence
	TArray<EBiomeType> BiomeSequence;
	EBiomeType TestBiome = EBiomeType::Plains; // Start with plains (most connected)

	// Generate a sequence of biomes
	for (int i = 0; i < 20; i++)
	{
		EBiomeType NextBiome = BiomeGen->GenerateNextBiome(TestBiome, false);
		BiomeSequence.Add(NextBiome);
		TestBiome = NextBiome;
	}

	// Check that we don't have too many consecutive same biomes
	int MaxConsecutiveFound = 0;
	int CurrentConsecutive = 1;

	for (int i = 1; i < BiomeSequence.Num(); i++)
	{
		if (BiomeSequence[i] == BiomeSequence[i-1])
		{
			CurrentConsecutive++;
		}
		else
		{
			MaxConsecutiveFound = FMath::Max(MaxConsecutiveFound, CurrentConsecutive);
			CurrentConsecutive = 1;
		}
	}
	MaxConsecutiveFound = FMath::Max(MaxConsecutiveFound, CurrentConsecutive);

	TestTrue("No more than 3 consecutive same biomes", MaxConsecutiveFound <= 3);

	// Test all biome types have valid transitions
	TArray<EBiomeType> AllBiomes = {
		EBiomeType::Forest, EBiomeType::Desert, EBiomeType::Mountains, 
		EBiomeType::Plains, EBiomeType::Coast, EBiomeType::Swamp
	};

	for (EBiomeType Biome : AllBiomes)
	{
		EBiomeType NextBiome = BiomeGen->GenerateNextBiome(Biome, false);
		TestTrue(FString::Printf(TEXT("Biome %d can generate next biome"), (int32)Biome), 
			NextBiome != EBiomeType::None);
	}

	return true;
}

// Biome diversity test
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBiomeDiversityTest,
	"BikeAdventure.Unit.WorldGen.BiomeDiversity",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBiomeDiversityTest::RunTest(const FString& Parameters)
{
	UBiomeGenerator* BiomeGen = NewObject<UBiomeGenerator>();
	TestNotNull("Biome generator created", BiomeGen);

	if (!BiomeGen)
	{
		return false;
	}

	// Generate a large sequence and check for diversity
	TMap<EBiomeType, int32> BiomeCounts;
	EBiomeType CurrentBiome = EBiomeType::Plains;

	for (int i = 0; i < 100; i++)
	{
		EBiomeType NextBiome = BiomeGen->GenerateNextBiome(CurrentBiome, false);
		
		if (!BiomeCounts.Contains(NextBiome))
		{
			BiomeCounts.Add(NextBiome, 0);
		}
		BiomeCounts[NextBiome]++;
		
		CurrentBiome = NextBiome;
	}

	// Check that we have reasonable diversity
	TestTrue("Generated multiple different biomes", BiomeCounts.Num() >= 3);

	// Check that no single biome dominates too much (should be less than 60% of total)
	for (auto& BiomeCount : BiomeCounts)
	{
		float Percentage = (float)BiomeCount.Value / 100.0f;
		TestTrue(FString::Printf(TEXT("Biome %d not overly dominant"), (int32)BiomeCount.Key), 
			Percentage < 0.6f);
	}

	return true;
}