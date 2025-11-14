#include "BiomeGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PCGComponent.h"
#include "PCGActor.h"
#include "../Core/BiomeTypes.h"
#include "../Gameplay/Intersection.h"
#include "AdvancedBiomePCGSettings.h"
#include "PerformanceOptimizationSystem.h"
#include "HAL/PlatformTime.h"
#include "DrawDebugHelpers.h"

UBiomePCGSettings::UBiomePCGSettings()
{
    BiomeType = EBiomeType::None;
    GenerationParams = UBiomeUtilities::GetDefaultBiomeParams(EBiomeType::Countryside);
}

FPCGElementPtr UBiomePCGSettings::CreateElement() const
{
    return MakeShared<FAdvancedBiomeGenerationElement>();
}

UBiomeGenerator::UBiomeGenerator()
{
        PrimaryComponentTick.bCanEverTick = false;
        bInitialized = false;
        BiomeSeed = 12345;
        RandomStream = FRandomStream(BiomeSeed);

        // Initialize PCG settings for all biome types
        InitializeBiomePCGSettings();

        // Initialize quality presets
        InitializeQualityPresets();

        // Detect platform and apply appropriate preset
#if PLATFORM_ANDROID || PLATFORM_IOS
        CurrentPlatform = EPlatformType::Mobile;
#elif PLATFORM_XBOXONE || PLATFORM_PS4 || PLATFORM_SWITCH
        CurrentPlatform = EPlatformType::Console;
#else
        CurrentPlatform = EPlatformType::MidRangePC; // Default to mid-range
#endif

        ApplyQualityPreset(CurrentPlatform);
}

void UBiomeGenerator::Initialize()
{
	if (!bInitialized)
	{
		bInitialized = true;
		UE_LOG(LogTemp, Log, TEXT("Biome Generator initialized with seed: %d"), BiomeSeed);
	}
}

void UBiomeGenerator::GenerateBiome(const FVector& Location, int32 BiomeType)
{
        double StartTime = FPlatformTime::Seconds();
        bool bSuccess = false;

        if (!GetWorld())
        {
                UE_LOG(LogTemp, Warning, TEXT("Cannot generate biome: World is null"));
                RecordBiomeGeneration(0.0f, false);
                return;
        }

        // Update adaptive quality if enabled
        if (bUseAdaptiveQuality)
        {
                UpdateAdaptiveQuality();
        }

        EBiomeType Biome = static_cast<EBiomeType>(BiomeType);
        UBiomePCGSettings* Settings = GetBiomePCGSettings(Biome);

        if (!Settings)
        {
                UE_LOG(LogTemp, Warning, TEXT("No PCG settings found for biome type %d"), BiomeType);
                RecordBiomeGeneration(0.0f, false);
                return;
        }

        // Spawn PCG actor for this biome section
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        APCGActor* PCGActor = GetWorld()->SpawnActor<APCGActor>(Location, FRotator::ZeroRotator, SpawnParams);
        if (PCGActor)
        {
                // Get the PCG component and configure it with our biome settings
                if (UPCGComponent* PCGComponent = PCGActor->GetPCGComponent())
                {
                        // Note: In a full implementation with PCG graph assets, you would set the graph here
                        // For now, we configure the component with basic parameters
                        PCGComponent->Seed = BiomeSeed;

                        // The PCG generation would be triggered when a graph is assigned
                        // PCGComponent->GenerateLocal(true);

                        bSuccess = true;
                        GenerationMetrics.TotalPCGActorsSpawned++;

                        UE_LOG(LogTemp, Log, TEXT("Generated biome %s at location %s with PCG actor (Quality: %d)"),
                               *UBiomeUtilities::GetBiomeName(Biome), *Location.ToString(),
                               static_cast<int32>(CurrentQualityLevel));
                }
                else
                {
                        UE_LOG(LogTemp, Warning, TEXT("PCG Actor spawned but has no PCG Component"));
                }
        }
        else
        {
                UE_LOG(LogTemp, Warning, TEXT("Failed to spawn PCG actor for biome %d at location %s"),
                       BiomeType, *Location.ToString());
        }

        double EndTime = FPlatformTime::Seconds();
        float GenerationTimeMs = (EndTime - StartTime) * 1000.0f;
        RecordBiomeGeneration(GenerationTimeMs, bSuccess);
}

EBiomeType UBiomeGenerator::GenerateNextBiome(EBiomeType CurrentBiome, bool bChooseLeftPath, const TArray<EBiomeType>& BiomeHistory)
{
        EBiomeType NextBiome = UBiomeUtilities::GetRandomValidTransition(CurrentBiome, BiomeHistory);

        UE_LOG(LogTemp, Log, TEXT("Transitioning from %s to %s via %s path"),
               *UBiomeUtilities::GetBiomeName(CurrentBiome),
               *UBiomeUtilities::GetBiomeName(NextBiome),
               bChooseLeftPath ? TEXT("left") : TEXT("right"));

        return NextBiome;
}

TArray<APCGActor*> UBiomeGenerator::GeneratePathSegment(const FVector& Location, EBiomeType BiomeType, const FVector& Direction)
{
        TArray<APCGActor*> SpawnedActors;
        double StartTime = FPlatformTime::Seconds();

        if (!GetWorld())
        {
                UE_LOG(LogTemp, Warning, TEXT("Cannot generate path segment: World is null"));
                RecordPathGeneration(0.0f, 0);
                return SpawnedActors;
        }

        UBiomePCGSettings* Settings = GetBiomePCGSettings(BiomeType);
        if (!Settings)
        {
                UE_LOG(LogTemp, Warning, TEXT("No PCG settings found for biome type %s"),
                       *UBiomeUtilities::GetBiomeName(BiomeType));
                RecordPathGeneration(0.0f, 0);
                return SpawnedActors;
        }

        FBiomeGenerationParams Params = Settings->GenerationParams;

        // Calculate segment length based on biome parameters
        float SegmentLength = Params.PathWidth * 50.0f; // Approximately 50 path widths per segment

        // Apply quality multiplier to actor count (with biome-specific override)
        float QualityMultiplier = GetQualityMultiplierForBiome(BiomeType);
        int32 BaseActorCount = FMath::Max(1, FMath::RoundToInt(SegmentLength / 10000.0f)); // One actor per 10km
        int32 NumActors = FMath::Max(1, FMath::RoundToInt(BaseActorCount * QualityMultiplier));

        // Normalize direction
        FVector NormalizedDirection = Direction.GetSafeNormal();
        if (NormalizedDirection.IsNearlyZero())
        {
                NormalizedDirection = FVector::ForwardVector;
        }

        // Spawn PCG actors along the path segment
        for (int32 i = 0; i < NumActors; i++)
        {
                float Offset = (SegmentLength / NumActors) * i;
                FVector SpawnLocation = Location + (NormalizedDirection * Offset);

                // Add some randomness perpendicular to the path for natural variation
                FVector PerpendicularOffset = FVector::CrossProduct(NormalizedDirection, FVector::UpVector);
                PerpendicularOffset *= RandomStream.FRandRange(-Params.PathWidth * 0.5f, Params.PathWidth * 0.5f);
                SpawnLocation += PerpendicularOffset;

                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

                APCGActor* PCGActor = GetWorld()->SpawnActor<APCGActor>(SpawnLocation, FRotator::ZeroRotator, SpawnParams);
                if (PCGActor)
                {
                        // Configure PCG component with biome settings
                        if (UPCGComponent* PCGComponent = PCGActor->GetPCGComponent())
                        {
                                // Set biome-specific generation parameters
                                PCGComponent->Seed = RandomStream.GetCurrentSeed() + i;

                                // Note: In a full implementation with PCG graph assets, you would:
                                // 1. Load or create a PCG graph for this biome type
                                // 2. Assign it to the component
                                // 3. Trigger generation with PCGComponent->GenerateLocal(true)
                        }

                        SpawnedActors.Add(PCGActor);
                        GenerationMetrics.TotalPCGActorsSpawned++;
                }
                else
                {
                        GenerationMetrics.FailedSpawns++;
                }
        }

        UE_LOG(LogTemp, Log, TEXT("Generated path segment for %s biome at %s with %d/%d PCG actors (Quality: %d)"),
               *UBiomeUtilities::GetBiomeName(BiomeType), *Location.ToString(),
               SpawnedActors.Num(), NumActors, static_cast<int32>(CurrentQualityLevel));

        // Draw debug visualization if enabled
        if (bShowDebugVisualization)
        {
                DrawDebugVisualization(Location, BiomeType, SpawnedActors.Num());
        }

        double EndTime = FPlatformTime::Seconds();
        float GenerationTimeMs = (EndTime - StartTime) * 1000.0f;
        RecordPathGeneration(GenerationTimeMs, SpawnedActors.Num());

        return SpawnedActors;
}

AIntersection* UBiomeGenerator::GenerateIntersection(const FVector& Location, EBiomeType CurrentBiome, EBiomeType LeftBiome, EBiomeType RightBiome)
{
        if (!GetWorld())
        {
                return nullptr;
        }

        FBiomeTransitionRules Rules = UBiomeUtilities::GetDefaultTransitionRules(CurrentBiome);
        EIntersectionType Type = EIntersectionType::YFork;
        if (Rules.PreferredIntersectionTypes.Num() > 0)
        {
                int32 Index = FMath::RandRange(0, Rules.PreferredIntersectionTypes.Num() - 1);
                Type = Rules.PreferredIntersectionTypes[Index];
        }

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AIntersection* Intersection = GetWorld()->SpawnActor<AIntersection>(Location, FRotator::ZeroRotator, Params);
        if (Intersection)
        {
                Intersection->SetIntersectionType(Type);
                Intersection->SetPathBiomes(LeftBiome, RightBiome);

                GenerationMetrics.TotalIntersections++;
        }

        UE_LOG(LogTemp, Log, TEXT("Generated intersection at %s leading to %s and %s"),
               *Location.ToString(),
               *UBiomeUtilities::GetBiomeName(LeftBiome),
               *UBiomeUtilities::GetBiomeName(RightBiome));

        return Intersection;
}

void UBiomeGenerator::SetGenerationSeed(int32 Seed)
{
        BiomeSeed = Seed;
        RandomStream = FRandomStream(Seed);

        UE_LOG(LogTemp, Log, TEXT("Biome generation seed set to %d"), Seed);
}

UBiomePCGSettings* UBiomeGenerator::GetBiomePCGSettings(EBiomeType BiomeType)
{
        UBiomePCGSettings** FoundSettings = BiomePCGSettingsMap.Find(BiomeType);

        if (FoundSettings && *FoundSettings)
        {
                return *FoundSettings;
        }

        // Create default settings if not found
        UBiomePCGSettings* DefaultSettings = NewObject<UBiomePCGSettings>(this);
        DefaultSettings->BiomeType = BiomeType;
        DefaultSettings->GenerationParams = UBiomeUtilities::GetDefaultBiomeParams(BiomeType);

        BiomePCGSettingsMap.Add(BiomeType, DefaultSettings);

        return DefaultSettings;
}

void UBiomeGenerator::InitializeBiomePCGSettings()
{
        // Initialize PCG settings for all biome types
        TArray<EBiomeType> AllBiomes = {
                EBiomeType::Forest,
                EBiomeType::Beach,
                EBiomeType::Desert,
                EBiomeType::Urban,
                EBiomeType::Countryside,
                EBiomeType::Mountains,
                EBiomeType::Wetlands
        };

        for (EBiomeType BiomeType : AllBiomes)
        {
                UBiomePCGSettings* Settings = NewObject<UBiomePCGSettings>(this);
                Settings->BiomeType = BiomeType;
                Settings->GenerationParams = UBiomeUtilities::GetDefaultBiomeParams(BiomeType);

                BiomePCGSettingsMap.Add(BiomeType, Settings);
        }
}

void UBiomeGenerator::SetGenerationQuality(EBiomeGenerationQuality Quality)
{
        if (CurrentQualityLevel != Quality)
        {
                CurrentQualityLevel = Quality;
                GenerationMetrics.CurrentQualityLevel = Quality;

                UE_LOG(LogTemp, Log, TEXT("Biome generation quality set to: %d"), static_cast<int32>(Quality));
        }
}

void UBiomeGenerator::ResetMetrics()
{
        GenerationMetrics = FBiomeGenerationMetrics();
        GenerationMetrics.CurrentQualityLevel = CurrentQualityLevel;
        TotalBiomeGenerationTime = 0.0f;
        TotalPathGenerationTime = 0.0f;

        UE_LOG(LogTemp, Log, TEXT("Biome generation metrics reset"));
}

void UBiomeGenerator::SetAdaptiveQuality(bool bEnable)
{
        bUseAdaptiveQuality = bEnable;

        UE_LOG(LogTemp, Log, TEXT("Adaptive quality %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

float UBiomeGenerator::GetQualityMultiplier() const
{
        switch (CurrentQualityLevel)
        {
                case EBiomeGenerationQuality::Low:
                        return 0.5f;  // 50% of actors
                case EBiomeGenerationQuality::Medium:
                        return 0.75f; // 75% of actors
                case EBiomeGenerationQuality::High:
                        return 1.0f;  // 100% of actors (baseline)
                case EBiomeGenerationQuality::Ultra:
                        return 1.5f;  // 150% of actors
                default:
                        return 1.0f;
        }
}

void UBiomeGenerator::UpdateAdaptiveQuality()
{
        // Get performance optimization system to check current performance
        if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
        {
                if (UPerformanceOptimizationSystem* PerfSystem = GameInstance->GetSubsystem<UPerformanceOptimizationSystem>())
                {
                        FPerformanceMetrics PerfMetrics = PerfSystem->GetCurrentMetrics();

                        // Adjust quality based on performance, respecting preset limits
                        if (!PerfMetrics.bWithinPerformanceTarget)
                        {
                                // Performance is poor, reduce quality
                                if (CurrentQualityLevel > MinAllowedQuality)
                                {
                                        EBiomeGenerationQuality NewQuality = static_cast<EBiomeGenerationQuality>(
                                                FMath::Max(static_cast<int32>(CurrentQualityLevel) - 1,
                                                          static_cast<int32>(MinAllowedQuality)));
                                        SetGenerationQuality(NewQuality);
                                        GenerationMetrics.QualityAdjustments++;

                                        UE_LOG(LogTemp, Warning, TEXT("Reduced biome generation quality due to performance"));
                                }
                        }
                        else if (PerfMetrics.FrameTimeMs < 13.0f && PerfMetrics.CPUUsagePercent < 70.0f)
                        {
                                // Performance is very good, can increase quality
                                if (CurrentQualityLevel < MaxAllowedQuality)
                                {
                                        EBiomeGenerationQuality NewQuality = static_cast<EBiomeGenerationQuality>(
                                                FMath::Min(static_cast<int32>(CurrentQualityLevel) + 1,
                                                          static_cast<int32>(MaxAllowedQuality)));
                                        SetGenerationQuality(NewQuality);
                                        GenerationMetrics.QualityAdjustments++;

                                        UE_LOG(LogTemp, Log, TEXT("Increased biome generation quality due to good performance"));
                                }
                        }
                }
        }
}

void UBiomeGenerator::RecordBiomeGeneration(float GenerationTimeMs, bool bSuccess)
{
        GenerationMetrics.TotalBiomesGenerated++;

        if (bSuccess)
        {
                TotalBiomeGenerationTime += GenerationTimeMs;
                GenerationMetrics.AverageBiomeGenerationTimeMs =
                        TotalBiomeGenerationTime / FMath::Max(1, GenerationMetrics.TotalBiomesGenerated);
        }
        else
        {
                GenerationMetrics.FailedSpawns++;
        }
}

void UBiomeGenerator::RecordPathGeneration(float GenerationTimeMs, int32 ActorsSpawned)
{
        GenerationMetrics.TotalPathSegments++;

        if (ActorsSpawned > 0)
        {
                TotalPathGenerationTime += GenerationTimeMs;
                GenerationMetrics.AveragePathGenerationTimeMs =
                        TotalPathGenerationTime / FMath::Max(1, GenerationMetrics.TotalPathSegments);
        }
}

void UBiomeGenerator::InitializeQualityPresets()
{
        // Low-End PC Preset
        FQualityPreset LowEndPC;
        LowEndPC.Platform = EPlatformType::LowEndPC;
        LowEndPC.DefaultQuality = EBiomeGenerationQuality::Low;
        LowEndPC.bEnableAdaptiveQuality = true;
        LowEndPC.MaxQuality = EBiomeGenerationQuality::Medium;
        LowEndPC.MinQuality = EBiomeGenerationQuality::Low;
        QualityPresets.Add(EPlatformType::LowEndPC, LowEndPC);

        // Mid-Range PC Preset
        FQualityPreset MidRangePC;
        MidRangePC.Platform = EPlatformType::MidRangePC;
        MidRangePC.DefaultQuality = EBiomeGenerationQuality::Medium;
        MidRangePC.bEnableAdaptiveQuality = true;
        MidRangePC.MaxQuality = EBiomeGenerationQuality::High;
        MidRangePC.MinQuality = EBiomeGenerationQuality::Low;
        QualityPresets.Add(EPlatformType::MidRangePC, MidRangePC);

        // High-End PC Preset
        FQualityPreset HighEndPC;
        HighEndPC.Platform = EPlatformType::HighEndPC;
        HighEndPC.DefaultQuality = EBiomeGenerationQuality::High;
        HighEndPC.bEnableAdaptiveQuality = true;
        HighEndPC.MaxQuality = EBiomeGenerationQuality::Ultra;
        HighEndPC.MinQuality = EBiomeGenerationQuality::Medium;
        QualityPresets.Add(EPlatformType::HighEndPC, HighEndPC);

        // Console Preset
        FQualityPreset Console;
        Console.Platform = EPlatformType::Console;
        Console.DefaultQuality = EBiomeGenerationQuality::Medium;
        Console.bEnableAdaptiveQuality = true;
        Console.MaxQuality = EBiomeGenerationQuality::High;
        Console.MinQuality = EBiomeGenerationQuality::Medium;
        QualityPresets.Add(EPlatformType::Console, Console);

        // Mobile Preset
        FQualityPreset Mobile;
        Mobile.Platform = EPlatformType::Mobile;
        Mobile.DefaultQuality = EBiomeGenerationQuality::Low;
        Mobile.bEnableAdaptiveQuality = false; // Keep stable on mobile
        Mobile.MaxQuality = EBiomeGenerationQuality::Low;
        Mobile.MinQuality = EBiomeGenerationQuality::Low;
        QualityPresets.Add(EPlatformType::Mobile, Mobile);
}

void UBiomeGenerator::ApplyQualityPreset(EPlatformType Platform)
{
        const FQualityPreset* Preset = QualityPresets.Find(Platform);
        if (Preset)
        {
                CurrentPlatform = Platform;
                SetGenerationQuality(Preset->DefaultQuality);
                SetAdaptiveQuality(Preset->bEnableAdaptiveQuality);
                MaxAllowedQuality = Preset->MaxQuality;
                MinAllowedQuality = Preset->MinQuality;

                UE_LOG(LogTemp, Log, TEXT("Applied %s quality preset: Quality=%d, Adaptive=%s"),
                       *UEnum::GetValueAsString(Platform),
                       static_cast<int32>(Preset->DefaultQuality),
                       Preset->bEnableAdaptiveQuality ? TEXT("On") : TEXT("Off"));
        }
}

FQualityPreset UBiomeGenerator::GetQualityPreset(EPlatformType Platform) const
{
        const FQualityPreset* Preset = QualityPresets.Find(Platform);
        if (Preset)
        {
                return *Preset;
        }

        // Return mid-range as default
        FQualityPreset Default;
        Default.Platform = Platform;
        Default.DefaultQuality = EBiomeGenerationQuality::Medium;
        return Default;
}

void UBiomeGenerator::SetBiomeQualityMultiplier(EBiomeType BiomeType, float Multiplier, bool bEnable)
{
        FBiomeQualityMultiplier BiomeMultiplier;
        BiomeMultiplier.BiomeType = BiomeType;
        BiomeMultiplier.QualityMultiplier = FMath::Clamp(Multiplier, 0.1f, 2.0f);
        BiomeMultiplier.bEnabled = bEnable;

        BiomeQualityMultipliers.Add(BiomeType, BiomeMultiplier);

        UE_LOG(LogTemp, Log, TEXT("Set %s biome quality multiplier to %.2fx (%s)"),
               *UBiomeUtilities::GetBiomeName(BiomeType),
               BiomeMultiplier.QualityMultiplier,
               bEnable ? TEXT("enabled") : TEXT("disabled"));
}

float UBiomeGenerator::GetBiomeQualityMultiplier(EBiomeType BiomeType) const
{
        const FBiomeQualityMultiplier* BiomeMultiplier = BiomeQualityMultipliers.Find(BiomeType);
        if (BiomeMultiplier && BiomeMultiplier->bEnabled)
        {
                return BiomeMultiplier->QualityMultiplier;
        }
        return 1.0f; // Default multiplier
}

float UBiomeGenerator::GetQualityMultiplierForBiome(EBiomeType BiomeType) const
{
        float BaseMultiplier = GetQualityMultiplier();
        float BiomeMultiplier = GetBiomeQualityMultiplier(BiomeType);
        return BaseMultiplier * BiomeMultiplier;
}

FString UBiomeGenerator::ExportMetricsToJSON() const
{
        FString JSON = TEXT("{\n");
        JSON += FString::Printf(TEXT("  \"TotalBiomesGenerated\": %d,\n"), GenerationMetrics.TotalBiomesGenerated);
        JSON += FString::Printf(TEXT("  \"TotalPCGActorsSpawned\": %d,\n"), GenerationMetrics.TotalPCGActorsSpawned);
        JSON += FString::Printf(TEXT("  \"TotalPathSegments\": %d,\n"), GenerationMetrics.TotalPathSegments);
        JSON += FString::Printf(TEXT("  \"TotalIntersections\": %d,\n"), GenerationMetrics.TotalIntersections);
        JSON += FString::Printf(TEXT("  \"AverageBiomeGenerationTimeMs\": %.2f,\n"), GenerationMetrics.AverageBiomeGenerationTimeMs);
        JSON += FString::Printf(TEXT("  \"AveragePathGenerationTimeMs\": %.2f,\n"), GenerationMetrics.AveragePathGenerationTimeMs);
        JSON += FString::Printf(TEXT("  \"FailedSpawns\": %d,\n"), GenerationMetrics.FailedSpawns);
        JSON += FString::Printf(TEXT("  \"CurrentQualityLevel\": %d,\n"), static_cast<int32>(GenerationMetrics.CurrentQualityLevel));
        JSON += FString::Printf(TEXT("  \"QualityAdjustments\": %d,\n"), GenerationMetrics.QualityAdjustments);
        JSON += FString::Printf(TEXT("  \"Platform\": \"%s\",\n"), *UEnum::GetValueAsString(CurrentPlatform));
        JSON += FString::Printf(TEXT("  \"AdaptiveQualityEnabled\": %s\n"), bUseAdaptiveQuality ? TEXT("true") : TEXT("false"));
        JSON += TEXT("}");

        return JSON;
}

void UBiomeGenerator::SetDebugVisualization(bool bEnable)
{
        bShowDebugVisualization = bEnable;
        UE_LOG(LogTemp, Log, TEXT("Debug visualization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UBiomeGenerator::DrawDebugVisualization(const FVector& Location, EBiomeType BiomeType, int32 ActorCount) const
{
        if (!GetWorld())
        {
                return;
        }

        // Draw biome color-coded sphere at generation location
        FColor BiomeColor = FColor::White;
        switch (BiomeType)
        {
                case EBiomeType::Forest:     BiomeColor = FColor::Green; break;
                case EBiomeType::Beach:      BiomeColor = FColor::Yellow; break;
                case EBiomeType::Desert:     BiomeColor = FColor::Orange; break;
                case EBiomeType::Urban:      BiomeColor = FColor::Silver; break;
                case EBiomeType::Countryside: BiomeColor = FColor::Cyan; break;
                case EBiomeType::Mountains:  BiomeColor = FColor::White; break;
                case EBiomeType::Wetlands:   BiomeColor = FColor::Blue; break;
                default: break;
        }

        // Draw sphere at location
        DrawDebugSphere(GetWorld(), Location, 500.0f, 12, BiomeColor, false, 30.0f, 0, 10.0f);

        // Draw text showing biome name and actor count
        FString DebugText = FString::Printf(TEXT("%s\n%d actors\nQ:%d"),
                                           *UBiomeUtilities::GetBiomeName(BiomeType),
                                           ActorCount,
                                           static_cast<int32>(CurrentQualityLevel));
        DrawDebugString(GetWorld(), Location + FVector(0, 0, 600), DebugText, nullptr, BiomeColor, 30.0f, true);
}
