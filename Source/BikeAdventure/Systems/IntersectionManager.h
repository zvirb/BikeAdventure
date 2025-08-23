#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/ActorComponent.h"
#include "IntersectionManager.generated.h"

class AIntersection;

/**
 * Manages all intersections in the game world
 * Handles intersection spawning, tracking, and coordination
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BIKEADVENTURE_API UIntersectionManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UIntersectionManager();

	/** Initialize the intersection management system */
	UFUNCTION(BlueprintCallable, Category = "Intersection Manager")
	void Initialize();

	/** Register an intersection with the manager */
	UFUNCTION(BlueprintCallable, Category = "Intersection Manager")
	void RegisterIntersection(AIntersection* Intersection);

	/** Unregister an intersection from the manager */
	UFUNCTION(BlueprintCallable, Category = "Intersection Manager")
	void UnregisterIntersection(AIntersection* Intersection);

	/** Get all registered intersections */
	UFUNCTION(BlueprintCallable, Category = "Intersection Manager")
	TArray<AIntersection*> GetAllIntersections() const { return RegisteredIntersections; }

	/** Get intersection count */
	UFUNCTION(BlueprintCallable, Category = "Intersection Manager")
	int32 GetIntersectionCount() const { return RegisteredIntersections.Num(); }

protected:
	/** Array of all registered intersections */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	TArray<TObjectPtr<AIntersection>> RegisteredIntersections;

	/** Whether the system has been initialized */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bInitialized = false;
};