#pragma once

#include "CoreMinimal.h"
#include "AdventureWeapon.h"
#include "AdventureGrenadeLauncherWeapon.generated.h"

class AAdventureGrenade;

UCLASS()
class SIMPLESHOOTER_API AAdventureGrenadeLauncherWeapon : public AAdventureWeapon
{
	GENERATED_BODY()

public:
	AAdventureGrenadeLauncherWeapon();

	UFUNCTION(BlueprintCallable, Category="GL")
	void SetGrenadeClass(TSubclassOf<AAdventureGrenade> InClass) { GrenadeClass = InClass; }

protected:
	UPROPERTY(EditDefaultsOnly, Category="GL")
	TSubclassOf<AAdventureGrenade> GrenadeClass;

	UPROPERTY(EditDefaultsOnly, Category="GL")
	float LaunchSpeed = 1900.f;

	UPROPERTY(EditDefaultsOnly, Category="GL")
	float ArcBoostZ = 0.45f;

	UPROPERTY(EditDefaultsOnly, Category="GL")
	float FuseOverride = 2.25f;

	UPROPERTY(EditDefaultsOnly, Category="GL")
	float SpawnForwardOffset = 35.f;

public:
	// overriding fire with “shoot but explosive”
	virtual bool FireAdvanced(AController* InstigatorController, const FVector& TraceStart, const FVector& AimTarget, AActor*& OutHitActor) override;
};
