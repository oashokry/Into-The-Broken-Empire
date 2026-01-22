#include "AdventureLockableDoor.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "AdventureInventoryComponent.h"

AAdventureLockableDoor::AAdventureLockableDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	SetRootComponent(DoorMesh);

	UseTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("UseTrigger"));
	UseTrigger->SetupAttachment(DoorMesh);
	UseTrigger->SetBoxExtent(FVector(120.f, 120.f, 140.f));
	UseTrigger->SetCollisionProfileName(TEXT("Trigger"));
}

void AAdventureLockableDoor::BeginPlay()
{
	Super::BeginPlay();

	UseTrigger->OnComponentBeginOverlap.AddDynamic(this, &AAdventureLockableDoor::OnOverlap);

	GetWorld()->GetTimerManager().SetTimer(DoorTickTimer, [this]()
	{
		TickDoor(0.016f);
	}, 0.016f, true);
}

void AAdventureLockableDoor::Unlock()
{
	bLocked = false;
}

void AAdventureLockableDoor::Lock()
{
	bLocked = true;
}

void AAdventureLockableDoor::Open()
{
	if (bLocked) return;

	bOpen = true;
	TargetYaw = OpenYaw;

	if (bAutoClose)
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoCloseTimer);
		GetWorld()->GetTimerManager().SetTimer(AutoCloseTimer, [this]()
		{
			Close();
		}, AutoCloseDelay, false);
	}
}

void AAdventureLockableDoor::Close()
{
	bOpen = false;
	TargetYaw = 0.f;
}

void AAdventureLockableDoor::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	ACharacter* Char = Cast<ACharacter>(OtherActor);
	if (!Char) return;

	if (bLocked)
	{
		if (!TryConsumeKey(Char))
		{
			return; // door says no, like every locked door ever invented
		}
		Unlock();
	}

	if (!bOpen) Open();
}

bool AAdventureLockableDoor::TryConsumeKey(AActor* ActorTryingToUse)
{
	if (RequiredKeyItem.IsNone()) return true;

	UAdventureInventoryComponent* Inv = ActorTryingToUse->FindComponentByClass<UAdventureInventoryComponent>();
	if (!Inv) return false;

	if (!Inv->HasItem(RequiredKeyItem, 1))
	{
		return false;
	}

	Inv->RemoveItem(RequiredKeyItem, 1);
	return true;
}

void AAdventureLockableDoor::TickDoor(float DeltaSeconds)
{
	const float OldYaw = CurrentYaw;
	CurrentYaw = FMath::FInterpTo(CurrentYaw, TargetYaw, DeltaSeconds, OpenSpeed);

	if (!FMath::IsNearlyEqual(OldYaw, CurrentYaw, 0.01f))
	{
		FRotator R = DoorMesh->GetRelativeRotation();
		R.Yaw = CurrentYaw;
		DoorMesh->SetRelativeRotation(R);
	}
}
