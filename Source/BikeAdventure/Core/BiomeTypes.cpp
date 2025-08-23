#include "BiomeTypes.h"
#include "Engine/Engine.h"

FString UBiomeUtilities::GetBiomeName(EBiomeType BiomeType)
{
    switch(BiomeType)
    {
        case EBiomeType::Forest:
            return TEXT("Forest");
        case EBiomeType::Beach:
            return TEXT("Beach");
        case EBiomeType::Desert:
            return TEXT("Desert");
        case EBiomeType::Urban:
            return TEXT("Urban");
        case EBiomeType::Countryside:
            return TEXT("Countryside");
        case EBiomeType::Mountains:
            return TEXT("Mountains");
        case EBiomeType::Wetlands:
            return TEXT("Wetlands");
        default:
            return TEXT("Unknown");
    }
}

FBiomeGenerationParams UBiomeUtilities::GetDefaultBiomeParams(EBiomeType BiomeType)
{
    FBiomeGenerationParams Params;
    
    switch(BiomeType)
    {
        case EBiomeType::Forest:
            Params.VegetationDensity = 0.85f;
            Params.RockDensity = 0.2f;
            Params.PathWindiness = 0.75f;
            Params.PathWidth = 350.0f;
            Params.DetailObjectDensity = 0.8f;
            Params.WildlifeSpawnRate = 0.4f;
            Params.WeatherEventProbability = 0.2f;
            break;
            
        case EBiomeType::Beach:
            Params.VegetationDensity = 0.2f;
            Params.RockDensity = 0.4f;
            Params.PathWindiness = 0.3f;
            Params.PathWidth = 450.0f;
            Params.DetailObjectDensity = 0.3f;
            Params.WildlifeSpawnRate = 0.25f;
            Params.WeatherEventProbability = 0.35f;
            break;
            
        case EBiomeType::Desert:
            Params.VegetationDensity = 0.15f;
            Params.RockDensity = 0.6f;
            Params.PathWindiness = 0.2f;
            Params.PathWidth = 500.0f;
            Params.DetailObjectDensity = 0.2f;
            Params.WildlifeSpawnRate = 0.1f;
            Params.WeatherEventProbability = 0.1f;
            break;
            
        case EBiomeType::Urban:
            Params.VegetationDensity = 0.4f;
            Params.RockDensity = 0.1f;
            Params.PathWindiness = 0.1f;
            Params.PathWidth = 600.0f;
            Params.DetailObjectDensity = 0.9f;
            Params.WildlifeSpawnRate = 0.05f;
            Params.WeatherEventProbability = 0.05f;
            break;
            
        case EBiomeType::Countryside:
            Params.VegetationDensity = 0.6f;
            Params.RockDensity = 0.2f;
            Params.PathWindiness = 0.4f;
            Params.PathWidth = 400.0f;
            Params.DetailObjectDensity = 0.5f;
            Params.WildlifeSpawnRate = 0.3f;
            Params.WeatherEventProbability = 0.15f;
            break;
            
        case EBiomeType::Mountains:
            Params.VegetationDensity = 0.3f;
            Params.RockDensity = 0.8f;
            Params.PathWindiness = 0.6f;
            Params.PathWidth = 300.0f;
            Params.DetailObjectDensity = 0.4f;
            Params.WildlifeSpawnRate = 0.2f;
            Params.WeatherEventProbability = 0.4f;
            break;
            
        case EBiomeType::Wetlands:
            Params.VegetationDensity = 0.7f;
            Params.RockDensity = 0.1f;
            Params.PathWindiness = 0.8f;
            Params.PathWidth = 320.0f;
            Params.DetailObjectDensity = 0.6f;
            Params.WildlifeSpawnRate = 0.5f;
            Params.WeatherEventProbability = 0.3f;
            break;
            
        default:
            // Default parameters already set in constructor
            break;
    }
    
    return Params;
}

FBiomeTransitionRules UBiomeUtilities::GetDefaultTransitionRules(EBiomeType BiomeType)
{
    FBiomeTransitionRules Rules;
    
    switch(BiomeType)
    {
        case EBiomeType::Forest:
            Rules.ValidTransitions = {
                EBiomeType::Mountains, 
                EBiomeType::Countryside, 
                EBiomeType::Wetlands
            };
            Rules.PreferredIntersectionTypes = {
                EIntersectionType::YFork, 
                EIntersectionType::CaveEntrance
            };
            break;
            
        case EBiomeType::Beach:
            Rules.ValidTransitions = {
                EBiomeType::Urban, 
                EBiomeType::Countryside, 
                EBiomeType::Wetlands
            };
            Rules.PreferredIntersectionTypes = {
                EIntersectionType::Boardwalk, 
                EIntersectionType::Bridge
            };
            break;
            
        case EBiomeType::Desert:
            Rules.ValidTransitions = {
                EBiomeType::Mountains, 
                EBiomeType::Urban, 
                EBiomeType::Countryside
            };
            Rules.PreferredIntersectionTypes = {
                EIntersectionType::RockPass, 
                EIntersectionType::YFork
            };
            break;
            
        case EBiomeType::Urban:
            Rules.ValidTransitions = {
                EBiomeType::Beach, 
                EBiomeType::Desert, 
                EBiomeType::Countryside
            };
            Rules.PreferredIntersectionTypes = {
                EIntersectionType::Roundabout, 
                EIntersectionType::TJunction
            };
            break;
            
        case EBiomeType::Countryside:
            Rules.ValidTransitions = {
                EBiomeType::Forest, 
                EBiomeType::Beach, 
                EBiomeType::Desert, 
                EBiomeType::Urban, 
                EBiomeType::Mountains
            };
            Rules.PreferredIntersectionTypes = {
                EIntersectionType::TJunction, 
                EIntersectionType::YFork, 
                EIntersectionType::Bridge
            };
            break;
            
        case EBiomeType::Mountains:
            Rules.ValidTransitions = {
                EBiomeType::Forest, 
                EBiomeType::Desert, 
                EBiomeType::Countryside
            };
            Rules.PreferredIntersectionTypes = {
                EIntersectionType::RockPass, 
                EIntersectionType::Bridge, 
                EIntersectionType::CaveEntrance
            };
            break;
            
        case EBiomeType::Wetlands:
            Rules.ValidTransitions = {
                EBiomeType::Forest, 
                EBiomeType::Beach, 
                EBiomeType::Countryside
            };
            Rules.PreferredIntersectionTypes = {
                EIntersectionType::RiverCrossing, 
                EIntersectionType::Bridge, 
                EIntersectionType::Boardwalk
            };
            break;
            
        default:
            // No valid transitions for unknown biomes
            break;
    }
    
    return Rules;
}

bool UBiomeUtilities::CanBiomesTransition(EBiomeType FromBiome, EBiomeType ToBiome)
{
    if(FromBiome == EBiomeType::None || ToBiome == EBiomeType::None)
    {
        return false;
    }
    
    FBiomeTransitionRules Rules = GetDefaultTransitionRules(FromBiome);
    return Rules.ValidTransitions.Contains(ToBiome);
}

EBiomeType UBiomeUtilities::GetRandomValidTransition(EBiomeType CurrentBiome, const TArray<EBiomeType>& RecentBiomes)
{
    FBiomeTransitionRules Rules = GetDefaultTransitionRules(CurrentBiome);
    TArray<EBiomeType> ValidOptions = Rules.ValidTransitions;
    
    // Remove recently visited biomes to avoid repetition
    if(RecentBiomes.Num() > 0)
    {
        EBiomeType PreviousBiome = RecentBiomes.Last();
        if(!Rules.bAllowImmediateReturn)
        {
            ValidOptions.Remove(PreviousBiome);
        }
        
        // Apply consecutive biome penalty
        int32 ConsecutiveCount = 0;
        for(int32 i = RecentBiomes.Num() - 1; i >= 0 && RecentBiomes[i] == CurrentBiome; i--)
        {
            ConsecutiveCount++;
        }
        
        if(ConsecutiveCount >= Rules.MaxConsecutiveSameBiome)
        {
            ValidOptions.Remove(CurrentBiome);
        }
    }
    
    if(ValidOptions.Num() == 0)
    {
        // Fallback to any valid transition if no options remain
        ValidOptions = Rules.ValidTransitions;
    }
    
    if(ValidOptions.Num() == 0)
    {
        // Ultimate fallback
        return EBiomeType::Countryside;
    }
    
    int32 RandomIndex = FMath::RandRange(0, ValidOptions.Num() - 1);
    return ValidOptions[RandomIndex];
}

float UBiomeUtilities::CalculateTransitionProbability(EBiomeType CurrentBiome, EBiomeType TargetBiome, const TArray<EBiomeType>& RecentBiomes)
{
    if(!CanBiomesTransition(CurrentBiome, TargetBiome))
    {
        return 0.0f;
    }
    
    FBiomeTransitionRules Rules = GetDefaultTransitionRules(CurrentBiome);
    float Probability = Rules.BaseTransitionProbability;
    
    // Calculate consecutive biome penalty
    int32 ConsecutiveCount = 0;
    for(int32 i = RecentBiomes.Num() - 1; i >= 0 && RecentBiomes[i] == CurrentBiome; i--)
    {
        ConsecutiveCount++;
    }
    
    if(TargetBiome == CurrentBiome)
    {
        // Penalize staying in the same biome
        Probability *= FMath::Pow(Rules.ConsecutiveBiomePenalty, ConsecutiveCount);
    }
    else if(RecentBiomes.Num() > 0 && TargetBiome == RecentBiomes.Last() && !Rules.bAllowImmediateReturn)
    {
        // Penalize immediate return to previous biome
        Probability *= 0.1f;
    }
    
    return FMath::Clamp(Probability, 0.0f, 1.0f);
}