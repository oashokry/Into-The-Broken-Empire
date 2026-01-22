#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AdventureStaminaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAdvOnStaminaChanged, UAdventureStaminaComponent*, Comp, float, NewValue, float, Delta);

UCLASS(ClassGroup=(Adventure), meta=(BlueprintSpawnableComponent))
class SIMPLESHOOTER_API UAdventureStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAdventureStaminaComponent();

	UFUNCTION(BlueprintCallable, Category="Stamina")
	float GetStamina() const { return Stamina; }

	UFUNCTION(BlueprintCallable, Category="Stamina")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintCallable, Category="Stamina")
	bool HasStamina(float Cost) const;

	UFUNCTION(BlueprintCallable, Category="Stamina")
	bool Spend(float Cost);

	UFUNCTION(BlueprintCallable, Category="Stamina")
	void Refill(float Amount);

	UFUNCTION(BlueprintCallable, Category="Stamina")
	bool IsExhausted() const { return bExhausted; }

	UPROPERTY(BlueprintAssignable, Category="Stamina")
	FAdvOnStaminaChanged OnStaminaChanged;

protected:
	virtual void BeginPlay() override;

	void TickRegen(float DeltaSeconds);

	UPROPERTY(EditDefaultsOnly, Category="Stamina")
	float MaxStamina = 100.f;

	UPROPERTY(EditDefaultsOnly, Category="Stamina")
	float RegenPerSecond = 18.f;

	UPROPERTY(EditDefaultsOnly, Category="Stamina")
	float RegenDelay = 0.75f;

	UPROPERTY(EditDefaultsOnly, Category="Stamina")
	float ExhaustedThreshold = 5.f;

	UPROPERTY(EditDefaultsOnly, Category="Stamina")
	float RecoverThreshold = 20.f;

private:
	float Stamina = 100.f;
	float TimeSinceSpend = 999.f;
	bool bExhausted = false;

	FTimerHandle RegenTimer;
};
