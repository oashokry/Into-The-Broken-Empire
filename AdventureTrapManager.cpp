#include "AdventureTrapManager.h"

int32 AAdventureTrapManager::FindGroup(FName GroupId) const
{
	for (int32 i = 0; i < Groups.Num(); ++i)
	{
		if (Groups[i].GroupId == GroupId) return i;
	}
	return INDEX_NONE;
}

AAdventureTrapManager::AAdventureTrapManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAdventureTrapManager::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoBuildOnBeginPlay)
	{
		BuildGroupsFromTags();
	}
}

void AAdventureTrapManager::BuildGroupsFromTags()
{
	Groups.Reset();

	UWorld* World = GetWorld();
	if (!World) return;

	TArray<AActor*> All;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		All.Add(*It);
	}

	for (AActor* A : All)
	{
		if (!A) continue;

		for (const FName& Tag : A->Tags)
		{
			const FString S = Tag.ToString();
			if (!S.StartsWith(TrapTagPrefix.ToString())) continue;

			const FString GroupStr = S.RightChop(TrapTagPrefix.ToString().Len());
			const FName GroupId(*GroupStr);

			int32 Idx = FindGroup(GroupId);
			if (Idx == INDEX_NONE)
			{
				FAdvTrapGroup G;
				G.GroupId = GroupId;
				G.bEnabled = true;
				Groups.Add(G);
				Idx = Groups.Num() - 1;
			}

			Groups[Idx].Traps.Add(A);
		}
	}
}

void AAdventureTrapManager::ApplyToActor(AActor* Trap, bool bEnabled)
{
	if (!Trap) return;

	Trap->SetActorTickEnabled(bEnabled);
	Trap->SetActorEnableCollision(bEnabled);

	// yes, disabling collision is a sledgehammer; no, I don’t care
}

void AAdventureTrapManager::SetGroupEnabled(FName GroupId, bool bEnabled)
{
	const int32 Idx = FindGroup(GroupId);
	if (Idx == INDEX_NONE) return;

	Groups[Idx].bEnabled = bEnabled;

	for (auto& Weak : Groups[Idx].Traps)
	{
		ApplyToActor(Weak.Get(), bEnabled);
	}
}

bool AAdventureTrapManager::IsGroupEnabled(FName GroupId) const
{
	const int32 Idx = FindGroup(GroupId);
	return (Idx != INDEX_NONE) ? Groups[Idx].bEnabled : false;
}

void AAdventureTrapManager::ToggleAll(bool bEnabled)
{
	for (FAdvTrapGroup& G : Groups)
	{
		G.bEnabled = bEnabled;
		for (auto& Weak : G.Traps)
		{
			ApplyToActor(Weak.Get(), bEnabled);
		}
	}
}
