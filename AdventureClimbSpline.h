#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdventureClimbSpline.generated.h"

class USplineComponent;

USTRUCT(BlueprintType)
struct FAdvClimbPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceOnSpline = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Difficulty = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRestSpot = false;
};

UCLASS()
class SIMPLESHOOTER_API AAdventureClimbSpline : public AActor
{
	GENERATED_BODY()

public:
	AAdventureClimbSpline();

	UFUNCTION(BlueprintCallable, Category="Climb")
	USplineComponent* GetSpline() const { return Spline; }

	UFUNCTION(BlueprintCallable, Category="Climb")
	bool GetClosestDistance(const FVector& WorldPos, float& OutDistance) const;

	UFUNCTION(BlueprintCallable, Category="Climb")
	FVector GetWorldAtDistance(float Distance) const;

	UFUNCTION(BlueprintCallable, Category="Climb")
	FRotator GetRotationAtDistance(float Distance) const;

	UFUNCTION(BlueprintCallable, Category="Climb")
	bool GetNearestClimbPoint(float Distance, FAdvClimbPoint& OutPoint) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Climb")
	TArray<FAdvClimbPoint> Points;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	USplineComponent* Spline;

	UPROPERTY(EditDefaultsOnly, Category="Climb")
	float SnapTolerance = 120.f;
};
