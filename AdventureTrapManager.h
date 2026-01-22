#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdventureTrapManager.generated.h"

USTRUCT(BlueprintType)
struct FAdvTrapGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GroupId = TEXT("Default");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TWeakObjectPtr<AActor>> Traps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;
};

UCLASS()
class SIMPLESHOOTER_API AAdventureTrapManager : public AActor
{
	GENERATED_BODY()

public:
	AAdventureTrapManager();

	UFUNCTION(BlueprintCallable, Category="Traps")
	void BuildGroupsFromTags();

	UFUNCTION(BlueprintCallable, Category="Traps")
	void SetGroupEnabled(FName GroupId, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category="Traps")
	bool IsGroupEnabled(FName GroupId) const;

	UFUNCTION(BlueprintCallable, Category="Traps")
	void ToggleAll(bool bEnabled);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Traps")
	FName TrapTagPrefix = TEXT("TrapGroup=");

	UPROPERTY(EditDefaultsOnly, Category="Traps")
	bool bAutoBuildOnBeginPlay = true;

	UPROPERTY(EditDefaultsOnly, Category="Debug")
	bool bDebug = false;

private:
	UPROPERTY()
	TArray<FAdvTrapGroup> Groups;

	int32 FindGroup(FName GroupId) const;
	void ApplyToActor(AActor* Trap, bool bEnabled);
};
