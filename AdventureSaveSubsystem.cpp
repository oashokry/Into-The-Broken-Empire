#include "AdventureSaveSubsystem.h"

#include "AdventureSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"

#include "AdventureInventoryComponent.h"
#include "AdventureObjectiveSubsystem.h"
#include "AdventureHealthComponent.h"
#include "AdventureStaminaComponent.h"

bool UAdventureSaveSubsystem::HasSave(const FString& SlotName, int32 UserIndex) const
{
	return UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

void UAdventureSaveSubsystem::DeleteSave(const FString& SlotName, int32 UserIndex)
{
	UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
}

UAdventureSaveGame* UAdventureSaveSubsystem::BuildSave(AActor* PlayerActor) const
{
	if (!PlayerActor) return nullptr;

	UAdventureSaveGame* Save = Cast<UAdventureSaveGame>(UGameplayStatics::CreateSaveGameObject(UAdventureSaveGame::StaticClass()));
	if (!Save) return nullptr;

	UWorld* World = PlayerActor->GetWorld();
	if (World)
	{
		Save->MapName = World->GetMapName();
	}

	Save->PlayerTransform = PlayerActor->GetActorTransform();

	PullVitals(PlayerActor, Save);
	PullInventory(PlayerActor, Save);
	PullObjectives(Save);
	PullCheckpoint(PlayerActor, Save);

	return Save;
}

bool UAdventureSaveSubsystem::SaveToSlot(const FString& SlotName, int32 UserIndex)
{
	UWorld* World = GetWorld();
	if (!World) return false;

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!Pawn) return false;

	UAdventureSaveGame* Save = BuildSave(Pawn);
	if (!Save) return false;

	const bool bOk = UGameplayStatics::SaveGameToSlot(Save, SlotName, UserIndex);
	if (bOk)
	{
		Loaded = Save;
	}
	return bOk;
}

bool UAdventureSaveSubsystem::LoadFromSlot(const FString& SlotName, int32 UserIndex)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		return false; // no save found, shocking
	}

	USaveGame* Raw = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex);
	Loaded = Cast<UAdventureSaveGame>(Raw);
	return Loaded != nullptr;
}

bool UAdventureSaveSubsystem::ApplyLoadedState(AActor* PlayerActor)
{
	if (!Loaded) return false;
	if (!PlayerActor) return false;

	PlayerActor->SetActorTransform(Loaded->PlayerTransform);

	PushVitals(PlayerActor, Loaded);
	PushInventory(PlayerActor, Loaded);
	PushObjectives(Loaded);
	PushCheckpoint(PlayerActor, Loaded);

	return true;
}

void UAdventureSaveSubsystem::PullInventory(AActor* PlayerActor, UAdventureSaveGame* Save) const
{
	if (!Save) return;

	UAdventureInventoryComponent* Inv = PlayerActor ? PlayerActor->FindComponentByClass<UAdventureInventoryComponent>() : nullptr;
	if (!Inv) return;

	Save->Inventory.Reset();

	for (const FAdventureInventoryItem& It : Inv->GetItems())
	{
		FAdvSavedInventoryItem S;
		S.ItemId = It.ItemId;
		S.Count = It.Count;
		Save->Inventory.Add(S);
	}
}

void UAdventureSaveSubsystem::PushInventory(AActor* PlayerActor, const UAdventureSaveGame* Save) const
{
	if (!Save) return;

	UAdventureInventoryComponent* Inv = PlayerActor ? PlayerActor->FindComponentByClass<UAdventureInventoryComponent>() : nullptr;
	if (!Inv) return;

	Inv->Clear();

	for (const FAdvSavedInventoryItem& S : Save->Inventory)
	{
		FAdventureInventoryItem It;
		It.ItemId = S.ItemId;
		It.Count = S.Count;
		It.bIsKeyItem = true;
		It.DisplayName = FText::FromName(S.ItemId);
		Inv->AddItem(It);
	}
}

void UAdventureSaveSubsystem::PullObjectives(UAdventureSaveGame* Save) const
{
	if (!Save) return;
	if (!GetGameInstance()) return;

	if (UAdventureObjectiveSubsystem* Obj = GetGameInstance()->GetSubsystem<UAdventureObjectiveSubsystem>())
	{
		Save->ObjectivesState = Obj->SerializeState();
	}
}

void UAdventureSaveSubsystem::PushObjectives(const UAdventureSaveGame* Save) const
{
	if (!Save) return;
	if (!GetGameInstance()) return;

	if (UAdventureObjectiveSubsystem* Obj = GetGameInstance()->GetSubsystem<UAdventureObjectiveSubsystem>())
	{
		Obj->DeserializeState(Save->ObjectivesState);
	}
}

void UAdventureSaveSubsystem::PullVitals(AActor* PlayerActor, UAdventureSaveGame* Save) const
{
	if (!Save || !PlayerActor) return;

	if (UAdventureHealthComponent* H = PlayerActor->FindComponentByClass<UAdventureHealthComponent>())
	{
		Save->SavedHealth = H->GetHealth();
	}

	if (UAdventureStaminaComponent* S = PlayerActor->FindComponentByClass<UAdventureStaminaComponent>())
	{
		Save->SavedStamina = S->GetStamina();
	}
}

void UAdventureSaveSubsystem::PushVitals(AActor* PlayerActor, const UAdventureSaveGame* Save) const
{
	if (!Save || !PlayerActor) return;

	if (UAdventureHealthComponent* H = PlayerActor->FindComponentByClass<UAdventureHealthComponent>())
	{
		const float Delta = Save->SavedHealth - H->GetHealth();
		if (Delta > 0) H->Heal(Delta, nullptr);
	}

	if (UAdventureStaminaComponent* S = PlayerActor->FindComponentByClass<UAdventureStaminaComponent>())
	{
		const float Delta = Save->SavedStamina - S->GetStamina();
		if (Delta > 0) S->Refill(Delta);
	}
}

void UAdventureSaveSubsystem::PullCheckpoint(AActor* PlayerActor, UAdventureSaveGame* Save) const
{
	if (!Save || !PlayerActor) return;

	// yes, tags, because nothing says “robust” like string hacks
	for (const FName& T : PlayerActor->Tags)
	{
		const FString TS = T.ToString();
		if (TS.StartsWith(TEXT("LastCheckpoint=")))
		{
			Save->LastCheckpointId = FName(*TS.RightChop(15));
		}
		else if (TS.StartsWith(TEXT("LastCheckpointTransform=")))
		{
			FString XformStr = TS.RightChop(24);
			FTransform Xf;
			Xf.InitFromString(XformStr);
			Save->LastCheckpointTransform = Xf;
		}
	}
}

void UAdventureSaveSubsystem::PushCheckpoint(AActor* PlayerActor, const UAdventureSaveGame* Save) const
{
	if (!Save || !PlayerActor) return;

	if (!Save->LastCheckpointId.IsNone())
	{
		PlayerActor->Tags.AddUnique(FName(*FString::Printf(TEXT("LastCheckpoint=%s"), *Save->LastCheckpointId.ToString())));
	}

	// yes, we’re doing this again
	if (!Save->LastCheckpointTransform.Equals(FTransform::Identity))
	{
		PlayerActor->Tags.AddUnique(FName(*FString::Printf(TEXT("LastCheckpointTransform=%s"), *Save->LastCheckpointTransform.ToString())));
	}
}
