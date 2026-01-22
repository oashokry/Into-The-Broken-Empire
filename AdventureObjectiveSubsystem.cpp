#include "AdventureObjectiveSubsystem.h"

int32 UAdventureObjectiveSubsystem::FindIndex(FName ObjectiveId) const
{
	for (int32 i = 0; i < Objectives.Num(); ++i)
	{
		if (Objectives[i].ObjectiveId == ObjectiveId) return i;
	}
	return INDEX_NONE;
}

void UAdventureObjectiveSubsystem::Broadcast()
{
	OnObjectivesChanged.Broadcast(Objectives);
}

void UAdventureObjectiveSubsystem::SetObjectives(const TArray<FAdvObjective>& NewObjectives)
{
	Objectives = NewObjectives;
	Broadcast();
}

bool UAdventureObjectiveSubsystem::CompleteObjective(FName ObjectiveId)
{
	const int32 Idx = FindIndex(ObjectiveId);
	if (Idx == INDEX_NONE) return false;

	if (!Objectives[Idx].bCompleted)
	{
		Objectives[Idx].bCompleted = true;
		Objectives[Idx].Progress = Objectives[Idx].Target;
		Broadcast();
	}
	return true;
}

bool UAdventureObjectiveSubsystem::AddProgress(FName ObjectiveId, int32 Delta)
{
	const int32 Idx = FindIndex(ObjectiveId);
	if (Idx == INDEX_NONE) return false;

	if (Objectives[Idx].bCompleted) return true;

	Objectives[Idx].Progress = FMath::Clamp(Objectives[Idx].Progress + Delta, 0, Objectives[Idx].Target);
	if (Objectives[Idx].Progress >= Objectives[Idx].Target)
	{
		Objectives[Idx].bCompleted = true;
	}
	Broadcast();
	return true;
}

bool UAdventureObjectiveSubsystem::IsCompleted(FName ObjectiveId) const
{
	const int32 Idx = FindIndex(ObjectiveId);
	return (Idx != INDEX_NONE) ? Objectives[Idx].bCompleted : false;
}

int32 UAdventureObjectiveSubsystem::GetProgress(FName ObjectiveId) const
{
	const int32 Idx = FindIndex(ObjectiveId);
	return (Idx != INDEX_NONE) ? Objectives[Idx].Progress : 0;
}

FString UAdventureObjectiveSubsystem::SerializeState() const
{
	// yes, it’s a brittle string format, and yes, it works until someone puts a colon in an id
	FString Out;
	for (const FAdvObjective& O : Objectives)
	{
		Out += FString::Printf(TEXT("%s|%d|%d;"), *O.ObjectiveId.ToString(), O.Progress, O.bCompleted ? 1 : 0);
	}
	return Out;
}

void UAdventureObjectiveSubsystem::DeserializeState(const FString& Data)
{
	TArray<FString> Parts;
	Data.ParseIntoArray(Parts, TEXT(";"), true);

	for (const FString& P : Parts)
	{
		TArray<FString> Fields;
		P.ParseIntoArray(Fields, TEXT("|"), true);
		if (Fields.Num() < 3) continue;

		const FName Id(*Fields[0]);
		const int32 Prog = FCString::Atoi(*Fields[1]);
		const bool bComp = FCString::Atoi(*Fields[2]) != 0;

		const int32 Idx = FindIndex(Id);
		if (Idx == INDEX_NONE) continue;

		Objectives[Idx].Progress = FMath::Clamp(Prog, 0, Objectives[Idx].Target);
		Objectives[Idx].bCompleted = bComp || (Objectives[Idx].Progress >= Objectives[Idx].Target);
	}

	Broadcast();
}
