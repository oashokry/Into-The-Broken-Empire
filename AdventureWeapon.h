#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdventureWeapon.generated.h"

class USkeletalMeshComponent;
class UParticleSystem;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoChanged, int32, NewAmmo);


UCLASS()
class SIMPLESHOOTER_API AAdventureWeapon : public AActor
{
	GENERATED_BODY()

public:
	AAdventureWeapon();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// Core usage
	bool TryFire(const FVector& TraceStart, const FVector& AimPoint, AController* InstigatorController);
	void StartReload();
	bool IsReloading() const { return bReloading; }

	void AttachTo(USkeletalMeshComponent* Parent, FName Socket);
	void SetOwnerController(AController* NewOwnerController) { OwnerController = NewOwnerController; }

	// Ammo
	int32 GetAmmoInMag() const { return AmmoInMag; }
	int32 GetSpareAmmo() const { return SpareAmmo; }
	int32 GetMagCapacity() const { return MagCapacity; }

	// Fire tuning
	void SetWantsAutoFire(bool bInAuto);
	bool IsAutoFiring() const { return bWantsAutoFire; }

	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnAmmoChanged OnAmmoChanged;

protected:
	UPROPERTY(VisibleAnywhere, Category="Components")
	USkeletalMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Sockets")
	FName MuzzleSocket = TEXT("MuzzleFlashSocket");

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Damage")
	float Damage = 22.f;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Damage")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Range")
	float MaxRange = 9000.f;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Fire")
	float FireRate = 0.12f; // seconds between shots

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Fire")
	bool bAutoFireCapable = true;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Fire")
	int32 BurstCount = 3;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Fire")
	float BurstInterval = 0.07f;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Ammo")
	int32 MagCapacity = 15;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Ammo")
	int32 AmmoInMag = 15;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Ammo")
	int32 SpareAmmo = 60;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Ammo")
	bool bInfiniteAmmo = false;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Reload")
	float ReloadTime = 1.35f;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Spread")
	float BaseSpreadDegrees = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Spread")
	float SpreadBloomPerShot = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Spread")
	float SpreadRecoveryPerSecond = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Spread")
	float MaxSpreadDegrees = 3.5f;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Heat")
	float HeatPerShot = 0.12f;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Heat")
	float HeatCoolPerSecond = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Heat")
	float HeatMax = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	UParticleSystem* MuzzleFX;

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	UParticleSystem* ImpactFX;

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	USoundBase* FireSFX;

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	USoundBase* ReloadSFX;

	UPROPERTY(EditDefaultsOnly, Category="Debug")
	bool bDrawDebug = false;

private:
	// ownership
	UPROPERTY()
	AController* OwnerController = nullptr;

	// state
	bool bReloading = false;
	bool bWantsAutoFire = false;
	float LastFireTime = -9999.f;

	float CurrentSpread = 0.f;
	float Heat = 0.f;

	FTimerHandle ReloadTimer;
	FTimerHandle AutoFireTimer;
	FTimerHandle BurstTimer;
	int32 BurstShotsRemaining = 0;

	bool CanFireNow() const;
	void ConsumeAmmo();
	void ApplyRecoilAndSpread();
	FVector ApplySpreadToAim(const FVector& Dir) const;
	bool TraceHitscan(const FVector& Start, const FVector& End, FHitResult& OutHit) const;

	void FireOnceInternal(const FVector& TraceStart, const FVector& AimPoint);
	void BeginAutoTick(const FVector& TraceStart, const FVector& AimPoint);
	void StopAutoTick();
	void AutoTickFire(const FVector TraceStart, const FVector AimPoint);

	void BeginBurst(const FVector& TraceStart, const FVector& AimPoint);
	void BurstTickFire(const FVector TraceStart, const FVector AimPoint);

	void FinishReload();
	void BroadcastAmmo();
};
