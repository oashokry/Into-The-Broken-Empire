#include "BTDecorator_HasLOSOrHeard.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_HasLOSOrHeard::UBTDecorator_HasLOSOrHeard()
{
	NodeName = TEXT("Has LOS Or Heard Something");
}

bool UBTDecorator_HasLOSOrHeard::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;

	const bool bHasLOS = BB->GetValueAsBool(HasLosKey.SelectedKeyName);
	if (bHasLOS) return true;

	const FVector Heard = BB->GetValueAsVector(HeardLocationKey.SelectedKeyName);
	return Heard.SizeSquared() > FMath::Square(HeardValidRadius);
}
