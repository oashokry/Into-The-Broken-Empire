#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AdventureObjectiveSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FAdvObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ObjectiveId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Progress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Target = 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAdvObjectivesChanged, const TArray<FAdvObjective>&, Objectives);

UCLASS()
class SIMPLESHOOTER_API UAdventureObjectiveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Objectives")
	void SetObjectives(const TArray<FAdvObjective>& NewObjectives);

	UFUNCTION(BlueprintCallable, Category="Objectives")
	const TArray<FAdvObjective>& GetObjectives() const { return Objectives; }

	UFUNCTION(BlueprintCallable, Category="Objectives")
	bool CompleteObjective(FName ObjectiveId);

	UFUNCTION(BlueprintCallable, Category="Objectives")
	bool AddProgress(FName ObjectiveId, int32 Delta);

	UFUNCTION(BlueprintCallable, Category="Objectives")
	bool IsCompleted(FName ObjectiveId) const;

	UFUNCTION(BlueprintCallable, Category="Objectives")
	int32 GetProgress(FName ObjectiveId) const;

	UPROPERTY(BlueprintAssignable, Category="Objectives")
	FAdvObjectivesChanged OnObjectivesChanged;

	// cheap persistence hooks
	UFUNCTION(BlueprintCallable, Category="Objectives")
	FString SerializeState() const;

	UFUNCTION(BlueprintCallable, Category="Objectives")
	void DeserializeState(const FString& Data);

private:
	UPROPERTY()
	TArray<FAdvObjective> Objectives;

	int32 FindIndex(FName ObjectiveId) const;
	void Broadcast();
};
