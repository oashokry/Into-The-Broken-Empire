#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdventureDialogueTrigger.generated.h"

class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAdvDialogueTriggered, FName, DialogueId);

UCLASS()
class SIMPLESHOOTER_API AAdventureDialogueTrigger : public AActor
{
	GENERATED_BODY()

public:
	AAdventureDialogueTrigger();

	UPROPERTY(BlueprintAssignable, Category="Dialogue")
	FAdvDialogueTriggered OnDialogueTriggered;

	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void Trigger(AActor* InstigatorActor);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* Box;

	UPROPERTY(EditDefaultsOnly, Category="Dialogue")
	FName DialogueId = TEXT("D_Default");

	UPROPERTY(EditDefaultsOnly, Category="Dialogue")
	bool bOneShot = true;

	UPROPERTY(EditDefaultsOnly, Category="Dialogue")
	bool bRequireKeyItem = false;

	UPROPERTY(EditDefaultsOnly, Category="Dialogue")
	FName RequiredItemId = TEXT("Key_Something");

	UPROPERTY(EditDefaultsOnly, Category="Dialogue")
	bool bAutoCompleteObjective = false;

	UPROPERTY(EditDefaultsOnly, Category="Dialogue")
	FName ObjectiveToComplete = TEXT("OBJ_SawDialogue");

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

private:
	bool bUsed = false;

	bool PassesRequirements(AActor* InstigatorActor) const;
};
