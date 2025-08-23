# Stream 1: UE5 Foundation & Core Systems Context Package
**Target Agent**: implementation_agent_1  
**Token Limit**: 4000 tokens  
**Success Criteria**: Functional UE5 project with basic bike movement

## Objective
Create the foundational UE5 project structure and implement core bike character systems for the BikeAdventure meditative exploration game.

## Technical Requirements
- **Engine**: Unreal Engine 5.4+
- **Platform**: Ubuntu Linux primary, Windows secondary
- **Performance**: 60+ FPS, <4GB memory, physics-based movement
- **Architecture**: Component-based design with C++/Blueprint hybrid

## Core Implementation Tasks

### 1. UE5 Project Initialization
```cpp
// Create BikeAdventure.uproject with these specifications:
{
  "FileVersion": 3,
  "EngineAssociation": "5.4",
  "Category": "",
  "Description": "Meditative bike exploration game",
  "Modules": [
    {
      "Name": "BikeAdventure",
      "Type": "Runtime",
      "LoadingPhase": "Default"
    }
  ],
  "Plugins": [
    {"Name": "ChaosPhysics", "Enabled": true},
    {"Name": "ChaosVehiclesPlugin", "Enabled": true},
    {"Name": "PCG", "Enabled": true}
  ]
}
```

### 2. Core C++ Classes
**ABikeAdventureGameMode**: Main game mode controller
```cpp
class BIKEADVENTURE_API ABikeAdventureGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ABikeAdventureGameMode();
    
    UPROPERTY(EditAnywhere, Category = "Core Systems")
    class UIntersectionManager* IntersectionManager;
    
    UPROPERTY(EditAnywhere, Category = "Core Systems") 
    class UBiomeGenerator* BiomeGenerator;
};
```

**ABikeCharacter**: Physics-based bike character
```cpp
class BIKEADVENTURE_API ABikeCharacter : public APawn
{
    GENERATED_BODY()
public:
    ABikeCharacter();
    
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBikeMovementComponent* BikeMovement;
    
    UFUNCTION(BlueprintCallable, Category = "Input")
    void HandleTurnInput(float Value);
    
    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void OnIntersectionReached(class AIntersection* Intersection);
};
```

**UBikeMovementComponent**: Custom movement system
```cpp 
class BIKEADVENTURE_API UBikeMovementComponent : public UPawnMovementComponent
{
    GENERATED_BODY()
public:
    // Forward movement (automatic)
    UPROPERTY(EditAnywhere, Category = "Movement")
    float ForwardSpeed = 1200.0f; // cm/s
    
    // Turning mechanics
    UPROPERTY(EditAnywhere, Category = "Movement") 
    float TurnRate = 45.0f; // degrees/sec
    
    virtual void UpdateMovement(float DeltaTime) override;
    void SetSteering(float SteeringInput);
};
```

### 3. Directory Structure
```
BikeAdventure/
├── BikeAdventure.uproject
├── Source/
│   ├── BikeAdventure/
│   │   ├── Core/
│   │   │   ├── BikeAdventureGameMode.h/.cpp
│   │   │   └── BikeCharacter.h/.cpp
│   │   ├── Systems/
│   │   │   ├── BikeMovementComponent.h/.cpp
│   │   │   └── IntersectionManager.h/.cpp
│   │   └── BikeAdventure.cpp/.h
│   └── BikeAdventure.Target.cs
├── Content/
│   ├── Core/
│   │   ├── GameModes/
│   │   └── Characters/
│   └── Gameplay/
│       ├── Intersections/
│       └── TestLevels/
└── Config/
    ├── DefaultEngine.ini
    ├── DefaultGame.ini  
    └── DefaultInput.ini
```

### 4. Movement Implementation Details
- **Physics Integration**: Use Chaos physics system for realistic bike behavior
- **Automatic Forward**: Constant forward velocity with momentum
- **Intersection Detection**: Overlap events with intersection trigger volumes
- **Smooth Turning**: Interpolated rotation for natural bike movement

### 5. Testing Requirements
```cpp
// Unit test example for movement
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBikeMovementTest,
    "BikeAdventure.Core.Movement.ForwardMovement",
    EAutomationTestFlags::ApplicationContextMask)

bool FBikeMovementTest::RunTest(const FString& Parameters)
{
    // Test continuous forward movement
    // Test turning mechanics
    // Test intersection detection
    return true;
}
```

### 6. Success Validation
- ✅ UE5 project compiles without errors
- ✅ Bike character spawns and moves forward automatically  
- ✅ Left/right input controls turning
- ✅ Basic intersection detection triggers
- ✅ 60+ FPS performance on target hardware
- ✅ Unit tests pass for core movement

### 7. Integration Points
- **Stream 2**: Provides world generation framework
- **Stream 4**: Validates all implementations with automated tests
- **Stream 5**: Ensures cross-platform compilation

## Implementation Notes
- Start with Blueprint prototyping for rapid iteration
- Convert performance-critical code to C++
- Follow UE5 coding standards from `/docs/Standards/`
- Use Git LFS for any binary assets created