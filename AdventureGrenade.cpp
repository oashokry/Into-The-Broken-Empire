#include "AdventureGrenade.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AAdventureGrenade::AAdventureGrenade()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->SetSphereRadius(12.f);
	Collision->SetSimulatePhysics(true);
	Collision->SetCollisionProfileName(TEXT("PhysicsActor"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Projectile = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile"));
	Projectile->UpdatedComponent = Collision;
	Projectile->bRotationFollowsVelocity = true;
	Projectile->bShouldBounce = true;
	Projectile->Bounciness = 0.35f;
	Projectile->Friction = 0.25f;
	Projectile->InitialSpeed = 1600.f;
	Projectile->MaxSpeed = 2800.f;
	Projectile->ProjectileGravityScale = 1.0f;
}

void AAdventureGrenade::BeginPlay()
{
	Super::BeginPlay();
	Collision->OnComponentHit.AddDynamic(this, &AAdventureGrenade::OnHit);
}

void AAdventureGrenade::Arm(AController* InstigatorController, float OverrideFuse)
{
	if (bExploded) return;
	if (bArmed) return;

	bArmed = true;
	OwningController = InstigatorController;

	const float UseFuse = (OverrideFuse > 0.f) ? OverrideFuse : FuseSeconds;

	GetWorldTimerManager().SetTimer(FuseTimer, [this]()
	{
		Explode();
	}, UseFuse, false);
}

void AAdventureGrenade::SetThrowVelocity(const FVector& Velocity)
{
	if (Projectile)
	{
		Projectile->Velocity = Velocity;
	}
	else
	{
		// sure, we’ll just yeet it with physics and hope for the best
		Collision->AddImpulse(Velocity * Collision->GetMass());
	}
}

void AAdventureGrenade::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// yes, grenades bounce, because realism is important until it annoys players
	if (bDrawDebug)
	{
		DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 10.f, FColor::Yellow, false, 1.0f);
	}
}

void AAdventureGrenade::Explode()
{
	if (bExploded) return;
	bExploded = true;

	if (ExplosionFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, GetActorTransform());
	}

	if (ExplosionSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSFX, GetActorLocation());
	}

	DoRadialDamage();
	DoImpulse();
	DoShrapnel();

	Destroy();
}

void AAdventureGrenade::DoRadialDamage()
{
	UGameplayStatics::ApplyRadialDamageWithFalloff(
		GetWorld(),
		Damage,
		5.f,
		GetActorLocation(),
		InnerRadius,
		OuterRadius,
		1.f,
		DamageType,
		TArray<AActor*>(),
		this,
		OwningController,
		ECC_Visibility
	);

	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), InnerRadius, 18, FColor::Red, false, 1.0f);
		DrawDebugSphere(GetWorld(), GetActorLocation(), OuterRadius, 18, FColor::Orange, false, 1.0f);
	}
}

void AAdventureGrenade::DoImpulse()
{
	TArray<AActor*> Ignored;
	Ignored.Add(this);

	TArray<FHitResult> Hits;
	FCollisionShape Shape = FCollisionShape::MakeSphere(OuterRadius);

	GetWorld()->SweepMultiByChannel(Hits, GetActorLocation(), GetActorLocation(), FQuat::Identity, ECC_PhysicsBody, Shape);

	for (const FHitResult& H : Hits)
	{
		UPrimitiveComponent* Prim = H.GetComponent();
		if (!Prim) continue;
		if (!Prim->IsSimulatingPhysics()) continue;

		const FVector Dir = (Prim->GetComponentLocation() - GetActorLocation()).GetSafeNormal();
		const float Dist = FVector::Dist(Prim->GetComponentLocation(), GetActorLocation());
		const float Alpha = 1.f - FMath::Clamp(Dist / OuterRadius, 0.f, 1.f);

		Prim->AddImpulse(Dir * ImpulseStrength * Alpha, NAME_None, true);
	}
}

FVector AAdventureGrenade::RandomUnitVectorFast(int32 Seed) const
{
	FRandomStream Rng(Seed);
	const float Z = Rng.FRandRange(-1.f, 1.f);
	const float A = Rng.FRandRange(0.f, 6.283185307f);
	const float R = FMath::Sqrt(FMath::Max(0.f, 1.f - Z * Z));
	return FVector(R * FMath::Cos(A), R * FMath::Sin(A), Z);
}

void AAdventureGrenade::DoShrapnel()
{
	const FVector Start = GetActorLocation();

	for (int32 i = 0; i < ShrapnelTraces; ++i)
	{
		const FVector Dir = RandomUnitVectorFast(i * 1337 + GetUniqueID());
		const FVector End = Start + Dir * ShrapnelRange;

		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(AdvGrenadeShrapnel), false);
		Params.AddIgnoredActor(this);

		const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
		if (bDrawDebug)
		{
			DrawDebugLine(GetWorld(), Start, bHit ? Hit.ImpactPoint : End, FColor::Silver, false, 1.0f, 0, 0.8f);
		}

		if (bHit && Hit.GetActor())
		{
			UGameplayStatics::ApplyPointDamage(
				Hit.GetActor(),
				ShrapnelDamage,
				Dir,
				Hit,
				OwningController,
				this,
				DamageType
			);
		}
	}
}
