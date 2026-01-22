#include "BTTask_ClearKeySafe.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ClearKeySafe::UBTTask_ClearKeySafe()
{
	NodeName = TEXT("Clear Blackboard Key (Safe)");
}

EBTNodeResult::Type UBTTask_ClearKeySafe::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	const FName KeyName = GetSelectedBlackboardKey();

	if (!bWriteDefaultInsteadOfClear)
	{
		BB->ClearValue(KeyName);
	}
	else
	{
		// We don’t know the key type at compile time here, so we do a best-effort pattern:
		// If your key is something else, yes, this does nothing. Life is hard.
		BB->SetValueAsVector(KeyName, DefaultVector);
		BB->SetValueAsFloat(KeyName, DefaultFloat);
		BB->SetValueAsBool(KeyName, DefaultBool);
	}

	if (bLog)
	{
		UE_LOG(LogTemp, Log, TEXT("[BTTask_ClearKeySafe] Cleared/defaulted key: %s"), *KeyName.ToString());
	}

	return EBTNodeResult::Succeeded;
}
