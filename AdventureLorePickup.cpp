#include "AdventureLorePickup.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "AdventureInventoryComponent.h"
#include "AdventureObjectiveSubsystem.h"

AAdventureLorePickup::AAdventureLorePickup()
{
	PrimaryActorTick.bCanEverTick = false;

	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("LoreTrigger"));
	SetRootComponent(Trigger);
	Trigger->SetSphereRadius(95.f);
	Trigger->SetCollisionProfileName(TEXT("Trigger"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LoreMesh"));
	Mesh->SetupAttachment(Trigger);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAdventureLorePickup::BeginPlay()
{
	Super::BeginPlay();
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &AAdventureLorePickup::OnOverlap);
}

void AAdventureLorePickup::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	ACharacter* C = Cast<ACharacter>(OtherActor);
	if (!C) return;

	Collect(C);
}

void AAdventureLorePickup::Collect(AActor* Collector)
{
	if (bOneShot && bUsed) return;
	if (!Collector) return;

	bUsed = true;

	if (bAddInventoryItem)
	{
		if (UAdventureInventoryComponent* Inv = Collector->FindComponentByClass<UAdventureInventoryComponent>())
		{
			FAdventureInventoryItem Item;
			Item.ItemId = InventoryItemId;
			Item.Count = 1;
			Item.DisplayName = LoreTitle.IsEmpty() ? FText::FromString(LoreId.ToString()) : LoreTitle;
			Item.bIsKeyItem = true;
			Inv->AddItem(Item);
		}
	}

	if (bAdvanceObjective && GetGameInstance())
	{
		if (UAdventureObjectiveSubsystem* Obj = GetGameInstance()->GetSubsystem<UAdventureObjectiveSubsystem>())
		{
			Obj->AddProgress(ObjectiveId, ObjectiveDelta);
		}
	}

	OnCollected.Broadcast(LoreId);

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}
