#include "AdventureTraversalComponent.h"

#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

UAdventureTraversalComponent::UAdventureTraversalComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAdventureTraversalComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerChar = Cast<ACharacter>(GetOwner());
}

void UAdventureTraversalComponent::SetMode(ETraversalMode NewMode)
{
	if (Mode == NewMode) return;

	const ETraversalMode Old = Mode;
	Mode = NewMode;
	OnModeChanged.Broadcast(NewMode, Old);
}

void UAdventureTraversalComponent::CancelTraversal()
{
	if (!OwnerChar) return;

	GetWorld()->GetTimerManager().ClearTimer(MoveTimer);
	OwnerChar->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	SetMode(ETraversalMode::None);
}

bool UAdventureTraversalComponent::TryVault()
{
	if (!OwnerChar) return false;
	if (Mode != ETraversalMode::None) return false;

	FVector Target;
	if (!FindVaultTarget(Target)) return false;

	StartMoveTo(Target, ETraversalMode::Vaulting);
	return true;
}

bool UAdventureTraversalComponent::TryClimbLedge()
{
	if (!OwnerChar) return false;
	if (Mode != ETraversalMode::None) return false;

	FVector HangPoint, ClimbPoint;
	if (!FindLedgeTarget(HangPoint, ClimbPoint)) return false;

	// first hang, then climb, because we’re dramatic
	StartMoveTo(HangPoint, ETraversalMode::LedgeHang);

	FTimerHandle Later;
	GetWorld()->GetTimerManager().SetTimer(Later, [this, ClimbPoint]()
	{
		if (!OwnerChar) return;
		if (Mode != ETraversalMode::LedgeHang) return;
		StartMoveTo(ClimbPoint, ETraversalMode::Climbing);
	}, 0.35f, false);

	return true;
}

void UAdventureTraversalComponent::StartMoveTo(const FVector& Target, ETraversalMode NewMode)
{
	if (!OwnerChar) return;

	GetWorld()->GetTimerManager().ClearTimer(MoveTimer);

	OwnerChar->GetCharacterMovement()->StopMovementImmediately();
	OwnerChar->GetCharacterMovement()->DisableMovement();

	SetMode(NewMode);

	MoveStart = OwnerChar->GetActorLocation();
	MoveEnd = Target;
	MoveAlpha = 0.f;

	GetWorld()->GetTimerManager().SetTimer(MoveTimer, [this]()
	{
		if (!OwnerChar) return;

		MoveAlpha = FMath::Clamp(MoveAlpha + 0.06f, 0.f, 1.f);
		const FVector NewLoc = FMath::Lerp(MoveStart, MoveEnd, MoveAlpha);
		OwnerChar->SetActorLocation(NewLoc, true);

		if (MoveAlpha >= 1.f)
		{
			GetWorld()->GetTimerManager().ClearTimer(MoveTimer);
			OwnerChar->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			SetMode(ETraversalMode::None); // wow look, we survived
		}
	}, 0.016f, true);
}

bool UAdventureTraversalComponent::FindVaultTarget(FVector& OutTarget) const
{
	if (!OwnerChar) return false;

	UWorld* World = GetWorld();
	const FVector Start = OwnerChar->GetActorLocation() + OwnerChar->GetActorForwardVector() * 40.f;
	const FVector End = Start + OwnerChar->GetActorForwardVector() * VaultTraceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AdvVault), false);
	Params.AddIgnoredActor(OwnerChar);

	if (!World->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, Params))
	{
		return false;
	}

	const float ObstacleTopZ = Hit.ImpactPoint.Z;
	const float FeetZ = OwnerChar->GetActorLocation().Z;

	const float Height = ObstacleTopZ - FeetZ;
	if (Height < VaultHeightMin || Height > VaultHeightMax)
	{
		return false; // apparently not every box deserves a vault animation
	}

	OutTarget = Hit.ImpactPoint + OwnerChar->GetActorForwardVector() * 90.f;
	OutTarget.Z = FeetZ;

	return true;
}

bool UAdventureTraversalComponent::FindLedgeTarget(FVector& OutHangPoint, FVector& OutClimbPoint) const
{
	if (!OwnerChar) return false;
	UWorld* World = GetWorld();

	const FVector Forward = OwnerChar->GetActorForwardVector();
	const FVector Base = OwnerChar->GetActorLocation();

	const FVector WallStart = Base + FVector(0,0,60.f);
	const FVector WallEnd = WallStart + Forward * ClimbTraceDistance;

	FHitResult WallHit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AdvLedgeWall), false);
	Params.AddIgnoredActor(OwnerChar);

	if (!World->LineTraceSingleByChannel(WallHit, WallStart, WallEnd, TraceChannel, Params))
	{
		return false;
	}

	const FVector TopStart = WallHit.ImpactPoint + FVector(0,0,LedgeGrabHeight);
	const FVector TopEnd = TopStart + FVector(0,0,-(LedgeGrabHeight + 60.f));

	FHitResult TopHit;
	if (!World->LineTraceSingleByChannel(TopHit, TopStart, TopEnd, TraceChannel, Params))
	{
		return false;
	}

	const FVector WallNormal = WallHit.ImpactNormal;
	const FVector HangOffset = WallNormal * 35.f;

	OutHangPoint = FVector(WallHit.ImpactPoint.X, WallHit.ImpactPoint.Y, TopHit.ImpactPoint.Z - 40.f) + HangOffset;
	OutClimbPoint = TopHit.ImpactPoint - WallNormal * 70.f;
	OutClimbPoint.Z = TopHit.ImpactPoint.Z + 5.f;

	return true;
}
