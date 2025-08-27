#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "BikeCharacter.generated.h"

class UBikeMovementComponent;
class AIntersection;

/**
 * Physics-based bike character for meditative exploration
 * Features manual throttle and steering control
 */
UCLASS()
class BIKEADVENTURE_API ABikeCharacter : public APawn
{
	GENERATED_BODY()

public:
	ABikeCharacter();

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

	/** Called to bind functionality to input */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//~ Components
	
	/** Collision capsule for the bike */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	/** Visual representation of the bike */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BikeMesh;

	/** Camera boom positioning the camera behind the bike */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	/** Custom movement component handling physics-based bike movement */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBikeMovementComponent> BikeMovement;

	//~ Input Handling

        /** Handle left/right turning input */
        UFUNCTION(BlueprintCallable, Category = "Input")
        void HandleTurnInput(float Value);

       /** Handle throttle input */
       UFUNCTION(BlueprintCallable, Category = "Input")
       void HandleThrottleInput(float Value);

	/** Handle intersection choice input (left direction) */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void HandleLeftChoice();

	/** Handle intersection choice input (right direction) */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void HandleRightChoice();

	//~ Gameplay Events

	/** Called when the bike reaches an intersection */
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	virtual void OnIntersectionReached(AIntersection* Intersection);

	/** Called when the bike leaves an intersection */
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	virtual void OnIntersectionLeft(AIntersection* Intersection);

	/** Called when a discovery is made */
	UFUNCTION(BlueprintImplementableEvent, Category = "Gameplay")
	void OnDiscoveryMade(const FString& DiscoveryName, const FString& DiscoveryDescription);

public:
	/** Get the bike movement component */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	UBikeMovementComponent* GetBikeMovement() const { return BikeMovement; }

	/** Get current speed in cm/s */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetCurrentSpeed() const;

	/** Check if bike is currently at an intersection */
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	bool IsAtIntersection() const { return CurrentIntersection != nullptr; }

	/** Get the current intersection (if any) */
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	AIntersection* GetCurrentIntersection() const { return CurrentIntersection; }

protected:
        /** Current intersection the bike is at (null if not at intersection) */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay")
        TObjectPtr<AIntersection> CurrentIntersection;

        /** Current steering input value (-1 to 1) */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
        float SteeringInput = 0.0f;

       /** Current throttle input value (0 to 1) */
       UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
       float ThrottleInput = 0.0f;

private:
	/** Set up default component values and relationships */
	void SetupComponents();

	/** Handle intersection overlap events */
	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};