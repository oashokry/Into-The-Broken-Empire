#include "BTTask_BurstFireWeapon.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

#include "AdventureCharacter.h"

UBTTask_BurstFireWeapon::UBTTask_BurstFireWeapon()
{
	NodeName = TEXT("Burst Fire Weapon");
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_BurstFireWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AIC || !BB) return EBTNodeResult::Failed;

	APawn* Pawn = AIC->GetPawn();
	AAdventureCharacter* Shooter = Pawn ? Cast<AAdventureCharacter>(Pawn) : nullptr;
	if (!Shooter || !Shooter->HasWeaponEquipped())
	{
		return EBTNodeResult::Failed;
	}

	ShotsRemaining = FMath::Max(Shots, 1);

	// Fire immediately once, then schedule rest.
	FireOne(&OwnerComp);

	if (ShotsRemaining > 0)
	{
		Pawn->GetWorldTimerManager().SetTimer(
			BurstTimer,
			FTimerDelegate::CreateUObject(this, &UBTTask_BurstFireWeapon::FireOne, &OwnerComp),
			ShotInterval,
			true
		);
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_BurstFireWeapon::FireOne(UBehaviorTreeComponent* OwnerComp)
{
	if (!OwnerComp) return;

	AAIController* AIC = OwnerComp->GetAIOwner();
	UBlackboardComponent* BB = OwnerComp->GetBlackboardComponent();
	if (!AIC || !BB)
	{
		FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
		return;
	}

	APawn* Pawn = AIC->GetPawn();
	AAdventureCharacter* Shooter = Pawn ? Cast<AAdventureCharacter>(Pawn) : nullptr;
	if (!Shooter || !Shooter->HasWeaponEquipped())
	{
		FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	const FVector TargetLoc = BB->GetValueAsVector(TargetLocationKey.SelectedKeyName);

	if (bStopIfNoTarget && !TargetActor && TargetLoc.IsNearlyZero())
	{
		Pawn->GetWorldTimerManager().ClearTimer(BurstTimer);
		FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
		return;
	}

	const FVector AimPoint = ComputeAimPoint(Pawn, TargetActor, TargetLoc);

	const float Dist = FVector::Dist(Pawn->GetActorLocation(), AimPoint);
	if (Dist > MaxRange)
	{
		Pawn->GetWorldTimerManager().ClearTimer(BurstTimer);
		FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (bFaceTarget)
	{
		const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(Pawn->GetActorLocation(), AimPoint);
		AIC->SetControlRotation(LookAt);
	}

	Shooter->SetAimOverride(TargetActor, AimPoint);
	Shooter->PullTrigger(); // delegates to weapon (ammo/spread/recoil inside weapon)

	ShotsRemaining--;

	if (ShotsRemaining <= 0)
	{
		Pawn->GetWorldTimerManager().ClearTimer(BurstTimer);
		FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
	}
}

FVector UBTTask_BurstFireWeapon::ComputeAimPoint(APawn* ShooterPawn, AActor* TargetActor, const FVector& FallbackLoc) const
{
	FVector Aim = TargetActor ? TargetActor->GetActorLocation() : FallbackLoc;

	// A cheap “lead target” for moving player/NPCs.
	if (bUseTargetVelocityLead && TargetActor)
	{
		const FVector Vel = TargetActor->GetVelocity();
		const float Dist = FVector::Dist(ShooterPawn->GetActorLocation(), Aim);
		const float LeadTime = (Dist / 2000.f) * LeadStrength; // assumes “bullet-ish” speed proxy
		Aim += Vel * LeadTime;
	}

	// Aim slightly upward like humans do when they want to hit torsos.
	Aim += FVector(0.f, 0.f, 55.f);
	return Aim;
}

void UBTTask_BurstFireWeapon::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	if (AAIController* AIC = OwnerComp.GetAIOwner())
	{
		if (APawn* Pawn = AIC->GetPawn())
		{
			Pawn->GetWorldTimerManager().ClearTimer(BurstTimer);
		}
	}

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
