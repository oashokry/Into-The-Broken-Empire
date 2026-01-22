#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_ClearKeySafe.generated.h"


UCLASS()
class SIMPLESHOOTER_API UBTTask_ClearKeySafe : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_ClearKeySafe();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category="Options")
	bool bWriteDefaultInsteadOfClear = false;

	UPROPERTY(EditAnywhere, Category="Options")
	bool bLog = false;

	UPROPERTY(EditAnywhere, Category="Defaults")
	FVector DefaultVector = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category="Defaults")
	float DefaultFloat = 0.f;

	UPROPERTY(EditAnywhere, Category="Defaults")
	bool DefaultBool = false;
};
