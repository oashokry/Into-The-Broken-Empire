#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AdventureCombatComponent.generated.h"

class AAdventureWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAdvHitMarker, AActor*, HitActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAdvADSChanged, bool, bIsADS, float, FOV);

USTRUCT(BlueprintType)
struct FAdvRecoilState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Pitch = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Yaw = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReturnSpeed = 18.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float KickSpeed = 35.f;
};

UCLASS(ClassGroup=(Adventure), meta=(BlueprintSpawnableComponent))
class SIMPLESHOOTER_API UAdventureCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAdventureCombatComponent();

	UFUNCTION(BlueprintCallable, Category="Combat")
	void SetWeapon(AAdventureWeapon* NewWeapon);

	UFUNCTION(BlueprintCallable, Category="Combat")
	AAdventureWeapon* GetWeapon() const { return Weapon; }

	UFUNCTION(BlueprintCallable, Category="Combat")
	void SetADS(bool bNewADS);

	UFUNCTION(BlueprintCallable, Category="Combat")
	bool IsADS() const { return bADS; }

	UFUNCTION(BlueprintCallable, Category="Combat")
	float GetCurrentFOV() const { return CurrentFOV; }

	UFUNCTION(BlueprintCallable, Category="Combat")
	void PullTrigger();

	UFUNCTION(BlueprintCallable, Category="Combat")
	void ReleaseTrigger();

	UFUNCTION(BlueprintCallable, Category="Combat")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category="Combat")
	void StopSprint();

	UPROPERTY(BlueprintAssignable, Category="Combat")
	FAdvHitMarker OnHitMarker;

	UPROPERTY(BlueprintAssignable, Category="Combat")
	FAdvADSChanged OnADSChanged;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	AAdventureWeapon* Weapon = nullptr;

	UPROPERTY()
	APawn* OwnerPawn = nullptr;

	UPROPERTY()
	APlayerController* OwnerPC = nullptr;

	FTimerHandle AutoFireTimer;

	bool bADS = false;
	bool bSprinting = false;
	bool bTriggerHeld = false;

	float DefaultFOV = 90.f;
	float ADSFOV = 65.f;
	float CurrentFOV = 90.f;
	float FOVInterpSpeed = 12.f;

	float BaseSpread = 1.25f;
	float ADSSpread = 0.35f;
	float SprintSpread = 2.5f;

	float SwayAmount = 0.25f;
	float SwaySpeed = 2.0f;
	float SwayTime = 0.f;

	FAdvRecoilState Recoil;

	void TickSoft(float DeltaSeconds);

	void ApplyViewKick();
	void ApplyWeaponSway(float DeltaSeconds);

	float ComputeSpread() const;
	FVector ComputeAimTargetWithSpread(const FVector& TraceStart, const FVector& AimDir) const;

	void FireOnce();
	bool CanFire() const;

	void SetFOVInternal(float TargetFOV);
	FTimerHandle SoftTickTimer;
};
