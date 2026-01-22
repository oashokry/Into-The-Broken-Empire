#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_HasLOSOrHeard.generated.h"

UCLASS()
class SIMPLESHOOTER_API UBTDecorator_HasLOSOrHeard : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_HasLOSOrHeard();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector HasLosKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector HeardLocationKey;

	UPROPERTY(EditAnywhere, Category="Tuning")
	float HeardValidRadius = 10.f;
};
