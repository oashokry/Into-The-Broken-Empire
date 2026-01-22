#include "BTTask_ThrowGrenade.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "AdventureGrenade.h"
#include "Engine/World.h"

UBTTask_ThrowGrenade::UBTTask_ThrowGrenade()
{
	NodeName = TEXT("Throw Grenade");
}

EBTNodeResult::Type UBTTask_ThrowGrenade::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AIC || !BB) return EBTNodeResult::Failed;

	APawn* Pawn = AIC->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;

	if (!GrenadeClass) return EBTNodeResult::Failed;

	const FVector Target = BB->GetValueAsVector(TargetLocationKey.SelectedKeyName);
	const FVector Start = Pawn->GetActorLocation() + FVector(0,0,70.f);

	const float Dist = FVector::Dist(Start, Target);
	if (Dist > MaxThrowDistance || Dist < MinThrowDistance)
	{
		return EBTNodeResult::Failed; // “too far” and “too close” both exist, just like my patience
	}

	FTransform T;
	T.SetLocation(Start);
	T.SetRotation(Pawn->GetActorRotation().Quaternion());

	AAdventureGrenade* G = Pawn->GetWorld()->SpawnActorDeferred<AAdventureGrenade>(GrenadeClass, T, Pawn, Pawn, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!G) return EBTNodeResult::Failed;

	G->FinishSpawning(T);

	FVector Dir = (Target - Start).GetSafeNormal();
	Dir.Z += ArcBoostZ;
	Dir = Dir.GetSafeNormal();

	const FVector Vel = Dir * ThrowSpeed;
	G->SetThrowVelocity(Vel);
	G->Arm(AIC, FuseOverride);

	return EBTNodeResult::Succeeded;
}
