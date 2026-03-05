#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Core/BiomeTypes.h"
#include "DiscoverySystem.generated.h"

USTRUCT(BlueprintType)
struct FDiscoveryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discovery")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discovery")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discovery")
	EBiomeType RequiredBiome = EBiomeType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discovery")
	bool bIsDiscovered = false;
};

UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BIKEADVENTURE_API UDiscoverySystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDiscoverySystem();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Discovery System")
	void Initialize();

	UFUNCTION(BlueprintCallable, Category = "Discovery System")
	void HandleBiomeEntered(EBiomeType BiomeType);

	UFUNCTION(BlueprintCallable, Category = "Discovery System")
	bool TriggerDiscovery(const FString& DiscoveryName);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discovery System")
	int32 GetTotalDiscoveriesFound() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discovery System")
	int32 GetTotalDiscoveriesAvailable() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discovery System")
	TArray<FDiscoveryData> GetDiscoveredItems() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discovery Data")
	TArray<FDiscoveryData> AvailableDiscoveries;

	void LoadDefaultDiscoveries();
};
