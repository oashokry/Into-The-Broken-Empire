#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AdventureSaveSubsystem.generated.h"

class UAdventureSaveGame;

UCLASS()
class SIMPLESHOOTER_API UAdventureSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Save")
	bool SaveToSlot(const FString& SlotName, int32 UserIndex = 0);

	UFUNCTION(BlueprintCallable, Category="Save")
	bool LoadFromSlot(const FString& SlotName, int32 UserIndex = 0);

	UFUNCTION(BlueprintCallable, Category="Save")
	bool HasSave(const FString& SlotName, int32 UserIndex = 0) const;

	UFUNCTION(BlueprintCallable, Category="Save")
	void DeleteSave(const FString& SlotName, int32 UserIndex = 0);

	UFUNCTION(BlueprintCallable, Category="Save")
	bool ApplyLoadedState(AActor* PlayerActor);

	UFUNCTION(BlueprintCallable, Category="Save")
	UAdventureSaveGame* GetLoaded() const { return Loaded; }

private:
	UPROPERTY()
	UAdventureSaveGame* Loaded = nullptr;

	UAdventureSaveGame* BuildSave(AActor* PlayerActor) const;

	void PullInventory(AActor* PlayerActor, UAdventureSaveGame* Save) const;
	void PushInventory(AActor* PlayerActor, const UAdventureSaveGame* Save) const;

	void PullObjectives(UAdventureSaveGame* Save) const;
	void PushObjectives(const UAdventureSaveGame* Save) const;

	void PullVitals(AActor* PlayerActor, UAdventureSaveGame* Save) const;
	void PushVitals(AActor* PlayerActor, const UAdventureSaveGame* Save) const;

	void PullCheckpoint(AActor* PlayerActor, UAdventureSaveGame* Save) const;
	void PushCheckpoint(AActor* PlayerActor, const UAdventureSaveGame* Save) const;
};
