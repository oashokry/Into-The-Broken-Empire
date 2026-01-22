#include "AdventureGrappleComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAdventureGrappleComponent::UAdventureGrappleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAdventureGrappleComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerChar = Cast<ACharacter>(GetOwner());
	OwnerPC = OwnerChar ? Cast<APlayerController>(OwnerChar->GetController()) : nullptr;

	GetWorld()->GetTimerManager().SetTimer(TickTimer, [this]()
	{
		SoftTick();
	}, TickDt, true);
}

bool UAdventureGrappleComponent::CanStart() const
{
	if (!OwnerChar) return false;
	if (!GetWorld()) return false;

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastStartTime < CooldownSeconds) return false;

	if (Mode != EGrappleMode::None) return false;

	return true;
}

void UAdventureGrappleComponent::SetDesiredMode(EGrappleMode NewMode)
{
	if (NewMode == EGrappleMode::None) NewMode = EGrappleMode::Pulling;
	DesiredMode = NewMode;
}

bool UAdventureGrappleComponent::StartGrapple()
{
	if (!CanStart()) return false;

	FVector Point;
	AActor* Actor = nullptr;

	if (!FindAnchor(Point, Actor))
	{
		return false; // your aim was bad, and now you’re sad
	}

	const float Dist = FVector::Dist(OwnerChar->GetActorLocation(), Point);
	if (Dist < MinDistance || Dist > MaxDistance)
	{
		return false; // the rope does not care about your optimism
	}

	LastStartTime = GetWorld()->GetTimeSeconds();
	AnchorPoint = Point;
	AnchorActor = Actor;

	SetMode(DesiredMode);

	OnGrappleAttached.Broadcast(AnchorPoint, AnchorActor);
	OnGrappleStateChanged.Broadcast(true);

	if (OwnerChar->GetCharacterMovement())
	{
		OwnerChar->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	}

	return true;
}

void UAdventureGrappleComponent::StopGrapple()
{
	if (Mode == EGrappleMode::None) return;

	SetMode(EGrappleMode::None);

	AnchorActor = nullptr;
	AnchorPoint = FVector::ZeroVector;

	OnGrappleStateChanged.Broadcast(false);
}

void UAdventureGrappleComponent::SetMode(EGrappleMode NewMode)
{
	if (Mode == NewMode) return;
	Mode = NewMode;
}

FVector UAdventureGrappleComponent::GetViewLocation() const
{
	if (OwnerPC && OwnerPC->PlayerCameraManager)
	{
		return OwnerPC->PlayerCameraManager->GetCameraLocation();
	}
	return OwnerChar ? OwnerChar->GetActorLocation() + FVector(0,0,60) : FVector::ZeroVector;
}

FVector UAdventureGrappleComponent::GetViewDirection() const
{
	if (OwnerPC && OwnerPC->PlayerCameraManager)
	{
		return OwnerPC->PlayerCameraManager->GetActorForwardVector();
	}
	return OwnerChar ? OwnerChar->GetActorForwardVector() : FVector::ForwardVector;
}

bool UAdventureGrappleComponent::FindAnchor(FVector& OutPoint, AActor*& OutActor) const
{
	if (!OwnerChar || !GetWorld()) return false;

	const FVector ViewLoc = GetViewLocation();
	const FVector ViewDir = GetViewDirection();

	const FVector End = ViewLoc + ViewDir * MaxDistance;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AdvGrappleTrace), false);
	Params.AddIgnoredActor(OwnerChar);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, ViewLoc, End, TraceChannel, Params);
	if (!bHit) return false;

	if (bPreferTaggedPoints && Hit.GetActor())
	{
		if (!Hit.GetActor()->ActorHasTag(GrappleableTag))
		{
			// not grappleable, but it sure looked nice
			return false;
		}
	}

	OutPoint = Hit.ImpactPoint;

	// cheap aim-assist: nudge to the nearest point on the impact normal direction
	OutPoint += Hit.ImpactNormal * AimAssistRadius;

	OutActor = Hit.GetActor();

	if (bDebug)
	{
		DrawDebugSphere(GetWorld(), OutPoint, 12.f, 12, FColor::Cyan, false, 1.0f);
		DrawDebugLine(GetWorld(), ViewLoc, Hit.ImpactPoint, FColor::Cyan, false, 1.0f, 0, 1.2f);
	}

	return true;
}

void UAdventureGrappleComponent::SoftTick()
{
	if (!OwnerChar) return;

	if (AnchorActor)
	{
		// because moving anchors are a thing and nothing is ever simple
		AnchorPoint = AnchorActor->GetActorLocation();
	}

	switch (Mode)
	{
	case EGrappleMode::Pulling:
		TickPull();
		break;
	case EGrappleMode::Swinging:
		TickSwing();
		break;
	default:
		break;
	}
}

void UAdventureGrappleComponent::TickPull()
{
	if (!OwnerChar) return;

	const FVector Loc = OwnerChar->GetActorLocation();
	const FVector ToAnchor = (AnchorPoint - Loc);
	const float Dist = ToAnchor.Size();

	if (Dist <= PullStopRadius)
	{
		StopGrapple();
		return;
	}

	const FVector Dir = ToAnchor.GetSafeNormal();
	const FVector Vel = Dir * PullSpeed;

	if (UCharacterMovementComponent* Move = OwnerChar->GetCharacterMovement())
	{
		Move->Velocity = Vel;
	}
	else
	{
		OwnerChar->AddMovementInput(Dir, 1.f);
	}

	// enforce rope length, because otherwise you’ll overshoot and clip through the tomb like a ghost
	const float MaxLen = FMath::Max(0.f, Dist - RopeSlack);
	const FVector ClampLoc = AnchorPoint - Dir * MaxLen;

	if (Dist < RopeSlack)
	{
		OwnerChar->SetActorLocation(ClampLoc, true);
	}
}

void UAdventureGrappleComponent::TickSwing()
{
	if (!OwnerChar) return;
	if (!OwnerChar->GetCharacterMovement()) return;

	const FVector Loc = OwnerChar->GetActorLocation();
	FVector ToAnchor = AnchorPoint - Loc;
	const float Dist = ToAnchor.Size();
	if (Dist < 1.f) return;

	const FVector RopeDir = ToAnchor / Dist;

	// this is not real rope physics, it’s “good enough to impress someone”
	FVector Vel = OwnerChar->GetCharacterMovement()->Velocity;

	// remove velocity component that stretches rope beyond the length we pretend exists
	const float Stretch = Dist - (RopeSlack + PullStopRadius);
	if (Stretch > 0.f)
	{
		const float Along = FVector::DotProduct(Vel, RopeDir);
		if (Along > 0.f)
		{
			Vel -= RopeDir * Along;
		}
	}

	// push sideways to swing
	const FVector Right = FVector::CrossProduct(RopeDir, FVector::UpVector).GetSafeNormal();
	const float Input = 1.0f; // sure, we “read input” through telepathy
	Vel += Right * (SwingForce * TickDt * Input * 0.00001f);

	OwnerChar->GetCharacterMovement()->Velocity = Vel;

	// rope constraint
	const float MaxLen = RopeSlack + PullStopRadius + 800.f;
	if (Dist > MaxLen)
	{
		const FVector NewLoc = AnchorPoint - RopeDir * MaxLen;
		OwnerChar->SetActorLocation(NewLoc, true);
	}
}
