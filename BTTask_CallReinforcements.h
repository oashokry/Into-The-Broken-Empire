#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTaskNode.h"
#include "BTTask_CallReinforcements.generated.h"

class AAdventureWaveDirector;
class AAdventureEnemySpawner;

UCLASS()
class SIMPLESHOOTER_API UBTTask_CallReinforcements : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_CallReinforcements();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector ThreatLocationKey;

	UPROPERTY(EditAnywhere, Category="Reinforcements")
	AAdventureWaveDirector* WaveDirector = nullptr;

	UPROPERTY(EditAnywhere, Category="Reinforcements")
	AAdventureEnemySpawner* SpecificSpawner = nullptr;

	UPROPERTY(EditAnywhere, Category="Reinforcements")
	bool bStartNextWave = true;

	UPROPERTY(EditAnywhere, Category="Reinforcements")
	int32 ForcedWaveIndex = -1;
};
