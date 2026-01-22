#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdventureGrenade.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class USoundBase;

UCLASS()
class SIMPLESHOOTER_API AAdventureGrenade : public AActor
{
	GENERATED_BODY()

public:
	AAdventureGrenade();

	UFUNCTION(BlueprintCallable, Category="Grenade")
	void Arm(AController* InstigatorController, float OverrideFuse = -1.f);

	UFUNCTION(BlueprintCallable, Category="Grenade")
	void SetThrowVelocity(const FVector& Velocity);

	UFUNCTION(BlueprintCallable, Category="Grenade")
	bool IsArmed() const { return bArmed; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* Collision;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* Projectile;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	float FuseSeconds = 2.5f;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	float Damage = 90.f;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	float InnerRadius = 240.f;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	float OuterRadius = 650.f;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	float ImpulseStrength = 1700.f;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	int32 ShrapnelTraces = 18;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	float ShrapnelRange = 900.f;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	float ShrapnelDamage = 18.f;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	UParticleSystem* ExplosionFX;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	USoundBase* ExplosionSFX;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	bool bDrawDebug = false;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

private:
	bool bArmed = false;
	bool bExploded = false;

	UPROPERTY()
	AController* OwningController = nullptr;

	FTimerHandle FuseTimer;

	void Explode();

	void DoRadialDamage();
	void DoImpulse();
	void DoShrapnel();

	FVector RandomUnitVectorFast(int32 Seed) const;
};
