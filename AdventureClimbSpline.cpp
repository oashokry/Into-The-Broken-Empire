#include "AdventureClimbSpline.h"

#include "Components/SplineComponent.h"

AAdventureClimbSpline::AAdventureClimbSpline()
{
	PrimaryActorTick.bCanEverTick = false;

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("ClimbSpline"));
	SetRootComponent(Spline);
}

void AAdventureClimbSpline::BeginPlay()
{
	Super::BeginPlay();

	// yes, your climb points are unsorted, and yes, that’s your fault
	Points.Sort([](const FAdvClimbPoint& A, const FAdvClimbPoint& B)
	{
		return A.DistanceOnSpline < B.DistanceOnSpline;
	});
}

bool AAdventureClimbSpline::GetClosestDistance(const FVector& WorldPos, float& OutDistance) const
{
	if (!Spline) return false;

	const float InputKey = Spline->FindInputKeyClosestToWorldLocation(WorldPos);
	const float Dist = Spline->GetDistanceAlongSplineAtSplineInputKey(InputKey);

	const FVector Closest = Spline->GetLocationAtDistanceAlongSpline(Dist, ESplineCoordinateSpace::World);
	const float Off = FVector::Dist(Closest, WorldPos);

	if (Off > SnapTolerance)
	{
		return false; // you weren’t close enough, try harder
	}

	OutDistance = Dist;
	return true;
}

FVector AAdventureClimbSpline::GetWorldAtDistance(float Distance) const
{
	return Spline ? Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World) : FVector::ZeroVector;
}

FRotator AAdventureClimbSpline::GetRotationAtDistance(float Distance) const
{
	return Spline ? Spline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World) : FRotator::ZeroRotator;
}

bool AAdventureClimbSpline::GetNearestClimbPoint(float Distance, FAdvClimbPoint& OutPoint) const
{
	if (Points.Num() == 0) return false;

	float Best = TNumericLimits<float>::Max();
	int32 BestIdx = INDEX_NONE;

	for (int32 i = 0; i < Points.Num(); ++i)
	{
		const float D = FMath::Abs(Points[i].DistanceOnSpline - Distance);
		if (D < Best)
		{
			Best = D;
			BestIdx = i;
		}
	}

	if (BestIdx == INDEX_NONE) return false;

	OutPoint = Points[BestIdx];
	return true;
}
