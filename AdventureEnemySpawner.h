#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdventureEnemySpawner.generated.h"

USTRUCT(BlueprintType)
struct FAdvSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APawn> PawnClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxCount = 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAdvSpawnerWaveStarted, AActor*, Spawner, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAdvSpawnerWaveEnded, AActor*, Spawner, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAdvSpawnerEnemySpawned, AActor*, Spawner, APawn*, Pawn);

UCLASS()
class SIMPLESHOOTER_API AAdventureEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AAdventureEnemySpawner();

	UFUNCTION(BlueprintCallable, Category="Spawner")
	void StartNextWave();

	UFUNCTION(BlueprintCallable, Category="Spawner")
	void StartWave(int32 WaveIndex);

	UFUNCTION(BlueprintCallable, Category="Spawner")
	void StopSpawning();

	UFUNCTION(BlueprintCallable, Category="Spawner")
	bool IsWaveActive() const { return bWaveActive; }

	UFUNCTION(BlueprintCallable, Category="Spawner")
	int32 GetCurrentWave() const { return CurrentWave; }

	UFUNCTION(BlueprintCallable, Category="Spawner")
	int32 GetAliveCount() const;

	UFUNCTION(BlueprintCallable, Category="Spawner")
	void DespawnAll();

	UFUNCTION(BlueprintCallable, Category="Spawner")
	void SetThreatLocation(FVector InThreat) { ThreatLocation = InThreat; bHasThreat = true; }

	UPROPERTY(BlueprintAssignable, Category="Spawner")
	FAdvSpawnerWaveStarted OnWaveStarted;

	UPROPERTY(BlueprintAssignable, Category="Spawner")
	FAdvSpawnerWaveEnded OnWaveEnded;

	UPROPERTY(BlueprintAssignable, Category="Spawner")
	FAdvSpawnerEnemySpawned OnEnemySpawned;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Spawner")
	TArray<FAdvSpawnEntry> SpawnTable;

	UPROPERTY(EditDefaultsOnly, Category="Spawner")
	int32 MaxSimultaneousAlive = 8;

	UPROPERTY(EditDefaultsOnly, Category="Spawner")
	float SpawnInterval = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category="Spawner")
	float SpawnRadius = 500.f;

	UPROPERTY(EditDefaultsOnly, Category="Spawner")
	float LeashRadius = 6000.f;

	UPROPERTY(EditDefaultsOnly, Category="Spawner")
	bool bRequireNavProjection = true;

	UPROPERTY(EditDefaultsOnly, Category="Spawner")
	bool bSpawnFacingThreat = true;

	UPROPERTY(EditDefaultsOnly, Category="Spawner")
	bool bEndWaveWhenQuotaMetAndDead = true;

	UPROPERTY(EditDefaultsOnly, Category="Spawner")
	bool bLoopWaves = false;

	UPROPERTY(EditDefaultsOnly, Category="Debug")
	bool bDebug = false;

private:
	int32 CurrentWave = -1;
	bool bWaveActive = false;

	int32 WaveQuotaToSpawn = 0;
	int32 WaveSpawnedSoFar = 0;

	UPROPERTY()
	TArray<TWeakObjectPtr<APawn>> AlivePawns;

	FTimerHandle SpawnTimer;

	bool bHasThreat = false;
	FVector ThreatLocation = FVector::ZeroVector;

	void TickSpawn();
	bool CanSpawnMore() const;

	const FAdvSpawnEntry* PickEntry(FRandomStream& Rng) const;
	FVector PickSpawnLocation(FRandomStream& Rng) const;

	void TrackPawn(APawn* Pawn);
	void CleanupDead();

	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);
};
