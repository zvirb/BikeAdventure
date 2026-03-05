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

	/** Discovery system for tracking encounters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Systems")
	TObjectPtr<class UDiscoverySystem> DiscoverySystem;

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

	/** Enable or disable Zen Mode (no UI, pure exploration) */
	UFUNCTION(BlueprintCallable, Category = "Special Mode")
	void SetZenMode(bool bEnable) { bIsZenMode = bEnable; }

	/** Check if Zen Mode is currently enabled */
	UFUNCTION(BlueprintCallable, Category = "Special Mode")
	bool IsZenModeEnabled() const { return bIsZenMode; }

	/** Enable or disable Journey Mode (guided to a destination) */
	UFUNCTION(BlueprintCallable, Category = "Special Mode")
	void SetJourneyMode(bool bEnable) { bIsJourneyMode = bEnable; }

	/** Check if Journey Mode is currently enabled */
	UFUNCTION(BlueprintCallable, Category = "Special Mode")
	bool IsJourneyModeEnabled() const { return bIsJourneyMode; }

	/** Enable or disable Photo Mode */
	UFUNCTION(BlueprintCallable, Category = "Special Mode")
	void SetPhotoMode(bool bEnable) { bIsPhotoMode = bEnable; }

	/** Check if Photo Mode is currently enabled */
	UFUNCTION(BlueprintCallable, Category = "Special Mode")
	bool IsPhotoModeEnabled() const { return bIsPhotoMode; }

protected:
	/** Initialize core game systems */
	UFUNCTION(BlueprintCallable, Category = "Core Systems")
	void InitializeCoreSystemsFromBlueprint();

	/** Flag indicating if Zen mode is active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special Mode")
	bool bIsZenMode = false;

	/** Flag indicating if Journey mode is active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special Mode")
	bool bIsJourneyMode = false;

	/** Flag indicating if Photo mode is active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special Mode")
	bool bIsPhotoMode = false;

private:
	/** Initialize core game systems */
	void InitializeCoreystems();
};