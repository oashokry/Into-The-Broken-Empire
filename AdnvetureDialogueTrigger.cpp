#include "AdventureDialogueTrigger.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "AdventureInventoryComponent.h"
#include "AdventureObjectiveSubsystem.h"
#include "Kismet/GameplayStatics.h"

AAdventureDialogueTrigger::AAdventureDialogueTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("DialogueBox"));
	SetRootComponent(Box);
	Box->SetBoxExtent(FVector(160,160,120));
	Box->SetCollisionProfileName(TEXT("Trigger"));
}

void AAdventureDialogueTrigger::BeginPlay()
{
	Super::BeginPlay();
	Box->OnComponentBeginOverlap.AddDynamic(this, &AAdventureDialogueTrigger::OnOverlap);
}

void AAdventureDialogueTrigger::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	ACharacter* C = Cast<ACharacter>(OtherActor);
	if (!C) return;

	Trigger(C);
}

bool AAdventureDialogueTrigger::PassesRequirements(AActor* InstigatorActor) const
{
	if (!bRequireKeyItem) return true;
	if (!InstigatorActor) return false;

	const UAdventureInventoryComponent* Inv = InstigatorActor->FindComponentByClass<UAdventureInventoryComponent>();
	if (!Inv) return false;

	return Inv->HasItem(RequiredItemId, 1);
}

void AAdventureDialogueTrigger::Trigger(AActor* InstigatorActor)
{
	if (bOneShot && bUsed) return;
	if (!PassesRequirements(InstigatorActor)) return;

	bUsed = true;

	OnDialogueTriggered.Broadcast(DialogueId);

	if (bAutoCompleteObjective && GetGameInstance())
	{
		if (UAdventureObjectiveSubsystem* Obj = GetGameInstance()->GetSubsystem<UAdventureObjectiveSubsystem>())
		{
			Obj->CompleteObjective(ObjectiveToComplete);
		}
	}
}
