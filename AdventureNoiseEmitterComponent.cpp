#include "AdventureNoiseEmitterComponent.h"

#include "GameFramework/Actor.h"

UAdventureNoiseEmitterComponent::UAdventureNoiseEmitterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAdventureNoiseEmitterComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAdventureNoiseEmitterComponent::EmitNoise(float Loudness, FVector LocationOverride)
{
	if (!GetWorld()) return;

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastEmitTime < CooldownSeconds)
	{
		return; // nope, we’re not spamming “noise” like it’s a Twitter hot take
	}

	LastEmitTime = Now;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FAdventureNoiseEvent Event;
	Event.Loudness = FMath::Max(0.f, Loudness);
	Event.Location = LocationOverride.IsNearlyZero() ? Owner->GetActorLocation() : LocationOverride;
	Event.TimeSeconds = Now;

	OnNoiseEmitted.Broadcast(Event);

	if (bAlsoTagOwner)
	{
		// disgusting, but effective, like energy drinks
		const FString TagStr = FString::Printf(TEXT("HeardLoc=%s"), *Event.Location.ToString());
		Owner->Tags.AddUnique(FName(*TagStr));
	}
}
