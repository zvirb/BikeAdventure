#include "WorldStreamingManager.h"
#include "BiomeGenerator.h"
#include "../Gameplay/Intersection.h"
#include "Engine/World.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Stats/Stats.h"

void UWorldStreamingManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize default settings
    MaxStreamingDistanceCm = 500000.0f; // 5km
    MaxActiveSections = 9; // 3x3 grid
    SectionSizeCm = 200000.0f; // 2km per section
    MaxMemoryBudgetKB = 4194304; // 4GB in KB
    UnloadTimeThreshold = 30.0f; // 30 seconds
    bEnablePredictiveLoading = true;
    PredictiveLoadingMultiplier = 2.0f;
    
    // Initialize performance metrics
    PerformanceMetrics = FStreamingPerformanceMetrics();
    LastPlayerPosition = FVector::ZeroVector;
    
    // Get biome generator reference
    BiomeGenerator = NewObject<UBiomeGenerator>();
    if (BiomeGenerator)
    {
        BiomeGenerator->Initialize();
    }
    
    UE_LOG(LogTemp, Log, TEXT("WorldStreamingManager initialized with memory budget: %d KB"), MaxMemoryBudgetKB);
}

void UWorldStreamingManager::Deinitialize()
{
    // Cleanup all active sections
    for (auto& SectionPair : ActiveSections)
    {
        UnloadSection(SectionPair.Key);
    }
    
    ActiveSections.Empty();
    PendingLoadSections.Empty();
    PendingUnloadSections.Empty();
    
    Super::Deinitialize();
}

bool UWorldStreamingManager::StreamInBiomeSection(const FVector& PlayerLocation, EBiomeType BiomeType, const FVector& Direction)
{
    FIntVector SectionCoords = WorldToSectionCoordinates(PlayerLocation + Direction * SectionSizeCm);
    
    // Check if section already exists
    if (ActiveSections.Contains(SectionCoords))
    {
        FWorldSection& ExistingSection = ActiveSections[SectionCoords];
        ExistingSection.LastAccessTime = GetWorld()->GetTimeSeconds();
        return true;
    }
    
    // Check memory budget
    if (!IsWithinMemoryBudget())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot stream in section - memory budget exceeded"));
        OnMemoryBudgetExceededEvent.Broadcast(GetTotalMemoryUsageKB());
        return false;
    }
    
    // Check active sections limit
    if (ActiveSections.Num() >= MaxActiveSections)
    {
        // Force cleanup of distant sections
        CleanupDistantSections(PlayerLocation, true);
        
        if (ActiveSections.Num() >= MaxActiveSections)
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot stream in section - active sections limit reached"));
            return false;
        }
    }
    
    // Create new section
    FWorldSection NewSection = CreateWorldSection(SectionCoords, BiomeType);
    ActiveSections.Add(SectionCoords, NewSection);
    
    // Start async loading
    LoadSection(SectionCoords);
    
    UE_LOG(LogTemp, Log, TEXT("Streaming in %s biome section at coordinates (%d, %d, %d)"), 
           *UBiomeUtilities::GetBiomeName(BiomeType), 
           SectionCoords.X, SectionCoords.Y, SectionCoords.Z);
    
    return true;
}

void UWorldStreamingManager::CleanupDistantSections(const FVector& PlayerLocation, bool bForceCleanup)
{
    SCOPE_CYCLE_COUNTER(STAT_WorldStreamingCleanup);
    
    TArray<FIntVector> SectionsToUnload;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (auto& SectionPair : ActiveSections)
    {
        const FIntVector& SectionCoords = SectionPair.Key;
        FWorldSection& Section = SectionPair.Value;
        
        float DistanceToPlayer = FVector::Dist(Section.WorldPosition, PlayerLocation);
        float TimeSinceAccess = CurrentTime - Section.LastAccessTime;
        
        bool bShouldUnload = false;
        
        if (bForceCleanup)
        {
            // Force unload if not in immediate vicinity
            if (DistanceToPlayer > SectionSizeCm * 1.5f)
            {
                bShouldUnload = true;
            }
        }
        else
        {
            // Normal unloading conditions
            if (DistanceToPlayer > MaxStreamingDistanceCm || TimeSinceAccess > UnloadTimeThreshold)
            {
                bShouldUnload = true;
            }
        }
        
        if (bShouldUnload)
        {
            SectionsToUnload.Add(SectionCoords);
        }
    }
    
    // Unload sections
    for (const FIntVector& SectionCoords : SectionsToUnload)
    {
        UnloadSection(SectionCoords);
    }
    
    if (SectionsToUnload.Num() > 0)
    {
        UpdatePerformanceMetrics();
        UE_LOG(LogTemp, Log, TEXT("Cleaned up %d distant sections"), SectionsToUnload.Num());
    }
}

void UWorldStreamingManager::UpdateStreamingForPlayer(const FVector& PlayerLocation, const FVector& PlayerVelocity)
{
    SCOPE_CYCLE_COUNTER(STAT_WorldStreamingUpdate);
    
    // Update last access time for current section
    FIntVector CurrentSectionCoords = WorldToSectionCoordinates(PlayerLocation);
    if (FWorldSection* CurrentSection = ActiveSections.Find(CurrentSectionCoords))
    {
        CurrentSection->LastAccessTime = GetWorld()->GetTimeSeconds();
        CurrentSection->bIsVisible = true;
    }
    
    // Get sections that should be loaded
    TArray<FIntVector> RequiredSections = GetSectionsInRange(PlayerLocation);
    
    // Predictive loading based on velocity
    if (bEnablePredictiveLoading && !PlayerVelocity.IsZero())
    {
        FVector PredictedLocation = PlayerLocation + (PlayerVelocity * PredictiveLoadingMultiplier);
        TArray<FIntVector> PredictedSections = GetSectionsInRange(PredictedLocation);
        
        for (const FIntVector& PredictedSection : PredictedSections)
        {
            RequiredSections.AddUnique(PredictedSection);
        }
    }
    
    // Load missing sections
    for (const FIntVector& SectionCoords : RequiredSections)
    {
        if (!ActiveSections.Contains(SectionCoords))
        {
            EBiomeType BiomeType = DetermineSectionBiome(SectionCoords, PlayerLocation);
            StreamInBiomeSection(SectionCoordinatesToWorld(SectionCoords), BiomeType);
        }
    }
    
    // Update visibility for all sections
    for (auto& SectionPair : ActiveSections)
    {
        FWorldSection& Section = SectionPair.Value;
        float DistanceToPlayer = FVector::Dist(Section.WorldPosition, PlayerLocation);
        
        // Update visibility based on distance
        bool bShouldBeVisible = DistanceToPlayer <= (SectionSizeCm * 1.5f);
        
        if (Section.bIsVisible != bShouldBeVisible)
        {
            Section.bIsVisible = bShouldBeVisible;
            
            if (Section.StreamingLevel)
            {
                Section.StreamingLevel->SetShouldBeVisible(bShouldBeVisible);
            }
        }
    }
    
    // Cleanup distant sections (non-forced)
    CleanupDistantSections(PlayerLocation, false);
    
    // Update metrics
    UpdatePerformanceMetrics();
    
    LastPlayerPosition = PlayerLocation;
}

FWorldSection UWorldStreamingManager::GetSectionAtLocation(const FVector& WorldLocation)
{
    FIntVector SectionCoords = WorldToSectionCoordinates(WorldLocation);
    
    if (FWorldSection* Section = ActiveSections.Find(SectionCoords))
    {
        return *Section;
    }
    
    return FWorldSection();
}

TArray<FWorldSection> UWorldStreamingManager::GetActiveSections()
{
    TArray<FWorldSection> Sections;
    
    for (auto& SectionPair : ActiveSections)
    {
        Sections.Add(SectionPair.Value);
    }
    
    return Sections;
}

FStreamingPerformanceMetrics UWorldStreamingManager::GetPerformanceMetrics()
{
    return PerformanceMetrics;
}

void UWorldStreamingManager::PreloadSections(const FVector& PlayerLocation, const FVector& MovementDirection, int32 PreloadDistance)
{
    if (MovementDirection.IsZero())
    {
        return;
    }
    
    FVector NormalizedDirection = MovementDirection.GetSafeNormal();
    
    for (int32 i = 1; i <= PreloadDistance; i++)
    {
        FVector PreloadLocation = PlayerLocation + (NormalizedDirection * SectionSizeCm * i);
        FIntVector SectionCoords = WorldToSectionCoordinates(PreloadLocation);
        
        if (!ActiveSections.Contains(SectionCoords))
        {
            EBiomeType BiomeType = DetermineSectionBiome(SectionCoords, PlayerLocation);
            StreamInBiomeSection(PreloadLocation, BiomeType);
        }
    }
}

void UWorldStreamingManager::ForceUnloadSection(const FIntVector& SectionCoordinates)
{
    if (ActiveSections.Contains(SectionCoordinates))
    {
        UnloadSection(SectionCoordinates);
        UpdatePerformanceMetrics();
    }
}

int32 UWorldStreamingManager::GetTotalMemoryUsageKB()
{
    int32 TotalUsage = 0;
    
    for (auto& SectionPair : ActiveSections)
    {
        TotalUsage += SectionPair.Value.MemoryUsageKB;
    }
    
    return TotalUsage;
}

bool UWorldStreamingManager::IsWithinMemoryBudget()
{
    return GetTotalMemoryUsageKB() < MaxMemoryBudgetKB;
}

FIntVector UWorldStreamingManager::WorldToSectionCoordinates(const FVector& WorldLocation)
{
    return FIntVector(
        FMath::FloorToInt(WorldLocation.X / SectionSizeCm),
        FMath::FloorToInt(WorldLocation.Y / SectionSizeCm),
        FMath::FloorToInt(WorldLocation.Z / SectionSizeCm)
    );
}

FVector UWorldStreamingManager::SectionCoordinatesToWorld(const FIntVector& SectionCoordinates)
{
    return FVector(
        SectionCoordinates.X * SectionSizeCm + (SectionSizeCm * 0.5f),
        SectionCoordinates.Y * SectionSizeCm + (SectionSizeCm * 0.5f),
        SectionCoordinates.Z * SectionSizeCm + (SectionSizeCm * 0.5f)
    );
}

FWorldSection UWorldStreamingManager::CreateWorldSection(const FIntVector& SectionCoordinates, EBiomeType BiomeType)
{
    FWorldSection NewSection;
    
    NewSection.SectionCoordinates = SectionCoordinates;
    NewSection.BiomeType = BiomeType;
    NewSection.WorldPosition = SectionCoordinatesToWorld(SectionCoordinates);
    
    // Calculate section bounds
    FVector SectionHalfSize(SectionSizeCm * 0.5f);
    NewSection.WorldBounds = FBox(
        NewSection.WorldPosition - SectionHalfSize,
        NewSection.WorldPosition + SectionHalfSize
    );
    
    NewSection.bIsLoaded = false;
    NewSection.bIsVisible = false;
    NewSection.LastAccessTime = GetWorld()->GetTimeSeconds();
    NewSection.MemoryUsageKB = 0;
    NewSection.bHasIntersection = false;
    
    return NewSection;
}

void UWorldStreamingManager::LoadSection(const FIntVector& SectionCoordinates)
{
    FWorldSection* Section = ActiveSections.Find(SectionCoordinates);
    if (!Section || Section->bIsLoaded)
    {
        return;
    }
    
    float LoadStartTime = FPlatformTime::Seconds();
    
    // Create dynamic streaming level
    FString LevelName = FString::Printf(TEXT("BiomeSection_%d_%d_%d"), 
                                       SectionCoordinates.X, 
                                       SectionCoordinates.Y, 
                                       SectionCoordinates.Z);
    
    ULevelStreamingDynamic* StreamingLevel = ULevelStreamingDynamic::LoadLevelInstance(
        GetWorld(),
        LevelName,
        Section->WorldPosition,
        FRotator::ZeroRotator
    );
    
    if (StreamingLevel)
    {
        Section->StreamingLevel = StreamingLevel;
        Section->bIsLoaded = true;
        
        // Bind completion delegate
        StreamingLevel->OnLevelShown.AddDynamic(this, &UWorldStreamingManager::OnSectionLoadCompleted);
        
        // Generate biome content using PCG
        if (BiomeGenerator)
        {
            // Generate the path segment for this section
            FVector PathDirection = (Section->WorldPosition - LastPlayerPosition).GetSafeNormal();
            BiomeGenerator->GeneratePathSegment(Section->WorldPosition, Section->BiomeType, PathDirection);
            
            // Determine if this section should have an intersection
            // For example, every 3rd section or based on some algorithm
            bool bShouldHaveIntersection = (FMath::Abs(SectionCoordinates.X + SectionCoordinates.Y) % 3 == 0);
            
            if (bShouldHaveIntersection)
            {
                // Generate intersection with random left/right biomes
                EBiomeType LeftBiome = UBiomeUtilities::GetRandomValidTransition(Section->BiomeType, TArray<EBiomeType>());
                EBiomeType RightBiome = UBiomeUtilities::GetRandomValidTransition(Section->BiomeType, {LeftBiome});
                
                Section->IntersectionActor = BiomeGenerator->GenerateIntersection(
                    Section->WorldPosition, 
                    Section->BiomeType, 
                    LeftBiome, 
                    RightBiome
                );
                
                Section->bHasIntersection = (Section->IntersectionActor != nullptr);
            }
        }
        
        // Calculate estimated memory usage
        Section->MemoryUsageKB = CalculateSectionMemoryUsage(*Section);
        
        float LoadTime = FPlatformTime::Seconds() - LoadStartTime;
        PerformanceMetrics.StreamingLoadTime = (PerformanceMetrics.StreamingLoadTime + LoadTime) * 0.5f;
        
        UE_LOG(LogTemp, Log, TEXT("Loaded section (%d, %d, %d) with %s biome in %.3fs, using %dKB"), 
               SectionCoordinates.X, SectionCoordinates.Y, SectionCoordinates.Z,
               *UBiomeUtilities::GetBiomeName(Section->BiomeType),
               LoadTime,
               Section->MemoryUsageKB);
        
        // Broadcast event
        OnSectionLoadedEvent.Broadcast(SectionCoordinates, Section->BiomeType);
    }
}

void UWorldStreamingManager::UnloadSection(const FIntVector& SectionCoordinates)
{
    FWorldSection* Section = ActiveSections.Find(SectionCoordinates);
    if (!Section || !Section->bIsLoaded)
    {
        return;
    }
    
    float UnloadStartTime = FPlatformTime::Seconds();
    
    EBiomeType BiomeType = Section->BiomeType;
    
    // Cleanup PCG actors
    for (APCGActor* PCGActor : Section->PCGActors)
    {
        if (IsValid(PCGActor))
        {
            PCGActor->Destroy();
        }
    }
    Section->PCGActors.Empty();
    
    // Cleanup intersection actor
    if (IsValid(Section->IntersectionActor))
    {
        Section->IntersectionActor->Destroy();
        Section->IntersectionActor = nullptr;
    }
    
    // Unload streaming level
    if (Section->StreamingLevel)
    {
        Section->StreamingLevel->OnLevelHidden.AddDynamic(this, &UWorldStreamingManager::OnSectionUnloadCompleted);
        Section->StreamingLevel->SetShouldBeLoaded(false);
        Section->StreamingLevel->SetShouldBeVisible(false);
    }
    
    // Remove from active sections
    ActiveSections.Remove(SectionCoordinates);
    
    float UnloadTime = FPlatformTime::Seconds() - UnloadStartTime;
    PerformanceMetrics.StreamingUnloadTime = (PerformanceMetrics.StreamingUnloadTime + UnloadTime) * 0.5f;
    
    UE_LOG(LogTemp, Log, TEXT("Unloaded section (%d, %d, %d) with %s biome in %.3fs"), 
           SectionCoordinates.X, SectionCoordinates.Y, SectionCoordinates.Z,
           *UBiomeUtilities::GetBiomeName(BiomeType),
           UnloadTime);
    
    // Broadcast event
    OnSectionUnloadedEvent.Broadcast(SectionCoordinates, BiomeType);
}

void UWorldStreamingManager::UpdatePerformanceMetrics()
{
    PerformanceMetrics.TotalMemoryUsageKB = GetTotalMemoryUsageKB();
    PerformanceMetrics.ActiveSections = 0;
    PerformanceMetrics.LoadedSections = ActiveSections.Num();
    
    // Count visible sections
    for (auto& SectionPair : ActiveSections)
    {
        if (SectionPair.Value.bIsVisible)
        {
            PerformanceMetrics.ActiveSections++;
        }
    }
    
    PerformanceMetrics.bWithinMemoryBudget = IsWithinMemoryBudget();
    
    // Estimate frame time impact (simplified)
    PerformanceMetrics.FrameTimeImpactMs = PerformanceMetrics.ActiveSections * 0.1f; // 0.1ms per active section estimate
}

int32 UWorldStreamingManager::CalculateSectionMemoryUsage(const FWorldSection& Section)
{
    int32 BaseMemoryKB = 10240; // 10MB base per section
    
    // Adjust based on biome complexity
    switch (Section.BiomeType)
    {
        case EBiomeType::Forest:
            BaseMemoryKB *= 1.5f; // Forests are memory-intensive due to vegetation
            break;
        case EBiomeType::Urban:
            BaseMemoryKB *= 1.3f; // Urban areas have many objects
            break;
        case EBiomeType::Desert:
            BaseMemoryKB *= 0.7f; // Deserts are relatively simple
            break;
        case EBiomeType::Beach:
            BaseMemoryKB *= 0.8f; // Beaches have fewer objects
            break;
        default:
            break;
    }
    
    // Add memory for intersection if present
    if (Section.bHasIntersection)
    {
        BaseMemoryKB += 2048; // 2MB for intersection
    }
    
    return BaseMemoryKB;
}

TArray<FIntVector> UWorldStreamingManager::GetSectionsInRange(const FVector& PlayerLocation)
{
    TArray<FIntVector> SectionsInRange;
    FIntVector PlayerSectionCoords = WorldToSectionCoordinates(PlayerLocation);
    
    // 3x3 grid around player (adjustable based on MaxActiveSections)
    int32 GridRadius = FMath::FloorToInt(FMath::Sqrt(MaxActiveSections)) / 2;
    
    for (int32 X = -GridRadius; X <= GridRadius; X++)
    {
        for (int32 Y = -GridRadius; Y <= GridRadius; Y++)
        {
            FIntVector SectionCoords = PlayerSectionCoords + FIntVector(X, Y, 0);
            
            // Check if within streaming distance
            FVector SectionWorldPos = SectionCoordinatesToWorld(SectionCoords);
            float Distance = FVector::Dist(SectionWorldPos, PlayerLocation);
            
            if (Distance <= MaxStreamingDistanceCm)
            {
                SectionsInRange.Add(SectionCoords);
            }
        }
    }
    
    return SectionsInRange;
}

EBiomeType UWorldStreamingManager::DetermineSectionBiome(const FIntVector& SectionCoordinates, const FVector& PlayerLocation)
{
    // Find the closest loaded section to determine transition context
    EBiomeType ContextBiome = EBiomeType::Countryside; // Default fallback
    float ClosestDistance = MAX_FLT;
    
    for (auto& SectionPair : ActiveSections)
    {
        float Distance = FVector::Dist(
            SectionCoordinatesToWorld(SectionCoordinates),
            SectionPair.Value.WorldPosition
        );
        
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ContextBiome = SectionPair.Value.BiomeType;
        }
    }
    
    // Use biome generator to determine next biome based on context
    if (BiomeGenerator)
    {
        // Create a simple biome history with the context biome
        TArray<EBiomeType> BiomeHistory = {ContextBiome};
        
        // Determine choice based on section coordinates (pseudo-random)
        bool bLeftChoice = (SectionCoordinates.X + SectionCoordinates.Y) % 2 == 0;
        
        return BiomeGenerator->GenerateNextBiome(ContextBiome, bLeftChoice, BiomeHistory);
    }
    
    return ContextBiome;
}

void UWorldStreamingManager::OnSectionLoadCompleted(ULevelStreamingDynamic* StreamingLevel)
{
    UE_LOG(LogTemp, Log, TEXT("Section load completed: %s"), *StreamingLevel->GetWorldAssetPackageName());
}

void UWorldStreamingManager::OnSectionUnloadCompleted(ULevelStreamingDynamic* StreamingLevel)
{
    UE_LOG(LogTemp, Log, TEXT("Section unload completed: %s"), *StreamingLevel->GetWorldAssetPackageName());
}