#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_LastKnownTarget.generated.h"


UCLASS()
class SIMPLESHOOTER_API UBTService_LastKnownTarget : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTService_LastKnownTarget();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector HasTargetKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector LastKnownTargetLocationKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetMemoryConfidenceKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TimeSinceLastSeenKey;

	UPROPERTY(EditAnywhere, Category="Memory")
	float MemoryHalfLifeSeconds = 6.0f;

	UPROPERTY(EditAnywhere, Category="Memory")
	float ForgetCompletelySeconds = 18.0f;

	UPROPERTY(EditAnywhere, Category="Memory")
	bool bClearTargetWhenForgotten = true;

private:
	float ComputeConfidence(float TimeSinceSeen) const;
};
