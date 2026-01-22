#include "AdventureWeapon.h"

#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

AAdventureWeapon::AAdventureWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CurrentSpread = BaseSpreadDegrees;
}

void AAdventureWeapon::BeginPlay()
{
	Super::BeginPlay();

	AmmoInMag = FMath::Clamp(AmmoInMag, 0, MagCapacity);
	SpareAmmo = FMath::Max(SpareAmmo, 0);

	BroadcastAmmo();
}

void AAdventureWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Recover spread + cool heat over time.
	CurrentSpread = FMath::Max(BaseSpreadDegrees, CurrentSpread - SpreadRecoveryPerSecond * DeltaSeconds);
	Heat = FMath::Max(0.f, Heat - HeatCoolPerSecond * DeltaSeconds);

	if (Heat > 0.75f)
	{
		const float Extra = (Heat - 0.75f) / 0.25f; // 0..1
		CurrentSpread = FMath::Min(MaxSpreadDegrees, CurrentSpread + Extra * 0.25f * DeltaSeconds);
	}
}

void AAdventureWeapon::AttachTo(USkeletalMeshComponent* Parent, FName Socket)
{
	if (!Parent) return;

	FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Parent, Rules, Socket);
	SetOwner(Parent->GetOwner());
}

void AAdventureWeapon::SetWantsAutoFire(bool bInAuto)
{
	if (!bAutoFireCapable)
	{
		bWantsAutoFire = false;
		return;
	}
	bWantsAutoFire = bInAuto;

	if (!bWantsAutoFire)
	{
		StopAutoTick();
	}
}

bool AAdventureWeapon::TryFire(const FVector& TraceStart, const FVector& AimPoint, AController* InstigatorController)
{
	OwnerController = InstigatorController ? InstigatorController : OwnerController;

	if (bReloading)
	{
		return false;
	}

	// If empty, attempt reload.
	if (!bInfiniteAmmo && AmmoInMag <= 0)
	{
		StartReload();
		return false;
	}

	// If holding auto-fire, schedule it.
	if (bWantsAutoFire && bAutoFireCapable)
	{
		BeginAutoTick(TraceStart, AimPoint);
		return true;
	}

	// Here we just do a single shot unless BurstCount > 1 and user chooses burst.
	FireOnceInternal(TraceStart, AimPoint);
	return true;
}

void AAdventureWeapon::StartReload()
{
	if (bReloading) return;
	if (bInfiniteAmmo) return;
	if (AmmoInMag >= MagCapacity) return;
	if (SpareAmmo <= 0) return;

	bReloading = true;

	if (ReloadSFX)
	{
		UGameplayStatics::SpawnSoundAttached(ReloadSFX, Mesh);
	}

	GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &AAdventureWeapon::FinishReload, ReloadTime, false);
}

void AAdventureWeapon::FinishReload()
{
	bReloading = false;

	if (bInfiniteAmmo) return;

	const int32 Need = MagCapacity - AmmoInMag;
	const int32 Load = FMath::Min(Need, SpareAmmo);
	AmmoInMag += Load;
	SpareAmmo -= Load;

	BroadcastAmmo();
}

bool AAdventureWeapon::CanFireNow() const
{
	const UWorld* World = GetWorld();
	if (!World) return false;

	const float Now = World->GetTimeSeconds();
	const bool bCooldownReady = (Now - LastFireTime) >= FireRate;

	if (!bCooldownReady) return false;

	if (bReloading) return false;

	// Heat gating: if fully overheated, no firing.
	if (Heat >= HeatMax) return false;

	if (bInfiniteAmmo) return true;
	return AmmoInMag > 0;
}

void AAdventureWeapon::ConsumeAmmo()
{
	if (bInfiniteAmmo) return;
	AmmoInMag = FMath::Max(0, AmmoInMag - 1);
	BroadcastAmmo();
}

void AAdventureWeapon::ApplyRecoilAndSpread()
{
	// Spread bloom
	CurrentSpread = FMath::Min(MaxSpreadDegrees, CurrentSpread + SpreadBloomPerShot);

	// Heat
	Heat = FMath::Min(HeatMax, Heat + HeatPerShot);
}

FVector AAdventureWeapon::ApplySpreadToAim(const FVector& Dir) const
{
	// Cone spread based on degrees.
	const float HalfRad = FMath::DegreesToRadians(CurrentSpread);
	return FMath::VRandCone(Dir, HalfRad);
}

bool AAdventureWeapon::TraceHitscan(const FVector& Start, const FVector& End, FHitResult& OutHit) const
{
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AdventureWeaponTrace), true);
	Params.AddIgnoredActor(this);
	if (AActor* OwnerActor = GetOwner())
	{
		Params.AddIgnoredActor(OwnerActor);
	}

	return GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
}

void AAdventureWeapon::FireOnceInternal(const FVector& TraceStart, const FVector& AimPoint)
{
	if (!CanFireNow()) return;

	LastFireTime = GetWorld()->GetTimeSeconds();

	const FVector Dir = (AimPoint - TraceStart).GetSafeNormal();
	const FVector SpreadDir = ApplySpreadToAim(Dir);
	const FVector TraceEnd = TraceStart + SpreadDir * MaxRange;

	// Effects
	if (MuzzleFX)
	{
		const FVector MuzzleLoc = Mesh->GetSocketLocation(MuzzleSocket);
		const FRotator MuzzleRot = Mesh->GetSocketRotation(MuzzleSocket);
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFX, MuzzleLoc, MuzzleRot);
	}
	if (FireSFX)
	{
		UGameplayStatics::SpawnSoundAttached(FireSFX, Mesh, MuzzleSocket);
	}

	FHitResult Hit;
	const bool bHit = TraceHitscan(TraceStart, TraceEnd, Hit);

	if (bDrawDebug)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, bHit ? FColor::Red : FColor::Green, false, 0.8f, 0, 1.2f);
	}

	if (bHit)
	{
		if (ImpactFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
		}

		if (AActor* HitActor = Hit.GetActor())
		{
			if (OwnerController)
			{
				UGameplayStatics::ApplyPointDamage(
					HitActor,
					Damage,
					SpreadDir,
					Hit,
					OwnerController,
					this,
					DamageType
				);
			}
		}
	}

	ConsumeAmmo();
	ApplyRecoilAndSpread();

	// If we ran out, optionally reload.
	if (!bInfiniteAmmo && AmmoInMag <= 0 && SpareAmmo > 0)
	{
		StartReload();
	}
}

void AAdventureWeapon::BeginAutoTick(const FVector& TraceStart, const FVector& AimPoint)
{
	if (!bAutoFireCapable) return;

	// Fire instantly then schedule.
	FireOnceInternal(TraceStart, AimPoint);

	if (!GetWorld()->GetTimerManager().IsTimerActive(AutoFireTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(
			AutoFireTimer,
			FTimerDelegate::CreateUObject(this, &AAdventureWeapon::AutoTickFire, TraceStart, AimPoint),
			FireRate,
			true
		);
	}
}

void AAdventureWeapon::StopAutoTick()
{
	GetWorld()->GetTimerManager().ClearTimer(AutoFireTimer);
}

void AAdventureWeapon::AutoTickFire(const FVector TraceStart, const FVector AimPoint)
{
	if (!bWantsAutoFire)
	{
		StopAutoTick();
		return;
	}
	FireOnceInternal(TraceStart, AimPoint);
}

void AAdventureWeapon::BeginBurst(const FVector& TraceStart, const FVector& AimPoint)
{
	BurstShotsRemaining = FMath::Max(BurstCount, 1);
	FireOnceInternal(TraceStart, AimPoint);
	BurstShotsRemaining--;

	if (BurstShotsRemaining > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(
			BurstTimer,
			FTimerDelegate::CreateUObject(this, &AAdventureWeapon::BurstTickFire, TraceStart, AimPoint),
			BurstInterval,
			true
		);
	}
}

void AAdventureWeapon::BurstTickFire(const FVector TraceStart, const FVector AimPoint)
{
	if (BurstShotsRemaining <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(BurstTimer);
		return;
	}

	FireOnceInternal(TraceStart, AimPoint);
	BurstShotsRemaining--;
}

void AAdventureWeapon::BroadcastAmmo()
{
	OnAmmoChanged.Broadcast(AmmoInMag);
}
