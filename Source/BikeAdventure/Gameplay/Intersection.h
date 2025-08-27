#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "../Core/BiomeTypes.h"
#include "Intersection.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UNiagaraComponent;

/**
 * Intersection actor representing decision points in the bike adventure
 * Handles biome-specific visual styles and path generation hints
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API AIntersection : public AActor
{
    GENERATED_BODY()
    
public:    
    AIntersection();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    /**
     * Set the intersection type which determines visual appearance
     */
    UFUNCTION(BlueprintCallable, Category = "Intersection")
    void SetIntersectionType(EIntersectionType NewType);

    /**
     * Get the current intersection type
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Intersection")
    EIntersectionType GetIntersectionType() const { return IntersectionType; }

    /**
     * Set the biomes that each path leads to
     */
    UFUNCTION(BlueprintCallable, Category = "Intersection")
    void SetPathBiomes(EBiomeType LeftBiome, EBiomeType RightBiome);

    /**
     * Get the biome that the left path leads to
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Intersection")
    EBiomeType GetLeftPathBiome() const { return LeftPathBiome; }

    /**
     * Get the biome that the right path leads to
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Intersection")
    EBiomeType GetRightPathBiome() const { return RightPathBiome; }

    /**
     * Set path hints for this intersection
     */
    UFUNCTION(BlueprintCallable, Category = "Intersection")
    void SetPathHints(const FPathHints& NewHints);

    /**
     * Get the current path hints
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Intersection")
    FPathHints GetPathHints() const { return PathHints; }

    /**
     * Get the left path direction vector
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Intersection")
    FVector GetLeftPathDirection() const { return LeftPathDirection; }

    /**
     * Get the right path direction vector
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Intersection")
    FVector GetRightPathDirection() const { return RightPathDirection; }

    /**
     * Handle player entering the intersection trigger
     */
    UFUNCTION()
    void OnPlayerEnterIntersection(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    /**
     * Handle player exiting the intersection trigger
     */
    UFUNCTION()
    void OnPlayerExitIntersection(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    /**
     * Called when player makes a choice at this intersection
     */
    UFUNCTION(BlueprintCallable, Category = "Intersection")
    void OnPlayerChoiceMade(bool bChoseLeftPath);

    /**
     * Handle player choice input
     */
    UFUNCTION(BlueprintCallable, Category = "Intersection")
    void HandlePlayerChoice(bool bChoseLeftPath);

protected:
    // Root scene component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USceneComponent* RootSceneComponent;

    // Collision trigger for player detection
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UBoxComponent* TriggerVolume;

    // Main intersection mesh
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* IntersectionMesh;

    // Left path indicator mesh
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* LeftPathIndicator;

    // Right path indicator mesh
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* RightPathIndicator;

    // Environmental effect component for atmosphere
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UNiagaraComponent* EnvironmentalEffect;

    // Audio component for ambient sounds
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* AmbientAudio;

    // Type of intersection (determines visual style)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intersection Settings")
    EIntersectionType IntersectionType;

    // Biome that the left path leads to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings")
    EBiomeType LeftPathBiome;

    // Biome that the right path leads to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings")
    EBiomeType RightPathBiome;

    // Path hints for this intersection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings")
    FPathHints PathHints;

    // Direction vector for left path
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings")
    FVector LeftPathDirection;

    // Direction vector for right path
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings")
    FVector RightPathDirection;

    // Whether a player is currently at this intersection
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bPlayerPresent;

    // Whether a choice has been made at this intersection
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bChoiceMade;

    // Mesh variations for different intersection types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    TMap<EIntersectionType, TSoftObjectPtr<UStaticMesh>> IntersectionMeshMap;

    // Material variations for different biomes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    TMap<EBiomeType, TSoftObjectPtr<UMaterialInterface>> BiomeMaterials;

    // Particle effects for different biomes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    TMap<EBiomeType, TSoftObjectPtr<class UNiagaraSystem>> BiomeEffects;

    // Audio cues for different biomes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TMap<EBiomeType, TSoftObjectPtr<class USoundCue>> BiomeAmbientSounds;

    // Path hint visual elements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint Settings")
    TMap<EPathPersonality, TSoftObjectPtr<UStaticMesh>> PathHintMeshes;

    // Path hint particle effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint Settings")
    TMap<EPathPersonality, TSoftObjectPtr<UNiagaraSystem>> PathHintEffects;

private:
    /**
     * Update visual appearance based on intersection type
     */
    void UpdateVisualAppearance();

    /**
     * Update path hints visual elements
     */
    void UpdatePathHints();

    /**
     * Apply biome-specific materials and effects
     */
    void ApplyBiomeVisuals(EBiomeType BiomeType, UStaticMeshComponent* TargetMesh, bool bIsLeftPath);

    /**
     * Initialize default assets
     */
    void InitializeDefaultAssets();

    /**
     * Calculate path directions based on intersection type
     */
    void CalculatePathDirections();

public:
    // Blueprint events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerEnterIntersection, AIntersection*, Intersection, AActor*, Player);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerExitIntersection, AIntersection*, Intersection, AActor*, Player);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerChoiceMade, AIntersection*, Intersection, bool, bChoseLeftPath, EBiomeType, ChosenBiome);

    // Event dispatchers
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPlayerEnterIntersection OnPlayerEnterIntersectionEvent;

    UPROPERTY(BlueprintAssignable, Category = "Events")  
    FOnPlayerExitIntersection OnPlayerExitIntersectionEvent;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPlayerChoiceMade OnPlayerChoiceMadeEvent;
};

/**
 * Specialized intersection classes for different biome types
 */

/**
 * Forest intersection with Y-fork design
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API AForestIntersection : public AIntersection
{
    GENERATED_BODY()

public:
    AForestIntersection();

protected:
    virtual void BeginPlay() override;

    // Forest-specific moss and vegetation on paths
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Settings")
    float VegetationDensity;

    // Wildlife activity level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Settings")
    float WildlifeActivity;
};

/**
 * Urban intersection with roundabout or T-junction design
 */
UCLASS(BlueprintType, Blueprintable)  
class BIKEADVENTURE_API AUrbanIntersection : public AIntersection
{
    GENERATED_BODY()

public:
    AUrbanIntersection();

protected:
    virtual void BeginPlay() override;

    // Urban lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Urban Components")
    TArray<class UPointLightComponent*> StreetLights;

    // Traffic elements (signs, signals, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Urban Settings")
    bool bHasTrafficSignals;

    // Street surface type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Urban Settings")
    FString StreetSurfaceType;
};

/**
 * Beach intersection with boardwalk design
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API ABeachIntersection : public AIntersection
{
    GENERATED_BODY()

public:
    ABeachIntersection();

protected:
    virtual void BeginPlay() override;

    // Sand particle effects
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beach Components")
    UNiagaraComponent* SandParticles;

    // Wave sound intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beach Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WaveSoundIntensity;

    // Palm tree placement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beach Settings")
    bool bHasPalmTrees;
};

/**
 * Mountain intersection with rock pass design
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API AMountainIntersection : public AIntersection
{
    GENERATED_BODY()

public:
    AMountainIntersection();

protected:
    virtual void BeginPlay() override;

    // Rock formation meshes
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mountain Components")
    TArray<UStaticMeshComponent*> RockFormations;

    // Elevation difference between paths
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Settings")
    float PathElevationDifference;

    // Wind effect intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WindIntensity;
};