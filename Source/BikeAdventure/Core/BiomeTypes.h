#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "BiomeTypes.generated.h"

/**
 * Enumeration of all available biome types in BikeAdventure
 * Each biome provides unique visual characteristics and generation rules
 */
UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Beach       UMETA(DisplayName = "Beach"), 
    Desert      UMETA(DisplayName = "Desert"),
    Urban       UMETA(DisplayName = "Urban"),
    Countryside UMETA(DisplayName = "Countryside"),
    Mountains   UMETA(DisplayName = "Mountains"),
    Wetlands    UMETA(DisplayName = "Wetlands"),
    None        UMETA(DisplayName = "None", Hidden)
};

/**
 * Intersection types for different biomes with unique visual styles
 */
UENUM(BlueprintType)
enum class EIntersectionType : uint8
{
    YFork          UMETA(DisplayName = "Y-Fork"),           // Classic Y-shaped forest paths
    TJunction      UMETA(DisplayName = "T-Junction"),      // Suburban T-intersections  
    Bridge         UMETA(DisplayName = "Bridge"),          // Bridge crossings over valleys/rivers
    CaveEntrance   UMETA(DisplayName = "Cave Entrance"),   // Cave/tunnel branching
    Boardwalk      UMETA(DisplayName = "Boardwalk"),       // Beach boardwalk splits
    RockPass       UMETA(DisplayName = "Rock Pass"),       // Mountain rock formation splits
    RiverCrossing  UMETA(DisplayName = "River Crossing"),  // Wetland river crossing splits
    Roundabout     UMETA(DisplayName = "Roundabout"),      // Urban roundabout
    None           UMETA(DisplayName = "None", Hidden)
};

/**
 * Path hint types indicating the personality and characteristics of different paths
 */
UENUM(BlueprintType)
enum class EPathPersonality : uint8
{
    Wild        UMETA(DisplayName = "Wild"),        // Natural, untamed paths
    Safe        UMETA(DisplayName = "Safe"),        // Well-maintained, civilized paths
    Scenic      UMETA(DisplayName = "Scenic"),      // Paths with beautiful views
    Challenge   UMETA(DisplayName = "Challenge"),   // Paths with interesting terrain
    Mystery     UMETA(DisplayName = "Mystery"),     // Paths leading to unknown discoveries
    Peaceful    UMETA(DisplayName = "Peaceful"),    // Calm, meditative paths
    None        UMETA(DisplayName = "None", Hidden)
};

/**
 * Structure containing rules for biome transitions and generation constraints
 */
USTRUCT(BlueprintType)
struct BIKEADVENTURE_API FBiomeTransitionRules
{
    GENERATED_BODY()

    FBiomeTransitionRules()
    {
        MaxConsecutiveSameBiome = 3;
        BaseTransitionProbability = 0.7f;
        ConsecutiveBiomePenalty = 0.3f;
        bAllowImmediateReturn = false;
    }

    // Maximum number of consecutive segments of the same biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition Rules")
    int32 MaxConsecutiveSameBiome;

    // Base probability of transitioning to a new biome type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition Rules", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BaseTransitionProbability;

    // Penalty multiplier applied for each consecutive same biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition Rules", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ConsecutiveBiomePenalty;

    // Whether players can immediately return to the previous biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition Rules")
    bool bAllowImmediateReturn;

    // Valid biomes that this biome can transition to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition Rules")
    TArray<EBiomeType> ValidTransitions;

    // Preferred intersection types for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intersection Rules")
    TArray<EIntersectionType> PreferredIntersectionTypes;
};

/**
 * Structure containing generation parameters for specific biomes
 */
USTRUCT(BlueprintType)
struct BIKEADVENTURE_API FBiomeGenerationParams
{
    GENERATED_BODY()

    FBiomeGenerationParams()
    {
        VegetationDensity = 0.5f;
        RockDensity = 0.3f;
        PathWindiness = 0.5f;
        PathWidth = 400.0f;
        BiomeTransitionLength = 2000.0f;
        DetailObjectDensity = 0.4f;
        WildlifeSpawnRate = 0.2f;
        WeatherEventProbability = 0.15f;
    }

    // Density of vegetation placement (0.0 - 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VegetationDensity;

    // Density of rock and geological features (0.0 - 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RockDensity;

    // How winding/curved the paths should be (0.0 = straight, 1.0 = very winding)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PathWindiness;

    // Width of the main path in Unreal units
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Generation", meta = (ClampMin = "100.0", ClampMax = "1000.0"))
    float PathWidth;

    // Distance over which biome transitions occur
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (ClampMin = "500.0", ClampMax = "5000.0"))
    float BiomeTransitionLength;

    // Density of small detail objects (flowers, bushes, debris, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DetailObjectDensity;

    // Rate at which wildlife encounters spawn
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discovery", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WildlifeSpawnRate;

    // Probability of weather events occurring in this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discovery", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherEventProbability;

    // Static mesh variations for this biome (trees, rocks, structures)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<TSoftObjectPtr<class UStaticMesh>> PrimaryMeshes;

    // Secondary mesh variations for detail and variety
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<TSoftObjectPtr<class UStaticMesh>> SecondaryMeshes;

    // Material variations for terrain and objects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<TSoftObjectPtr<class UMaterialInterface>> BiomeMaterials;

    // Audio ambience for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<class USoundCue> AmbienceSound;

    // Particle systems for environmental effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TArray<TSoftObjectPtr<class UNiagaraSystem>> EnvironmentalEffects;
};

/**
 * Path hints structure for indicating path characteristics
 */
USTRUCT(BlueprintType)
struct BIKEADVENTURE_API FPathHints
{
    GENERATED_BODY()

    FPathHints()
    {
        LeftPathPersonality = EPathPersonality::Wild;
        RightPathPersonality = EPathPersonality::Safe;
        LeftPathChallengeFactor = 0.7f;
        RightPathSceneryFactor = 0.8f;
        HintSubtlety = 0.6f;
    }

    // Personality type for the left path choice
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Personality")
    EPathPersonality LeftPathPersonality;

    // Personality type for the right path choice
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Personality")
    EPathPersonality RightPathPersonality;

    // Challenge/difficulty factor for left path (0.0 = easy, 1.0 = challenging)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Characteristics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LeftPathChallengeFactor;

    // Scenery/beauty factor for right path (0.0 = plain, 1.0 = scenic)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Characteristics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RightPathSceneryFactor;

    // How subtle the hints should be (0.0 = obvious, 1.0 = very subtle)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Characteristics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HintSubtlety;

    // Visual hint elements for left path (lighting, particle effects, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Hints")
    TArray<TSoftObjectPtr<class AActor>> LeftPathVisualHints;

    // Visual hint elements for right path
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Hints")
    TArray<TSoftObjectPtr<class AActor>> RightPathVisualHints;
};

/**
 * Utility class for biome-related functionality
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API UBiomeUtilities : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Get a human-readable name for a biome type
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome Utilities")
    static FString GetBiomeName(EBiomeType BiomeType);

    /**
     * Get the default generation parameters for a specific biome
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome Utilities")
    static FBiomeGenerationParams GetDefaultBiomeParams(EBiomeType BiomeType);

    /**
     * Get the default transition rules for a specific biome
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome Utilities")
    static FBiomeTransitionRules GetDefaultTransitionRules(EBiomeType BiomeType);

    /**
     * Check if two biomes can transition between each other
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome Utilities")
    static bool CanBiomesTransition(EBiomeType FromBiome, EBiomeType ToBiome);

    /**
     * Get a random valid transition biome from the current biome
     */
    UFUNCTION(BlueprintCallable, Category = "Biome Utilities")
    static EBiomeType GetRandomValidTransition(EBiomeType CurrentBiome, const TArray<EBiomeType>& RecentBiomes);

    /**
     * Calculate transition probability based on recent biome history
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome Utilities")
    static float CalculateTransitionProbability(EBiomeType CurrentBiome, EBiomeType TargetBiome, const TArray<EBiomeType>& RecentBiomes);
};