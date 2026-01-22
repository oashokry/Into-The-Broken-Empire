#include "AdventureBossDoor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"

#include "AdventureInventoryComponent.h"
#include "AdventureObjectiveSubsystem.h"
#include "AdventureWaveDirector.h"

AAdventureBossDoor::AAdventureBossDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BossDoorMesh"));
	SetRootComponent(DoorMesh);
	DoorMesh->SetCollisionProfileName(TEXT("BlockAll"));

	UseZone = CreateDefaultSubobject<UBoxComponent>(TEXT("UseZone"));
	UseZone->SetupAttachment(DoorMesh);
	UseZone->SetBoxExtent(FVector(180, 180, 180));
	UseZone->SetCollisionProfileName(TEXT("Trigger"));
}

void AAdventureBossDoor::BeginPlay()
{
	Super::BeginPlay();

	UseZone->OnComponentBeginOverlap.AddDynamic(this, &AAdventureBossDoor::OnOverlap);

	GetWorld()->GetTimerManager().SetTimer(DoorTickTimer, [this]()
	{
		TickDoor(0.016f);
	}, 0.016f, true);
}

void AAdventureBossDoor::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (!OtherActor || OtherActor == this) return;
	TryOpen(OtherActor);
}

void AAdventureBossDoor::ForceUnlock()
{
	bUnlocked = true;
}

bool AAdventureBossDoor::HasAllKeys(AActor* User) const
{
	if (!bRequireKeyItems) return true;
	if (!User) return false;

	const UAdventureInventoryComponent* Inv = User->FindComponentByClass<UAdventureInventoryComponent>();
	if (!Inv) return false;

	for (const FName& Key : RequiredKeys)
	{
		if (!Inv->HasItem(Key, 1)) return false;
	}
	return true;
}

bool AAdventureBossDoor::HasAllObjectives() const
{
	if (!bRequireObjectives) return true;
	if (!GetGameInstance()) return false;

	UAdventureObjectiveSubsystem* Obj = GetGameInstance()->GetSubsystem<UAdventureObjectiveSubsystem>();
	if (!Obj) return false;

	for (const FName& Id : RequiredObjectives)
	{
		if (!Obj->IsCompleted(Id)) return false;
	}
	return true;
}

bool AAdventureBossDoor::MeetsRequirements(AActor* User) const
{
	if (!HasAllKeys(User)) return false;
	if (!HasAllObjectives()) return false;

	if (bRequireWavesCleared)
	{
		if (!WaveDirector) return false;
		if (!WaveDirector->AreAllWavesCleared()) return false;
	}

	return true;
}

void AAdventureBossDoor::TryOpen(AActor* User)
{
	if (bOpen) return;

	if (!bUnlocked)
	{
		if (!MeetsRequirements(User))
		{
			return; // door says “no” and honestly it’s the healthiest boundary in this game
		}
		bUnlocked = true;
	}

	GetWorld()->GetTimerManager().ClearTimer(OpenDelayTimer);
	GetWorld()->GetTimerManager().SetTimer(OpenDelayTimer, [this]()
	{
		bOpen = true;
		TargetYaw = OpenYaw;
	}, CinematicDelay, false);
}

void AAdventureBossDoor::TickDoor(float DeltaSeconds)
{
	const float Old = CurrentYaw;
	CurrentYaw = FMath::FInterpTo(CurrentYaw, TargetYaw, DeltaSeconds, OpenSpeed);

	if (!FMath::IsNearlyEqual(Old, CurrentYaw, 0.01f))
	{
		FRotator R = DoorMesh->GetRelativeRotation();
		R.Yaw = CurrentYaw;
		DoorMesh->SetRelativeRotation(R);
	}
}
