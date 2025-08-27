#include "PerformanceOptimizationSystem.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "PCGActor.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "RHI.h"
#include "Stats/Stats.h"

void UPerformanceOptimizationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize default settings
    OptimizationSettings = FPerformanceOptimizationSettings();
    CurrentMetrics = FPerformanceMetrics();
    OptimizationUpdateTimer = 0.0f;
    CurrentLODBias = 1.0f;
    
    // Initialize default LOD configurations
    InitializeDefaultLODConfigs();
    
    // Initialize frame time history
    FrameTimeHistory.Reserve(60); // 1 second at 60 FPS
    MemoryUsageHistory.Reserve(60);
    
    UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizationSystem initialized"));
}

void UPerformanceOptimizationSystem::Deinitialize()
{
    // Clean up tracked objects
    TrackedMeshComponents.Empty();
    TrackedParticleSystems.Empty();
    TrackedPCGActors.Empty();
    FrameTimeHistory.Empty();
    MemoryUsageHistory.Empty();
    
    Super::Deinitialize();
}

void UPerformanceOptimizationSystem::UpdateOptimization(const FVector& PlayerLocation, const FVector& PlayerVelocity)
{
    SCOPE_CYCLE_COUNTER(STAT_PerformanceOptimizationUpdate);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Apply adaptive optimizations if enabled
    if (OptimizationSettings.bEnableAdaptiveOptimization)
    {
        ApplyAdaptiveOptimizations();
    }
    
    // Update component LODs
    UpdateComponentLODs(PlayerLocation);
    
    // Optimize particle systems
    OptimizeParticleSystems(PlayerLocation);
    
    // Optimize PCG actors
    OptimizePCGActors(PlayerLocation);
    
    // Clean up invalid tracked objects periodically
    OptimizationUpdateTimer += GetWorld()->GetDeltaSeconds();
    if (OptimizationUpdateTimer >= 1.0f) // Every second
    {
        CleanupTrackedObjects();
        OptimizationUpdateTimer = 0.0f;
    }
}

FPerformanceMetrics UPerformanceOptimizationSystem::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerformanceOptimizationSystem::SetOptimizationSettings(const FPerformanceOptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    
    // Recalculate adaptive LOD bias if adaptive optimization is enabled
    if (OptimizationSettings.bEnableAdaptiveOptimization)
    {
        CalculateAdaptiveLODBias();
    }
}

FPerformanceOptimizationSettings UPerformanceOptimizationSystem::GetOptimizationSettings() const
{
    return OptimizationSettings;
}

void UPerformanceOptimizationSystem::SetAdaptiveOptimizationEnabled(bool bEnabled)
{
    OptimizationSettings.bEnableAdaptiveOptimization = bEnabled;
    
    if (bEnabled)
    {
        CalculateAdaptiveLODBias();
    }
    else
    {
        CurrentLODBias = OptimizationSettings.AdaptiveLODBias;
    }
}

void UPerformanceOptimizationSystem::SetBiomeLODConfig(EBiomeType BiomeType, const FBiomeLODConfig& LODConfig)
{
    BiomeLODConfigs.Add(BiomeType, LODConfig);
}

FBiomeLODConfig UPerformanceOptimizationSystem::GetBiomeLODConfig(EBiomeType BiomeType) const
{
    const FBiomeLODConfig* Config = BiomeLODConfigs.Find(BiomeType);
    if (Config)
    {
        return *Config;
    }
    
    // Return default configuration if not found
    return FBiomeLODConfig();
}

void UPerformanceOptimizationSystem::OptimizeObjectsInRadius(const FVector& Center, float Radius)
{
    // Optimize mesh components in radius
    for (auto& WeakComponent : TrackedMeshComponents)
    {
        if (UStaticMeshComponent* MeshComp = WeakComponent.Get())
        {
            float Distance = FVector::Dist(MeshComp->GetComponentLocation(), Center);
            if (Distance <= Radius)
            {
                // Determine biome type (simplified - in practice this would be more sophisticated)
                EBiomeType BiomeType = EBiomeType::Countryside; // Default
                int32 LODLevel = CalculateLODLevel(Distance, BiomeType);
                ApplyMeshLOD(MeshComp, LODLevel);
            }
        }
    }
    
    // Optimize particle systems in radius
    for (auto& WeakParticle : TrackedParticleSystems)
    {
        if (UNiagaraComponent* ParticleComp = WeakParticle.Get())
        {
            float Distance = FVector::Dist(ParticleComp->GetComponentLocation(), Center);
            if (Distance <= Radius)
            {
                ApplyParticleOptimization(ParticleComp, Distance, OptimizationSettings.ParticleOptimizationLevel);
            }
        }
    }
}

void UPerformanceOptimizationSystem::ForceApplyLODLevel(int32 LODLevel)
{
    for (auto& WeakComponent : TrackedMeshComponents)
    {
        if (UStaticMeshComponent* MeshComp = WeakComponent.Get())
        {
            ApplyMeshLOD(MeshComp, LODLevel);
        }
    }
    
    CurrentMetrics.LODLevel = LODLevel;
    
    UE_LOG(LogTemp, Log, TEXT("Forced LOD level %d applied to all tracked objects"), LODLevel);
}

TMap<FString, float> UPerformanceOptimizationSystem::GetMemoryUsageBreakdown()
{
    TMap<FString, float> Breakdown;
    
    // Get basic memory statistics
    const FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    Breakdown.Add(TEXT("TotalPhysical"), MemStats.TotalPhysical / (1024.0f * 1024.0f)); // MB
    Breakdown.Add(TEXT("UsedPhysical"), MemStats.UsedPhysical / (1024.0f * 1024.0f));
    Breakdown.Add(TEXT("TotalVirtual"), MemStats.TotalVirtual / (1024.0f * 1024.0f));
    Breakdown.Add(TEXT("UsedVirtual"), MemStats.UsedVirtual / (1024.0f * 1024.0f));
    
    // Estimate mesh component memory usage
    float MeshMemoryMB = TrackedMeshComponents.Num() * 2.0f; // Rough estimate: 2MB per mesh component
    Breakdown.Add(TEXT("MeshComponents"), MeshMemoryMB);
    
    // Estimate particle system memory usage
    float ParticleMemoryMB = TrackedParticleSystems.Num() * 1.0f; // Rough estimate: 1MB per particle system
    Breakdown.Add(TEXT("ParticleSystems"), ParticleMemoryMB);
    
    // Estimate PCG actor memory usage
    float PCGMemoryMB = TrackedPCGActors.Num() * 5.0f; // Rough estimate: 5MB per PCG actor
    Breakdown.Add(TEXT("PCGActors"), PCGMemoryMB);
    
    return Breakdown;
}

void UPerformanceOptimizationSystem::UpdatePerformanceMetrics()
{
    // Update frame time
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    CurrentMetrics.FrameTimeMs = DeltaTime * 1000.0f;
    
    // Update frame time history
    FrameTimeHistory.Add(CurrentMetrics.FrameTimeMs);
    if (FrameTimeHistory.Num() > 60)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Update memory usage
    const FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Update memory usage history
    MemoryUsageHistory.Add(CurrentMetrics.MemoryUsageMB);
    if (MemoryUsageHistory.Num() > 60)
    {
        MemoryUsageHistory.RemoveAt(0);
    }
    
    // Update object counts
    CleanupTrackedObjects(); // Clean up first to get accurate counts
    CurrentMetrics.VisibleObjects = TrackedMeshComponents.Num();
    CurrentMetrics.ActiveParticleSystems = TrackedParticleSystems.Num();
    
    // Estimate draw calls (simplified)
    CurrentMetrics.DrawCalls = CurrentMetrics.VisibleObjects + CurrentMetrics.ActiveParticleSystems;
    
    // Calculate average LOD level
    int32 TotalLODLevel = 0;
    int32 LODCount = 0;
    for (auto& WeakComponent : TrackedMeshComponents)
    {
        if (UStaticMeshComponent* MeshComp = WeakComponent.Get())
        {
            // In practice, you'd get the actual LOD level from the component
            // For now, use a placeholder
            TotalLODLevel += 1; // Placeholder
            LODCount++;
        }
    }
    CurrentMetrics.LODLevel = (LODCount > 0) ? (TotalLODLevel / LODCount) : 0;
    
    // Check if within performance targets
    float TargetFrameTimeMs = 1000.0f / OptimizationSettings.TargetFrameRate;
    CurrentMetrics.bWithinPerformanceTarget = 
        (CurrentMetrics.FrameTimeMs <= TargetFrameTimeMs * 1.1f) && 
        (CurrentMetrics.MemoryUsageMB <= OptimizationSettings.MaxMemoryBudgetMB);
    
    // Estimate CPU/GPU usage (simplified)
    CurrentMetrics.CPUUsagePercent = FMath::Clamp((CurrentMetrics.FrameTimeMs / TargetFrameTimeMs) * 100.0f, 0.0f, 100.0f);
    CurrentMetrics.GPUUsagePercent = FMath::Clamp((CurrentMetrics.DrawCalls / 1000.0f) * 100.0f, 0.0f, 100.0f);
    
    // Fire events if necessary
    if (!CurrentMetrics.bWithinPerformanceTarget)
    {
        OnPerformanceTargetMissedEvent.Broadcast(CurrentMetrics);
    }
    
    if (CurrentMetrics.MemoryUsageMB > OptimizationSettings.MaxMemoryBudgetMB)
    {
        OnMemoryBudgetExceededEvent.Broadcast(CurrentMetrics.MemoryUsageMB);
    }
}

void UPerformanceOptimizationSystem::ApplyAdaptiveOptimizations()
{
    if (!OptimizationSettings.bEnableAdaptiveOptimization || FrameTimeHistory.Num() < 10)
    {
        return;
    }
    
    // Calculate average frame time over recent history
    float AverageFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        AverageFrameTime += FrameTime;
    }
    AverageFrameTime /= FrameTimeHistory.Num();
    
    float TargetFrameTime = 1000.0f / OptimizationSettings.TargetFrameRate;
    
    // Adjust optimization level based on performance
    if (AverageFrameTime > TargetFrameTime * 1.2f) // 20% over target
    {
        // Performance is poor, increase optimization
        if (OptimizationSettings.ParticleOptimizationLevel < 2)
        {
            OptimizationSettings.ParticleOptimizationLevel++;
            OnAdaptiveOptimizationAppliedEvent.Broadcast(OptimizationSettings.ParticleOptimizationLevel, TEXT("ParticleOptimization"));
        }
        
        // Increase LOD bias to reduce detail
        CurrentLODBias = FMath::Min(CurrentLODBias * 1.1f, 2.0f);
        
        // Apply emergency optimizations if very poor performance
        if (AverageFrameTime > TargetFrameTime * 2.0f && OptimizationSettings.bEnableAggressiveOptimization)
        {
            ApplyEmergencyOptimizations();
        }
    }
    else if (AverageFrameTime < TargetFrameTime * 0.8f) // 20% under target
    {
        // Performance is good, can reduce optimization
        if (OptimizationSettings.ParticleOptimizationLevel > 0)
        {
            OptimizationSettings.ParticleOptimizationLevel--;
            OnAdaptiveOptimizationAppliedEvent.Broadcast(OptimizationSettings.ParticleOptimizationLevel, TEXT("ParticleOptimization"));
        }
        
        // Reduce LOD bias to increase detail
        CurrentLODBias = FMath::Max(CurrentLODBias * 0.95f, 0.5f);
    }
}

void UPerformanceOptimizationSystem::UpdateComponentLODs(const FVector& PlayerLocation)
{
    for (auto& WeakComponent : TrackedMeshComponents)
    {
        if (UStaticMeshComponent* MeshComp = WeakComponent.Get())
        {
            float Distance = FVector::Dist(MeshComp->GetComponentLocation(), PlayerLocation);
            
            // Determine biome type for this component (simplified - would be more sophisticated in practice)
            EBiomeType BiomeType = EBiomeType::Countryside; // Default
            
            int32 LODLevel = CalculateLODLevel(Distance, BiomeType);
            ApplyMeshLOD(MeshComp, LODLevel);
        }
    }
}

void UPerformanceOptimizationSystem::OptimizeParticleSystems(const FVector& PlayerLocation)
{
    int32 ActiveParticles = 0;
    
    for (auto& WeakParticle : TrackedParticleSystems)
    {
        if (UNiagaraComponent* ParticleComp = WeakParticle.Get())
        {
            float Distance = FVector::Dist(ParticleComp->GetComponentLocation(), PlayerLocation);
            
            ApplyParticleOptimization(ParticleComp, Distance, OptimizationSettings.ParticleOptimizationLevel);
            
            if (ParticleComp->IsActive())
            {
                ActiveParticles++;
            }
        }
    }
    
    CurrentMetrics.ActiveParticleSystems = ActiveParticles;
}

void UPerformanceOptimizationSystem::OptimizePCGActors(const FVector& PlayerLocation)
{
    for (auto& WeakPCGActor : TrackedPCGActors)
    {
        if (APCGActor* PCGActor = WeakPCGActor.Get())
        {
            float Distance = FVector::Dist(PCGActor->GetActorLocation(), PlayerLocation);
            
            // Optimize PCG actor based on distance and performance settings
            if (Distance > 5000.0f) // 5km
            {
                // Disable or reduce PCG generation at long distances
                PCGActor->SetActorHiddenInGame(true);
            }
            else if (Distance > 2000.0f) // 2km
            {
                // Reduce PCG detail at medium distances
                PCGActor->SetActorHiddenInGame(false);
                // Could reduce PCG generation density here
            }
            else
            {
                // Full detail at close distances
                PCGActor->SetActorHiddenInGame(false);
            }
        }
    }
}

int32 UPerformanceOptimizationSystem::CalculateLODLevel(float Distance, EBiomeType BiomeType) const
{
    const FBiomeLODConfig* Config = BiomeLODConfigs.Find(BiomeType);
    if (!Config)
    {
        Config = BiomeLODConfigs.Find(EBiomeType::Countryside); // Default fallback
    }
    
    if (!Config || !Config->bEnableLOD)
    {
        return 0; // No LOD
    }
    
    // Apply adaptive LOD bias
    float AdjustedDistance = Distance / CurrentLODBias;
    
    if (AdjustedDistance <= Config->LOD0Distance)
    {
        return 0; // Highest detail
    }
    else if (AdjustedDistance <= Config->LOD1Distance)
    {
        return 1; // Medium detail
    }
    else if (AdjustedDistance <= Config->LOD2Distance)
    {
        return 2; // Low detail
    }
    else if (AdjustedDistance <= Config->CullingDistance)
    {
        return 3; // Lowest detail
    }
    else
    {
        return -1; // Cull completely
    }
}

void UPerformanceOptimizationSystem::ApplyMeshLOD(UStaticMeshComponent* MeshComponent, int32 LODLevel)
{
    if (!MeshComponent)
    {
        return;
    }
    
    if (LODLevel == -1)
    {
        // Cull the mesh
        MeshComponent->SetVisibility(false);
        return;
    }
    else
    {
        MeshComponent->SetVisibility(true);
    }
    
    // Force specific LOD level
    MeshComponent->SetForcedLodModel(LODLevel);
}

void UPerformanceOptimizationSystem::ApplyParticleOptimization(UNiagaraComponent* ParticleSystem, float Distance, int32 OptimizationLevel)
{
    if (!ParticleSystem)
    {
        return;
    }
    
    bool bShouldBeActive = true;
    float IntensityMultiplier = 1.0f;
    
    // Distance-based optimization
    if (Distance > 3000.0f)
    {
        bShouldBeActive = false;
    }
    else if (Distance > 1500.0f)
    {
        IntensityMultiplier = 0.5f;
    }
    else if (Distance > 500.0f)
    {
        IntensityMultiplier = 0.7f;
    }
    
    // Apply optimization level
    switch (OptimizationLevel)
    {
        case 0: // No optimization
            break;
        case 1: // Moderate optimization
            IntensityMultiplier *= 0.7f;
            if (Distance > 2000.0f)
            {
                bShouldBeActive = false;
            }
            break;
        case 2: // Aggressive optimization
            IntensityMultiplier *= 0.4f;
            if (Distance > 1000.0f)
            {
                bShouldBeActive = false;
            }
            break;
    }
    
    // Apply settings
    if (bShouldBeActive && !ParticleSystem->IsActive())
    {
        ParticleSystem->Activate();
    }
    else if (!bShouldBeActive && ParticleSystem->IsActive())
    {
        ParticleSystem->Deactivate();
    }
    
    // Apply intensity multiplier (simplified - in practice would set specific parameters)
    if (bShouldBeActive)
    {
        // Could set custom parameters here based on IntensityMultiplier
        // For example: ParticleSystem->SetFloatParameter(TEXT("SpawnRate"), BaseSpawnRate * IntensityMultiplier);
    }
}

void UPerformanceOptimizationSystem::RegisterComponentForOptimization(UStaticMeshComponent* MeshComponent)
{
    if (MeshComponent && !TrackedMeshComponents.Contains(MeshComponent))
    {
        TrackedMeshComponents.Add(MeshComponent);
    }
}

void UPerformanceOptimizationSystem::RegisterParticleSystemForOptimization(UNiagaraComponent* ParticleSystem)
{
    if (ParticleSystem && !TrackedParticleSystems.Contains(ParticleSystem))
    {
        TrackedParticleSystems.Add(ParticleSystem);
    }
}

void UPerformanceOptimizationSystem::RegisterPCGActorForOptimization(APCGActor* PCGActor)
{
    if (PCGActor && !TrackedPCGActors.Contains(PCGActor))
    {
        TrackedPCGActors.Add(PCGActor);
    }
}

void UPerformanceOptimizationSystem::CleanupTrackedObjects()
{
    // Remove invalid mesh components
    TrackedMeshComponents.RemoveAll([](const TWeakObjectPtr<UStaticMeshComponent>& WeakPtr)
    {
        return !WeakPtr.IsValid();
    });
    
    // Remove invalid particle systems
    TrackedParticleSystems.RemoveAll([](const TWeakObjectPtr<UNiagaraComponent>& WeakPtr)
    {
        return !WeakPtr.IsValid();
    });
    
    // Remove invalid PCG actors
    TrackedPCGActors.RemoveAll([](const TWeakObjectPtr<APCGActor>& WeakPtr)
    {
        return !WeakPtr.IsValid();
    });
}

void UPerformanceOptimizationSystem::InitializeDefaultLODConfigs()
{
    // Forest LOD configuration
    FBiomeLODConfig ForestLOD;
    ForestLOD.LOD0Distance = 800.0f;
    ForestLOD.LOD1Distance = 2500.0f;
    ForestLOD.LOD2Distance = 5000.0f;
    ForestLOD.CullingDistance = 8000.0f;
    ForestLOD.ParticleLODMultiplier = 0.8f; // Dense biome, reduce particles
    BiomeLODConfigs.Add(EBiomeType::Forest, ForestLOD);
    
    // Urban LOD configuration
    FBiomeLODConfig UrbanLOD;
    UrbanLOD.LOD0Distance = 1200.0f;
    UrbanLOD.LOD1Distance = 3500.0f;
    UrbanLOD.LOD2Distance = 6000.0f;
    UrbanLOD.CullingDistance = 10000.0f;
    UrbanLOD.ParticleLODMultiplier = 0.6f; // Complex biome, reduce particles more
    BiomeLODConfigs.Add(EBiomeType::Urban, UrbanLOD);
    
    // Desert LOD configuration
    FBiomeLODConfig DesertLOD;
    DesertLOD.LOD0Distance = 1500.0f; // Open area, can see further
    DesertLOD.LOD1Distance = 4000.0f;
    DesertLOD.LOD2Distance = 8000.0f;
    DesertLOD.CullingDistance = 12000.0f;
    DesertLOD.ParticleLODMultiplier = 1.0f; // Simple biome, keep particles
    BiomeLODConfigs.Add(EBiomeType::Desert, DesertLOD);
    
    // Beach LOD configuration
    FBiomeLODConfig BeachLOD;
    BeachLOD.LOD0Distance = 1200.0f;
    BeachLOD.LOD1Distance = 3000.0f;
    BeachLOD.LOD2Distance = 6000.0f;
    BeachLOD.CullingDistance = 10000.0f;
    BeachLOD.ParticleLODMultiplier = 0.9f;
    BiomeLODConfigs.Add(EBiomeType::Beach, BeachLOD);
    
    // Mountains LOD configuration
    FBiomeLODConfig MountainsLOD;
    MountainsLOD.LOD0Distance = 1000.0f;
    MountainsLOD.LOD1Distance = 3000.0f;
    MountainsLOD.LOD2Distance = 7000.0f;
    MountainsLOD.CullingDistance = 12000.0f;
    MountainsLOD.ParticleLODMultiplier = 0.7f;
    BiomeLODConfigs.Add(EBiomeType::Mountains, MountainsLOD);
    
    // Countryside LOD configuration (default/balanced)
    FBiomeLODConfig CountrysideLOD;
    CountrysideLOD.LOD0Distance = 1000.0f;
    CountrysideLOD.LOD1Distance = 3000.0f;
    CountrysideLOD.LOD2Distance = 6000.0f;
    CountrysideLOD.CullingDistance = 10000.0f;
    CountrysideLOD.ParticleLODMultiplier = 1.0f;
    BiomeLODConfigs.Add(EBiomeType::Countryside, CountrysideLOD);
    
    // Wetlands LOD configuration
    FBiomeLODConfig WetlandsLOD;
    WetlandsLOD.LOD0Distance = 800.0f; // Dense environment
    WetlandsLOD.LOD1Distance = 2500.0f;
    WetlandsLOD.LOD2Distance = 5000.0f;
    WetlandsLOD.CullingDistance = 8000.0f;
    WetlandsLOD.ParticleLODMultiplier = 0.8f;
    BiomeLODConfigs.Add(EBiomeType::Wetlands, WetlandsLOD);
}

void UPerformanceOptimizationSystem::CalculateAdaptiveLODBias()
{
    if (FrameTimeHistory.Num() < 5)
    {
        CurrentLODBias = OptimizationSettings.AdaptiveLODBias;
        return;
    }
    
    // Calculate average frame time
    float AverageFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        AverageFrameTime += FrameTime;
    }
    AverageFrameTime /= FrameTimeHistory.Num();
    
    float TargetFrameTime = 1000.0f / OptimizationSettings.TargetFrameRate;
    float PerformanceRatio = AverageFrameTime / TargetFrameTime;
    
    // Adjust LOD bias based on performance ratio
    if (PerformanceRatio > 1.2f)
    {
        // Poor performance, increase LOD bias (lower detail)
        CurrentLODBias = FMath::Min(OptimizationSettings.AdaptiveLODBias * PerformanceRatio, 2.0f);
    }
    else if (PerformanceRatio < 0.8f)
    {
        // Good performance, decrease LOD bias (higher detail)
        CurrentLODBias = FMath::Max(OptimizationSettings.AdaptiveLODBias * PerformanceRatio, 0.5f);
    }
    else
    {
        // Performance is acceptable, use base bias
        CurrentLODBias = OptimizationSettings.AdaptiveLODBias;
    }
}

void UPerformanceOptimizationSystem::ApplyEmergencyOptimizations()
{
    UE_LOG(LogTemp, Warning, TEXT("Applying emergency performance optimizations"));
    
    // Force high LOD on all objects
    ForceApplyLODLevel(2);
    
    // Disable all particle systems temporarily
    for (auto& WeakParticle : TrackedParticleSystems)
    {
        if (UNiagaraComponent* ParticleComp = WeakParticle.Get())
        {
            ParticleComp->Deactivate();
        }
    }
    
    // Hide distant PCG actors
    for (auto& WeakPCGActor : TrackedPCGActors)
    {
        if (APCGActor* PCGActor = WeakPCGActor.Get())
        {
            PCGActor->SetActorHiddenInGame(true);
        }
    }
    
    OnAdaptiveOptimizationAppliedEvent.Broadcast(2, TEXT("EmergencyOptimization"));
}

// Auto Optimization Component Implementation
UAutoOptimizationComponent::UAutoOptimizationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    bAutoOptimizationEnabled = true;
    BiomeType = EBiomeType::Countryside;
    OptimizationPriority = 5;
    bRegisteredWithOptimizationSystem = false;
}

void UAutoOptimizationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoOptimizationEnabled)
    {
        RegisterWithOptimizationSystem();
    }
}

void UAutoOptimizationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterFromOptimizationSystem();
    Super::EndPlay(EndPlayReason);
}

void UAutoOptimizationComponent::EnableAutoOptimization(bool bEnable)
{
    if (bAutoOptimizationEnabled != bEnable)
    {
        bAutoOptimizationEnabled = bEnable;
        
        if (bEnable && !bRegisteredWithOptimizationSystem)
        {
            RegisterWithOptimizationSystem();
        }
        else if (!bEnable && bRegisteredWithOptimizationSystem)
        {
            UnregisterFromOptimizationSystem();
        }
    }
}

void UAutoOptimizationComponent::SetBiomeType(EBiomeType NewBiomeType)
{
    BiomeType = NewBiomeType;
}

void UAutoOptimizationComponent::SetOptimizationPriority(int32 Priority)
{
    OptimizationPriority = FMath::Clamp(Priority, 0, 10);
}

void UAutoOptimizationComponent::RegisterWithOptimizationSystem()
{
    UPerformanceOptimizationSystem* OptimizationSystem = GetGameInstance()->GetSubsystem<UPerformanceOptimizationSystem>();
    if (!OptimizationSystem)
    {
        return;
    }
    
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Register all static mesh components
    TArray<UStaticMeshComponent*> MeshComponents;
    Owner->GetComponents<UStaticMeshComponent>(MeshComponents);
    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        OptimizationSystem->RegisterComponentForOptimization(MeshComp);
    }
    
    // Register all particle systems
    TArray<UNiagaraComponent*> ParticleComponents;
    Owner->GetComponents<UNiagaraComponent>(ParticleComponents);
    for (UNiagaraComponent* ParticleComp : ParticleComponents)
    {
        OptimizationSystem->RegisterParticleSystemForOptimization(ParticleComp);
    }
    
    // Register PCG actor if this is one
    if (APCGActor* PCGActor = Cast<APCGActor>(Owner))
    {
        OptimizationSystem->RegisterPCGActorForOptimization(PCGActor);
    }
    
    bRegisteredWithOptimizationSystem = true;
}

void UAutoOptimizationComponent::UnregisterFromOptimizationSystem()
{
    // Note: The optimization system uses weak pointers, so components will be automatically
    // cleaned up when the actor is destroyed. This is just for early cleanup if needed.
    bRegisteredWithOptimizationSystem = false;
}