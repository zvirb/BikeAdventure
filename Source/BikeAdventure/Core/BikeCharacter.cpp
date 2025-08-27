#include "BikeCharacter.h"
#include "Systems/BikeMovementComponent.h"
#include "Gameplay/Intersection.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"

ABikeCharacter::ABikeCharacter()
{
	// Set this pawn to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	SetupComponents();
}

void ABikeCharacter::SetupComponents()
{
	// Create and set up collision capsule
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	RootComponent = CapsuleComponent;
	CapsuleComponent->SetCapsuleHalfHeight(88.0f);
	CapsuleComponent->SetCapsuleRadius(34.0f);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleComponent->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	CapsuleComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	CapsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// Create bike mesh
	BikeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BikeMesh"));
	BikeMesh->SetupAttachment(RootComponent);
	BikeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create camera boom
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 600.0f;
	SpringArm->bUsePawnControlRotation = false; // Don't rotate with controller input
	SpringArm->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f)); // Slight downward angle
	SpringArm->bDoCollisionTest = true;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	// Create camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // Camera doesn't rotate relative to arm

	// Create custom movement component
	BikeMovement = CreateDefaultSubobject<UBikeMovementComponent>(TEXT("BikeMovement"));

	// Bind overlap events for intersection detection
	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &ABikeCharacter::OnCapsuleBeginOverlap);
	CapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &ABikeCharacter::OnCapsuleEndOverlap);
}

void ABikeCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize movement component with this pawn
	if (BikeMovement)
	{
		BikeMovement->UpdatedComponent = RootComponent;
		BikeMovement->SetUpdatedComponent(RootComponent);
	}

	UE_LOG(LogTemp, Warning, TEXT("Bike Character spawned and initialized"));
}

void ABikeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

        // Update movement component
        if (BikeMovement && RootComponent)
        {
                BikeMovement->SetSteering(SteeringInput);
               BikeMovement->SetThrottle(ThrottleInput);
                BikeMovement->UpdateMovement(DeltaTime);
        }
}

void ABikeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
        Super::SetupPlayerInputComponent(PlayerInputComponent);

        // Bind traditional input actions
       PlayerInputComponent->BindAxis("Turn", this, &ABikeCharacter::HandleTurnInput);
       PlayerInputComponent->BindAxis("Throttle", this, &ABikeCharacter::HandleThrottleInput);
        PlayerInputComponent->BindAction("LeftChoice", IE_Pressed, this, &ABikeCharacter::HandleLeftChoice);
        PlayerInputComponent->BindAction("RightChoice", IE_Pressed, this, &ABikeCharacter::HandleRightChoice);
}

void ABikeCharacter::HandleTurnInput(float Value)
{
        SteeringInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

void ABikeCharacter::HandleThrottleInput(float Value)
{
       ThrottleInput = FMath::Clamp(Value, 0.0f, 1.0f);
}

void ABikeCharacter::HandleLeftChoice()
{
	if (IsAtIntersection() && CurrentIntersection)
	{
		CurrentIntersection->HandlePlayerChoice(false); // false = left
		UE_LOG(LogTemp, Log, TEXT("Player chose LEFT path at intersection"));
	}
}

void ABikeCharacter::HandleRightChoice()
{
	if (IsAtIntersection() && CurrentIntersection)
	{
		CurrentIntersection->HandlePlayerChoice(true); // true = right
		UE_LOG(LogTemp, Log, TEXT("Player chose RIGHT path at intersection"));
	}
}

float ABikeCharacter::GetCurrentSpeed() const
{
	if (BikeMovement)
	{
		return BikeMovement->GetCurrentSpeed();
	}
	return 0.0f;
}

void ABikeCharacter::OnIntersectionReached(AIntersection* Intersection)
{
	if (Intersection && !CurrentIntersection)
	{
		CurrentIntersection = Intersection;
		UE_LOG(LogTemp, Warning, TEXT("Reached intersection: %s"), *Intersection->GetName());
		
		// Notify intersection that player has arrived
		Intersection->OnPlayerArrived(this);
		
		// Optional: Slow down at intersection for decision making
		if (BikeMovement)
		{
			BikeMovement->SetIntersectionMode(true);
		}
	}
}

void ABikeCharacter::OnIntersectionLeft(AIntersection* Intersection)
{
	if (Intersection && CurrentIntersection == Intersection)
	{
		UE_LOG(LogTemp, Log, TEXT("Left intersection: %s"), *Intersection->GetName());
		
		// Notify intersection that player has left
		Intersection->OnPlayerLeft(this);
		
		// Resume normal speed
		if (BikeMovement)
		{
			BikeMovement->SetIntersectionMode(false);
		}
		
		CurrentIntersection = nullptr;
	}
}

void ABikeCharacter::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if we overlapped with an intersection
	if (AIntersection* Intersection = Cast<AIntersection>(OtherActor))
	{
		OnIntersectionReached(Intersection);
	}
}

void ABikeCharacter::OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Check if we left an intersection
	if (AIntersection* Intersection = Cast<AIntersection>(OtherActor))
	{
		OnIntersectionLeft(Intersection);
	}
}