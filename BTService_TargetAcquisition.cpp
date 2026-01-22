#include "BTService_TargetAcquisition.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UBTService_TargetAcquisition::UBTService_TargetAcquisition()
{
	NodeName = TEXT("Target Acquisition (Perception + LoS)");
	Interval = 0.2f;
	RandomDeviation = 0.05f;
}

void UBTService_TargetAcquisition::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIC = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!AIC || !BB)
	{
		return;
	}

	APawn* SelfPawn = AIC->GetPawn();
	if (!SelfPawn)
	{
		ClearTarget(BB);
		return;
	}

	AActor* Target = nullptr;

	// 1) Perception: best target from perceived actors (sight)
	if (bPreferPerception)
	{
		Target = FindBestTargetByPerception(AIC);
	}

	// 2) Fallback: just use player pawn if LoS works
	if (!Target && bFallbackToLineTrace)
	{
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(SelfPawn->GetWorld(), 0);
		if (PlayerPawn && HasLineOfSightTo(AIC, SelfPawn, PlayerPawn))
		{
			Target = PlayerPawn;
		}
	}

	if (!Target)
	{
		ClearTarget(BB);
		return;
	}

	const float Dist = FVector::Dist(SelfPawn->GetActorLocation(), Target->GetActorLocation());
	if (Dist > MaxAcquireDistance)
	{
		ClearTarget(BB);
		return;
	}

	const FVector TargetLoc = Target->GetActorLocation();
	WriteTarget(BB, Target, TargetLoc, Dist);

	if (bDebugDraw)
	{
		DrawDebugSphere(SelfPawn->GetWorld(), TargetLoc, 30.f, 16, FColor::Yellow, false, 0.2f);
		DrawDebugLine(SelfPawn->GetWorld(), SelfPawn->GetPawnViewLocation(), TargetLoc, FColor::Yellow, false, 0.2f, 0, 1.5f);
	}
}

AActor* UBTService_TargetAcquisition::FindBestTargetByPerception(AAIController* AIC) const
{
	UAIPerceptionComponent* Perception = AIC->FindComponentByClass<UAIPerceptionComponent>();
	if (!Perception)
	{
		return nullptr;
	}

	TArray<AActor*> SeenActors;
	Perception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SeenActors);

	APawn* SelfPawn = AIC->GetPawn();
	if (!SelfPawn || SeenActors.Num() == 0)
	{
		return nullptr;
	}

	AActor* Best = nullptr;
	float BestScore = -1.f;

	for (AActor* A : SeenActors)
	{
		if (!A || A == SelfPawn) continue;

		const float Dist = FVector::Dist(SelfPawn->GetActorLocation(), A->GetActorLocation());
		if (Dist > MaxAcquireDistance) continue;

		// Cheap scoring: closer is better, plus LoS confirmation
		float Score = 1.f / FMath::Max(Dist, 1.f);
		if (HasLineOfSightTo(AIC, SelfPawn, A))
		{
			Score *= 2.0f;
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			Best = A;
		}
	}

	return Best;
}

bool UBTService_TargetAcquisition::HasLineOfSightTo(AAIController* AIC, APawn* SelfPawn, AActor* Target) const
{
	if (!SelfPawn || !Target) return false;

	const FVector From = SelfPawn->GetPawnViewLocation();
	const FVector To = Target->GetActorLocation() + FVector(0.f, 0.f, 60.f);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(TargetAcquireLoS), false);
	Params.AddIgnoredActor(SelfPawn);

	const bool bHit = SelfPawn->GetWorld()->LineTraceSingleByChannel(Hit, From, To, TraceChannel, Params);
	if (!bHit) return true;

	return Hit.GetActor() == Target;
}

void UBTService_TargetAcquisition::WriteTarget(UBlackboardComponent* BB, AActor* Target, const FVector& TargetLoc, float Dist) const
{
	BB->SetValueAsObject(TargetActorKey.SelectedKeyName, Target);
	BB->SetValueAsBool(HasTargetKey.SelectedKeyName, true);
	BB->SetValueAsVector(TargetLocationKey.SelectedKeyName, TargetLoc);
	BB->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, Dist);
}

void UBTService_TargetAcquisition::ClearTarget(UBlackboardComponent* BB) const
{
	BB->ClearValue(TargetActorKey.SelectedKeyName);
	BB->SetValueAsBool(HasTargetKey.SelectedKeyName, false);
	BB->ClearValue(TargetLocationKey.SelectedKeyName);
	BB->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, 0.f);
}
