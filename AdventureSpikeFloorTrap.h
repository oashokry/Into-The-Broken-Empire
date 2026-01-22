#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdventureSpikeFloorTrap.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class SIMPLESHOOTER_API AAdventureSpikeFloorTrap : public AActor
{
	GENERATED_BODY()

public:
	AAdventureSpikeFloorTrap();

	UFUNCTION(BlueprintCallable, Category="Trap")
	bool IsActive() const { return bActive; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* DamageVolume;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Visual;

	UPROPERTY(EditDefaultsOnly, Category="Trap")
	float DamagePerTick = 22.f;

	UPROPERTY(EditDefaultsOnly, Category="Trap")
	float TickInterval = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category="Trap")
	float ActiveSeconds = 1.25f;

	UPROPERTY(EditDefaultsOnly, Category="Trap")
	float InactiveSeconds = 1.75f;

	UPROPERTY(EditDefaultsOnly, Category="Trap")
	float WarmupSeconds = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category="Trap")
	bool bStartActive = false;

	UPROPERTY(EditDefaultsOnly, Category="Trap")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category="Debug")
	bool bDebug = false;

private:
	bool bActive = false;
	bool bWarming = false;

	FTimerHandle CycleTimer;
	FTimerHandle DamageTimer;

	void SetTrapState(bool bNewActive, bool bNewWarming);
	void StartCycle();
	void DoDamageTick();

	TSet<TWeakObjectPtr<AActor>> Overlapping;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
