#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AdventureNoiseEmitterComponent.generated.h"

USTRUCT(BlueprintType)
struct FAdventureNoiseEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Loudness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeSeconds = 0.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAdvNoiseEmitted, const FAdventureNoiseEvent&, Event);

UCLASS(ClassGroup=(Adventure), meta=(BlueprintSpawnableComponent))
class SIMPLESHOOTER_API UAdventureNoiseEmitterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAdventureNoiseEmitterComponent();

	UFUNCTION(BlueprintCallable, Category="Noise")
	void EmitNoise(float Loudness = 1.0f, FVector LocationOverride = FVector::ZeroVector);

	UPROPERTY(BlueprintAssignable, Category="Noise")
	FAdvNoiseEmitted OnNoiseEmitted;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Noise")
	float CooldownSeconds = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category="Noise")
	bool bAlsoTagOwner = true;

private:
	float LastEmitTime = -9999.f;
};
