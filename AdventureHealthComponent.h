#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AdventureHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FAdvOnHealthChanged, UAdventureHealthComponent*, Comp, float, NewHealth, float, Delta, AActor*, InstigatorActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAdvOnDeath, AActor*, DeadActor, AActor*, KillerActor);

UCLASS(ClassGroup=(Adventure), meta=(BlueprintSpawnableComponent))
class SIMPLESHOOTER_API UAdventureHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAdventureHealthComponent();

	UFUNCTION(BlueprintCallable, Category="Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable, Category="Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable, Category="Health")
	bool IsDead() const { return bDead; }

	UFUNCTION(BlueprintCallable, Category="Health")
	void SetDamageMultiplier(float NewMultiplier);

	UFUNCTION(BlueprintCallable, Category="Health")
	bool Heal(float Amount, AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable, Category="Health")
	bool ApplyDamage(float Amount, AActor* InstigatorActor);

	UFUNCTION(BlueprintCallable, Category="Health")
	void Revive(float NewHealthRatio = 1.0f);

	UPROPERTY(BlueprintAssignable, Category="Health")
	FAdvOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="Health")
	FAdvOnDeath OnDeath;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
		class AController* InstigatedBy, AActor* DamageCauser);

	void HandleDeath(AActor* Killer);

	void TickRegen(float DeltaSeconds);

	UPROPERTY(EditDefaultsOnly, Category="Health")
	float MaxHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, Category="Health")
	float RegenPerSecond = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category="Health")
	float RegenDelay = 4.0f;

	UPROPERTY(EditDefaultsOnly, Category="Health")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category="Health")
	bool bGodMode = false;

private:
	UPROPERTY(VisibleAnywhere, Category="Health")
	float Health = 100.f;

	UPROPERTY(VisibleAnywhere, Category="Health")
	bool bDead = false;

	float TimeSinceLastDamage = 9999.f;

	FTimerHandle RegenTimer;
};
