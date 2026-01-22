#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdventureWaveDirector.generated.h"

class AAdventureEnemySpawner;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAdvPhaseChanged, int32, Phase);

UCLASS()
class SIMPLESHOOTER_API AAdventureWaveDirector : public AActor
{
	GENERATED_BODY()

public:
	AAdventureWaveDirector();

	UFUNCTION(BlueprintCallable, Category="Waves")
	void RegisterSpawner(AAdventureEnemySpawner* Spawner);

	UFUNCTION(BlueprintCallable, Category="Waves")
	void UnregisterSpawner(AAdventureEnemySpawner* Spawner);

	UFUNCTION(BlueprintCallable, Category="Waves")
	void StartEncounter();

	UFUNCTION(BlueprintCallable, Category="Waves")
	void EndEncounter();

	UFUNCTION(BlueprintCallable, Category="Waves")
	bool IsEncounterActive() const { return bEncounterActive; }

	UFUNCTION(BlueprintCallable, Category="Waves")
	void SetThreatLocation(FVector Threat);

	UFUNCTION(BlueprintCallable, Category="Waves")
	int32 GetPhase() const { return PhaseIndex; }

	UFUNCTION(BlueprintCallable, Category="Waves")
	bool AreAllWavesCleared() const;

	UPROPERTY(BlueprintAssignable, Category="Waves")
	FAdvPhaseChanged OnPhaseChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Waves")
	TArray<TSoftObjectPtr<AAdventureEnemySpawner>> InitialSpawners;

	UPROPERTY(EditDefaultsOnly, Category="Waves")
	int32 MaxPhases = 3;

	UPROPERTY(EditDefaultsOnly, Category="Waves")
	float PhaseDelay = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category="Waves")
	bool bEscalateQuotas = true;

	UPROPERTY(EditDefaultsOnly, Category="Waves")
	bool bStopAfterMaxPhase = true;

	UPROPERTY(EditDefaultsOnly, Category="Debug")
	bool bDebug = false;

private:
	UPROPERTY()
	TArray<AAdventureEnemySpawner*> Spawners;

	bool bEncounterActive = false;
	int32 PhaseIndex = 0;

	bool bHasThreat = false;
	FVector ThreatLocation = FVector::ZeroVector;

	FTimerHandle PhaseTimer;

	void StartPhase(int32 NewPhase);
	void TryAdvancePhase();

	UFUNCTION()
	void HandleSpawnerWaveEnded(AActor* SpawnerActor, int32 WaveIndex);
};
