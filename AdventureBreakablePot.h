#include "AdventureDartTrap.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

AAdventureDartTrap::AAdventureDartTrap()
{
	PrimaryActorTick.bCanEverTick = false;

	Base = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
	SetRootComponent(Base);
	Base->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(Base);
	Trigger->SetBoxExtent(FVector(220,220,120));
	Trigger->SetCollisionProfileName(TEXT("Trigger"));
}

void AAdventureDartTrap::BeginPlay()
{
	Super::BeginPlay();
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &AAdventureDartTrap::OnOverlap);
}

void AAdventureDartTrap::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (!OtherActor || OtherActor == this) return;
	Fire();
}

void AAdventureDartTrap::Fire()
{
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastFireTime < Cooldown) return;

	LastFireTime = Now;

	BurstRemaining = (int32)BurstCount;
	GetWorldTimerManager().ClearTimer(BurstTimer);

	FireOne();
	BurstRemaining--;

	if (BurstRemaining > 0)
	{
		GetWorldTimerManager().SetTimer(BurstTimer, [this]()
		{
			FireOne();
			BurstRemaining--;
			if (BurstRemaining <= 0)
			{
				GetWorldTimerManager().ClearTimer(BurstTimer);
			}
		}, BurstInterval, true);
	}
}

void AAdventureDartTrap::FireOne()
{
	const FVector Start = Base->GetSocketLocation(MuzzleSocket);
	const FVector End = Start + Base->GetForwardVector() * Range;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AdvDartTrap), false);
	Params.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	if (bDebug)
	{
		DrawDebugLine(GetWorld(), Start, bHit ? Hit.ImpactPoint : End, FColor::Purple, false, 1.0f, 0, 1.2f);
	}

	if (bHit && Hit.GetActor())
	{
		UGameplayStatics::ApplyPointDamage(Hit.GetActor(), Damage, Base->GetForwardVector(), Hit, nullptr, this, DamageType);
	}
}
