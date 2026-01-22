#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AdventureCoverUtils.generated.h"

UCLASS()
class SIMPLESHOOTER_API UAdventureCoverUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Cover")
	static bool FindCoverPoint(UWorld* World, const FVector& From, const FVector& Threat, float SearchRadius,
		int32 Samples, FVector& OutCoverPoint);

	UFUNCTION(BlueprintCallable, Category="Cover")
	static bool HasLineOfSight(UWorld* World, const FVector& From, const FVector& To, AActor* Ignore, ECollisionChannel Channel);

	UFUNCTION(BlueprintCallable, Category="Cover")
	static bool IsCoverValid(UWorld* World, const FVector& CoverPoint, const FVector& Threat, AActor* Ignore, float MinDot,
		ECollisionChannel Channel);

	UFUNCTION(BlueprintCallable, Category="Cover")
	static FVector JitterAround(const FVector& Center, float Radius, int32 Seed);
};
