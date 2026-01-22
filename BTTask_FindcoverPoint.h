#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTaskNode.h"
#include "BTTask_FindCoverPoint.generated.h"

UCLASS()
class SIMPLESHOOTER_API UBTTask_FindCoverPoint : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindCoverPoint();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector ThreatActorKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector CoverLocationKey;

	UPROPERTY(EditAnywhere, Category="Tuning")
	float SearchRadius = 900.f;

	UPROPERTY(EditAnywhere, Category="Tuning")
	int32 Samples = 24;
};
