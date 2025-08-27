#pragma once

#include "CoreMinimal.h"
#include "BiomeGenerator.h"
#include "PCGSettings.h"
#include "PCGElement.h"
#include "AdvancedBiomePCGSettings.generated.h"

/**
 * Urban-specific PCG settings with detailed city generation
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API UUrbanPCGSettings : public UBiomePCGSettings
{
    GENERATED_BODY()

public:
    UUrbanPCGSettings();

    // Building density
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Urban Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BuildingDensity;

    // Street furniture density (benches, lamp posts, etc.)
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Urban Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StreetFurnitureDensity;

    // Park and green space probability
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Urban Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float GreenSpaceChance;

    // Traffic elements (signals, signs)
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Urban Generation")
    bool bIncludeTrafficElements;

    // Building mesh variations
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Urban Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> BuildingMeshes;

    // Street furniture meshes
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Urban Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> StreetFurnitureMeshes;

    // Traffic element meshes
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Urban Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> TrafficElementMeshes;

    // Urban lighting setups
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Urban Lighting")
    TArray<TSoftObjectPtr<class UPointLightComponent>> LightingPresets;
};

/**
 * Countryside-specific PCG settings with farms and villages
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API UCountrysidePCGSettings : public UBiomePCGSettings
{
    GENERATED_BODY()

public:
    UCountrysidePCGSettings();

    // Farm structure density
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Countryside Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FarmDensity;

    // Field and crop density
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Countryside Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CropFieldDensity;

    // Fence and boundary density
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Countryside Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FenceDensity;

    // Village probability
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Countryside Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VillageChance;

    // Pastoral animal density
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Countryside Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AnimalDensity;

    // Farm building meshes
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Countryside Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> FarmBuildingMeshes;

    // Crop and vegetation meshes
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Countryside Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> CropMeshes;

    // Fence and boundary meshes
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Countryside Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> FenceMeshes;

    // Animal static meshes
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Countryside Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> AnimalMeshes;
};

/**
 * Mountain-specific PCG settings with elevation and terrain features
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API UMountainPCGSettings : public UBiomePCGSettings
{
    GENERATED_BODY()

public:
    UMountainPCGSettings();

    // Rock formation density
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mountain Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RockFormationDensity;

    // Cliff and ledge probability
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mountain Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CliffChance;

    // Alpine vegetation density
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mountain Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AlpineVegetationDensity;

    // Snow coverage (0 = none, 1 = full)
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mountain Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SnowCoverage;

    // Cave entrance probability
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mountain Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CaveEntranceChance;

    // Elevation variation intensity
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mountain Generation", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float ElevationVariation;

    // Rock and boulder meshes
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mountain Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> RockMeshes;

    // Cliff wall meshes
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mountain Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> CliffMeshes;

    // Alpine plant meshes
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mountain Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> AlpinePlantMeshes;

    // Snow and ice effects
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mountain Effects")
    TArray<TSoftObjectPtr<UNiagaraSystem>> SnowEffects;
};

/**
 * Wetlands-specific PCG settings with water and marsh features
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API UWetlandsPCGSettings : public UBiomePCGSettings
{
    GENERATED_BODY()

public:
    UWetlandsPCGSettings();

    // Water body density
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wetlands Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WaterBodyDensity;

    // Marsh vegetation density
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wetlands Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MarshVegetationDensity;

    // Bridge and boardwalk probability
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wetlands Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BridgeChance;

    // Wildlife activity level
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wetlands Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WildlifeActivity;

    // Mist and fog intensity
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wetlands Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FogIntensity;

    // Water surface meshes and materials
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wetlands Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> WaterSurfaceMeshes;

    // Marsh plant meshes (cattails, reeds, etc.)
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wetlands Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> MarshPlantMeshes;

    // Bridge and walkway meshes
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wetlands Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> BridgeMeshes;

    // Water and mist effects
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Wetlands Effects")
    TArray<TSoftObjectPtr<UNiagaraSystem>> WaterEffects;
};

/**
 * Forest-specific PCG settings
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API UForestPCGSettings : public UBiomePCGSettings
{
    GENERATED_BODY()

public:
    UForestPCGSettings();

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Forest Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TreeDensity;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Forest Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float UndergrowthDensity;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Forest Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WildlifePresence;
};

/**
 * Desert-specific PCG settings
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API UDesertPCGSettings : public UBiomePCGSettings
{
    GENERATED_BODY()

public:
    UDesertPCGSettings();

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Desert Generation", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
    float DuneHeight;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Desert Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RockFormationDensity;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Desert Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float OasisChance;
};

/**
 * Beach-specific PCG settings
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API UBeachPCGSettings : public UBiomePCGSettings
{
    GENERATED_BODY()

public:
    UBeachPCGSettings();

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Beach Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PalmTreeDensity;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Beach Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BoardwalkChance;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Beach Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ShellDensity;
};

/**
 * Advanced PCG element for specialized biome generation
 */
class BIKEADVENTURE_API FAdvancedBiomeGenerationElement : public IPCGElement
{
public:
    virtual FPCGContext* Initialize(const FPCGDataCollection& InputData, TWeakObjectPtr<UPCGComponent> SourceComponent, const UPCGNode* Node) override;
    virtual bool ExecuteInternal(FPCGContext* Context) const override;

protected:
    virtual bool CanExecuteOnlyOnMainThread(FPCGContext* Context) const override { return false; }
    virtual bool IsCacheable(const UPCGSettings* InSettings) const override { return true; }

private:
    // Generate urban layout
    void GenerateUrbanLayout(FPCGContext* Context, const UUrbanPCGSettings* Settings, TArray<FPCGPoint>& OutPoints) const;

    // Generate countryside farms
    void GenerateCountrysideLayout(FPCGContext* Context, const UCountrysidePCGSettings* Settings, TArray<FPCGPoint>& OutPoints) const;

    // Generate mountain terrain
    void GenerateMountainTerrain(FPCGContext* Context, const UMountainPCGSettings* Settings, TArray<FPCGPoint>& OutPoints) const;

    // Generate wetlands ecosystem
    void GenerateWetlandsEcosystem(FPCGContext* Context, const UWetlandsPCGSettings* Settings, TArray<FPCGPoint>& OutPoints) const;

    // Helper function to create point with biome-specific attributes
    FPCGPoint CreateBiomePoint(const FVector& Location, const FRotator& Rotation, const FVector& Scale, EBiomeType BiomeType, int32 MeshIndex = 0) const;

    // Apply biome-specific material and mesh variations
    void ApplyBiomeAttributes(FPCGPoint& Point, EBiomeType BiomeType, const FString& ObjectType) const;
};

/**
 * Data asset for storing biome generation presets
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API UBiomeGenerationPreset : public UDataAsset
{
    GENERATED_BODY()

public:
    // Name of this preset
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preset Info")
    FString PresetName;

    // Description of this preset
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preset Info")
    FString Description;

    // Target biome type
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preset Info")
    EBiomeType TargetBiome;

    // Generation parameters for this preset
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    FBiomeGenerationParams GenerationParams;

    // Asset references for this biome
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> PrimaryMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    TArray<TSoftObjectPtr<UStaticMesh>> SecondaryMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assets")
    TArray<TSoftObjectPtr<UMaterialInterface>> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    TArray<TSoftObjectPtr<UNiagaraSystem>> ParticleEffects;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TArray<TSoftObjectPtr<USoundCue>> AmbientSounds;
};

/**
 * Manager class for biome generation presets
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API UBiomePresetManager : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Load all available biome presets
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Presets")
    void LoadBiomePresets();

    /**
     * Get preset for a specific biome type
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome Presets")
    UBiomeGenerationPreset* GetPresetForBiome(EBiomeType BiomeType);

    /**
     * Get all available presets for a biome type
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome Presets")
    TArray<UBiomeGenerationPreset*> GetPresetsForBiome(EBiomeType BiomeType);

    /**
     * Create PCG settings from preset
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Presets")
    UBiomePCGSettings* CreatePCGSettingsFromPreset(UBiomeGenerationPreset* Preset);

protected:
    // Loaded biome presets mapped by biome type
    UPROPERTY()
    TMap<EBiomeType, TArray<UBiomeGenerationPreset*>> BiomePresets;

    // Default presets for each biome
    UPROPERTY()
    TMap<EBiomeType, UBiomeGenerationPreset*> DefaultPresets;
};