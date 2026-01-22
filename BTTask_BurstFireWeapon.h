#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTaskNode.h"
#include "BTTask_BurstFireWeapon.generated.h"


UCLASS()
class SIMPLESHOOTER_API UBTTask_BurstFireWeapon : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_BurstFireWeapon();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetLocationKey;

	UPROPERTY(EditAnywhere, Category="Firing")
	int32 Shots = 5;

	UPROPERTY(EditAnywhere, Category="Firing")
	float ShotInterval = 0.09f;

	UPROPERTY(EditAnywhere, Category="Firing")
	float MaxRange = 4500.f;

	UPROPERTY(EditAnywhere, Category="Firing")
	bool bStopIfNoTarget = true;

	UPROPERTY(EditAnywhere, Category="Firing")
	bool bFaceTarget = true;

	UPROPERTY(EditAnywhere, Category="Firing")
	bool bUseTargetVelocityLead = true;

	UPROPERTY(EditAnywhere, Category="Firing")
	float LeadStrength = 0.65f;

private:
	FTimerHandle BurstTimer;
	int32 ShotsRemaining = 0;

	void FireOne(UBehaviorTreeComponent* OwnerComp);
	FVector ComputeAimPoint(APawn* ShooterPawn, AActor* TargetActor, const FVector& FallbackLoc) const;
};
