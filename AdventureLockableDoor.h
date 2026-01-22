#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdventureLockableDoor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class SIMPLESHOOTER_API AAdventureLockableDoor : public AActor
{
	GENERATED_BODY()

public:
	AAdventureLockableDoor();

	UFUNCTION(BlueprintCallable, Category="Door")
	void Unlock();

	UFUNCTION(BlueprintCallable, Category="Door")
	void Lock();

	UFUNCTION(BlueprintCallable, Category="Door")
	bool IsLocked() const { return bLocked; }

	UFUNCTION(BlueprintCallable, Category="Door")
	void Open();

	UFUNCTION(BlueprintCallable, Category="Door")
	void Close();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* UseTrigger;

	UPROPERTY(EditDefaultsOnly, Category="Door")
	bool bLocked = true;

	UPROPERTY(EditDefaultsOnly, Category="Door")
	FName RequiredKeyItem = TEXT("Key_Door");

	UPROPERTY(EditDefaultsOnly, Category="Door")
	float OpenYaw = 90.f;

	UPROPERTY(EditDefaultsOnly, Category="Door")
	float OpenSpeed = 4.f;

	UPROPERTY(EditDefaultsOnly, Category="Door")
	bool bAutoClose = false;

	UPROPERTY(EditDefaultsOnly, Category="Door")
	float AutoCloseDelay = 3.f;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

private:
	bool bOpen = false;
	float CurrentYaw = 0.f;
	float TargetYaw = 0.f;

	FTimerHandle AutoCloseTimer;

	void TickDoor(float DeltaSeconds);
	FTimerHandle DoorTickTimer;

	bool TryConsumeKey(AActor* ActorTryingToUse);
};
