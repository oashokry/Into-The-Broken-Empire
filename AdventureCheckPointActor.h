#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdventureCheckpointActor.generated.h"

class UBoxComponent;

UCLASS()
class SIMPLESHOOTER_API AAdventureCheckpointActor : public AActor
{
	GENERATED_BODY()

public:
	AAdventureCheckpointActor();

	UFUNCTION(BlueprintCallable, Category="Checkpoint")
	FTransform GetCheckpointTransform() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* Trigger;

	UPROPERTY(EditDefaultsOnly, Category="Checkpoint")
	FName CheckpointId = TEXT("CP_Default");

	UPROPERTY(EditDefaultsOnly, Category="Checkpoint")
	bool bOneShot = true;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

private:
	bool bUsed = false;
};
