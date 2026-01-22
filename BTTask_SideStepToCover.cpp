#include "BTTask_SidestepToCover.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"

UBTTask_SidestepToCover::UBTTask_SidestepToCover()
{
	NodeName = TEXT("Sidestep (Micro-Positioning)");
}

EBTNodeResult::Type UBTTask_SidestepToCover::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return EBTNodeResult::Failed;

	APawn* Pawn = AIC->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	const FVector Right = Pawn->GetActorRightVector();
	const float Sign = bRandomLeftRight ? (FMath::RandBool() ? 1.f : -1.f) : (bPreferRight ? 1.f : -1.f);

	const FVector Desired = Pawn->GetActorLocation() + (Right * SidestepDistance * Sign);

	UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(Pawn->GetWorld());
	if (!Nav) return EBTNodeResult::Failed;

	FNavLocation Projected;
	if (!Nav->ProjectPointToNavigation(Desired, Projected, FVector(200.f,200.f,400.f)))
	{
		return EBTNodeResult::Failed;
	}

	const EPathFollowingRequestResult::Type MoveResult = AIC->MoveToLocation(Projected.Location, AcceptableRadius, true, true, false, true);
	return (MoveResult == EPathFollowingRequestResult::Failed) ? EBTNodeResult::Failed : EBTNodeResult::Succeeded;
}
