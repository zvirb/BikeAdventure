#include "Intersection.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Components/PointLightComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"

AIntersection::AIntersection()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create trigger volume
    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    TriggerVolume->SetupAttachment(RootComponent);
    TriggerVolume->SetBoxExtent(FVector(300.0f, 300.0f, 200.0f));
    TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create main intersection mesh
    IntersectionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IntersectionMesh"));
    IntersectionMesh->SetupAttachment(RootComponent);
    IntersectionMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    IntersectionMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Create path indicator meshes
    LeftPathIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftPathIndicator"));
    LeftPathIndicator->SetupAttachment(RootComponent);
    LeftPathIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    RightPathIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightPathIndicator"));
    RightPathIndicator->SetupAttachment(RootComponent);
    RightPathIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create environmental effect component
    EnvironmentalEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EnvironmentalEffect"));
    EnvironmentalEffect->SetupAttachment(RootComponent);

    // Create ambient audio component
    AmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudio->SetupAttachment(RootComponent);
    AmbientAudio->bAutoActivate = true;

    // Initialize default values
    IntersectionType = EIntersectionType::YFork;
    LeftPathBiome = EBiomeType::Forest;
    RightPathBiome = EBiomeType::Countryside;
    bPlayerPresent = false;
    bChoiceMade = false;

    // Set default path directions (45-degree angles)
    LeftPathDirection = FVector(0.707f, -0.707f, 0.0f);  // Left-forward
    RightPathDirection = FVector(0.707f, 0.707f, 0.0f);   // Right-forward

    // Bind trigger events
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AIntersection::OnPlayerEnterIntersection);
    TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AIntersection::OnPlayerExitIntersection);
}

void AIntersection::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultAssets();
    UpdateVisualAppearance();
    UpdatePathHints();
    CalculatePathDirections();
}

void AIntersection::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update any dynamic visual effects here if needed
    if (bPlayerPresent && !bChoiceMade)
    {
        // Could add pulsing effects or other dynamic elements
    }
}

void AIntersection::SetIntersectionType(EIntersectionType NewType)
{
    if (IntersectionType != NewType)
    {
        IntersectionType = NewType;
        UpdateVisualAppearance();
        CalculatePathDirections();
    }
}

void AIntersection::SetPathBiomes(EBiomeType LeftBiome, EBiomeType RightBiome)
{
    LeftPathBiome = LeftBiome;
    RightPathBiome = RightBiome;
    
    ApplyBiomeVisuals(LeftBiome, LeftPathIndicator, true);
    ApplyBiomeVisuals(RightBiome, RightPathIndicator, false);
}

void AIntersection::SetPathHints(const FPathHints& NewHints)
{
    PathHints = NewHints;
    UpdatePathHints();
}

void AIntersection::OnPlayerEnterIntersection(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        bPlayerPresent = true;
        
        UE_LOG(LogTemp, Log, TEXT("Player entered intersection: %s -> Left: %s, Right: %s"), 
               *GetName(), 
               *UBiomeUtilities::GetBiomeName(LeftPathBiome), 
               *UBiomeUtilities::GetBiomeName(RightPathBiome));

        // Broadcast event
        OnPlayerEnterIntersectionEvent.Broadcast(this, OtherActor);

        // Activate environmental effects
        if (EnvironmentalEffect && !EnvironmentalEffect->IsActive())
        {
            EnvironmentalEffect->Activate();
        }
    }
}

void AIntersection::OnPlayerExitIntersection(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA<APawn>())
    {
        bPlayerPresent = false;
        
        UE_LOG(LogTemp, Log, TEXT("Player exited intersection: %s"), *GetName());

        // Broadcast event
        OnPlayerExitIntersectionEvent.Broadcast(this, OtherActor);

        // Deactivate environmental effects to save performance
        if (EnvironmentalEffect && EnvironmentalEffect->IsActive())
        {
            EnvironmentalEffect->Deactivate();
        }
    }
}

void AIntersection::HandlePlayerChoice(bool bChoseLeftPath)
{
    OnPlayerChoiceMade(bChoseLeftPath);
}

void AIntersection::OnPlayerChoiceMade(bool bChoseLeftPath)
{
    if (bChoiceMade)
    {
        return; // Choice already made
    }

    bChoiceMade = true;
    EBiomeType ChosenBiome = bChoseLeftPath ? LeftPathBiome : RightPathBiome;
    
    UE_LOG(LogTemp, Log, TEXT("Player chose %s path at intersection %s, leading to %s biome"), 
           bChoseLeftPath ? TEXT("left") : TEXT("right"), 
           *GetName(), 
           *UBiomeUtilities::GetBiomeName(ChosenBiome));

    // Broadcast choice event
    OnPlayerChoiceMadeEvent.Broadcast(this, bChoseLeftPath, ChosenBiome);

    // Could add visual feedback for the chosen path
    UStaticMeshComponent* ChosenIndicator = bChoseLeftPath ? LeftPathIndicator : RightPathIndicator;
    if (ChosenIndicator)
    {
        // Add glow effect or other visual feedback
        // This could be implemented with dynamic material parameters
    }
}

void AIntersection::UpdateVisualAppearance()
{
    // Update intersection mesh based on type
    TSoftObjectPtr<UStaticMesh>* MeshPtr = IntersectionMeshMap.Find(IntersectionType);
    if (MeshPtr && MeshPtr->IsValid())
    {
        IntersectionMesh->SetStaticMesh(MeshPtr->LoadSynchronous());
    }

    // Update materials and effects based on current biome context
    ApplyBiomeVisuals(LeftPathBiome, LeftPathIndicator, true);
    ApplyBiomeVisuals(RightPathBiome, RightPathIndicator, false);
}

void AIntersection::UpdatePathHints()
{
    // Apply visual hints based on path personalities
    if (PathHints.LeftPathPersonality != EPathPersonality::None)
    {
        TSoftObjectPtr<UStaticMesh>* HintMeshPtr = PathHintMeshes.Find(PathHints.LeftPathPersonality);
        if (HintMeshPtr && HintMeshPtr->IsValid() && LeftPathIndicator)
        {
            // Could create additional components for hint elements
            // For now, modify the existing indicator
        }

        TSoftObjectPtr<UNiagaraSystem>* HintEffectPtr = PathHintEffects.Find(PathHints.LeftPathPersonality);
        if (HintEffectPtr && HintEffectPtr->IsValid() && EnvironmentalEffect)
        {
            EnvironmentalEffect->SetAsset(HintEffectPtr->LoadSynchronous());
        }
    }

    // Apply hint subtlety by adjusting opacity/intensity
    float HintIntensity = 1.0f - PathHints.HintSubtlety;
    
    if (LeftPathIndicator)
    {
        // Adjust material parameters for hint intensity
        UMaterialInstanceDynamic* LeftDynamicMaterial = LeftPathIndicator->CreateAndSetMaterialInstanceDynamic(0);
        if (LeftDynamicMaterial)
        {
            LeftDynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), HintIntensity);
            LeftDynamicMaterial->SetScalarParameterValue(TEXT("Challenge"), PathHints.LeftPathChallengeFactor);
        }
    }

    if (RightPathIndicator)
    {
        UMaterialInstanceDynamic* RightDynamicMaterial = RightPathIndicator->CreateAndSetMaterialInstanceDynamic(0);
        if (RightDynamicMaterial)
        {
            RightDynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), HintIntensity);
            RightDynamicMaterial->SetScalarParameterValue(TEXT("Scenery"), PathHints.RightPathSceneryFactor);
        }
    }
}

void AIntersection::ApplyBiomeVisuals(EBiomeType BiomeType, UStaticMeshComponent* TargetMesh, bool bIsLeftPath)
{
    if (!TargetMesh)
    {
        return;
    }

    // Apply biome-specific material
    TSoftObjectPtr<UMaterialInterface>* MaterialPtr = BiomeMaterials.Find(BiomeType);
    if (MaterialPtr && MaterialPtr->IsValid())
    {
        TargetMesh->SetMaterial(0, MaterialPtr->LoadSynchronous());
    }

    // Set up biome-specific particle effect
    TSoftObjectPtr<UNiagaraSystem>* EffectPtr = BiomeEffects.Find(BiomeType);
    if (EffectPtr && EffectPtr->IsValid() && EnvironmentalEffect)
    {
        // For now, use the effect for the intersection as a whole
        // In a more complex implementation, each path could have its own effect component
        if (!EnvironmentalEffect->GetAsset())
        {
            EnvironmentalEffect->SetAsset(EffectPtr->LoadSynchronous());
        }
    }

    // Set up biome-specific ambient audio
    TSoftObjectPtr<USoundCue>* AudioPtr = BiomeAmbientSounds.Find(BiomeType);
    if (AudioPtr && AudioPtr->IsValid() && AmbientAudio && !AmbientAudio->GetSound())
    {
        AmbientAudio->SetSound(AudioPtr->LoadSynchronous());
    }
}

void AIntersection::InitializeDefaultAssets()
{
    // Initialize default intersection meshes
    // Note: In a real implementation, these would reference actual mesh assets
    // For now, we're setting up the structure
    
    // Initialize default materials and effects maps
    // These would typically be set in Blueprint or loaded from data assets
}

void AIntersection::CalculatePathDirections()
{
    // Calculate path directions based on intersection type
    switch (IntersectionType)
    {
        case EIntersectionType::YFork:
            LeftPathDirection = FVector(0.707f, -0.707f, 0.0f);   // 45 degrees left
            RightPathDirection = FVector(0.707f, 0.707f, 0.0f);   // 45 degrees right
            break;
            
        case EIntersectionType::TJunction:
            LeftPathDirection = FVector(0.0f, -1.0f, 0.0f);       // 90 degrees left
            RightPathDirection = FVector(0.0f, 1.0f, 0.0f);       // 90 degrees right
            break;
            
        case EIntersectionType::Bridge:
            LeftPathDirection = FVector(1.0f, -0.5f, 0.0f);       // Slight left with elevation
            RightPathDirection = FVector(1.0f, 0.5f, 0.0f);       // Slight right with elevation
            break;
            
        case EIntersectionType::CaveEntrance:
            LeftPathDirection = FVector(0.5f, -0.866f, -0.2f);    // Left and down
            RightPathDirection = FVector(0.5f, 0.866f, 0.2f);     // Right and up
            break;
            
        case EIntersectionType::Boardwalk:
            LeftPathDirection = FVector(0.866f, -0.5f, 0.0f);     // Gentle left curve
            RightPathDirection = FVector(0.866f, 0.5f, 0.0f);     // Gentle right curve
            break;
            
        case EIntersectionType::RockPass:
            LeftPathDirection = FVector(0.6f, -0.8f, 0.1f);       // Steep left path
            RightPathDirection = FVector(0.6f, 0.8f, -0.1f);      // Steep right path
            break;
            
        case EIntersectionType::RiverCrossing:
            LeftPathDirection = FVector(0.8f, -0.6f, 0.0f);       // Bridge left
            RightPathDirection = FVector(0.8f, 0.6f, 0.0f);       // Bridge right
            break;
            
        case EIntersectionType::Roundabout:
            LeftPathDirection = FVector(0.5f, -0.866f, 0.0f);     // Curved left exit
            RightPathDirection = FVector(0.5f, 0.866f, 0.0f);     // Curved right exit
            break;
            
        default:
            // Default Y-fork directions
            LeftPathDirection = FVector(0.707f, -0.707f, 0.0f);
            RightPathDirection = FVector(0.707f, 0.707f, 0.0f);
            break;
    }

    // Normalize directions
    LeftPathDirection.Normalize();
    RightPathDirection.Normalize();

    // Update indicator positions based on directions
    if (LeftPathIndicator)
    {
        FVector LeftPosition = LeftPathDirection * 200.0f; // 2 meters forward
        LeftPathIndicator->SetRelativeLocation(LeftPosition);
        LeftPathIndicator->SetRelativeRotation(LeftPathDirection.Rotation());
    }

    if (RightPathIndicator)
    {
        FVector RightPosition = RightPathDirection * 200.0f; // 2 meters forward
        RightPathIndicator->SetRelativeLocation(RightPosition);
        RightPathIndicator->SetRelativeRotation(RightPathDirection.Rotation());
    }
}

// Specialized intersection implementations

AForestIntersection::AForestIntersection()
{
    IntersectionType = EIntersectionType::YFork;
    VegetationDensity = 0.8f;
    WildlifeActivity = 0.6f;
}

void AForestIntersection::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply forest-specific settings
    if (EnvironmentalEffect)
    {
        // Set up forest particle effects (leaves, fireflies, etc.)
        EnvironmentalEffect->SetFloatParameter(TEXT("VegetationDensity"), VegetationDensity);
        EnvironmentalEffect->SetFloatParameter(TEXT("WildlifeActivity"), WildlifeActivity);
    }
}

AUrbanIntersection::AUrbanIntersection()
{
    IntersectionType = EIntersectionType::Roundabout;
    bHasTrafficSignals = true;
    StreetSurfaceType = TEXT("Asphalt");
}

void AUrbanIntersection::BeginPlay()
{
    Super::BeginPlay();
    
    // Create street lights
    for (int32 i = 0; i < 4; i++)
    {
        UPointLightComponent* StreetLight = CreateDefaultSubobject<UPointLightComponent>(
            *FString::Printf(TEXT("StreetLight_%d"), i));
        
        if (StreetLight)
        {
            StreetLight->SetupAttachment(RootComponent);
            StreetLight->SetIntensity(1000.0f);
            StreetLight->SetLightColor(FLinearColor(1.0f, 0.9f, 0.8f)); // Warm white
            StreetLight->SetAttenuationRadius(800.0f);
            
            // Position lights around the intersection
            float Angle = (i * 90.0f) * PI / 180.0f;
            FVector LightPosition(FMath::Cos(Angle) * 400.0f, FMath::Sin(Angle) * 400.0f, 500.0f);
            StreetLight->SetRelativeLocation(LightPosition);
            
            StreetLights.Add(StreetLight);
        }
    }
}

ABeachIntersection::ABeachIntersection()
{
    IntersectionType = EIntersectionType::Boardwalk;
    WaveSoundIntensity = 0.7f;
    bHasPalmTrees = true;
}

void ABeachIntersection::BeginPlay()
{
    Super::BeginPlay();
    
    // Create sand particle effects
    SandParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SandParticles"));
    if (SandParticles)
    {
        SandParticles->SetupAttachment(RootComponent);
        SandParticles->SetFloatParameter(TEXT("IntensityScale"), WaveSoundIntensity);
    }
    
    // Adjust ambient audio volume for wave sounds
    if (AmbientAudio)
    {
        AmbientAudio->SetVolumeMultiplier(WaveSoundIntensity);
    }
}

AMountainIntersection::AMountainIntersection()
{
    IntersectionType = EIntersectionType::RockPass;
    PathElevationDifference = 100.0f;
    WindIntensity = 0.8f;
}

void AMountainIntersection::BeginPlay()
{
    Super::BeginPlay();
    
    // Create rock formations
    for (int32 i = 0; i < 6; i++)
    {
        UStaticMeshComponent* Rock = CreateDefaultSubobject<UStaticMeshComponent>(
            *FString::Printf(TEXT("RockFormation_%d"), i));
        
        if (Rock)
        {
            Rock->SetupAttachment(RootComponent);
            Rock->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            
            // Randomly position rocks around the intersection
            FVector RockPosition(
                FMath::RandRange(-500.0f, 500.0f),
                FMath::RandRange(-500.0f, 500.0f),
                FMath::RandRange(-50.0f, PathElevationDifference)
            );
            Rock->SetRelativeLocation(RockPosition);
            
            // Random rotation and scale
            Rock->SetRelativeRotation(FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f));
            float Scale = FMath::RandRange(0.8f, 1.5f);
            Rock->SetRelativeScale3D(FVector(Scale));
            
            RockFormations.Add(Rock);
        }
    }
    
    // Set up wind effects
    if (EnvironmentalEffect)
    {
        EnvironmentalEffect->SetFloatParameter(TEXT("WindIntensity"), WindIntensity);
    }
}