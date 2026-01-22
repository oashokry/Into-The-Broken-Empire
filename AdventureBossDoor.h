#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdventureBossDoor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class AAdventureWaveDirector;

UCLASS()
class SIMPLESHOOTER_API AAdventureBossDoor : public AActor
{
	GENERATED_BODY()

public:
	AAdventureBossDoor();

	UFUNCTION(BlueprintCallable, Category="BossDoor")
	void TryOpen(AActor* User);

	UFUNCTION(BlueprintCallable, Category="BossDoor")
	void ForceUnlock();

	UFUNCTION(BlueprintCallable, Category="BossDoor")
	bool IsUnlocked() const { return bUnlocked; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* UseZone;

	UPROPERTY(EditDefaultsOnly, Category="BossDoor")
	bool bUnlocked = false;

	UPROPERTY(EditDefaultsOnly, Category="BossDoor")
	float OpenYaw = 110.f;

	UPROPERTY(EditDefaultsOnly, Category="BossDoor")
	float OpenSpeed = 3.5f;

	UPROPERTY(EditDefaultsOnly, Category="BossDoor")
	bool bRequireKeyItems = true;

	UPROPERTY(EditDefaultsOnly, Category="BossDoor")
	TArray<FName> RequiredKeys;

	UPROPERTY(EditDefaultsOnly, Category="BossDoor")
	bool bRequireObjectives = true;

	UPROPERTY(EditDefaultsOnly, Category="BossDoor")
	TArray<FName> RequiredObjectives;

	UPROPERTY(EditDefaultsOnly, Category="BossDoor")
	bool bRequireWavesCleared = true;

	UPROPERTY(EditDefaultsOnly, Category="BossDoor")
	AAdventureWaveDirector* WaveDirector = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="BossDoor")
	float CinematicDelay = 0.75f;

	UPROPERTY(EditDefaultsOnly, Category="Debug")
	bool bDebug = false;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

private:
	bool bOpen = false;
	float CurrentYaw = 0.f;
	float TargetYaw = 0.f;

	FTimerHandle DoorTickTimer;
	FTimerHandle OpenDelayTimer;

	void TickDoor(float DeltaSeconds);

	bool MeetsRequirements(AActor* User) const;
	bool HasAllKeys(AActor* User) const;
	bool HasAllObjectives() const;
};
