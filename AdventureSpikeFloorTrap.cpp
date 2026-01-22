#include "AdventureSpikeFloorTrap.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AAdventureSpikeFloorTrap::AAdventureSpikeFloorTrap()
{
	PrimaryActorTick.bCanEverTick = false;

	DamageVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageVolume"));
	SetRootComponent(DamageVolume);
	DamageVolume->SetBoxExtent(FVector(120,120,50));
	DamageVolume->SetCollisionProfileName(TEXT("Trigger"));

	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(DamageVolume);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAdventureSpikeFloorTrap::BeginPlay()
{
	Super::BeginPlay();

	DamageVolume->OnComponentBeginOverlap.AddDynamic(this, &AAdventureSpikeFloorTrap::OnBeginOverlap);
	DamageVolume->OnComponentEndOverlap.AddDynamic(this, &AAdventureSpikeFloorTrap::OnEndOverlap);

	bActive = bStartActive;
	bWarming = false;

	StartCycle();

	GetWorldTimerManager().SetTimer(DamageTimer, [this]()
	{
		DoDamageTick();
	}, TickInterval, true);
}

void AAdventureSpikeFloorTrap::StartCycle()
{
	GetWorldTimerManager().ClearTimer(CycleTimer);

	GetWorldTimerManager().SetTimer(CycleTimer, [this]()
	{
		if (bActive)
		{
			SetTrapState(false, false);
			StartCycle();
		}
		else
		{
			SetTrapState(false, true);
			GetWorldTimerManager().SetTimer(CycleTimer, [this]()
			{
				SetTrapState(true, false);
				StartCycle();
			}, WarmupSeconds, false);
		}
	}, bActive ? ActiveSeconds : InactiveSeconds, false);
}

void AAdventureSpikeFloorTrap::SetTrapState(bool bNewActive, bool bNewWarming)
{
	bActive = bNewActive;
	bWarming = bNewWarming;

	// sure, swap material or raise spikes here, but that would require joy
}

void AAdventureSpikeFloorTrap::DoDamageTick()
{
	if (!bActive) return;

	for (auto It = Overlapping.CreateIterator(); It; ++It)
	{
		AActor* A = It->Get();
		if (!A)
		{
			It.RemoveCurrent();
			continue;
		}

		UGameplayStatics::ApplyDamage(A, DamagePerTick, nullptr, this, DamageType);
	}
}

void AAdventureSpikeFloorTrap::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (!OtherActor || OtherActor == this) return;
	Overlapping.Add(OtherActor);
}

void AAdventureSpikeFloorTrap::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;
	Overlapping.Remove(OtherActor);
}
