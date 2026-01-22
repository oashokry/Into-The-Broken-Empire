#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdventureInteractable.h"
#include "AdventurePickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;


UENUM(BlueprintType)
enum class EPickupType : uint8
{
	Ammo,
	Stamina,
	Relic
};

UCLASS()
class SIMPLESHOOTER_API AAdventurePickup : public AActor, public IAdventureInteractable
{
	GENERATED_BODY()

public:
	AAdventurePickup();

	virtual void BeginPlay() override;

	// IAdventureInteractable
	virtual bool Interact_Implementation(AActor* Interactor) override;
	virtual FText GetInteractPrompt_Implementation() const override;

protected:
	UPROPERTY(VisibleAnywhere, Category="Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, Category="Components")
	USphereComponent* Sphere;

	UPROPERTY(EditDefaultsOnly, Category="Pickup")
	EPickupType Type = EPickupType::Ammo;

	UPROPERTY(EditDefaultsOnly, Category="Pickup")
	int32 Amount = 15;

	UPROPERTY(EditDefaultsOnly, Category="Pickup")
	bool bAutoPickupOnOverlap = false;

	UPROPERTY(EditDefaultsOnly, Category="Pickup")
	float DestroyDelay = 0.05f;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                    const FHitResult& SweepResult);

private:
	void GrantTo(AActor* Interactor);
};
