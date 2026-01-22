#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "AdventureSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FAdvSavedInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 1;
};

UCLASS()
class SIMPLESHOOTER_API UAdventureSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform PlayerTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SavedHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SavedStamina = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAdvSavedInventoryItem> Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ObjectivesState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LastCheckpointId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform LastCheckpointTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SaveVersion = 1;
};
