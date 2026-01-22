#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdventureGrapplePointActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class SIMPLESHOOTER_API AAdventureGrapplePointActor : public AActor
{
	GENERATED_BODY()

public:
	AAdventureGrapplePointActor();

	UFUNCTION(BlueprintCallable, Category="GrapplePoint")
	bool IsBroken() const { return bBroken; }

	UFUNCTION(BlueprintCallable, Category="GrapplePoint")
	void BreakPoint();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly, Category="GrapplePoint")
	bool bBreakable = false;

	UPROPERTY(EditDefaultsOnly, Category="GrapplePoint")
	float BreakAfterSecondsAttached = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="GrapplePoint")
	FName GrappleTag = TEXT("GrapplePoint");

	UPROPERTY(EditDefaultsOnly, Category="GrapplePoint")
	bool bMoves = false;

	UPROPERTY(EditDefaultsOnly, Category="GrapplePoint")
	FVector MoveOffset = FVector(0,0,120);

	UPROPERTY(EditDefaultsOnly, Category="GrapplePoint")
	float MoveSpeed = 1.0f;

private:
	bool bBroken = false;
	FVector StartLoc = FVector::ZeroVector;
	float MoveTime = 0.f;

	FTimerHandle MoveTimer;

	void SoftTick();
};
