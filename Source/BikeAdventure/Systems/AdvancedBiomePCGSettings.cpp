#include "AdvancedBiomePCGSettings.h"
#include "PCGContext.h"
#include "PCGData.h"
#include "Elements/PCGPointData.h"
#include "Engine/Engine.h"
#include "AssetRegistry/AssetRegistryModule.h"

// Urban PCG Settings
UUrbanPCGSettings::UUrbanPCGSettings()
{
    BiomeType = EBiomeType::Urban;
    BuildingDensity = 0.4f;
    StreetFurnitureDensity = 0.3f;
    GreenSpaceChance = 0.2f;
    bIncludeTrafficElements = true;
}

// Countryside PCG Settings
UCountrysidePCGSettings::UCountrysidePCGSettings()
{
    BiomeType = EBiomeType::Countryside;
    FarmDensity = 0.3f;
    CropFieldDensity = 0.5f;
    FenceDensity = 0.4f;
    VillageChance = 0.1f;
    AnimalDensity = 0.2f;
}

// Mountain PCG Settings
UMountainPCGSettings::UMountainPCGSettings()
{
    BiomeType = EBiomeType::Mountains;
    RockFormationDensity = 0.7f;
    CliffChance = 0.3f;
    AlpineVegetationDensity = 0.4f;
    SnowCoverage = 0.2f;
    CaveEntranceChance = 0.05f;
    ElevationVariation = 1.5f;
}

// Wetlands PCG Settings
UWetlandsPCGSettings::UWetlandsPCGSettings()
{
    BiomeType = EBiomeType::Wetlands;
    WaterBodyDensity = 0.6f;
    MarshVegetationDensity = 0.8f;
    BridgeChance = 0.3f;
    WildlifeActivity = 0.7f;
    FogIntensity = 0.5f;
}

// Advanced Biome Generation Element Implementation
FPCGContext* FAdvancedBiomeGenerationElement::Initialize(const FPCGDataCollection& InputData, TWeakObjectPtr<UPCGComponent> SourceComponent, const UPCGNode* Node)
{
    FPCGContext* Context = new FPCGContext();
    Context->InputData = InputData;
    Context->SourceComponent = SourceComponent;
    Context->Node = Node;
    return Context;
}

bool FAdvancedBiomeGenerationElement::ExecuteInternal(FPCGContext* Context) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(FAdvancedBiomeGenerationElement::Execute);
    
    if (!Context)
    {
        return true;
    }
    
    const UBiomePCGSettings* Settings = Context->GetInputSettings<UBiomePCGSettings>();
    if (!Settings)
    {
        return true;
    }
    
    // Create output point data
    UPCGPointData* OutputData = NewObject<UPCGPointData>();
    TArray<FPCGPoint>& OutputPoints = OutputData->GetMutablePoints();
    
    // Generate points based on biome type
    switch (Settings->BiomeType)
    {
        case EBiomeType::Urban:
            if (const UUrbanPCGSettings* UrbanSettings = Cast<UUrbanPCGSettings>(Settings))
            {
                GenerateUrbanLayout(Context, UrbanSettings, OutputPoints);
            }
            break;
            
        case EBiomeType::Countryside:
            if (const UCountrysidePCGSettings* CountrysideSettings = Cast<UCountrysidePCGSettings>(Settings))
            {
                GenerateCountrysideLayout(Context, CountrysideSettings, OutputPoints);
            }
            break;
            
        case EBiomeType::Mountains:
            if (const UMountainPCGSettings* MountainSettings = Cast<UMountainPCGSettings>(Settings))
            {
                GenerateMountainTerrain(Context, MountainSettings, OutputPoints);
            }
            break;
            
        case EBiomeType::Wetlands:
            if (const UWetlandsPCGSettings* WetlandSettings = Cast<UWetlandsPCGSettings>(Settings))
            {
                GenerateWetlandsEcosystem(Context, WetlandSettings, OutputPoints);
            }
            break;
            
        default:
            // Fall back to base implementation for other biomes
            {
                const FBiomeGenerationParams& Params = Settings->GenerationParams;
                FRandomStream LocalRandom(FMath::Rand());
                int32 NumPoints = FMath::RoundToInt(800.0f * Params.VegetationDensity);
                
                for (int32 i = 0; i < NumPoints; i++)
                {
                    FVector Location(
                        LocalRandom.FRandRange(-2000.0f, 2000.0f),
                        LocalRandom.FRandRange(-2000.0f, 2000.0f),
                        0.0f
                    );
                    
                    FRotator Rotation(0.0f, LocalRandom.FRandRange(0.0f, 360.0f), 0.0f);
                    FVector Scale(LocalRandom.FRandRange(0.8f, 1.2f));
                    
                    FPCGPoint Point = CreateBiomePoint(Location, Rotation, Scale, Settings->BiomeType);
                    Point.Density = Params.VegetationDensity;
                    
                    OutputPoints.Add(Point);
                }
            }
            break;
    }
    
    Context->OutputData.TaggedData.Emplace_GetRef().Data = OutputData;
    return true;
}

void FAdvancedBiomeGenerationElement::GenerateUrbanLayout(FPCGContext* Context, const UUrbanPCGSettings* Settings, TArray<FPCGPoint>& OutPoints) const
{
    FRandomStream Random(FMath::Rand());
    const FBiomeGenerationParams& Params = Settings->GenerationParams;
    
    // Generate buildings
    int32 NumBuildings = FMath::RoundToInt(50.0f * Settings->BuildingDensity);
    for (int32 i = 0; i < NumBuildings; i++)
    {
        // Create building plots in a grid-like pattern
        int32 GridX = i % 10;
        int32 GridY = i / 10;
        
        FVector Location(
            GridX * 400.0f + Random.FRandRange(-100.0f, 100.0f) - 2000.0f,
            GridY * 400.0f + Random.FRandRange(-100.0f, 100.0f) - 1000.0f,
            0.0f
        );
        
        // Buildings face the street
        FRotator Rotation(0.0f, (GridX % 2) * 90.0f, 0.0f);
        FVector Scale(
            Random.FRandRange(0.8f, 1.5f),
            Random.FRandRange(0.8f, 1.5f),
            Random.FRandRange(1.0f, 3.0f) // Variable height
        );
        
        FPCGPoint BuildingPoint = CreateBiomePoint(Location, Rotation, Scale, EBiomeType::Urban, 0);
        BuildingPoint.Density = Settings->BuildingDensity;
        ApplyBiomeAttributes(BuildingPoint, EBiomeType::Urban, TEXT("Building"));
        
        OutPoints.Add(BuildingPoint);
    }
    
    // Generate street furniture
    int32 NumStreetFurniture = FMath::RoundToInt(200.0f * Settings->StreetFurnitureDensity);
    for (int32 i = 0; i < NumStreetFurniture; i++)
    {
        FVector Location(
            Random.FRandRange(-2200.0f, 2200.0f),
            Random.FRandRange(-1200.0f, 1200.0f),
            0.0f
        );
        
        FRotator Rotation(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);
        FVector Scale(Random.FRandRange(0.5f, 1.0f));
        
        FPCGPoint FurniturePoint = CreateBiomePoint(Location, Rotation, Scale, EBiomeType::Urban, 1);
        FurniturePoint.Density = Settings->StreetFurnitureDensity;
        ApplyBiomeAttributes(FurniturePoint, EBiomeType::Urban, TEXT("StreetFurniture"));
        
        OutPoints.Add(FurniturePoint);
    }
    
    // Generate green spaces
    if (Random.FRand() < Settings->GreenSpaceChance)
    {
        FVector ParkCenter(Random.FRandRange(-1500.0f, 1500.0f), Random.FRandRange(-800.0f, 800.0f), 0.0f);
        int32 NumTrees = Random.RandRange(10, 30);
        
        for (int32 i = 0; i < NumTrees; i++)
        {
            FVector TreeLocation = ParkCenter + FVector(
                Random.FRandRange(-300.0f, 300.0f),
                Random.FRandRange(-300.0f, 300.0f),
                0.0f
            );
            
            FRotator TreeRotation(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);
            FVector TreeScale(Random.FRandRange(0.8f, 1.5f));
            
            FPCGPoint TreePoint = CreateBiomePoint(TreeLocation, TreeRotation, TreeScale, EBiomeType::Urban, 2);
            TreePoint.Density = Params.VegetationDensity;
            ApplyBiomeAttributes(TreePoint, EBiomeType::Urban, TEXT("ParkTree"));
            
            OutPoints.Add(TreePoint);
        }
    }
    
    // Generate traffic elements if enabled
    if (Settings->bIncludeTrafficElements)
    {
        int32 NumTrafficElements = Random.RandRange(5, 15);
        for (int32 i = 0; i < NumTrafficElements; i++)
        {
            FVector Location(
                Random.FRandRange(-2000.0f, 2000.0f),
                Random.FRandRange(-1000.0f, 1000.0f),
                0.0f
            );
            
            FRotator Rotation(0.0f, Random.RandRange(0, 3) * 90.0f, 0.0f); // Face cardinal directions
            FVector Scale(1.0f);
            
            FPCGPoint TrafficPoint = CreateBiomePoint(Location, Rotation, Scale, EBiomeType::Urban, 3);
            ApplyBiomeAttributes(TrafficPoint, EBiomeType::Urban, TEXT("Traffic"));
            
            OutPoints.Add(TrafficPoint);
        }
    }
}

void FAdvancedBiomeGenerationElement::GenerateCountrysideLayout(FPCGContext* Context, const UCountrysidePCGSettings* Settings, TArray<FPCGPoint>& OutPoints) const
{
    FRandomStream Random(FMath::Rand());
    
    // Generate farms
    int32 NumFarms = FMath::RoundToInt(10.0f * Settings->FarmDensity);
    for (int32 i = 0; i < NumFarms; i++)
    {
        FVector FarmCenter(
            Random.FRandRange(-1800.0f, 1800.0f),
            Random.FRandRange(-1800.0f, 1800.0f),
            0.0f
        );
        
        // Farm buildings
        int32 NumBuildings = Random.RandRange(1, 4);
        for (int32 j = 0; j < NumBuildings; j++)
        {
            FVector BuildingLocation = FarmCenter + FVector(
                Random.FRandRange(-200.0f, 200.0f),
                Random.FRandRange(-200.0f, 200.0f),
                0.0f
            );
            
            FRotator Rotation(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);
            FVector Scale(Random.FRandRange(0.8f, 1.2f));
            
            FPCGPoint BuildingPoint = CreateBiomePoint(BuildingLocation, Rotation, Scale, EBiomeType::Countryside, 0);
            ApplyBiomeAttributes(BuildingPoint, EBiomeType::Countryside, TEXT("FarmBuilding"));
            
            OutPoints.Add(BuildingPoint);
        }
    }
    
    // Generate crop fields
    int32 NumCropFields = FMath::RoundToInt(500.0f * Settings->CropFieldDensity);
    for (int32 i = 0; i < NumCropFields; i++)
    {
        FVector Location(
            Random.FRandRange(-2000.0f, 2000.0f),
            Random.FRandRange(-2000.0f, 2000.0f),
            0.0f
        );
        
        FRotator Rotation(0.0f, Random.RandRange(0, 1) * 90.0f, 0.0f); // Aligned crops
        FVector Scale(Random.FRandRange(0.5f, 1.0f));
        
        FPCGPoint CropPoint = CreateBiomePoint(Location, Rotation, Scale, EBiomeType::Countryside, 1);
        CropPoint.Density = Settings->CropFieldDensity;
        ApplyBiomeAttributes(CropPoint, EBiomeType::Countryside, TEXT("Crops"));
        
        OutPoints.Add(CropPoint);
    }
    
    // Generate fences
    int32 NumFencePosts = FMath::RoundToInt(300.0f * Settings->FenceDensity);
    for (int32 i = 0; i < NumFencePosts; i++)
    {
        FVector Location(
            Random.FRandRange(-2200.0f, 2200.0f),
            Random.FRandRange(-2200.0f, 2200.0f),
            0.0f
        );
        
        FRotator Rotation(0.0f, Random.RandRange(0, 3) * 45.0f, 0.0f);
        FVector Scale(Random.FRandRange(0.8f, 1.0f));
        
        FPCGPoint FencePoint = CreateBiomePoint(Location, Rotation, Scale, EBiomeType::Countryside, 2);
        ApplyBiomeAttributes(FencePoint, EBiomeType::Countryside, TEXT("Fence"));
        
        OutPoints.Add(FencePoint);
    }
    
    // Generate animals
    int32 NumAnimals = FMath::RoundToInt(100.0f * Settings->AnimalDensity);
    for (int32 i = 0; i < NumAnimals; i++)
    {
        FVector Location(
            Random.FRandRange(-1500.0f, 1500.0f),
            Random.FRandRange(-1500.0f, 1500.0f),
            0.0f
        );
        
        FRotator Rotation(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);
        FVector Scale(Random.FRandRange(0.7f, 1.3f));
        
        FPCGPoint AnimalPoint = CreateBiomePoint(Location, Rotation, Scale, EBiomeType::Countryside, 3);
        ApplyBiomeAttributes(AnimalPoint, EBiomeType::Countryside, TEXT("Animal"));
        
        OutPoints.Add(AnimalPoint);
    }
    
    // Generate village if random chance hits
    if (Random.FRand() < Settings->VillageChance)
    {
        FVector VillageCenter(Random.FRandRange(-1000.0f, 1000.0f), Random.FRandRange(-1000.0f, 1000.0f), 0.0f);
        int32 NumHouses = Random.RandRange(3, 8);
        
        for (int32 i = 0; i < NumHouses; i++)
        {
            float Angle = (2.0f * PI * i) / NumHouses;
            FVector HouseLocation = VillageCenter + FVector(
                FMath::Cos(Angle) * 300.0f,
                FMath::Sin(Angle) * 300.0f,
                0.0f
            );
            
            FRotator Rotation(0.0f, FMath::RadiansToDegrees(Angle) + 180.0f, 0.0f); // Face inward
            FVector Scale(Random.FRandRange(0.8f, 1.2f));
            
            FPCGPoint HousePoint = CreateBiomePoint(HouseLocation, Rotation, Scale, EBiomeType::Countryside, 4);
            ApplyBiomeAttributes(HousePoint, EBiomeType::Countryside, TEXT("VillageHouse"));
            
            OutPoints.Add(HousePoint);
        }
    }
}

void FAdvancedBiomeGenerationElement::GenerateMountainTerrain(FPCGContext* Context, const UMountainPCGSettings* Settings, TArray<FPCGPoint>& OutPoints) const
{
    FRandomStream Random(FMath::Rand());
    
    // Generate rock formations
    int32 NumRockFormations = FMath::RoundToInt(400.0f * Settings->RockFormationDensity);
    for (int32 i = 0; i < NumRockFormations; i++)
    {
        FVector Location(
            Random.FRandRange(-2000.0f, 2000.0f),
            Random.FRandRange(-2000.0f, 2000.0f),
            Random.FRandRange(0.0f, 200.0f * Settings->ElevationVariation)
        );
        
        FRotator Rotation(
            Random.FRandRange(-15.0f, 15.0f),
            Random.FRandRange(0.0f, 360.0f),
            Random.FRandRange(-10.0f, 10.0f)
        );
        
        FVector Scale(Random.FRandRange(0.5f, 2.0f * Settings->ElevationVariation));
        
        FPCGPoint RockPoint = CreateBiomePoint(Location, Rotation, Scale, EBiomeType::Mountains, 0);
        RockPoint.Density = Settings->RockFormationDensity;
        ApplyBiomeAttributes(RockPoint, EBiomeType::Mountains, TEXT("Rock"));
        
        OutPoints.Add(RockPoint);
    }
    
    // Generate cliffs
    if (Random.FRand() < Settings->CliffChance)
    {
        int32 NumCliffSections = Random.RandRange(3, 8);
        for (int32 i = 0; i < NumCliffSections; i++)
        {
            FVector Location(
                Random.FRandRange(-2200.0f, 2200.0f),
                Random.FRandRange(-1000.0f, 1000.0f),
                Random.FRandRange(100.0f, 400.0f)
            );
            
            FRotator Rotation(0.0f, Random.RandRange(0, 3) * 90.0f, 0.0f);
            FVector Scale(Random.FRandRange(1.0f, 2.0f));
            
            FPCGPoint CliffPoint = CreateBiomePoint(Location, Rotation, Scale, EBiomeType::Mountains, 1);
            ApplyBiomeAttributes(CliffPoint, EBiomeType::Mountains, TEXT("Cliff"));
            
            OutPoints.Add(CliffPoint);
        }
    }
    
    // Generate alpine vegetation
    int32 NumAlpinePlants = FMath::RoundToInt(300.0f * Settings->AlpineVegetationDensity);
    for (int32 i = 0; i < NumAlpinePlants; i++)
    {
        FVector Location(
            Random.FRandRange(-1800.0f, 1800.0f),
            Random.FRandRange(-1800.0f, 1800.0f),
            Random.FRandRange(0.0f, 150.0f)
        );
        
        FRotator Rotation(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);
        FVector Scale(Random.FRandRange(0.3f, 0.8f)); // Alpine plants are smaller
        
        FPCGPoint PlantPoint = CreateBiomePoint(Location, Rotation, Scale, EBiomeType::Mountains, 2);
        PlantPoint.Density = Settings->AlpineVegetationDensity;
        ApplyBiomeAttributes(PlantPoint, EBiomeType::Mountains, TEXT("AlpinePlant"));
        
        OutPoints.Add(PlantPoint);
    }
    
    // Generate cave entrances
    if (Random.FRand() < Settings->CaveEntranceChance)
    {
        FVector CaveLocation(
            Random.FRandRange(-1500.0f, 1500.0f),
            Random.FRandRange(-1500.0f, 1500.0f),
            Random.FRandRange(50.0f, 300.0f)
        );
        
        FRotator CaveRotation(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);
        FVector CaveScale(Random.FRandRange(1.2f, 2.0f));
        
        FPCGPoint CavePoint = CreateBiomePoint(CaveLocation, CaveRotation, CaveScale, EBiomeType::Mountains, 3);
        ApplyBiomeAttributes(CavePoint, EBiomeType::Mountains, TEXT("CaveEntrance"));
        
        OutPoints.Add(CavePoint);
    }
}

void FAdvancedBiomeGenerationElement::GenerateWetlandsEcosystem(FPCGContext* Context, const UWetlandsPCGSettings* Settings, TArray<FPCGPoint>& OutPoints) const
{
    FRandomStream Random(FMath::Rand());
    
    // Generate water bodies
    int32 NumWaterBodies = FMath::RoundToInt(20.0f * Settings->WaterBodyDensity);
    for (int32 i = 0; i < NumWaterBodies; i++)
    {
        FVector Location(
            Random.FRandRange(-1800.0f, 1800.0f),
            Random.FRandRange(-1800.0f, 1800.0f),
            Random.FRandRange(-20.0f, 0.0f) // Below ground level
        );
        
        FRotator Rotation(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);
        FVector Scale(Random.FRandRange(2.0f, 5.0f)); // Water bodies are large
        
        FPCGPoint WaterPoint = CreateBiomePoint(Location, Rotation, Scale, EBiomeType::Wetlands, 0);
        WaterPoint.Density = Settings->WaterBodyDensity;
        ApplyBiomeAttributes(WaterPoint, EBiomeType::Wetlands, TEXT("Water"));
        
        OutPoints.Add(WaterPoint);
    }
    
    // Generate marsh vegetation
    int32 NumMarshPlants = FMath::RoundToInt(600.0f * Settings->MarshVegetationDensity);
    for (int32 i = 0; i < NumMarshPlants; i++)
    {
        FVector Location(
            Random.FRandRange(-2000.0f, 2000.0f),
            Random.FRandRange(-2000.0f, 2000.0f),
            Random.FRandRange(-10.0f, 10.0f)
        );
        
        FRotator Rotation(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);
        FVector Scale(Random.FRandRange(0.8f, 1.5f));
        
        FPCGPoint PlantPoint = CreateBiomePoint(Location, Rotation, Scale, EBiomeType::Wetlands, 1);
        PlantPoint.Density = Settings->MarshVegetationDensity;
        ApplyBiomeAttributes(PlantPoint, EBiomeType::Wetlands, TEXT("MarshPlant"));
        
        OutPoints.Add(PlantPoint);
    }
    
    // Generate bridges and boardwalks
    if (Random.FRand() < Settings->BridgeChance)
    {
        int32 NumBridgeSections = Random.RandRange(3, 10);
        FVector StartLocation(Random.FRandRange(-1500.0f, 0.0f), Random.FRandRange(-1000.0f, 1000.0f), 5.0f);
        FVector EndLocation(Random.FRandRange(0.0f, 1500.0f), Random.FRandRange(-1000.0f, 1000.0f), 5.0f);
        
        for (int32 i = 0; i < NumBridgeSections; i++)
        {
            float Alpha = static_cast<float>(i) / (NumBridgeSections - 1);
            FVector Location = FMath::Lerp(StartLocation, EndLocation, Alpha);
            
            FVector Direction = (EndLocation - StartLocation).GetSafeNormal();
            FRotator Rotation = Direction.Rotation();
            FVector Scale(Random.FRandRange(0.8f, 1.2f));
            
            FPCGPoint BridgePoint = CreateBiomePoint(Location, Rotation, Scale, EBiomeType::Wetlands, 2);
            ApplyBiomeAttributes(BridgePoint, EBiomeType::Wetlands, TEXT("Bridge"));
            
            OutPoints.Add(BridgePoint);
        }
    }
    
    // Add wildlife indicators (not actual animals, but signs of wildlife)
    int32 NumWildlifeSigns = FMath::RoundToInt(100.0f * Settings->WildlifeActivity);
    for (int32 i = 0; i < NumWildlifeSigns; i++)
    {
        FVector Location(
            Random.FRandRange(-1500.0f, 1500.0f),
            Random.FRandRange(-1500.0f, 1500.0f),
            Random.FRandRange(-5.0f, 15.0f)
        );
        
        FRotator Rotation(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);
        FVector Scale(Random.FRandRange(0.3f, 0.8f));
        
        FPCGPoint WildlifePoint = CreateBiomePoint(Location, Rotation, Scale, EBiomeType::Wetlands, 3);
        ApplyBiomeAttributes(WildlifePoint, EBiomeType::Wetlands, TEXT("WildlifeSign"));
        
        OutPoints.Add(WildlifePoint);
    }
}

FPCGPoint FAdvancedBiomeGenerationElement::CreateBiomePoint(const FVector& Location, const FRotator& Rotation, const FVector& Scale, EBiomeType BiomeType, int32 MeshIndex) const
{
    FPCGPoint Point;
    Point.Transform.SetLocation(Location);
    Point.Transform.SetRotation(Rotation.Quaternion());
    Point.Transform.SetScale3D(Scale);
    Point.Density = 1.0f;
    Point.Color = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
    
    // Store biome type and mesh index in metadata
    Point.MetadataEntry = MeshIndex; // Simple way to store mesh index
    
    return Point;
}

void FAdvancedBiomeGenerationElement::ApplyBiomeAttributes(FPCGPoint& Point, EBiomeType BiomeType, const FString& ObjectType) const
{
    // Set color based on biome type for identification
    switch (BiomeType)
    {
        case EBiomeType::Urban:
            Point.Color = FVector4(0.5f, 0.5f, 0.7f, 1.0f); // Blue-gray for urban
            break;
        case EBiomeType::Countryside:
            Point.Color = FVector4(0.4f, 0.8f, 0.3f, 1.0f); // Green for countryside
            break;
        case EBiomeType::Mountains:
            Point.Color = FVector4(0.7f, 0.6f, 0.5f, 1.0f); // Brown-gray for mountains
            break;
        case EBiomeType::Wetlands:
            Point.Color = FVector4(0.3f, 0.5f, 0.8f, 1.0f); // Blue for wetlands
            break;
        default:
            Point.Color = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
            break;
    }
}

// Biome Preset Manager Implementation
void UBiomePresetManager::LoadBiomePresets()
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    
    TArray<FAssetData> PresetAssets;
    AssetRegistryModule.Get().GetAssetsByClass(UBiomeGenerationPreset::StaticClass()->GetFName(), PresetAssets);
    
    for (const FAssetData& AssetData : PresetAssets)
    {
        UBiomeGenerationPreset* Preset = Cast<UBiomeGenerationPreset>(AssetData.GetAsset());
        if (Preset)
        {
            BiomePresets.FindOrAdd(Preset->TargetBiome).Add(Preset);
            
            // Set first preset as default for each biome
            if (!DefaultPresets.Contains(Preset->TargetBiome))
            {
                DefaultPresets.Add(Preset->TargetBiome, Preset);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Loaded %d biome presets"), PresetAssets.Num());
}

UBiomeGenerationPreset* UBiomePresetManager::GetPresetForBiome(EBiomeType BiomeType)
{
    UBiomeGenerationPreset** DefaultPreset = DefaultPresets.Find(BiomeType);
    return DefaultPreset ? *DefaultPreset : nullptr;
}

TArray<UBiomeGenerationPreset*> UBiomePresetManager::GetPresetsForBiome(EBiomeType BiomeType)
{
    TArray<UBiomeGenerationPreset*>* Presets = BiomePresets.Find(BiomeType);
    return Presets ? *Presets : TArray<UBiomeGenerationPreset*>();
}

UBiomePCGSettings* UBiomePresetManager::CreatePCGSettingsFromPreset(UBiomeGenerationPreset* Preset)
{
    if (!Preset)
    {
        return nullptr;
    }
    
    // Create appropriate PCG settings class based on biome type
    UBiomePCGSettings* Settings = nullptr;
    
    switch (Preset->TargetBiome)
    {
        case EBiomeType::Urban:
            Settings = NewObject<UUrbanPCGSettings>();
            break;
        case EBiomeType::Countryside:
            Settings = NewObject<UCountrysidePCGSettings>();
            break;
        case EBiomeType::Mountains:
            Settings = NewObject<UMountainPCGSettings>();
            break;
        case EBiomeType::Wetlands:
            Settings = NewObject<UWetlandsPCGSettings>();
            break;
        case EBiomeType::Forest:
            Settings = NewObject<UForestPCGSettings>();
            break;
        case EBiomeType::Desert:
            Settings = NewObject<UDesertPCGSettings>();
            break;
        case EBiomeType::Beach:
            Settings = NewObject<UBeachPCGSettings>();
            break;
        default:
            Settings = NewObject<UBiomePCGSettings>();
            break;
    }
    
    if (Settings)
    {
        Settings->BiomeType = Preset->TargetBiome;
        Settings->GenerationParams = Preset->GenerationParams;
    }
    
    return Settings;
}