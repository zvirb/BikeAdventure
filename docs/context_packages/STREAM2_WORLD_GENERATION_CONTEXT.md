# Stream 2: World Generation & Procedural Systems Context Package
**Target Agent**: implementation_agent_2  
**Token Limit**: 4000 tokens  
**Success Criteria**: Procedural world generation with seamless biome transitions

## Objective
Implement sophisticated procedural world generation system using UE5's PCG Framework for the BikeAdventure exploration game, creating seamless biome transitions and dynamic path generation.

## Core Biome System (7 Biomes)
1. **Forest** → Mountains, Meadows
2. **Beach** → Cliffs, Boardwalks  
3. **Desert** → Canyons, Oases
4. **Urban** → Parks, Industrial
5. **Countryside** → Farms, Villages
6. **Mountains** → Valleys, Peaks
7. **Wetlands** → Rivers, Marshes

## Technical Implementation

### 1. UBiomeGenerator System
```cpp
UCLASS()
class BIKEADVENTURE_API UBiomeGenerator : public UObject
{
    GENERATED_BODY()
public:
    // Generate next biome based on current biome and choice
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EBiomeType GenerateNextBiome(EBiomeType CurrentBiome, bool bLeftChoice);
    
    // Procedural path generation
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GeneratePathSegment(const FVector& StartLocation, EBiomeType BiomeType);
    
private:
    UPROPERTY(EditAnywhere, Category = "Generation Rules")
    TMap<EBiomeType, FBiomeTransitionRules> TransitionRules;
    
    UPROPERTY(EditAnywhere, Category = "Generation Rules")
    int32 MaxSameBiomeCount = 3; // Prevent repetition
};

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Beach       UMETA(DisplayName = "Beach"), 
    Desert      UMETA(DisplayName = "Desert"),
    Urban       UMETA(DisplayName = "Urban"),
    Countryside UMETA(DisplayName = "Countryside"),
    Mountains   UMETA(DisplayName = "Mountains"),
    Wetlands    UMETA(DisplayName = "Wetlands")
};
```

### 2. PCG Integration
```cpp
// PCG-based biome generation
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API UBiomePCGSettings : public UPCGSettings
{
    GENERATED_BODY()
public:
    // PCG node for biome-specific generation
    virtual TArray<FPCGPinProperties> InputPinProperties() const override;
    virtual TArray<FPCGPinProperties> OutputPinProperties() const override;

protected:
    virtual FPCGElementPtr CreateElement() const override;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Biome Settings")
    EBiomeType BiomeType;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Density")
    float VegetationDensity = 0.5f;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Density")
    float RockDensity = 0.3f;
};
```

### 3. Intersection Generation System
```cpp
UCLASS()
class BIKEADVENTURE_API AIntersection : public AActor
{
    GENERATED_BODY()
public:
    AIntersection();
    
    // Intersection types for different biomes
    UPROPERTY(EditAnywhere, Category = "Intersection")
    EIntersectionType IntersectionType;
    
    // Path directions
    UPROPERTY(EditAnywhere, Category = "Paths")
    FVector LeftPathDirection;
    
    UPROPERTY(EditAnywhere, Category = "Paths")  
    FVector RightPathDirection;
    
    // Biome hints for each path
    UPROPERTY(EditAnywhere, Category = "Generation")
    EBiomeType LeftPathBiome;
    
    UPROPERTY(EditAnywhere, Category = "Generation")
    EBiomeType RightPathBiome;

protected:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* TriggerVolume;
    
    UFUNCTION()
    void OnPlayerEnterIntersection(UPrimitiveComponent* OverlappedComp, 
                                 AActor* OtherActor, 
                                 UPrimitiveComponent* OtherComp, 
                                 int32 OtherBodyIndex, 
                                 bool bFromSweep, 
                                 const FHitResult& SweepResult);
};

UENUM()
enum class EIntersectionType : uint8
{
    YFork,          // Classic Y-shaped forest paths
    TJunction,      // Suburban T-intersections
    Bridge,         // Bridge crossings over valleys/rivers
    CaveEntrance,   // Cave/tunnel branching
    Boardwalk       // Beach boardwalk splits
};
```

### 4. World Streaming Architecture
```cpp
// World streaming for seamless exploration
class BIKEADVENTURE_API UWorldStreamingManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    // Stream in new world sections based on player movement
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void StreamInBiomeSection(const FVector& PlayerLocation, EBiomeType BiomeType);
    
    // Cleanup old sections to manage memory
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void CleanupDistantSections(const FVector& PlayerLocation);

private:
    // Active streaming sections
    UPROPERTY()
    TMap<FIntVector, class ULevelStreamingDynamic*> ActiveSections;
    
    // Memory budget management
    UPROPERTY(EditAnywhere, Category = "Performance")
    float MaxStreamingDistanceCm = 500000.0f; // 5km
    
    UPROPERTY(EditAnywhere, Category = "Performance")  
    int32 MaxActiveSections = 9; // 3x3 grid around player
};
```

### 5. Biome-Specific Generation Rules

**Forest Biome**: Dense vegetation, winding paths, wildlife clearings
```cpp
struct FForestGenerationRules
{
    float TreeDensity = 0.8f;
    float PathWindiness = 0.7f; 
    TArray<UStaticMesh*> TreeVariations;
    TArray<UStaticMesh*> UndergrowthMeshes;
};
```

**Desert Biome**: Sparse vegetation, straight paths, rock formations
```cpp 
struct FDesertGenerationRules
{
    float CactusDensity = 0.2f;
    float PathStraightness = 0.9f;
    float RockFormationChance = 0.4f;
    TArray<UStaticMesh*> CactiVariations;
};
```

### 6. Performance Optimization
- **LOD System**: Dynamic mesh LOD based on distance
- **Culling**: Frustum and occlusion culling for dense biomes  
- **Streaming**: Async loading of biome assets
- **Memory Budget**: Target <4GB with aggressive cleanup

### 7. Path Personality System
```cpp
// Subtle hints about path destinations
struct FPathHints
{
    // Left paths tend toward natural/wild
    bool bLeftPathWild = true;
    float LeftPathChallengeFactor = 0.7f;
    
    // Right paths tend toward civilized/safe  
    bool bRightPathSafe = true;
    float RightPathSceneryFactor = 0.8f;
};
```

### 8. Testing Framework
```cpp
IMPLEMENT_AUTOMATION_TEST(FBiomeTransitionTest,
    "BikeAdventure.WorldGen.BiomeTransition",
    EAutomationTestFlags::ApplicationContextMask)

bool FBiomeTransitionTest::RunTest(const FString& Parameters)
{
    // Test no same biome for >3 consecutive choices
    // Test smooth biome transitions (no desert→arctic)
    // Test memory usage within budget
    // Test streaming performance
    return true;
}
```

### 9. Success Criteria
- ✅ 7 distinct biomes generate correctly
- ✅ Smooth transitions without jarring changes
- ✅ No biome repetition >3 consecutive segments
- ✅ Intersection placement follows biome rules
- ✅ World streaming maintains 60+ FPS
- ✅ Memory usage stays <4GB during generation
- ✅ Path generation creates interesting choices

### 10. Integration Dependencies
- **Requires Stream 1**: UE5 foundation and character systems
- **Provides to Stream 3**: World context for discovery placement
- **Coordinates with Stream 4**: Performance testing and validation