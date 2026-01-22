#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdventureLorePickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAdvLoreCollected, FName, LoreId);

UCLASS()
class SIMPLESHOOTER_API AAdventureLorePickup : public AActor
{
	GENERATED_BODY()

public:
	AAdventureLorePickup();

	UPROPERTY(BlueprintAssignable, Category="Lore")
	FAdvLoreCollected OnCollected;

	UFUNCTION(BlueprintCallable, Category="Lore")
	void Collect(AActor* Collector);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* Trigger;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly, Category="Lore")
	FName LoreId = TEXT("Lore_Default");

	UPROPERTY(EditDefaultsOnly, Category="Lore")
	FText LoreTitle;

	UPROPERTY(EditDefaultsOnly, Category="Lore")
	FText LoreBody;

	UPROPERTY(EditDefaultsOnly, Category="Lore")
	bool bOneShot = true;

	UPROPERTY(EditDefaultsOnly, Category="Lore")
	bool bAddInventoryItem = true;

	UPROPERTY(EditDefaultsOnly, Category="Lore")
	FName InventoryItemId = TEXT("LoreToken");

	UPROPERTY(EditDefaultsOnly, Category="Lore")
	bool bAdvanceObjective = true;

	UPROPERTY(EditDefaultsOnly, Category="Lore")
	FName ObjectiveId = TEXT("OBJ_CollectLore");

	UPROPERTY(EditDefaultsOnly, Category="Lore")
	int32 ObjectiveDelta = 1;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

private:
	bool bUsed = false;
};
