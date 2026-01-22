#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTaskNode.h"
#include "BTTask_ThrowGrenade.generated.h"

class AAdventureGrenade;

UCLASS()
class SIMPLESHOOTER_API UBTTask_ThrowGrenade : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ThrowGrenade();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetLocationKey;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	TSubclassOf<AAdventureGrenade> GrenadeClass;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	float ThrowSpeed = 1700.f;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	float ArcBoostZ = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	float FuseOverride = -1.f;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	float MaxThrowDistance = 2200.f;

	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	float MinThrowDistance = 400.f;
};
