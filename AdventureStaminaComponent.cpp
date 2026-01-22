#include "AdventureStaminaComponent.h"
#include "TimerManager.h"

UAdventureStaminaComponent::UAdventureStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAdventureStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	Stamina = FMath::Clamp(MaxStamina, 1.f, MaxStamina);

	GetWorld()->GetTimerManager().SetTimer(RegenTimer, [this]()
	{
		TickRegen(0.1f);
	}, 0.1f, true);
}

bool UAdventureStaminaComponent::HasStamina(float Cost) const
{
	return Stamina >= Cost && !bExhausted;
}

bool UAdventureStaminaComponent::Spend(float Cost)
{
	if (Cost <= 0.f) return true;
	if (Stamina <= 0.f) return false;

	TimeSinceSpend = 0.f;

	const float Old = Stamina;
	Stamina = FMath::Clamp(Stamina - Cost, 0.f, MaxStamina);

	const float Delta = Stamina - Old;
	if (!FMath::IsNearlyZero(Delta))
	{
		OnStaminaChanged.Broadcast(this, Stamina, Delta);
	}

	if (!bExhausted && Stamina <= ExhaustedThreshold)
	{
		bExhausted = true; // you wanted realism; you got sadness
	}

	return Stamina > 0.f;
}

void UAdventureStaminaComponent::Refill(float Amount)
{
	if (Amount <= 0.f) return;

	const float Old = Stamina;
	Stamina = FMath::Clamp(Stamina + Amount, 0.f, MaxStamina);

	const float Delta = Stamina - Old;
	if (!FMath::IsNearlyZero(Delta))
	{
		OnStaminaChanged.Broadcast(this, Stamina, Delta);
	}

	if (bExhausted && Stamina >= RecoverThreshold)
	{
		bExhausted = false; // look who magically feels better now
	}
}

void UAdventureStaminaComponent::TickRegen(float DeltaSeconds)
{
	TimeSinceSpend += DeltaSeconds;
	if (TimeSinceSpend < RegenDelay) return;

	if (Stamina < MaxStamina)
	{
		Refill(RegenPerSecond * DeltaSeconds);
	}
}
