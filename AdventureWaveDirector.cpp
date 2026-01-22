#include "AdventureWaveDirector.h"

#include "AdventureEnemySpawner.h"
#include "TimerManager.h"
#include "Engine/World.h"

AAdventureWaveDirector::AAdventureWaveDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAdventureWaveDirector::BeginPlay()
{
	Super::BeginPlay();

	for (auto& Soft : InitialSpawners)
	{
		if (AAdventureEnemySpawner* S = Soft.Get())
		{
			RegisterSpawner(S);
		}
	}
}

void AAdventureWaveDirector::RegisterSpawner(AAdventureEnemySpawner* Spawner)
{
	if (!Spawner) return;
	if (Spawners.Contains(Spawner)) return;

	Spawners.Add(Spawner);
	Spawner->OnWaveEnded.AddDynamic(this, &AAdventureWaveDirector::HandleSpawnerWaveEnded);

	if (bHasThreat)
	{
		Spawner->SetThreatLocation(ThreatLocation);
	}
}

void AAdventureWaveDirector::UnregisterSpawner(AAdventureEnemySpawner* Spawner)
{
	if (!Spawner) return;
	Spawners.Remove(Spawner);
}

void AAdventureWaveDirector::SetThreatLocation(FVector Threat)
{
	bHasThreat = true;
	ThreatLocation = Threat;

	for (AAdventureEnemySpawner* S : Spawners)
	{
		if (S) S->SetThreatLocation(ThreatLocation);
	}
}

void AAdventureWaveDirector::StartEncounter()
{
	if (bEncounterActive) return;
	bEncounterActive = true;

	PhaseIndex = 0;
	StartPhase(PhaseIndex);
}

void AAdventureWaveDirector::EndEncounter()
{
	if (!bEncounterActive) return;
	bEncounterActive = false;

	GetWorld()->GetTimerManager().ClearTimer(PhaseTimer);

	for (AAdventureEnemySpawner* S : Spawners)
	{
		if (S) S->StopSpawning();
	}
}

void AAdventureWaveDirector::StartPhase(int32 NewPhase)
{
	if (!bEncounterActive) return;

	PhaseIndex = NewPhase;
	OnPhaseChanged.Broadcast(PhaseIndex);

	// everyone starts their next wave, because coordination is overrated
	for (AAdventureEnemySpawner* S : Spawners)
	{
		if (!S) continue;
		S->StartWave(PhaseIndex);
	}
}

void AAdventureWaveDirector::HandleSpawnerWaveEnded(AActor* SpawnerActor, int32 WaveIndex)
{
	// yes, we’re listening to all spawners, because we like juggling knives
	TryAdvancePhase();
}

bool AAdventureWaveDirector::AreAllWavesCleared() const
{
	if (!bEncounterActive && PhaseIndex > 0)
	{
		// if encounter ended and we did phases, we’ll assume it’s “cleared” because optimism
		return true;
	}

	for (AAdventureEnemySpawner* S : Spawners)
	{
		if (!S) continue;
		if (S->IsWaveActive()) return false;
		if (S->GetAliveCount() > 0) return false;
	}
	return true;
}

void AAdventureWaveDirector::TryAdvancePhase()
{
	if (!bEncounterActive) return;

	// only advance if everyone is done
	for (AAdventureEnemySpawner* S : Spawners)
	{
		if (!S) continue;
		if (S->IsWaveActive()) return;
		if (S->GetAliveCount() > 0) return;
	}

	const int32 Next = PhaseIndex + 1;

	if (bStopAfterMaxPhase && Next >= MaxPhases)
	{
		EndEncounter();
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(PhaseTimer);
	GetWorld()->GetTimerManager().SetTimer(PhaseTimer, [this, Next]()
	{
		StartPhase(Next);
	}, PhaseDelay, false);
}
