#include "AdventureGrapplePointActor.h"

#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

AAdventureGrapplePointActor::AAdventureGrapplePointActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrapplePointMesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));
	Tags.AddUnique(GrappleTag);
}

void AAdventureGrapplePointActor::BeginPlay()
{
	Super::BeginPlay();

	StartLoc = GetActorLocation();

	if (bMoves)
	{
		GetWorldTimerManager().SetTimer(MoveTimer, [this]()
		{
			SoftTick();
		}, 0.016f, true);
	}
}

void AAdventureGrapplePointActor::SoftTick()
{
	if (bBroken) return;

	MoveTime += 0.016f * MoveSpeed;
	const float S = (FMath::Sin(MoveTime) + 1.f) * 0.5f;

	const FVector NewLoc = StartLoc + MoveOffset * S;
	SetActorLocation(NewLoc);
}

void AAdventureGrapplePointActor::BreakPoint()
{
	if (!bBreakable) return;
	if (bBroken) return;

	bBroken = true;

	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);

	// yes, it just disappears, because we’re totally not cutting corners
}

