#include "AdventureCombatComponent.h"

#include "AdventureWeapon.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "TimerManager.h"

UAdventureCombatComponent::UAdventureCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAdventureCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPawn = Cast<APawn>(GetOwner());
	OwnerPC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;

	if (OwnerPC && OwnerPC->PlayerCameraManager)
	{
		DefaultFOV = OwnerPC->PlayerCameraManager->GetFOVAngle();
		CurrentFOV = DefaultFOV;
	}

	GetWorld()->GetTimerManager().SetTimer(SoftTickTimer, [this]()
	{
		TickSoft(0.016f);
	}, 0.016f, true);
}

void UAdventureCombatComponent::SetWeapon(AAdventureWeapon* NewWeapon)
{
	Weapon = NewWeapon;
}

void UAdventureCombatComponent::SetADS(bool bNewADS)
{
	bADS = bNewADS && !bSprinting; // because sprint-ADS is for people who enjoy pain
	SetFOVInternal(bADS ? ADSFOV : DefaultFOV);
	OnADSChanged.Broadcast(bADS, CurrentFOV);
}

void UAdventureCombatComponent::StartSprint()
{
	bSprinting = true;
	if (bADS)
	{
		SetADS(false); // nope, pick one
	}
}

void UAdventureCombatComponent::StopSprint()
{
	bSprinting = false;
}

void UAdventureCombatComponent::PullTrigger()
{
	bTriggerHeld = true;
	FireOnce();

	if (Weapon && Weapon->IsAutomatic())
	{
		const float Rate = Weapon->GetFireInterval();
		GetWorld()->GetTimerManager().SetTimer(AutoFireTimer, [this]()
		{
			if (!bTriggerHeld) return;
			FireOnce();
		}, Rate, true);
	}
}

void UAdventureCombatComponent::ReleaseTrigger()
{
	bTriggerHeld = false;
	GetWorld()->GetTimerManager().ClearTimer(AutoFireTimer);
}

bool UAdventureCombatComponent::CanFire() const
{
	if (!Weapon) return false;
	if (!OwnerPawn) return false;
	if (bSprinting) return false; // sprinting and shooting: choose your own adventure, except you can’t
	return true;
}

void UAdventureCombatComponent::FireOnce()
{
	if (!CanFire()) return;

	const FVector TraceStart = Weapon->GetTraceStart(OwnerPawn);
	const FVector AimDir = Weapon->GetAimDirection(OwnerPawn);

	const FVector AimTarget = ComputeAimTargetWithSpread(TraceStart, AimDir);

	AActor* HitActor = nullptr;
	const bool bFired = Weapon->FireAdvanced(OwnerPawn->GetController(), TraceStart, AimTarget, HitActor);

	if (bFired)
	{
		ApplyViewKick();
	}

	if (HitActor)
	{
		OnHitMarker.Broadcast(HitActor);
	}
}

void UAdventureCombatComponent::ApplyViewKick()
{
	if (!OwnerPC) return;

	FRotator R = OwnerPC->GetControlRotation();

	const float KickPitch = FMath::FRandRange(0.75f, 1.35f) * (bADS ? 0.65f : 1.f);
	const float KickYaw = FMath::FRandRange(-0.35f, 0.35f) * (bADS ? 0.6f : 1.f);

	Recoil.Pitch += KickPitch;
	Recoil.Yaw += KickYaw;

	R.Pitch -= KickPitch;
	R.Yaw += KickYaw;

	OwnerPC->SetControlRotation(R);
}

float UAdventureCombatComponent::ComputeSpread() const
{
	float Spread = BaseSpread;
	if (bADS) Spread = ADSSpread;
	if (bSprinting) Spread = SprintSpread;
	return Spread;
}

FVector UAdventureCombatComponent::ComputeAimTargetWithSpread(const FVector& TraceStart, const FVector& AimDir) const
{
	const float Spread = ComputeSpread();

	const FVector Right = FVector::CrossProduct(AimDir, FVector::UpVector).GetSafeNormal();
	const FVector Up = FVector::CrossProduct(Right, AimDir).GetSafeNormal();

	const float Rx = FMath::FRandRange(-Spread, Spread);
	const float Ry = FMath::FRandRange(-Spread, Spread);

	const FVector Jittered = (AimDir + Right * (Rx * 0.01f) + Up * (Ry * 0.01f)).GetSafeNormal();
	return TraceStart + Jittered * 100000.f;
}

void UAdventureCombatComponent::SetFOVInternal(float TargetFOV)
{
	if (!OwnerPC || !OwnerPC->PlayerCameraManager) return;

	CurrentFOV = TargetFOV;
	OwnerPC->PlayerCameraManager->SetFOV(TargetFOV);
}

void UAdventureCombatComponent::ApplyWeaponSway(float DeltaSeconds)
{
	if (!OwnerPC) return;

	SwayTime += DeltaSeconds * SwaySpeed;

	const float S = FMath::Sin(SwayTime) * SwayAmount * (bADS ? 0.35f : 1.f);
	const float C = FMath::Cos(SwayTime * 0.85f) * SwayAmount * (bADS ? 0.35f : 1.f);

	FRotator R = OwnerPC->GetControlRotation();
	R.Yaw += S * 0.05f;
	R.Pitch += C * 0.05f;

	OwnerPC->SetControlRotation(R);
}

void UAdventureCombatComponent::TickSoft(float DeltaSeconds)
{
	if (OwnerPC && (Recoil.Pitch != 0.f || Recoil.Yaw != 0.f))
	{
		FRotator R = OwnerPC->GetControlRotation();

		const float ReturnPitch = FMath::FInterpTo(Recoil.Pitch, 0.f, DeltaSeconds, Recoil.ReturnSpeed);
		const float ReturnYaw = FMath::FInterpTo(Recoil.Yaw, 0.f, DeltaSeconds, Recoil.ReturnSpeed);

		const float PitchDelta = ReturnPitch - Recoil.Pitch;
		const float YawDelta = ReturnYaw - Recoil.Yaw;

		Recoil.Pitch = ReturnPitch;
		Recoil.Yaw = ReturnYaw;

		R.Pitch -= PitchDelta;
		R.Yaw -= YawDelta;

		OwnerPC->SetControlRotation(R);
	}

	ApplyWeaponSway(DeltaSeconds);

	if (OwnerPC && OwnerPC->PlayerCameraManager)
	{
		const float Target = bADS ? ADSFOV : DefaultFOV;
		const float NewFOV = FMath::FInterpTo(OwnerPC->PlayerCameraManager->GetFOVAngle(), Target, DeltaSeconds, FOVInterpSpeed);
		OwnerPC->PlayerCameraManager->SetFOV(NewFOV);
	}
}
