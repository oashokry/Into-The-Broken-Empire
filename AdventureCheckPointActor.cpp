#include "AdventureCheckpointActor.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AAdventureCheckpointActor::AAdventureCheckpointActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	SetRootComponent(Trigger);
	Trigger->SetBoxExtent(FVector(120.f, 120.f, 120.f));
	Trigger->SetCollisionProfileName(TEXT("Trigger"));
}

void AAdventureCheckpointActor::BeginPlay()
{
	Super::BeginPlay();
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &AAdventureCheckpointActor::OnOverlap);
}

FTransform AAdventureCheckpointActor::GetCheckpointTransform() const
{
	return GetActorTransform();
}

void AAdventureCheckpointActor::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (bOneShot && bUsed) return;

	ACharacter* Char = Cast<ACharacter>(OtherActor);
	if (!Char) return;

	APlayerController* PC = Cast<APlayerController>(Char->GetController());
	if (!PC) return;

	bUsed = true;

	// storing this on the player state would be nicer, but sure, let’s do it the quick way
	PC->Tags.AddUnique(FName(*FString::Printf(TEXT("LastCheckpoint=%s"), *CheckpointId.ToString())));
	PC->Tags.AddUnique(FName(*FString::Printf(TEXT("LastCheckpointTransform=%s"), *GetActorTransform().ToString())));
}
