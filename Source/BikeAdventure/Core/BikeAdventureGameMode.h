#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BikeAdventureGameMode.generated.h"

class UIntersectionManager;
class UBiomeGenerator;

/**
 * Main Game Mode for BikeAdventure
 * Manages core game systems and orchestrates the meditative exploration experience
 */
UCLASS()
class BIKEADVENTURE_API ABikeAdventureGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABikeAdventureGameMode();

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Intersection management system */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Systems")
	TObjectPtr<UIntersectionManager> IntersectionManager;

	/** Procedural biome generation system */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Systems")
	TObjectPtr<UBiomeGenerator> BiomeGenerator;

	/** Default forward speed for all bikes in cm/s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Settings")
	float DefaultBikeSpeed = 1200.0f;

	/** Default turn rate for all bikes in degrees/sec */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Settings")
	float DefaultTurnRate = 45.0f;

public:
	/** Get the intersection manager instance */
	UFUNCTION(BlueprintCallable, Category = "Core Systems")
	UIntersectionManager* GetIntersectionManager() const { return IntersectionManager; }

	/** Get the biome generator instance */
	UFUNCTION(BlueprintCallable, Category = "Core Systems")
	UBiomeGenerator* GetBiomeGenerator() const { return BiomeGenerator; }

	/** Get default bike speed */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Settings")
	float GetDefaultBikeSpeed() const { return DefaultBikeSpeed; }

	/** Get default turn rate */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Settings")
	float GetDefaultTurnRate() const { return DefaultTurnRate; }

protected:
	/** Initialize core game systems */
	UFUNCTION(BlueprintCallable, Category = "Core Systems")
	void InitializeCoreSystemsFromBlueprint();

private:
	/** Initialize core game systems */
	void InitializeCoreystems();
};