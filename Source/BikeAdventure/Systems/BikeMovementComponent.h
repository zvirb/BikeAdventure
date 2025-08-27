#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Engine/Engine.h"
#include "BikeMovementComponent.generated.h"

/**
 * Custom movement component for physics-based bike movement
 * Handles player-controlled forward movement with smooth turning mechanics
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BIKEADVENTURE_API UBikeMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	UBikeMovementComponent();

protected:
	virtual void BeginPlay() override;

public:
        /** Called every frame to update movement */
        virtual void UpdateMovement(float DeltaTime);

        /** Set steering input (-1.0 to 1.0) */
        void SetSteering(float SteeringInput);

       /** Set throttle input (0.0 to 1.0) */
       void SetThrottle(float ThrottleInput);

	/** Get current forward speed */
	float GetCurrentSpeed() const { return CurrentForwardSpeed; }

	/** Enable/disable intersection mode (slower movement) */
	void SetIntersectionMode(bool bEnabled);

	//~ Movement Parameters

	/** Base forward speed in cm/s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed")
	float ForwardSpeed = 1200.0f;

	/** Speed when at intersections (slower for decision making) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Speed") 
	float IntersectionSpeed = 600.0f;

	/** Maximum turn rate in degrees per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Steering")
	float MaxTurnRate = 45.0f;

	/** How quickly the bike responds to steering input (higher = more responsive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Steering")
	float SteeringResponsiveness = 2.0f;

	/** How much the bike tilts when turning (visual effect) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Visual")
	float TiltAngle = 15.0f;

	/** How quickly the bike tilts and un-tilts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Visual")
	float TiltSpeed = 3.0f;

	//~ Physics Parameters

	/** Friction coefficient affecting how quickly the bike can change direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics")
	float Friction = 0.3f;

	/** Air resistance affecting top speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics")
	float AirResistance = 0.01f;

	/** Ground detection distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics")
	float GroundTraceDistance = 150.0f;

protected:
        /** Current forward velocity */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|State")
        float CurrentForwardSpeed = 0.0f;

        /** Current steering input value */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|State")
        float SteeringValue = 0.0f;

       /** Current throttle input value */
       UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|State")
       float ThrottleValue = 0.0f;

	/** Current turn rate being applied */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|State")
	float CurrentTurnRate = 0.0f;

	/** Current tilt angle for visual feedback */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|State")
	float CurrentTiltAngle = 0.0f;

	/** Whether we're currently in intersection mode */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|State")
	bool bIntersectionMode = false;

	/** Whether we're currently on the ground */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|State")
	bool bOnGround = true;

private:
	/** Update forward movement with physics */
	void UpdateForwardMovement(float DeltaTime);

	/** Update turning and steering */
	void UpdateSteering(float DeltaTime);

	/** Update visual effects (tilting) */
	void UpdateVisualEffects(float DeltaTime);

	/** Check if bike is on the ground */
	bool CheckGroundContact();

	/** Apply movement to the pawn */
	void ApplyMovement(const FVector& MovementVector, float DeltaTime);

	/** Get the target forward speed based on current state */
	float GetTargetForwardSpeed() const;

	/** Smooth interpolation helper */
	float SmoothInterp(float Current, float Target, float Speed, float DeltaTime) const;
};