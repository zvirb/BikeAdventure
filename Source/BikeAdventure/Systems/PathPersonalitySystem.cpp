#include "PathPersonalitySystem.h"
#include "Engine/Engine.h"

UPathPersonalitySystem::UPathPersonalitySystem()
{
    RandomStream.Initialize(FMath::Rand());
}

void UPathPersonalitySystem::Initialize()
{
    InitializeBiomeRules();
    InitializePersonalityCharacteristics();
    InitializeVisualHints();
    
    UE_LOG(LogTemp, Log, TEXT("PathPersonalitySystem initialized"));
}

FPathHints UPathPersonalitySystem::GeneratePathHintsForIntersection(EBiomeType CurrentBiome, EBiomeType LeftPathBiome, EBiomeType RightPathBiome, const FPlayerChoiceHistory& PlayerHistory)
{
    FPathHints Hints;
    
    // Determine personalities for each path
    EPathPersonality LeftPersonality = DeterminePathPersonality(CurrentBiome, LeftPathBiome, true, PlayerHistory);
    EPathPersonality RightPersonality = DeterminePathPersonality(CurrentBiome, RightPathBiome, false, PlayerHistory);
    
    // Set path personalities
    Hints.LeftPathPersonality = LeftPersonality;
    Hints.RightPathPersonality = RightPersonality;
    
    // Generate path characteristics
    FPathCharacteristics LeftCharacteristics = GeneratePathCharacteristics(LeftPersonality, LeftPathBiome, true);
    FPathCharacteristics RightCharacteristics = GeneratePathCharacteristics(RightPersonality, RightPathBiome, false);
    
    // Set hint factors based on characteristics
    Hints.LeftPathChallengeFactor = LeftCharacteristics.DifficultyLevel;
    Hints.RightPathSceneryFactor = RightCharacteristics.SceneryRating;
    
    // Calculate hint subtlety
    Hints.HintSubtlety = CalculateHintSubtlety(CurrentBiome, PlayerHistory);
    
    // Get visual hints for each personality
    FPathVisualHints LeftVisualHints = GetVisualHintsForPersonality(LeftPersonality, LeftPathBiome, Hints.HintSubtlety);
    FPathVisualHints RightVisualHints = GetVisualHintsForPersonality(RightPersonality, RightPathBiome, Hints.HintSubtlety);
    
    // Combine visual hint elements (simplified for base implementation)
    // In a full implementation, these would be populated with actual assets
    Hints.LeftPathVisualHints.Empty();
    Hints.RightPathVisualHints.Empty();
    
    // Broadcast event
    OnPathPersonalityGeneratedEvent.Broadcast(LeftPersonality, RightPersonality, Hints.HintSubtlety);
    
    UE_LOG(LogTemp, Log, TEXT("Generated path hints: Left=%s, Right=%s, Subtlety=%.2f"), 
           *UEnum::GetValueAsString(LeftPersonality), 
           *UEnum::GetValueAsString(RightPersonality), 
           Hints.HintSubtlety);
    
    return Hints;
}

FPathCharacteristics UPathPersonalitySystem::GeneratePathCharacteristics(EPathPersonality Personality, EBiomeType BiomeType, bool bIsLeftPath)
{
    FPathCharacteristics Characteristics;
    
    // Get default characteristics for the personality
    FPathCharacteristics* DefaultCharacteristics = DefaultPathCharacteristics.Find(Personality);
    if (DefaultCharacteristics)
    {
        Characteristics = *DefaultCharacteristics;
    }
    
    // Apply biome-specific modifiers
    ApplyBiomeModifiers(Characteristics, BiomeType);
    
    // Apply left/right path tendencies
    if (bIsLeftPath)
    {
        // Left paths tend to be more challenging and wild
        Characteristics.DifficultyLevel = FMath::Clamp(Characteristics.DifficultyLevel + 0.1f, 0.0f, 1.0f);
        Characteristics.WildlifeEncounterRate = FMath::Clamp(Characteristics.WildlifeEncounterRate + 0.15f, 0.0f, 1.0f);
        Characteristics.PathWidth *= 0.9f; // Slightly narrower
        Characteristics.Windiness = FMath::Clamp(Characteristics.Windiness + 0.1f, 0.0f, 1.0f);
    }
    else
    {
        // Right paths tend to be more scenic and safe
        Characteristics.SceneryRating = FMath::Clamp(Characteristics.SceneryRating + 0.15f, 0.0f, 1.0f);
        Characteristics.DifficultyLevel = FMath::Clamp(Characteristics.DifficultyLevel - 0.1f, 0.0f, 1.0f);
        Characteristics.PathWidth *= 1.1f; // Slightly wider
        Characteristics.WeatherResistance = FMath::Clamp(Characteristics.WeatherResistance + 0.1f, 0.0f, 1.0f);
    }
    
    // Add some randomization to avoid predictability
    float RandomFactor = RandomStream.FRandRange(0.9f, 1.1f);
    Characteristics.DifficultyLevel *= RandomFactor;
    Characteristics.SceneryRating *= RandomFactor;
    Characteristics.WildlifeEncounterRate *= RandomFactor;
    Characteristics.DiscoveryProbability *= RandomFactor;
    
    // Clamp all values to valid ranges
    Characteristics.DifficultyLevel = FMath::Clamp(Characteristics.DifficultyLevel, 0.0f, 1.0f);
    Characteristics.SceneryRating = FMath::Clamp(Characteristics.SceneryRating, 0.0f, 1.0f);
    Characteristics.WildlifeEncounterRate = FMath::Clamp(Characteristics.WildlifeEncounterRate, 0.0f, 1.0f);
    Characteristics.DiscoveryProbability = FMath::Clamp(Characteristics.DiscoveryProbability, 0.0f, 1.0f);
    Characteristics.WeatherResistance = FMath::Clamp(Characteristics.WeatherResistance, 0.0f, 1.0f);
    
    return Characteristics;
}

EPathPersonality UPathPersonalitySystem::DeterminePathPersonality(EBiomeType FromBiome, EBiomeType ToBiome, bool bIsLeftPath, const FPlayerChoiceHistory& PlayerHistory)
{
    // Get generation rules for the target biome
    FPathGenerationRules* Rules = BiomeGenerationRules.Find(ToBiome);
    if (!Rules)
    {
        return EPathPersonality::Peaceful; // Default fallback
    }
    
    // Calculate personality weights based on player history and biome rules
    TMap<EPathPersonality, float> PersonalityWeights = CalculatePersonalityWeights(ToBiome, PlayerHistory);
    
    // Apply left/right bias from rules
    float BiasMultiplier = bIsLeftPath ? Rules->LeftPathBias : Rules->RightPathBias;
    
    // Adjust weights based on path side tendencies
    for (auto& WeightPair : PersonalityWeights)
    {
        EPathPersonality Personality = WeightPair.Key;
        float& Weight = WeightPair.Value;
        
        switch (Personality)
        {
            case EPathPersonality::Wild:
            case EPathPersonality::Challenge:
            case EPathPersonality::Mystery:
                // These personalities favor left paths
                Weight *= bIsLeftPath ? (1.0f + BiasMultiplier) : (1.0f - BiasMultiplier * 0.5f);
                break;
                
            case EPathPersonality::Safe:
            case EPathPersonality::Scenic:
            case EPathPersonality::Peaceful:
                // These personalities favor right paths
                Weight *= bIsLeftPath ? (1.0f - BiasMultiplier * 0.5f) : (1.0f + BiasMultiplier);
                break;
                
            default:
                break;
        }
    }
    
    // Apply biome transition context
    switch (ToBiome)
    {
        case EBiomeType::Forest:
        case EBiomeType::Mountains:
        case EBiomeType::Wetlands:
            // Natural biomes favor wild and mystery personalities
            PersonalityWeights[EPathPersonality::Wild] *= 1.3f;
            PersonalityWeights[EPathPersonality::Mystery] *= 1.2f;
            break;
            
        case EBiomeType::Urban:
            // Urban areas favor safe and structured personalities
            PersonalityWeights[EPathPersonality::Safe] *= 1.4f;
            PersonalityWeights[EPathPersonality::Scenic] *= 0.8f;
            break;
            
        case EBiomeType::Countryside:
            // Countryside is balanced and peaceful
            PersonalityWeights[EPathPersonality::Peaceful] *= 1.3f;
            PersonalityWeights[EPathPersonality::Scenic] *= 1.2f;
            break;
            
        case EBiomeType::Beach:
            // Beaches are scenic and peaceful
            PersonalityWeights[EPathPersonality::Scenic] *= 1.4f;
            PersonalityWeights[EPathPersonality::Peaceful] *= 1.2f;
            break;
            
        case EBiomeType::Desert:
            // Deserts can be challenging but also peaceful
            PersonalityWeights[EPathPersonality::Challenge] *= 1.2f;
            PersonalityWeights[EPathPersonality::Peaceful] *= 1.1f;
            break;
            
        default:
            break;
    }
    
    // Select personality based on weighted random selection
    float TotalWeight = 0.0f;
    for (const auto& WeightPair : PersonalityWeights)
    {
        TotalWeight += WeightPair.Value;
    }
    
    if (TotalWeight <= 0.0f)
    {
        return EPathPersonality::Peaceful;
    }
    
    float RandomValue = RandomStream.FRandRange(0.0f, TotalWeight);
    float CurrentWeight = 0.0f;
    
    for (const auto& WeightPair : PersonalityWeights)
    {
        CurrentWeight += WeightPair.Value;
        if (RandomValue <= CurrentWeight)
        {
            return WeightPair.Key;
        }
    }
    
    return EPathPersonality::Peaceful; // Fallback
}

void UPathPersonalitySystem::UpdatePlayerChoiceHistory(FPlayerChoiceHistory& PlayerHistory, bool bChoseLeftPath, EBiomeType BiomeChosen, EPathPersonality PersonalityChosen)
{
    // Update basic statistics
    PlayerHistory.TotalChoices++;
    if (bChoseLeftPath)
    {
        PlayerHistory.LeftChoices++;
    }
    else
    {
        PlayerHistory.RightChoices++;
    }
    
    // Update recent choice history (keep last 10)
    PlayerHistory.RecentChoices.Add(bChoseLeftPath);
    if (PlayerHistory.RecentChoices.Num() > 10)
    {
        PlayerHistory.RecentChoices.RemoveAt(0);
    }
    
    // Update recent biome history
    PlayerHistory.RecentBiomes.Add(BiomeChosen);
    if (PlayerHistory.RecentBiomes.Num() > 10)
    {
        PlayerHistory.RecentBiomes.RemoveAt(0);
    }
    
    // Update recent personality history
    PlayerHistory.RecentPersonalities.Add(PersonalityChosen);
    if (PlayerHistory.RecentPersonalities.Num() > 10)
    {
        PlayerHistory.RecentPersonalities.RemoveAt(0);
    }
    
    // Update personality preferences
    float* CurrentPreference = PlayerHistory.PersonalityPreferences.Find(PersonalityChosen);
    if (CurrentPreference)
    {
        *CurrentPreference = FMath::Clamp(*CurrentPreference + 0.1f, 0.0f, 1.0f);
    }
    else
    {
        PlayerHistory.PersonalityPreferences.Add(PersonalityChosen, 0.1f);
    }
    
    // Decay other personality preferences slightly
    for (auto& PrefPair : PlayerHistory.PersonalityPreferences)
    {
        if (PrefPair.Key != PersonalityChosen)
        {
            PrefPair.Value = FMath::Max(PrefPair.Value * 0.95f, 0.0f);
        }
    }
    
    // Determine preferred personality
    EPathPersonality NewPreferredPersonality = EPathPersonality::None;
    float HighestPreference = 0.0f;
    
    for (const auto& PrefPair : PlayerHistory.PersonalityPreferences)
    {
        if (PrefPair.Value > HighestPreference)
        {
            HighestPreference = PrefPair.Value;
            NewPreferredPersonality = PrefPair.Key;
        }
    }
    
    // Update preferred personality if it changed and confidence is high enough
    if (NewPreferredPersonality != PlayerHistory.PreferredPersonality && HighestPreference > 0.5f)
    {
        PlayerHistory.PreferredPersonality = NewPreferredPersonality;
        OnPlayerPatternDetectedEvent.Broadcast(NewPreferredPersonality, HighestPreference);
        
        UE_LOG(LogTemp, Log, TEXT("Detected player preference for %s personality (confidence: %.2f)"), 
               *UEnum::GetValueAsString(NewPreferredPersonality), HighestPreference);
    }
    
    // Calculate adaptive weight based on choice patterns
    float LeftRatio = PlayerHistory.TotalChoices > 0 ? static_cast<float>(PlayerHistory.LeftChoices) / PlayerHistory.TotalChoices : 0.5f;
    PlayerHistory.AdaptiveWeight = LeftRatio;
}

FPathVisualHints UPathPersonalitySystem::GetVisualHintsForPersonality(EPathPersonality Personality, EBiomeType BiomeType, float HintSubtlety)
{
    FPathVisualHints Hints;
    
    // Get base configuration for this personality
    FPathVisualHints* BaseHints = PersonalityVisualHints.Find(Personality);
    if (BaseHints)
    {
        Hints = *BaseHints;
    }
    
    // Adjust hint intensity based on subtlety
    Hints.HintIntensity = (1.0f - HintSubtlety) * Hints.HintIntensity;
    Hints.ParticleIntensity *= (1.0f - HintSubtlety);
    Hints.LightIntensity *= (1.0f - HintSubtlety);
    Hints.AudioVolume *= (1.0f - HintSubtlety);
    
    // Apply biome-specific adjustments
    switch (BiomeType)
    {
        case EBiomeType::Forest:
        case EBiomeType::Wetlands:
            // Natural environments have more subtle hints
            Hints.HintIntensity *= 0.8f;
            Hints.bUseColorCoding = true;
            Hints.bUseParticleEffects = true;
            Hints.bUseLighting = false; // Natural lighting
            break;
            
        case EBiomeType::Urban:
            // Urban environments can have more obvious hints
            Hints.HintIntensity *= 1.2f;
            Hints.bUseLighting = true;
            Hints.bUseAudio = true;
            break;
            
        case EBiomeType::Desert:
            // Desert has clear visibility, so hints can be more subtle
            Hints.HintIntensity *= 0.7f;
            Hints.bUseColorCoding = true;
            Hints.bUseLighting = false;
            break;
            
        default:
            break;
    }
    
    // Set personality-specific colors
    switch (Personality)
    {
        case EPathPersonality::Wild:
            Hints.ColorTint = FLinearColor::Green;
            break;
        case EPathPersonality::Safe:
            Hints.ColorTint = FLinearColor::Blue;
            break;
        case EPathPersonality::Scenic:
            Hints.ColorTint = FLinearColor(1.0f, 0.8f, 0.4f, 1.0f); // Golden
            break;
        case EPathPersonality::Challenge:
            Hints.ColorTint = FLinearColor::Red;
            break;
        case EPathPersonality::Mystery:
            Hints.ColorTint = FLinearColor(0.6f, 0.4f, 0.9f, 1.0f); // Purple
            break;
        case EPathPersonality::Peaceful:
            Hints.ColorTint = FLinearColor(0.7f, 0.9f, 1.0f, 1.0f); // Light blue
            break;
        default:
            Hints.ColorTint = FLinearColor::White;
            break;
    }
    
    return Hints;
}

FPathGenerationRules UPathPersonalitySystem::GenerateAdaptiveRules(EBiomeType BiomeType, const FPlayerChoiceHistory& PlayerHistory)
{
    FPathGenerationRules Rules;
    
    // Get base rules for the biome
    FPathGenerationRules* BaseRules = BiomeGenerationRules.Find(BiomeType);
    if (BaseRules)
    {
        Rules = *BaseRules;
    }
    
    // Adapt based on player choice patterns
    if (PlayerHistory.TotalChoices > 5) // Only adapt after some choices
    {
        float LeftRatio = static_cast<float>(PlayerHistory.LeftChoices) / PlayerHistory.TotalChoices;
        
        // Adjust biases based on player preferences
        if (LeftRatio > 0.7f)
        {
            // Player strongly prefers left paths
            Rules.LeftPathBias = FMath::Min(Rules.LeftPathBias * 1.2f, 1.0f);
            Rules.RightPathBias = FMath::Max(Rules.RightPathBias * 0.8f, 0.0f);
        }
        else if (LeftRatio < 0.3f)
        {
            // Player strongly prefers right paths
            Rules.LeftPathBias = FMath::Max(Rules.LeftPathBias * 0.8f, 0.0f);
            Rules.RightPathBias = FMath::Min(Rules.RightPathBias * 1.2f, 1.0f);
        }
        
        // Adjust personality weights based on preferences
        for (const auto& PrefPair : PlayerHistory.PersonalityPreferences)
        {
            if (PrefPair.Value > 0.3f)
            {
                Rules.PersonalityWeights[PrefPair.Key] = FMath::Min(Rules.PersonalityWeights[PrefPair.Key] * (1.0f + PrefPair.Value), 2.0f);
            }
        }
    }
    
    return Rules;
}

float UPathPersonalitySystem::CalculateHintSubtlety(EBiomeType BiomeType, const FPlayerChoiceHistory& PlayerHistory)
{
    float BaseSubtlety = 0.6f; // Default subtlety
    
    // Adjust based on biome type
    switch (BiomeType)
    {
        case EBiomeType::Urban:
            BaseSubtlety = 0.3f; // More obvious hints in urban areas
            break;
        case EBiomeType::Forest:
        case EBiomeType::Wetlands:
            BaseSubtlety = 0.8f; // Very subtle in natural areas
            break;
        case EBiomeType::Desert:
        case EBiomeType::Beach:
            BaseSubtlety = 0.5f; // Moderate subtlety in open areas
            break;
        default:
            BaseSubtlety = 0.6f;
            break;
    }
    
    // Adjust based on player experience (more experienced = more subtle)
    if (PlayerHistory.TotalChoices > 20)
    {
        BaseSubtlety = FMath::Min(BaseSubtlety + 0.2f, 0.9f);
    }
    else if (PlayerHistory.TotalChoices < 5)
    {
        BaseSubtlety = FMath::Max(BaseSubtlety - 0.2f, 0.1f);
    }
    
    return BaseSubtlety;
}

void UPathPersonalitySystem::InitializeBiomeRules()
{
    // Initialize rules for each biome type
    for (uint8 BiomeIndex = 0; BiomeIndex < (uint8)EBiomeType::Wetlands + 1; BiomeIndex++)
    {
        EBiomeType BiomeType = (EBiomeType)BiomeIndex;
        FPathGenerationRules Rules;
        
        Rules.BiomeType = BiomeType;
        Rules.LeftPathBias = 0.6f;  // Left paths slightly more challenging
        Rules.RightPathBias = 0.7f; // Right paths slightly more scenic
        Rules.PersonalityConsistency = 0.7f;
        Rules.AdaptationRate = 0.3f;
        Rules.bAllowPersonalityOverride = true;
        Rules.MinimumHintSubtlety = 0.1f;
        Rules.MaximumHintSubtlety = 0.9f;
        
        // Set biome-specific allowed personalities and weights
        switch (BiomeType)
        {
            case EBiomeType::Forest:
                Rules.AllowedPersonalities = {EPathPersonality::Wild, EPathPersonality::Mystery, EPathPersonality::Scenic, EPathPersonality::Peaceful};
                Rules.PersonalityWeights.Add(EPathPersonality::Wild, 1.2f);
                Rules.PersonalityWeights.Add(EPathPersonality::Mystery, 1.1f);
                Rules.PersonalityWeights.Add(EPathPersonality::Scenic, 0.9f);
                Rules.PersonalityWeights.Add(EPathPersonality::Peaceful, 0.8f);
                break;
                
            case EBiomeType::Urban:
                Rules.AllowedPersonalities = {EPathPersonality::Safe, EPathPersonality::Scenic, EPathPersonality::Challenge};
                Rules.PersonalityWeights.Add(EPathPersonality::Safe, 1.3f);
                Rules.PersonalityWeights.Add(EPathPersonality::Scenic, 0.9f);
                Rules.PersonalityWeights.Add(EPathPersonality::Challenge, 0.7f);
                break;
                
            case EBiomeType::Mountains:
                Rules.AllowedPersonalities = {EPathPersonality::Challenge, EPathPersonality::Scenic, EPathPersonality::Wild};
                Rules.PersonalityWeights.Add(EPathPersonality::Challenge, 1.3f);
                Rules.PersonalityWeights.Add(EPathPersonality::Scenic, 1.2f);
                Rules.PersonalityWeights.Add(EPathPersonality::Wild, 1.0f);
                break;
                
            case EBiomeType::Beach:
                Rules.AllowedPersonalities = {EPathPersonality::Scenic, EPathPersonality::Peaceful, EPathPersonality::Safe};
                Rules.PersonalityWeights.Add(EPathPersonality::Scenic, 1.4f);
                Rules.PersonalityWeights.Add(EPathPersonality::Peaceful, 1.2f);
                Rules.PersonalityWeights.Add(EPathPersonality::Safe, 1.0f);
                break;
                
            case EBiomeType::Countryside:
                Rules.AllowedPersonalities = {EPathPersonality::Peaceful, EPathPersonality::Scenic, EPathPersonality::Safe};
                Rules.PersonalityWeights.Add(EPathPersonality::Peaceful, 1.3f);
                Rules.PersonalityWeights.Add(EPathPersonality::Scenic, 1.1f);
                Rules.PersonalityWeights.Add(EPathPersonality::Safe, 1.0f);
                break;
                
            case EBiomeType::Desert:
                Rules.AllowedPersonalities = {EPathPersonality::Challenge, EPathPersonality::Peaceful, EPathPersonality::Mystery};
                Rules.PersonalityWeights.Add(EPathPersonality::Challenge, 1.1f);
                Rules.PersonalityWeights.Add(EPathPersonality::Peaceful, 1.0f);
                Rules.PersonalityWeights.Add(EPathPersonality::Mystery, 0.8f);
                break;
                
            case EBiomeType::Wetlands:
                Rules.AllowedPersonalities = {EPathPersonality::Mystery, EPathPersonality::Wild, EPathPersonality::Scenic};
                Rules.PersonalityWeights.Add(EPathPersonality::Mystery, 1.3f);
                Rules.PersonalityWeights.Add(EPathPersonality::Wild, 1.1f);
                Rules.PersonalityWeights.Add(EPathPersonality::Scenic, 0.9f);
                break;
                
            default:
                Rules.AllowedPersonalities = {EPathPersonality::Peaceful, EPathPersonality::Scenic};
                Rules.PersonalityWeights.Add(EPathPersonality::Peaceful, 1.0f);
                Rules.PersonalityWeights.Add(EPathPersonality::Scenic, 1.0f);
                break;
        }
        
        BiomeGenerationRules.Add(BiomeType, Rules);
    }
}

void UPathPersonalitySystem::InitializePersonalityCharacteristics()
{
    // Wild personality
    FPathCharacteristics WildCharacteristics;
    WildCharacteristics.PathPersonality = EPathPersonality::Wild;
    WildCharacteristics.DifficultyLevel = 0.7f;
    WildCharacteristics.SceneryRating = 0.6f;
    WildCharacteristics.WildlifeEncounterRate = 0.8f;
    WildCharacteristics.DiscoveryProbability = 0.6f;
    WildCharacteristics.PathWidth = 300.0f;
    WildCharacteristics.Windiness = 0.7f;
    WildCharacteristics.SurfaceType = TEXT("Natural");
    WildCharacteristics.WeatherResistance = 0.3f;
    DefaultPathCharacteristics.Add(EPathPersonality::Wild, WildCharacteristics);
    
    // Safe personality
    FPathCharacteristics SafeCharacteristics;
    SafeCharacteristics.PathPersonality = EPathPersonality::Safe;
    SafeCharacteristics.DifficultyLevel = 0.2f;
    SafeCharacteristics.SceneryRating = 0.5f;
    SafeCharacteristics.WildlifeEncounterRate = 0.2f;
    SafeCharacteristics.DiscoveryProbability = 0.3f;
    SafeCharacteristics.PathWidth = 500.0f;
    SafeCharacteristics.Windiness = 0.2f;
    SafeCharacteristics.SurfaceType = TEXT("Maintained");
    SafeCharacteristics.WeatherResistance = 0.8f;
    DefaultPathCharacteristics.Add(EPathPersonality::Safe, SafeCharacteristics);
    
    // Scenic personality
    FPathCharacteristics ScenicCharacteristics;
    ScenicCharacteristics.PathPersonality = EPathPersonality::Scenic;
    ScenicCharacteristics.DifficultyLevel = 0.4f;
    ScenicCharacteristics.SceneryRating = 0.9f;
    ScenicCharacteristics.WildlifeEncounterRate = 0.4f;
    ScenicCharacteristics.DiscoveryProbability = 0.5f;
    ScenicCharacteristics.PathWidth = 400.0f;
    ScenicCharacteristics.Windiness = 0.5f;
    ScenicCharacteristics.SurfaceType = TEXT("Mixed");
    ScenicCharacteristics.WeatherResistance = 0.6f;
    DefaultPathCharacteristics.Add(EPathPersonality::Scenic, ScenicCharacteristics);
    
    // Challenge personality
    FPathCharacteristics ChallengeCharacteristics;
    ChallengeCharacteristics.PathPersonality = EPathPersonality::Challenge;
    ChallengeCharacteristics.DifficultyLevel = 0.9f;
    ChallengeCharacteristics.SceneryRating = 0.7f;
    ChallengeCharacteristics.WildlifeEncounterRate = 0.5f;
    ChallengeCharacteristics.DiscoveryProbability = 0.8f;
    ChallengeCharacteristics.PathWidth = 250.0f;
    ChallengeCharacteristics.Windiness = 0.8f;
    ChallengeCharacteristics.ElevationChange = 50.0f;
    ChallengeCharacteristics.SurfaceType = TEXT("Rough");
    ChallengeCharacteristics.WeatherResistance = 0.2f;
    DefaultPathCharacteristics.Add(EPathPersonality::Challenge, ChallengeCharacteristics);
    
    // Mystery personality
    FPathCharacteristics MysteryCharacteristics;
    MysteryCharacteristics.PathPersonality = EPathPersonality::Mystery;
    MysteryCharacteristics.DifficultyLevel = 0.6f;
    MysteryCharacteristics.SceneryRating = 0.7f;
    MysteryCharacteristics.WildlifeEncounterRate = 0.6f;
    MysteryCharacteristics.DiscoveryProbability = 0.9f;
    MysteryCharacteristics.PathWidth = 350.0f;
    MysteryCharacteristics.Windiness = 0.6f;
    MysteryCharacteristics.SurfaceType = TEXT("Hidden");
    MysteryCharacteristics.LightingCondition = TEXT("Mysterious");
    MysteryCharacteristics.WeatherResistance = 0.4f;
    DefaultPathCharacteristics.Add(EPathPersonality::Mystery, MysteryCharacteristics);
    
    // Peaceful personality
    FPathCharacteristics PeacefulCharacteristics;
    PeacefulCharacteristics.PathPersonality = EPathPersonality::Peaceful;
    PeacefulCharacteristics.DifficultyLevel = 0.3f;
    PeacefulCharacteristics.SceneryRating = 0.8f;
    PeacefulCharacteristics.WildlifeEncounterRate = 0.3f;
    PeacefulCharacteristics.DiscoveryProbability = 0.4f;
    PeacefulCharacteristics.PathWidth = 450.0f;
    PeacefulCharacteristics.Windiness = 0.3f;
    PeacefulCharacteristics.SurfaceType = TEXT("Smooth");
    PeacefulCharacteristics.LightingCondition = TEXT("Serene");
    PeacefulCharacteristics.WeatherResistance = 0.7f;
    DefaultPathCharacteristics.Add(EPathPersonality::Peaceful, PeacefulCharacteristics);
}

void UPathPersonalitySystem::InitializeVisualHints()
{
    // Initialize base visual hint configurations for each personality
    for (uint8 PersonalityIndex = 0; PersonalityIndex < (uint8)EPathPersonality::Peaceful + 1; PersonalityIndex++)
    {
        EPathPersonality Personality = (EPathPersonality)PersonalityIndex;
        FPathVisualHints Hints;
        
        Hints.HintIntensity = 0.6f;
        Hints.bUseColorCoding = true;
        Hints.bUseParticleEffects = true;
        Hints.bUseLighting = false;
        Hints.bUseAudio = false;
        Hints.ParticleIntensity = 0.5f;
        Hints.LightIntensity = 0.3f;
        Hints.AudioVolume = 0.2f;
        
        PersonalityVisualHints.Add(Personality, Hints);
    }
}

TMap<EPathPersonality, float> UPathPersonalitySystem::CalculatePersonalityWeights(EBiomeType BiomeType, const FPlayerChoiceHistory& PlayerHistory)
{
    TMap<EPathPersonality, float> Weights;
    
    // Get base weights from biome rules
    FPathGenerationRules* Rules = BiomeGenerationRules.Find(BiomeType);
    if (Rules)
    {
        Weights = Rules->PersonalityWeights;
    }
    else
    {
        // Default weights
        Weights.Add(EPathPersonality::Peaceful, 1.0f);
        Weights.Add(EPathPersonality::Scenic, 1.0f);
    }
    
    // Apply player preference adjustments
    for (const auto& PrefPair : PlayerHistory.PersonalityPreferences)
    {
        float* Weight = Weights.Find(PrefPair.Key);
        if (Weight)
        {
            *Weight *= (1.0f + PrefPair.Value);
        }
    }
    
    return Weights;
}

void UPathPersonalitySystem::ApplyBiomeModifiers(FPathCharacteristics& Characteristics, EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Forest:
            Characteristics.WildlifeEncounterRate *= 1.3f;
            Characteristics.PathWidth *= 0.9f;
            Characteristics.Windiness *= 1.2f;
            break;
            
        case EBiomeType::Urban:
            Characteristics.DifficultyLevel *= 0.7f;
            Characteristics.WildlifeEncounterRate *= 0.3f;
            Characteristics.PathWidth *= 1.2f;
            Characteristics.WeatherResistance *= 1.3f;
            break;
            
        case EBiomeType::Mountains:
            Characteristics.DifficultyLevel *= 1.3f;
            Characteristics.ElevationChange += 30.0f;
            Characteristics.SceneryRating *= 1.2f;
            break;
            
        case EBiomeType::Beach:
            Characteristics.SceneryRating *= 1.3f;
            Characteristics.PathWidth *= 1.1f;
            Characteristics.WeatherResistance *= 0.8f;
            break;
            
        case EBiomeType::Desert:
            Characteristics.WildlifeEncounterRate *= 0.5f;
            Characteristics.WeatherResistance *= 0.6f;
            Characteristics.Windiness *= 0.7f;
            break;
            
        case EBiomeType::Countryside:
            Characteristics.DifficultyLevel *= 0.8f;
            Characteristics.SceneryRating *= 1.1f;
            Characteristics.WeatherResistance *= 1.1f;
            break;
            
        case EBiomeType::Wetlands:
            Characteristics.WildlifeEncounterRate *= 1.4f;
            Characteristics.DiscoveryProbability *= 1.2f;
            Characteristics.Windiness *= 1.1f;
            break;
            
        default:
            break;
    }
    
    // Clamp all values after modification
    Characteristics.DifficultyLevel = FMath::Clamp(Characteristics.DifficultyLevel, 0.0f, 1.0f);
    Characteristics.SceneryRating = FMath::Clamp(Characteristics.SceneryRating, 0.0f, 1.0f);
    Characteristics.WildlifeEncounterRate = FMath::Clamp(Characteristics.WildlifeEncounterRate, 0.0f, 1.0f);
    Characteristics.DiscoveryProbability = FMath::Clamp(Characteristics.DiscoveryProbability, 0.0f, 1.0f);
    Characteristics.WeatherResistance = FMath::Clamp(Characteristics.WeatherResistance, 0.0f, 1.0f);
    Characteristics.Windiness = FMath::Clamp(Characteristics.Windiness, 0.0f, 1.0f);
    Characteristics.PathWidth = FMath::Clamp(Characteristics.PathWidth, 100.0f, 1000.0f);
}

FPathCharacteristics UPathPersonalitySystem::BlendPathCharacteristics(const FPathCharacteristics& A, const FPathCharacteristics& B, float BlendFactor)
{
    FPathCharacteristics Result;
    
    Result.DifficultyLevel = FMath::Lerp(A.DifficultyLevel, B.DifficultyLevel, BlendFactor);
    Result.SceneryRating = FMath::Lerp(A.SceneryRating, B.SceneryRating, BlendFactor);
    Result.WildlifeEncounterRate = FMath::Lerp(A.WildlifeEncounterRate, B.WildlifeEncounterRate, BlendFactor);
    Result.DiscoveryProbability = FMath::Lerp(A.DiscoveryProbability, B.DiscoveryProbability, BlendFactor);
    Result.PathWidth = FMath::Lerp(A.PathWidth, B.PathWidth, BlendFactor);
    Result.Windiness = FMath::Lerp(A.Windiness, B.Windiness, BlendFactor);
    Result.ElevationChange = FMath::Lerp(A.ElevationChange, B.ElevationChange, BlendFactor);
    Result.WeatherResistance = FMath::Lerp(A.WeatherResistance, B.WeatherResistance, BlendFactor);
    
    // Use the primary personality of the stronger influence
    Result.PathPersonality = (BlendFactor < 0.5f) ? A.PathPersonality : B.PathPersonality;
    Result.SurfaceType = (BlendFactor < 0.5f) ? A.SurfaceType : B.SurfaceType;
    Result.LightingCondition = (BlendFactor < 0.5f) ? A.LightingCondition : B.LightingCondition;
    
    return Result;
}