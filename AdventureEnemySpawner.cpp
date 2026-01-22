#include "AdventureEnemySpawner.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "Kismet/KismetMathLibrary.h"

AAdventureEnemySpawner::AAdventureEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAdventureEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
}

int32 AAdventureEnemySpawner::GetAliveCount() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<APawn>& P : AlivePawns)
	{
		if (P.IsValid()) Count++;
	}
	return Count;
}

void AAdventureEnemySpawner::CleanupDead()
{
	for (int32 i = AlivePawns.Num() - 1; i >= 0; --i)
	{
		if (!AlivePawns[i].IsValid())
		{
			AlivePawns.RemoveAt(i);
		}
	}
}

bool AAdventureEnemySpawner::CanSpawnMore() const
{
	if (!bWaveActive) return false;
	if (SpawnTable.Num() == 0) return false;
	if (WaveSpawnedSoFar >= WaveQuotaToSpawn) return false;
	if (GetAliveCount() >= MaxSimultaneousAlive) return false;
	return true;
}

const FAdvSpawnEntry* AAdventureEnemySpawner::PickEntry(FRandomStream& Rng) const
{
	float Total = 0.f;
	for (const FAdvSpawnEntry& E : SpawnTable)
	{
		if (!E.PawnClass) continue;
		Total += FMath::Max(0.0001f, E.Weight);
	}

	if (Total <= 0.f) return nullptr;

	const float Pick = Rng.FRandRange(0.f, Total);
	float Run = 0.f;

	for (const FAdvSpawnEntry& E : SpawnTable)
	{
		if (!E.PawnClass) continue;
		Run += FMath::Max(0.0001f, E.Weight);
		if (Run >= Pick) return &E;
	}

	return &SpawnTable.Last(); // because sure
}

FVector AAdventureEnemySpawner::PickSpawnLocation(FRandomStream& Rng) const
{
	FVector Candidate = GetActorLocation();
	Candidate.X += Rng.FRandRange(-SpawnRadius, SpawnRadius);
	Candidate.Y += Rng.FRandRange(-SpawnRadius, SpawnRadius);

	if (!bRequireNavProjection)
	{
		return Candidate;
	}

	UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!Nav) return Candidate;

	FNavLocation Out;
	if (Nav->ProjectPointToNavigation(Candidate, Out, FVector(500, 500, 500)))
	{
		return Out.Location;
	}

	return Candidate; // if nav says no, we pretend it said yes
}

void AAdventureEnemySpawner::StartNextWave()
{
	int32 Next = CurrentWave + 1;
	StartWave(Next);
}

void AAdventureEnemySpawner::StartWave(int32 WaveIndex)
{
	if (bWaveActive)
	{
		// yes, stacking waves would be hilarious, but no
		return;
	}

	CurrentWave = WaveIndex;

	FRandomStream Rng;
	Rng.Initialize(GetUniqueID() ^ (WaveIndex * 9973));

	// wave quota scales with wave index because “difficulty curve”
	const int32 Base = 4 + WaveIndex * 2;
	const int32 Jitter = Rng.RandRange(0, 3);
	WaveQuotaToSpawn = FMath::Clamp(Base + Jitter, 1, 64);

	WaveSpawnedSoFar = 0;
	bWaveActive = true;

	OnWaveStarted.Broadcast(this, CurrentWave);

	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
	GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &AAdventureEnemySpawner::TickSpawn, SpawnInterval, true);
}

void AAdventureEnemySpawner::StopSpawning()
{
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
	bWaveActive = false;

	OnWaveEnded.Broadcast(this, CurrentWave);
}

void AAdventureEnemySpawner::TickSpawn()
{
	CleanupDead();

	if (!bWaveActive)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
		return;
	}

	// leash check, because enemies migrating to Cairo wasn’t in the design
	if (bHasThreat)
	{
		const float Dist = FVector::Dist(GetActorLocation(), ThreatLocation);
		if (Dist > LeashRadius)
		{
			StopSpawning();
			return;
		}
	}

	if (!CanSpawnMore())
	{
		if (bEndWaveWhenQuotaMetAndDead)
		{
			if (WaveSpawnedSoFar >= WaveQuotaToSpawn && GetAliveCount() <= 0)
			{
				StopSpawning();

				if (bLoopWaves)
				{
					StartWave(0);
				}
			}
		}
		return;
	}

	FRandomStream Rng;
	Rng.Initialize(GetUniqueID() ^ (int32)(GetWorld()->TimeSeconds * 1000.f) ^ (WaveSpawnedSoFar * 31));

	const FAdvSpawnEntry* Entry = PickEntry(Rng);
	if (!Entry || !Entry->PawnClass) return;

	const FVector Loc = PickSpawnLocation(Rng);

	FRotator Rot = GetActorRotation();
	if (bSpawnFacingThreat && bHasThreat)
	{
		Rot = UKismetMathLibrary::FindLookAtRotation(Loc, ThreatLocation);
	}

	FActorSpawnParameters Params;
	Params.Owner = this;

	APawn* Spawned = GetWorld()->SpawnActor<APawn>(Entry->PawnClass, Loc, Rot, Params);
	if (!Spawned) return;

	WaveSpawnedSoFar++;
	TrackPawn(Spawned);

	OnEnemySpawned.Broadcast(this, Spawned);
}

void AAdventureEnemySpawner::TrackPawn(APawn* Pawn)
{
	if (!Pawn) return;

	AlivePawns.Add(Pawn);
	Pawn->OnDestroyed.AddDynamic(this, &AAdventureEnemySpawner::OnPawnDestroyed);
}

void AAdventureEnemySpawner::OnPawnDestroyed(AActor* DestroyedActor)
{
	// yes, we “clean up” by pretending garbage collection is our friend
	CleanupDead();

	// wave end check on death for dramatic pacing
	if (bWaveActive && bEndWaveWhenQuotaMetAndDead)
	{
		if (WaveSpawnedSoFar >= WaveQuotaToSpawn && GetAliveCount() <= 0)
		{
			StopSpawning();
			if (bLoopWaves)
			{
				StartWave(0);
			}
		}
	}
}

void AAdventureEnemySpawner::DespawnAll()
{
	for (TWeakObjectPtr<APawn>& P : AlivePawns)
	{
		if (P.IsValid())
		{
			P->Destroy();
		}
	}
	AlivePawns.Reset();
}
