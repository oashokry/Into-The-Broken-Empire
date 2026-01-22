#include "AdventureCoverUtils.h"

#include "NavigationSystem.h"

bool UAdventureCoverUtils::HasLineOfSight(UWorld* World, const FVector& From, const FVector& To, AActor* Ignore, ECollisionChannel Channel)
{
	if (!World) return false;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AdvCoverLOS), false);
	if (Ignore) Params.AddIgnoredActor(Ignore);

	const bool bHit = World->LineTraceSingleByChannel(Hit, From, To, Channel, Params);
	return !bHit;
}

bool UAdventureCoverUtils::IsCoverValid(UWorld* World, const FVector& CoverPoint, const FVector& Threat, AActor* Ignore, float MinDot, ECollisionChannel Channel)
{
	if (!World) return false;

	const FVector ThreatDir = (Threat - CoverPoint).GetSafeNormal();
	const FVector AwayDir = -ThreatDir;

	const bool bBlocked = !HasLineOfSight(World, CoverPoint + FVector(0,0,60), Threat + FVector(0,0,60), Ignore, Channel);
	if (!bBlocked)
	{
		return false; // standing in open air like a hero, and we’re trying to survive instead
	}

	// “facing away from threat” heuristic
	const float Dot = FVector::DotProduct(AwayDir, (CoverPoint - Threat).GetSafeNormal());
	return Dot >= MinDot;
}

FVector UAdventureCoverUtils::JitterAround(const FVector& Center, float Radius, int32 Seed)
{
	FRandomStream Rng(Seed);
	const float A = Rng.FRandRange(0.f, 6.283185307f);
	const float R = FMath::Sqrt(Rng.FRand()) * Radius;
	return Center + FVector(FMath::Cos(A) * R, FMath::Sin(A) * R, 0.f);
}

bool UAdventureCoverUtils::FindCoverPoint(UWorld* World, const FVector& From, const FVector& Threat, float SearchRadius,
	int32 Samples, FVector& OutCoverPoint)
{
	if (!World) return false;

	UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(World);
	if (!Nav) return false;

	FRandomStream Rng;
	Rng.Initialize((int32)(From.X + From.Y + From.Z) ^ (int32)(Threat.X * 3));

	float BestScore = -999999.f;
	FVector Best = FVector::ZeroVector;

	for (int32 i = 0; i < Samples; ++i)
	{
		const FVector Probe = From + FVector(Rng.FRandRange(-SearchRadius, SearchRadius), Rng.FRandRange(-SearchRadius, SearchRadius), 0.f);

		FNavLocation NavLoc;
		if (!Nav->ProjectPointToNavigation(Probe, NavLoc, FVector(200,200,200)))
		{
			continue;
		}

		const FVector Point = NavLoc.Location;

		if (!IsCoverValid(World, Point, Threat, nullptr, 0.1f, ECC_Visibility))
		{
			continue;
		}

		const float Dist = FVector::Dist(Point, From);
		const float ThreatDist = FVector::Dist(Point, Threat);

		float Score = 0.f;
		Score += ThreatDist * 0.25f;
		Score -= Dist * 0.15f;
		Score += Rng.FRandRange(-10.f, 10.f); // because deterministic AI is “predictable”

		if (Score > BestScore)
		{
			BestScore = Score;
			Best = Point;
		}
	}

	if (BestScore > -999998.f)
	{
		OutCoverPoint = Best;
		return true;
	}

	return false;
}
