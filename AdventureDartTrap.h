#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdventureDartTrap.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class SIMPLESHOOTER_API AAdventureDartTrap : public AActor
{
	GENERATED_BODY()

public:
	AAdventureDartTrap();

	UFUNCTION(BlueprintCallable, Category="Trap")
	void Fire();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Base;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* Trigger;

	UPROPERTY(EditDefaultsOnly, Category="Trap")
	float Damage = 35.f;

	UPROPERTY(EditDefaultsOnly, Category="Trap")
	float Range = 2400.f;

	UPROPERTY(EditDefaultsOnly, Category="Trap")
	float Cooldown = 1.25f;

	UPROPERTY(EditDefaultsOnly, Category="Trap")
	float BurstCount = 3;

	UPROPERTY(EditDefaultsOnly, Category="Trap")
	float BurstInterval = 0.12f;

	UPROPERTY(EditDefaultsOnly, Category="Trap")
	FName MuzzleSocket = TEXT("Muzzle");

	UPROPERTY(EditDefaultsOnly, Category="Trap")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category="Debug")
	bool bDebug = false;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

private:
	float LastFireTime = -9999.f;

	void FireOne();

	FTimerHandle BurstTimer;
	int32 BurstRemaining = 0;
};
