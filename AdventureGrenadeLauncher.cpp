#include "AdventureGrenadeLauncherWeapon.h"

#include "AdventureGrenade.h"
#include "Engine/World.h"

AAdventureGrenadeLauncherWeapon::AAdventureGrenadeLauncherWeapon()
{
	// yes, it’s still a weapon, and yes, it “fires” grenades, because vocabulary is flexible
}

bool AAdventureGrenadeLauncherWeapon::FireAdvanced(AController* InstigatorController, const FVector& TraceStart, const FVector& AimTarget, AActor*& OutHitActor)
{
	OutHitActor = nullptr;

	if (!GrenadeClass) return false;

	// reuse ammo/cooldown logic from base class like a responsible adult
	if (!ConsumeAmmoAndCooldown())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World) return false;

	const FVector Start = TraceStart + (GetActorForwardVector() * SpawnForwardOffset);
	FTransform T;
	T.SetLocation(Start);
	T.SetRotation(GetActorRotation().Quaternion());

	AAdventureGrenade* G = World->SpawnActorDeferred<AAdventureGrenade>(GrenadeClass, T, GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!G) return false;

	G->FinishSpawning(T);

	FVector Dir = (AimTarget - Start).GetSafeNormal();
	Dir.Z += ArcBoostZ;
	Dir = Dir.GetSafeNormal();

	G->SetThrowVelocity(Dir * LaunchSpeed);
	G->Arm(InstigatorController, FuseOverride);

	return true;
}
