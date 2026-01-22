#include "BTTask_CallReinforcements.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

#include "AdventureWaveDirector.h"
#include "AdventureEnemySpawner.h"

UBTTask_CallReinforcements::UBTTask_CallReinforcements()
{
	NodeName = TEXT("Call Reinforcements");
}

EBTNodeResult::Type UBTTask_CallReinforcements::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!BB || !AIC) return EBTNodeResult::Failed;

	const FVector Threat = BB->GetValueAsVector(ThreatLocationKey.SelectedKeyName);

	if (WaveDirector)
	{
		WaveDirector->SetThreatLocation(Threat);

		if (!WaveDirector->IsEncounterActive())
		{
			WaveDirector->StartEncounter(); // because apparently we’re the manager now
			return EBTNodeResult::Succeeded;
		}

		return EBTNodeResult::Succeeded;
	}

	if (SpecificSpawner)
	{
		SpecificSpawner->SetThreatLocation(Threat);

		if (ForcedWaveIndex >= 0)
		{
			SpecificSpawner->StartWave(ForcedWaveIndex);
			return EBTNodeResult::Succeeded;
		}

		if (bStartNextWave)
		{
			SpecificSpawner->StartNextWave();
			return EBTNodeResult::Succeeded;
		}

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed; // nobody to call, so we just… panic quietly
}
