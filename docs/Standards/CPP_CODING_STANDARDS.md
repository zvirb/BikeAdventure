# C++ Coding Standards for BikeAdventure

## Style Guide Example

```cpp
// BikeAdventure C++ Style Guide
// File: BikeCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BikeCharacter.generated.h"

/**
 * Main character class for bike adventure game
 * Handles player input and bike physics
 */
UCLASS()
class BIKEADVENTURE_API ABikeCharacter : public APawn
{
    GENERATED_BODY()

public:
    ABikeCharacter();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

private:
    // Movement component for bike physics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", 
              meta = (AllowPrivateAccess = "true"))
    class UBikeMovementComponent* BikeMovement;
    
    // Current speed in cm/s
    UPROPERTY(BlueprintReadOnly, Category = "Movement", 
              meta = (AllowPrivateAccess = "true"))
    float CurrentSpeed;
    
    // Is the bike currently at an intersection?
    UPROPERTY(BlueprintReadOnly, Category = "Gameplay", 
              meta = (AllowPrivateAccess = "true"))
    bool bIsAtIntersection;

public:
    // Input handlers
    UFUNCTION(BlueprintCallable, Category = "Input")
    void HandleTurnInput(float Value);
    
    UFUNCTION(BlueprintCallable, Category = "Input")
    void HandleThrottleInput(float Value);
    
    // Intersection system
    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void OnIntersectionReached(class AIntersection* Intersection);
    
    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void ChooseDirection(bool bGoLeft);
};
```

## Blueprint Naming Conventions

```
Blueprint Classes:       BP_[ClassName]
Widget Blueprints:      WBP_[WidgetName]
Blueprint Interfaces:   BPI_[InterfaceName]
Animation Blueprints:   ABP_[CharacterName]
Materials:              M_[MaterialName]
Material Instances:     MI_[InstanceName]
Textures:               T_[TextureName]_[TextureType]
Static Meshes:          SM_[MeshName]
Skeletal Meshes:        SK_[MeshName]
```