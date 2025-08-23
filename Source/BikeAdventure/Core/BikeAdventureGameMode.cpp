#include "BikeAdventureGameMode.h"
#include "Core/BikeCharacter.h"
#include "Systems/IntersectionManager.h"
#include "Systems/BiomeGenerator.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

ABikeAdventureGameMode::ABikeAdventureGameMode()
{
	// Set default pawn class to our custom bike character
	DefaultPawnClass = ABikeCharacter::StaticClass();

	// Initialize core systems
	IntersectionManager = CreateDefaultSubobject<UIntersectionManager>(TEXT("IntersectionManager"));
	BiomeGenerator = CreateDefaultSubobject<UBiomeGenerator>(TEXT("BiomeGenerator"));

	// Set default gameplay settings optimized for meditative experience
	DefaultBikeSpeed = 1200.0f; // 12 m/s - comfortable exploration speed
	DefaultTurnRate = 45.0f;    // Smooth, non-jarring turn rate

	// Enable tick for real-time system updates
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ABikeAdventureGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeCoreystems();

	UE_LOG(LogTemp, Warning, TEXT("BikeAdventure Game Mode initialized successfully"));
}

void ABikeAdventureGameMode::InitializeCoreystemsFromBlueprint()
{
	InitializeCoreystems();
}

void ABikeAdventureGameMode::InitializeCoreystems()
{
	// Initialize intersection management system
	if (IntersectionManager)
	{
		IntersectionManager->Initialize();
		UE_LOG(LogTemp, Log, TEXT("Intersection Manager initialized"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize Intersection Manager - null reference"));
	}

	// Initialize biome generation system
	if (BiomeGenerator)
	{
		BiomeGenerator->Initialize();
		UE_LOG(LogTemp, Log, TEXT("Biome Generator initialized"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize Biome Generator - null reference"));
	}
}