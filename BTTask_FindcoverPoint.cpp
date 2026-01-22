#include "BTTask_FindCoverPoint.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "AdventureCoverUtils.h"

UBTTask_FindCoverPoint::UBTTask_FindCoverPoint()
{
	NodeName = TEXT("Find Cover Point");
}

EBTNodeResult::Type UBTTask_FindCoverPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AIC || !BB) return EBTNodeResult::Failed;

	APawn* Pawn = AIC->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	AActor* Threat = Cast<AActor>(BB->GetValueAsObject(ThreatActorKey.SelectedKeyName));
	if (!Threat) return EBTNodeResult::Failed;

	FVector Cover;
	const bool bFound = UAdventureCoverUtils::FindCoverPoint(Pawn->GetWorld(), Pawn->GetActorLocation(), Threat->GetActorLocation(), SearchRadius, Samples, Cover);

	if (!bFound)
	{
		return EBTNodeResult::Failed; // no cover found, guess we die like it’s a cutscene
	}

	BB->SetValueAsVector(CoverLocationKey.SelectedKeyName, Cover);
	return EBTNodeResult::Succeeded;
}
