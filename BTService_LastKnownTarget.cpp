#include "BTService_LastKnownTarget.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

UBTService_LastKnownTarget::UBTService_LastKnownTarget()
{
	NodeName = TEXT("Last Known Target Memory");
	Interval = 0.2f;
	RandomDeviation = 0.03f;
}

void UBTService_LastKnownTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!BB || !AIC) return;

	const bool bHasTarget = BB->GetValueAsBool(HasTargetKey.SelectedKeyName);
	const float PrevTime = BB->GetValueAsFloat(TimeSinceLastSeenKey.SelectedKeyName);
	float NewTime = PrevTime + DeltaSeconds;

	if (bHasTarget)
	{
		AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
		if (Target)
		{
			BB->SetValueAsVector(LastKnownTargetLocationKey.SelectedKeyName, Target->GetActorLocation());
			NewTime = 0.f;
		}
	}

	BB->SetValueAsFloat(TimeSinceLastSeenKey.SelectedKeyName, NewTime);

	const float Confidence = ComputeConfidence(NewTime);
	BB->SetValueAsFloat(TargetMemoryConfidenceKey.SelectedKeyName, Confidence);

	if (bClearTargetWhenForgotten && NewTime >= ForgetCompletelySeconds)
	{
		// Target is "forgotten". Whether that makes tactical sense is between you and your designer.
		BB->ClearValue(TargetActorKey.SelectedKeyName);
		BB->SetValueAsBool(HasTargetKey.SelectedKeyName, false);
	}
}

float UBTService_LastKnownTarget::ComputeConfidence(float TimeSinceSeen) const
{
	if (TimeSinceSeen <= 0.f) return 1.f;
	if (TimeSinceSeen >= ForgetCompletelySeconds) return 0.f;

	// Exponential decay-ish: confidence halves every MemoryHalfLifeSeconds.
	const float Halves = TimeSinceSeen / FMath::Max(MemoryHalfLifeSeconds, 0.01f);
	const float Conf = FMath::Pow(0.5f, Halves);
	return FMath::Clamp(Conf, 0.f, 1.f);
}
