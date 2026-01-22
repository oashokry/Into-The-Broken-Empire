#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTaskNode.h"
#include "BTTask_SidestepToCover.generated.h"

/**
 * Task: makes AI do a quick sidestep (micro-movement) to feel less turret-like.
 * Not real cover finding (that’s EQS/Nav queries), but it sells the illusion.
 */
UCLASS()
class SIMPLESHOOTER_API UBTTask_SidestepToCover : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SidestepToCover();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category="Movement")
	float SidestepDistance = 300.f;

	UPROPERTY(EditAnywhere, Category="Movement")
	float AcceptableRadius = 40.f;

	UPROPERTY(EditAnywhere, Category="Movement")
	bool bRandomLeftRight = true;

	UPROPERTY(EditAnywhere, Category="Movement")
	bool bPreferRight = true;
};
