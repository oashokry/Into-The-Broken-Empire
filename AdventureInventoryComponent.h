#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AdventureInventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FAdventureInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsKeyItem = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAdvOnInventoryChanged, const TArray<FAdventureInventoryItem>&, Items);

UCLASS(ClassGroup=(Adventure), meta=(BlueprintSpawnableComponent))
class SIMPLESHOOTER_API UAdventureInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAdventureInventoryComponent();

	UFUNCTION(BlueprintCallable, Category="Inventory")
	const TArray<FAdventureInventoryItem>& GetItems() const { return Items; }

	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool HasItem(FName ItemId, int32 MinCount = 1) const;

	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 GetCount(FName ItemId) const;

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void AddItem(const FAdventureInventoryItem& Item);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool RemoveItem(FName ItemId, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void Clear();

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FAdvOnInventoryChanged OnInventoryChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	int32 MaxDistinctItems = 32;

private:
	UPROPERTY()
	TArray<FAdventureInventoryItem> Items;

	int32 FindIndex(FName ItemId) const;
	void Broadcast();
};
