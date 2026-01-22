#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTaskNode.h"
#include "BTTask_ReloadWeaponSafe.generated.h"

/**
 * AI reload task that politely avoids reloading while dying or doing something equally inconvenient.
 */
UCLASS()
class SIMPLESHOOTER_API UBTTask_ReloadWeaponSafe : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ReloadWeaponSafe();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category="Tuning")
	float MinSpareAmmoToReload = 1;

	UPROPERTY(EditAnywhere, Category="Tuning")
	bool bRequireLowMag = true;

	UPROPERTY(EditAnywhere, Category="Tuning")
	int32 LowMagThreshold = 2;
};
