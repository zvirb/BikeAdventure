#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "../Core/BiomeTypes.h"
#include "PathPersonalitySystem.generated.h"

class UNiagaraSystem;
class UStaticMesh;
class UMaterialInterface;

/**
 * Detailed path characteristics for generation
 */
USTRUCT(BlueprintType)
struct BIKEADVENTURE_API FPathCharacteristics
{
    GENERATED_BODY()

    FPathCharacteristics()
    {
        PathPersonality = EPathPersonality::None;
        DifficultyLevel = 0.5f;
        SceneryRating = 0.5f;
        WildlifeEncounterRate = 0.3f;
        DiscoveryProbability = 0.2f;
        PathWidth = 400.0f;
        Windiness = 0.5f;
        ElevationChange = 0.0f;
        SurfaceType = TEXT("Natural");
        LightingCondition = TEXT("Natural");
        WeatherResistance = 0.5f;
    }

    // Primary personality type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Personality")
    EPathPersonality PathPersonality;

    // Difficulty factor (0.0 = very easy, 1.0 = very challenging)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Metrics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DifficultyLevel;

    // Scenery quality (0.0 = plain, 1.0 = breathtaking)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Metrics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SceneryRating;

    // Rate of wildlife encounters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Metrics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WildlifeEncounterRate;

    // Probability of finding discoveries
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Metrics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DiscoveryProbability;

    // Physical path width in Unreal units
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Physical", meta = (ClampMin = "100.0", ClampMax = "1000.0"))
    float PathWidth;

    // How winding the path is (0.0 = straight, 1.0 = very winding)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Physical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Windiness;

    // Elevation change along the path (positive = uphill, negative = downhill)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Physical")
    float ElevationChange;

    // Surface material type description
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Physical")
    FString SurfaceType;

    // Lighting conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Environment")
    FString LightingCondition;

    // Resistance to weather effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherResistance;
};

/**
 * Visual hint configuration for paths
 */
USTRUCT(BlueprintType)
struct BIKEADVENTURE_API FPathVisualHints
{
    GENERATED_BODY()

    FPathVisualHints()
    {
        HintIntensity = 0.6f;
        bUseColorCoding = true;
        bUseParticleEffects = true;
        bUseLighting = true;
        bUseAudio = false;
        ColorTint = FLinearColor::White;
        ParticleIntensity = 0.5f;
        LightIntensity = 0.7f;
        AudioVolume = 0.3f;
    }

    // Overall intensity of hints (0.0 = no hints, 1.0 = very obvious)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HintIntensity;

    // Whether to use color coding for path hints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint Methods")
    bool bUseColorCoding;

    // Whether to use particle effects for hints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint Methods")
    bool bUseParticleEffects;

    // Whether to use lighting for hints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint Methods")
    bool bUseLighting;

    // Whether to use audio cues for hints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint Methods")
    bool bUseAudio;

    // Color tint for path hints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Properties")
    FLinearColor ColorTint;

    // Intensity of particle effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ParticleIntensity;

    // Intensity of lighting effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LightIntensity;

    // Volume of audio cues
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AudioVolume;

    // Particle systems for different hint types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TMap<EPathPersonality, TSoftObjectPtr<UNiagaraSystem>> PersonalityParticles;

    // Light components for different hint types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TMap<EPathPersonality, FLinearColor> PersonalityLightColors;

    // Audio cues for different personalities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TMap<EPathPersonality, TSoftObjectPtr<USoundCue>> PersonalityAudioCues;
};

/**
 * Path generation rules based on biome and context
 */
USTRUCT(BlueprintType)
struct BIKEADVENTURE_API FPathGenerationRules
{
    GENERATED_BODY()

    FPathGenerationRules()
    {
        BiomeType = EBiomeType::None;
        LeftPathBias = 0.5f;
        RightPathBias = 0.5f;
        PersonalityConsistency = 0.7f;
        AdaptationRate = 0.3f;
        bAllowPersonalityOverride = true;
        MinimumHintSubtlety = 0.2f;
        MaximumHintSubtlety = 0.9f;
    }

    // Biome context for path generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Context")
    EBiomeType BiomeType;

    // Bias toward left path characteristics (0.0 = never left-biased, 1.0 = always left-biased)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Biases", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LeftPathBias;

    // Bias toward right path characteristics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Biases", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RightPathBias;

    // How consistent personalities should be within a biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Rules", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PersonalityConsistency;

    // How quickly paths adapt to player choices
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Rules", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AdaptationRate;

    // Whether specific personalities can override biome defaults
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Rules")
    bool bAllowPersonalityOverride;

    // Minimum hint subtlety allowed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint Rules", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MinimumHintSubtlety;

    // Maximum hint subtlety allowed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hint Rules", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MaximumHintSubtlety;

    // Default characteristics for left paths in this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Characteristics")
    FPathCharacteristics DefaultLeftPathCharacteristics;

    // Default characteristics for right paths in this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Characteristics")
    FPathCharacteristics DefaultRightPathCharacteristics;

    // Allowed personalities for paths in this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Allowed Personalities")
    TArray<EPathPersonality> AllowedPersonalities;

    // Probability weights for each personality type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality Weights")
    TMap<EPathPersonality, float> PersonalityWeights;
};

/**
 * Player choice history for adaptive path generation
 */
USTRUCT(BlueprintType)
struct BIKEADVENTURE_API FPlayerChoiceHistory
{
    GENERATED_BODY()

    FPlayerChoiceHistory()
    {
        TotalChoices = 0;
        LeftChoices = 0;
        RightChoices = 0;
        PreferredPersonality = EPathPersonality::None;
        AdaptiveWeight = 0.5f;
    }

    // Total number of choices made
    UPROPERTY(BlueprintReadOnly, Category = "Choice Statistics")
    int32 TotalChoices;

    // Number of left path choices
    UPROPERTY(BlueprintReadOnly, Category = "Choice Statistics")
    int32 LeftChoices;

    // Number of right path choices
    UPROPERTY(BlueprintReadOnly, Category = "Choice Statistics")
    int32 RightChoices;

    // Recent choice history (last 10 choices)
    UPROPERTY(BlueprintReadOnly, Category = "Recent History")
    TArray<bool> RecentChoices; // true = left, false = right

    // Recent biome history
    UPROPERTY(BlueprintReadOnly, Category = "Recent History")
    TArray<EBiomeType> RecentBiomes;

    // Recent personality choices
    UPROPERTY(BlueprintReadOnly, Category = "Recent History")
    TArray<EPathPersonality> RecentPersonalities;

    // Player's apparent preferred personality
    UPROPERTY(BlueprintReadOnly, Category = "Player Profile")
    EPathPersonality PreferredPersonality;

    // Adaptive weight for future generation (based on player patterns)
    UPROPERTY(BlueprintReadOnly, Category = "Player Profile")
    float AdaptiveWeight;

    // Personality preference scores
    UPROPERTY(BlueprintReadOnly, Category = "Player Profile")
    TMap<EPathPersonality, float> PersonalityPreferences;
};

/**
 * Path personality system for generating meaningful path choices
 */
UCLASS(BlueprintType, Blueprintable)
class BIKEADVENTURE_API UPathPersonalitySystem : public UObject
{
    GENERATED_BODY()

public:
    UPathPersonalitySystem();

    /**
     * Initialize the path personality system
     */
    UFUNCTION(BlueprintCallable, Category = "Path Personality")
    void Initialize();

    /**
     * Generate path hints for an intersection based on biome and path personalities
     */
    UFUNCTION(BlueprintCallable, Category = "Path Personality")
    FPathHints GeneratePathHintsForIntersection(EBiomeType CurrentBiome, EBiomeType LeftPathBiome, EBiomeType RightPathBiome, const FPlayerChoiceHistory& PlayerHistory);

    /**
     * Generate path characteristics based on personality and biome
     */
    UFUNCTION(BlueprintCallable, Category = "Path Personality")
    FPathCharacteristics GeneratePathCharacteristics(EPathPersonality Personality, EBiomeType BiomeType, bool bIsLeftPath);

    /**
     * Determine path personality based on biome transition and player history
     */
    UFUNCTION(BlueprintCallable, Category = "Path Personality")
    EPathPersonality DeterminePathPersonality(EBiomeType FromBiome, EBiomeType ToBiome, bool bIsLeftPath, const FPlayerChoiceHistory& PlayerHistory);

    /**
     * Update player choice history with new choice
     */
    UFUNCTION(BlueprintCallable, Category = "Path Personality")
    void UpdatePlayerChoiceHistory(UPARAM(ref) FPlayerChoiceHistory& PlayerHistory, bool bChoseLeftPath, EBiomeType BiomeChosen, EPathPersonality PersonalityChosen);

    /**
     * Get visual hints configuration for a specific path personality
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Path Personality")
    FPathVisualHints GetVisualHintsForPersonality(EPathPersonality Personality, EBiomeType BiomeType, float HintSubtlety = 0.6f);

    /**
     * Generate adaptive path generation rules based on player history
     */
    UFUNCTION(BlueprintCallable, Category = "Path Personality")
    FPathGenerationRules GenerateAdaptiveRules(EBiomeType BiomeType, const FPlayerChoiceHistory& PlayerHistory);

    /**
     * Calculate hint subtlety based on biome and player experience
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Path Personality")
    float CalculateHintSubtlety(EBiomeType BiomeType, const FPlayerChoiceHistory& PlayerHistory);

protected:
    // Default path generation rules for each biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Rules")
    TMap<EBiomeType, FPathGenerationRules> BiomeGenerationRules;

    // Visual hint configurations for each personality
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Configuration")
    TMap<EPathPersonality, FPathVisualHints> PersonalityVisualHints;

    // Default path characteristics for each personality
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Characteristics")
    TMap<EPathPersonality, FPathCharacteristics> DefaultPathCharacteristics;

    // Random stream for consistent generation
    UPROPERTY()
    FRandomStream RandomStream;

private:
    /**
     * Initialize default biome generation rules
     */
    void InitializeBiomeRules();

    /**
     * Initialize default personality characteristics
     */
    void InitializePersonalityCharacteristics();

    /**
     * Initialize visual hint configurations
     */
    void InitializeVisualHints();

    /**
     * Calculate personality weights based on player history
     */
    TMap<EPathPersonality, float> CalculatePersonalityWeights(EBiomeType BiomeType, const FPlayerChoiceHistory& PlayerHistory);

    /**
     * Apply biome-specific modifiers to path characteristics
     */
    void ApplyBiomeModifiers(FPathCharacteristics& Characteristics, EBiomeType BiomeType);

    /**
     * Blend two path characteristics based on a factor
     */
    FPathCharacteristics BlendPathCharacteristics(const FPathCharacteristics& A, const FPathCharacteristics& B, float BlendFactor);

public:
    // Delegates for path personality events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPathPersonalityGenerated, EPathPersonality, LeftPersonality, EPathPersonality, RightPersonality, float, HintSubtlety);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerPatternDetected, EPathPersonality, DetectedPreference, float, Confidence);
    
    UPROPERTY(BlueprintAssignable, Category = "Path Events")
    FOnPathPersonalityGenerated OnPathPersonalityGeneratedEvent;
    
    UPROPERTY(BlueprintAssignable, Category = "Path Events")
    FOnPlayerPatternDetected OnPlayerPatternDetectedEvent;
};