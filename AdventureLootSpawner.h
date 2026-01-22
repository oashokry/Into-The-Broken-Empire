#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdventureLootSpawner.generated.h"

USTRUCT(BlueprintType)
struct FAdvLootEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> LootClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Rarity = 0.0f; // 0 common, 1 rare-ish, etc
};

UCLASS()
class SIMPLESHOOTER_API AAdventureLootSpawner : public AActor
{
	GENERATED_BODY()

public:
	AAdventureLootSpawner();

	UFUNCTION(BlueprintCallable, Category="Loot")
	void SpawnLootNow();

	UFUNCTION(BlueprintCallable, Category="Loot")
	void ClearSpawned();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Loot")
	TArray<FAdvLootEntry> LootTable;

	UPROPERTY(EditDefaultsOnly, Category="Loot")
	int32 Rolls = 2;

	UPROPERTY(EditDefaultsOnly, Category="Loot")
	float NoSpawnChance = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category="Loot")
	float SpreadRadius = 85.f;

	UPROPERTY(EditDefaultsOnly, Category="Loot")
	bool bOneShot = true;

	UPROPERTY(EditDefaultsOnly, Category="Loot")
	bool bPreferUnique = true;

private:
	bool bUsed = false;

	UPROPERTY()
	TArray<AActor*> Spawned;

	const FAdvLootEntry* PickEntry(FRandomStream& Rng, TSet<TSubclassOf<AActor>>& Used) const;
	FVector RandomPointInCircle(FRandomStream& Rng) const;
};
