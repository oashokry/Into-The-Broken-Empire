#include "AdventureHealthComponent.h"

#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UAdventureHealthComponent::UAdventureHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAdventureHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = FMath::Clamp(MaxHealth, 1.f, MaxHealth);

	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UAdventureHealthComponent::OnAnyDamage);
	}

	if (RegenPerSecond > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(RegenTimer, [this]()
		{
			TickRegen(0.25f);
		}, 0.25f, true);
	}
}

void UAdventureHealthComponent::SetDamageMultiplier(float NewMultiplier)
{
	DamageMultiplier = FMath::Max(0.f, NewMultiplier); // because negative damage is definitely not going to be abused
}

bool UAdventureHealthComponent::Heal(float Amount, AActor* InstigatorActor)
{
	if (bDead) return false;
	if (Amount <= 0.f) return false;

	const float Old = Health;
	Health = FMath::Clamp(Health + Amount, 0.f, MaxHealth);

	const float Delta = Health - Old;
	if (!FMath::IsNearlyZero(Delta))
	{
		OnHealthChanged.Broadcast(this, Health, Delta, InstigatorActor);
	}
	return !FMath::IsNearlyZero(Delta);
}

bool UAdventureHealthComponent::ApplyDamage(float Amount, AActor* InstigatorActor)
{
	if (bGodMode) return false;
	if (bDead) return false;
	if (Amount <= 0.f) return false;

	const float Scaled = Amount * DamageMultiplier;

	TimeSinceLastDamage = 0.f;

	const float Old = Health;
	Health = FMath::Clamp(Health - Scaled, 0.f, MaxHealth);

	const float Delta = Health - Old;
	if (!FMath::IsNearlyZero(Delta))
	{
		OnHealthChanged.Broadcast(this, Health, Delta, InstigatorActor);
	}

	if (Health <= 0.f && !bDead)
	{
		HandleDeath(InstigatorActor);
		return true;
	}

	return !FMath::IsNearlyZero(Delta);
}

void UAdventureHealthComponent::Revive(float NewHealthRatio)
{
	bDead = false;
	Health = FMath::Clamp(MaxHealth * FMath::Clamp(NewHealthRatio, 0.05f, 1.f), 1.f, MaxHealth);
	OnHealthChanged.Broadcast(this, Health, +Health, nullptr);
}

void UAdventureHealthComponent::OnAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	// yes, we’re here again, because life is pain
	AActor* InstigatorActor = DamageCauser ? DamageCauser : (InstigatedBy ? InstigatedBy->GetPawn() : nullptr);
	ApplyDamage(Damage, InstigatorActor);
}

void UAdventureHealthComponent::HandleDeath(AActor* Killer)
{
	bDead = true;
	OnDeath.Broadcast(GetOwner(), Killer);

	// nothing screams “adventure” like instantly losing control of your body
	if (AActor* Owner = GetOwner())
	{
		Owner->SetActorEnableCollision(false);
		Owner->SetActorTickEnabled(false);
	}
}

void UAdventureHealthComponent::TickRegen(float DeltaSeconds)
{
	if (bDead) return;

	TimeSinceLastDamage += DeltaSeconds;
	if (TimeSinceLastDamage < RegenDelay) return;

	if (Health < MaxHealth)
	{
		Heal(RegenPerSecond * DeltaSeconds, nullptr);
	}
}
