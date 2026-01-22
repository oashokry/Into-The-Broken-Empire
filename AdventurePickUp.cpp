#include "AdventurePickup.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"

#include "AdventureCharacter.h"

AAdventurePickup::AAdventurePickup()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(Mesh);
	Sphere->SetSphereRadius(120.f);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AAdventurePickup::BeginPlay()
{
	Super::BeginPlay();

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAdventurePickup::OnSphereOverlap);
}

void AAdventurePickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                      const FHitResult& SweepResult)
{
	if (!bAutoPickupOnOverlap) return;
	if (!OtherActor || OtherActor == this) return;

	GrantTo(OtherActor);
}

bool AAdventurePickup::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor) return false;
	GrantTo(Interactor);
	return true;
}

FText AAdventurePickup::GetInteractPrompt_Implementation() const
{
	switch (Type)
	{
	case EPickupType::Ammo:    return FText::FromString(TEXT("Pick up Ammo"));
	case EPickupType::Stamina: return FText::FromString(TEXT("Drink Stamina Tonic"));
	case EPickupType::Relic:   return FText::FromString(TEXT("Collect Relic"));
	default:                  return FText::FromString(TEXT("Interact"));
	}
}

void AAdventurePickup::GrantTo(AActor* Interactor)
{
	AAdventureCharacter* Char = Cast<AAdventureCharacter>(Interactor);
	if (!Char)
	{
		// Not our character. Fine. Walk away from the treasure, I guess.
		return;
	}

	switch (Type)
	{
	case EPickupType::Ammo:
		Char->AddSpareAmmo(Amount);
		break;
	case EPickupType::Stamina:
		Char->RestoreStamina((float)Amount);
		break;
	case EPickupType::Relic:
		Char->AddRelic(Amount);
		break;
	default:
		break;
	}

	// Destroy shortly after, to allow any cosmetic BP effects to play
	FTimerHandle T;
	GetWorld()->GetTimerManager().SetTimer(T, [this]()
	{
		Destroy();
	}, DestroyDelay, false);
}
