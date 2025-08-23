#include "BikeMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UBikeMovementComponent::UBikeMovementComponent()
{
	// Set this component to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// Initialize default values optimized for meditative bike experience
	ForwardSpeed = 1200.0f;        // 12 m/s - comfortable exploration speed
	IntersectionSpeed = 600.0f;    // 6 m/s - slower for decision making
	MaxTurnRate = 45.0f;           // 45 degrees/second - smooth turning
	SteeringResponsiveness = 2.0f;  // Responsive but not twitchy
	TiltAngle = 15.0f;             // Subtle visual feedback
	TiltSpeed = 3.0f;              // Smooth tilt animation

	// Physics parameters for realistic feel
	Friction = 0.3f;               // Moderate friction
	AirResistance = 0.01f;         // Minimal air resistance
	GroundTraceDistance = 150.0f;   // 1.5 meter ground detection

	// Initialize state
	CurrentForwardSpeed = 0.0f;
	SteeringValue = 0.0f;
	CurrentTurnRate = 0.0f;
	CurrentTiltAngle = 0.0f;
	bIntersectionMode = false;
	bOnGround = true;
}

void UBikeMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Start with initial forward speed
	CurrentForwardSpeed = GetTargetForwardSpeed();
}

void UBikeMovementComponent::UpdateMovement(float DeltaTime)
{
	if (!UpdatedComponent || !PawnOwner)
	{
		return;
	}

	// Check ground contact
	bOnGround = CheckGroundContact();

	// Only apply movement if we're on the ground
	if (bOnGround)
	{
		// Update different aspects of movement
		UpdateForwardMovement(DeltaTime);
		UpdateSteering(DeltaTime);
		UpdateVisualEffects(DeltaTime);

		// Calculate and apply final movement vector
		FVector ForwardVector = UpdatedComponent->GetForwardVector();
		FVector MovementVector = ForwardVector * CurrentForwardSpeed * DeltaTime;

		ApplyMovement(MovementVector, DeltaTime);
	}
}

void UBikeMovementComponent::SetSteering(float SteeringInput)
{
	SteeringValue = FMath::Clamp(SteeringInput, -1.0f, 1.0f);
}

void UBikeMovementComponent::SetIntersectionMode(bool bEnabled)
{
	bIntersectionMode = bEnabled;
	UE_LOG(LogTemp, Log, TEXT("Intersection mode %s"), bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UBikeMovementComponent::UpdateForwardMovement(float DeltaTime)
{
	float TargetSpeed = GetTargetForwardSpeed();
	
	// Smooth acceleration/deceleration to target speed
	float AccelerationRate = (CurrentForwardSpeed < TargetSpeed) ? 800.0f : 1200.0f; // Faster deceleration
	CurrentForwardSpeed = SmoothInterp(CurrentForwardSpeed, TargetSpeed, AccelerationRate, DeltaTime);

	// Apply air resistance (subtle effect)
	CurrentForwardSpeed *= (1.0f - AirResistance * DeltaTime);
	
	// Ensure minimum speed (bike never stops completely)
	CurrentForwardSpeed = FMath::Max(CurrentForwardSpeed, TargetSpeed * 0.1f);
}

void UBikeMovementComponent::UpdateSteering(float DeltaTime)
{
	// Calculate target turn rate based on steering input
	float TargetTurnRate = SteeringValue * MaxTurnRate;
	
	// Smooth steering interpolation for natural feel
	CurrentTurnRate = SmoothInterp(CurrentTurnRate, TargetTurnRate, SteeringResponsiveness * MaxTurnRate, DeltaTime);

	// Apply rotation if we have turn rate
	if (FMath::Abs(CurrentTurnRate) > 0.1f)
	{
		float RotationAmount = CurrentTurnRate * DeltaTime;
		FRotator DeltaRotation(0.0f, RotationAmount, 0.0f);
		UpdatedComponent->AddWorldRotation(DeltaRotation);
	}
}

void UBikeMovementComponent::UpdateVisualEffects(float DeltaTime)
{
	// Calculate target tilt based on steering and speed
	float SpeedFactor = FMath::Clamp(CurrentForwardSpeed / ForwardSpeed, 0.0f, 1.0f);
	float TargetTilt = SteeringValue * TiltAngle * SpeedFactor;

	// Smooth tilt interpolation
	CurrentTiltAngle = SmoothInterp(CurrentTiltAngle, TargetTilt, TiltSpeed * TiltAngle, DeltaTime);

	// Apply tilt rotation (roll)
	if (FMath::Abs(CurrentTiltAngle) > 0.1f)
	{
		FRotator CurrentRotation = UpdatedComponent->GetComponentRotation();
		FRotator NewRotation = CurrentRotation;
		NewRotation.Roll = CurrentTiltAngle;
		UpdatedComponent->SetWorldRotation(NewRotation);
	}
}

bool UBikeMovementComponent::CheckGroundContact()
{
	if (!UpdatedComponent || !GetWorld())
	{
		return false;
	}

	FVector StartLocation = UpdatedComponent->GetComponentLocation();
	FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, GroundTraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(PawnOwner);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_WorldStatic,
		QueryParams
	);

	// Optional: Draw debug line in development builds
	#if WITH_EDITOR
	if (CVarDebugBikeMovement.GetValueOnGameThread())
	{
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, bHit ? FColor::Green : FColor::Red, false, 0.0f, 0, 1.0f);
	}
	#endif

	return bHit;
}

void UBikeMovementComponent::ApplyMovement(const FVector& MovementVector, float DeltaTime)
{
	if (!UpdatedComponent)
	{
		return;
	}

	// Apply movement with collision detection
	FHitResult HitResult;
	SafeMoveUpdatedComponent(MovementVector, UpdatedComponent->GetComponentRotation(), true, HitResult);

	// Handle collision
	if (HitResult.bBlockingHit)
	{
		// Slide along the surface
		SlideAlongSurface(MovementVector, 1.0f - HitResult.Time, HitResult.Normal, HitResult);
	}
}

float UBikeMovementComponent::GetTargetForwardSpeed() const
{
	return bIntersectionMode ? IntersectionSpeed : ForwardSpeed;
}

float UBikeMovementComponent::SmoothInterp(float Current, float Target, float Speed, float DeltaTime) const
{
	if (FMath::IsNearlyEqual(Current, Target, 0.1f))
	{
		return Target;
	}
	
	return FMath::FInterpTo(Current, Target, DeltaTime, Speed);
}

// Console variable for debug drawing (declare in header if needed elsewhere)
#if WITH_EDITOR
TAutoConsoleVariable<bool> CVarDebugBikeMovement(
	TEXT("bike.DebugMovement"),
	false,
	TEXT("Enable debug drawing for bike movement"),
	ECVF_Cheat
);
#endif