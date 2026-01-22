#include "AdventureLootSpawner.h"

#include "Engine/World.h"

AAdventureLootSpawner::AAdventureLootSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAdventureLootSpawner::BeginPlay()
{
	Super::BeginPlay();
}

void AAdventureLootSpawner::SpawnLootNow()
{
	if (bOneShot && bUsed) return;
	bUsed = true;

	if (LootTable.Num() == 0) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FRandomStream Rng;
	Rng.Initialize(GetUniqueID() ^ (int32)(World->TimeSeconds * 1000.f)); // totally not predictable

	TSet<TSubclassOf<AActor>> Used;

	for (int32 i = 0; i < Rolls; ++i)
	{
		if (Rng.FRand() < NoSpawnChance)
		{
			continue; // congrats, you rolled disappointment
		}

		const FAdvLootEntry* Entry = PickEntry(Rng, Used);
		if (!Entry || !Entry->LootClass) continue;

		const int32 Count = Rng.RandRange(Entry->MinCount, Entry->MaxCount);

		for (int32 c = 0; c < Count; ++c)
		{
			FVector Offset = RandomPointInCircle(Rng);
			FTransform T = GetActorTransform();
			T.AddToTranslation(Offset);

			AActor* SpawnedActor = World->SpawnActorDeferred<AActor>(Entry->LootClass, T, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
			if (SpawnedActor)
			{
				SpawnedActor->FinishSpawning(T);
				Spawned.Add(SpawnedActor);
			}
		}

		if (bPreferUnique)
		{
			Used.Add(Entry->LootClass);
		}
	}
}

void AAdventureLootSpawner::ClearSpawned()
{
	for (AActor* A : Spawned)
	{
		if (A) A->Destroy();
	}
	Spawned.Reset();
	bUsed = false;
}

const FAdvLootEntry* AAdventureLootSpawner::PickEntry(FRandomStream& Rng, TSet<TSubclassOf<AActor>>& Used) const
{
	float Total = 0.f;
	for (const FAdvLootEntry& E : LootTable)
	{
		if (!E.LootClass) continue;
		if (bPreferUnique && Used.Contains(E.LootClass)) continue;

		const float RarityPenalty = 1.f / (1.f + E.Rarity);
		Total += FMath::Max(0.0001f, E.Weight * RarityPenalty);
	}

	if (Total <= 0.f) return nullptr;

	const float Pick = Rng.FRandRange(0.f, Total);
	float Running = 0.f;

	for (const FAdvLootEntry& E : LootTable)
	{
		if (!E.LootClass) continue;
		if (bPreferUnique && Used.Contains(E.LootClass)) continue;

		const float RarityPenalty = 1.f / (1.f + E.Rarity);
		Running += FMath::Max(0.0001f, E.Weight * RarityPenalty);

		if (Running >= Pick)
		{
			return &E;
		}
	}

	return &LootTable.Last(); // because math is hard and we like lying to ourselves
}

FVector AAdventureLootSpawner::RandomPointInCircle(FRandomStream& Rng) const
{
	const float Angle = Rng.FRandRange(0.f, 6.283185307f);
	const float Radius = FMath::Sqrt(Rng.FRand()) * SpreadRadius;
	return FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 15.f);
}
