#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_TargetAcquisition.generated.h"

class UAIPerceptionComponent;


UCLASS()
class SIMPLESHOOTER_API UBTService_TargetAcquisition : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTService_TargetAcquisition();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector HasTargetKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetLocationKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector DistanceToTargetKey;

	UPROPERTY(EditAnywhere, Category="Targeting")
	float MaxAcquireDistance = 6000.f;

	UPROPERTY(EditAnywhere, Category="Targeting")
	bool bPreferPerception = true;

	UPROPERTY(EditAnywhere, Category="Targeting")
	bool bFallbackToLineTrace = true;

	UPROPERTY(EditAnywhere, Category="Targeting")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, Category="Debug")
	bool bDebugDraw = false;

private:
	AActor* FindBestTargetByPerception(AAIController* AIC) const;
	bool HasLineOfSightTo(AAIController* AIC, APawn* SelfPawn, AActor* Target) const;
	void WriteTarget(UBlackboardComponent* BB, AActor* Target, const FVector& TargetLoc, float Dist) const;
	void ClearTarget(UBlackboardComponent* BB) const;
};
