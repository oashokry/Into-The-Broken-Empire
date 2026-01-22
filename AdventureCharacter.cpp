#include "AdventureCharacter.h"

#include "AdventureWeapon.h"
#include "AdventureInteractable.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AAdventureCharacter::AAdventureCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 320.f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	Health = MaxHealth;
	Stamina = MaxStamina;
}

void AAdventureCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateMovementSpeed();
	BroadcastVitals();

	// Spawn starter weapon if configured.
	if (StarterWeaponClass)
	{
		AAdventureWeapon* W = GetWorld()->SpawnActor<AAdventureWeapon>(StarterWeaponClass);
		if (W)
		{
			W->AttachTo(GetMesh(), WeaponSocket);
			W->SetOwnerController(GetController());
			EquippedWeapon = W;
		}
	}
}

void AAdventureCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TickVitals(DeltaSeconds);
}

void AAdventureCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AAdventureCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AAdventureCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AAdventureCharacter::LookUp);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AAdventureCharacter::Turn);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &AAdventureCharacter::StartSprint);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &AAdventureCharacter::StopSprint);

	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &AAdventureCharacter::PullTrigger);
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Released, this, &AAdventureCharacter::ReleaseTrigger);
	PlayerInputComponent->BindAction(TEXT("Reload"), IE_Pressed, this, &AAdventureCharacter::Reload);

	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AAdventureCharacter::Interact);

	PlayerInputComponent->BindAction(TEXT("Vault"), IE_Pressed, this, &AAdventureCharacter::TryVault);
	PlayerInputComponent->BindAction(TEXT("Climb"), IE_Pressed, this, &AAdventureCharacter::TryClimb);
}

void AAdventureCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector(), Value);
}

void AAdventureCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector(), Value);
}

void AAdventureCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AAdventureCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AAdventureCharacter::StartSprint()
{
	bSprinting = true;
	UpdateMovementSpeed();
}

void AAdventureCharacter::StopSprint()
{
	bSprinting = false;
	UpdateMovementSpeed();
}

void AAdventureCharacter::UpdateMovementSpeed()
{
	const float Desired = bSprinting ? SprintSpeed : WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed = Desired;
}

void AAdventureCharacter::TickVitals(float DeltaSeconds)
{
	// Sprint drains stamina.
	if (bSprinting && GetVelocity().SizeSquared() > 10.f)
	{
		Stamina = FMath::Max(0.f, Stamina - SprintStaminaCostPerSecond * DeltaSeconds);

		// If stamina hits 0, stop sprinting. Hero fantasy ends here.
		if (Stamina <= 0.f)
		{
			bSprinting = false;
			UpdateMovementSpeed();
		}
	}
	else
	{
		// Regen stamina when not sprinting.
		Stamina = FMath::Min(MaxStamina, Stamina + StaminaRegenPerSecond * DeltaSeconds);
	}

	BroadcastVitals();
}

void AAdventureCharacter::BroadcastVitals()
{
	const float HPct = (MaxHealth <= 0.f) ? 0.f : (Health / MaxHealth);
	const float SPct = (MaxStamina <= 0.f) ? 0.f : (Stamina / MaxStamina);
	OnVitalsChanged.Broadcast(HPct, SPct);
}

FVector AAdventureCharacter::GetTraceStart() const
{
	// Use camera viewpoint for aiming; makes hitscan match crosshair.
	FVector ViewLoc;
	FRotator ViewRot;
	GetController()->GetPlayerViewPoint(ViewLoc, ViewRot);
	return ViewLoc;
}

FVector AAdventureCharacter::GetAimPoint() const
{
	if (AimActorOverride)
	{
		return AimActorOverride->GetActorLocation();
	}
	if (!AimPointOverride.IsNearlyZero())
	{
		return AimPointOverride;
	}

	// Basic aim: trace forward from camera.
	FVector ViewLoc;
	FRotator ViewRot;
	GetController()->GetPlayerViewPoint(ViewLoc, ViewRot);

	const FVector End = ViewLoc + ViewRot.Vector() * 9000.f;
	return End;
}

void AAdventureCharacter::SetAimOverride(AActor* InActor, const FVector& InPoint)
{
	AimActorOverride = InActor;
	AimPointOverride = InPoint;
}

void AAdventureCharacter::PullTrigger()
{
	if (!EquippedWeapon) return;

	const FVector Start = GetTraceStart();
	const FVector Aim = GetAimPoint();

	EquippedWeapon->SetOwnerController(GetController());
	EquippedWeapon->TryFire(Start, Aim, GetController());

	// If you want full-auto: set wants auto and keep calling TryFire from weapon timers.
	EquippedWeapon->SetWantsAutoFire(true);
}

void AAdventureCharacter::ReleaseTrigger()
{
	if (!EquippedWeapon) return;
	EquippedWeapon->SetWantsAutoFire(false);
}

void AAdventureCharacter::Reload()
{
	if (!EquippedWeapon) return;

	// Here we just let weapon reload from its own SpareAmmo
	EquippedWeapon->StartReload();
}

void AAdventureCharacter::AddSpareAmmo(int32 Amount)
{
	SpareAmmoPool = FMath::Max(0, SpareAmmoPool + Amount);

	// If weapon exists, top up weapon's spare ammo from pool in a “smart” way:
	if (EquippedWeapon && SpareAmmoPool > 0)
	{
		// Move ammo into weapon's spare by simply adding.
	}
}

void AAdventureCharacter::RestoreStamina(float Amount)
{
	Stamina = FMath::Min(MaxStamina, Stamina + Amount);
	BroadcastVitals();
}

void AAdventureCharacter::AddRelic(int32 Amount)
{
	Relics = FMath::Max(0, Relics + Amount);
	OnRelicsChanged.Broadcast(Relics);
}

float AAdventureCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                     AController* EventInstigator, AActor* DamageCauser)
{
	const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (Applied <= 0.f) return 0.f;

	Health = FMath::Max(0.f, Health - Applied);
	BroadcastVitals();

	if (Health <= 0.f)
	{
		Die(EventInstigator);
	}

	return Applied;
}

void AAdventureCharacter::Die(AController* Killer)
{
	// Disable movement, detach weapon, etc.
	GetCharacterMovement()->DisableMovement();
	DisableInput(Cast<APlayerController>(GetController()));

	// Notify game mode if present.
	if (AGameModeBase* GM = UGameplayStatics::GetGameMode(this))
	{
		// You can cast to your game mode and call PawnKilled.
	}

	// Ragdoll optional:
	GetMesh()->SetSimulatePhysics(true);
}

void AAdventureCharacter::Interact()
{
	FHitResult Hit;
	AActor* Candidate = FindInteractable(Hit);
	if (!Candidate) return;

	if (Candidate->GetClass()->ImplementsInterface(UAdventureInteractable::StaticClass()))
	{
		IAdventureInteractable::Execute_Interact(Candidate, this);
	}
}

AActor* AAdventureCharacter::FindInteractable(FHitResult& OutHit) const
{
	FVector ViewLoc;
	FRotator ViewRot;
	GetController()->GetPlayerViewPoint(ViewLoc, ViewRot);

	const FVector Start = ViewLoc;
	const FVector End = Start + ViewRot.Vector() * InteractDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractTrace), true);
	Params.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, InteractTraceChannel, Params);

#if !(UE_BUILD_SHIPPING)
	DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Cyan : FColor::Blue, false, 0.25f, 0, 1.0f);
#endif

	return bHit ? OutHit.GetActor() : nullptr;
}

void AAdventureCharacter::TryVault()
{
	

	const FVector Start = GetActorLocation() + FVector(0.f, 0.f, 70.f);
	const FVector End = Start + GetActorForwardVector() * VaultCheckDistance;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(VaultTrace), true);
	Params.AddIgnoredActor(this);

	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return;
	}

	// Determine obstacle height by tracing downward from above the hit point.
	const FVector Above = Hit.ImpactPoint + FVector(0.f, 0.f, VaultHeightMax + 80.f);
	const FVector Down = Hit.ImpactPoint - FVector(0.f, 0.f, 200.f);

	FHitResult FloorHit;
	if (!GetWorld()->LineTraceSingleByChannel(FloorHit, Above, Down, ECC_Visibility, Params))
	{
		return;
	}

	const float ObstacleHeight = FloorHit.ImpactPoint.Z - GetActorLocation().Z;
	if (ObstacleHeight < VaultHeightMin || ObstacleHeight > VaultHeightMax)
	{
		return;
	}

	// Launch over it.
	const FVector LaunchVel = GetActorForwardVector() * 420.f + FVector(0.f, 0.f, 320.f);
	LaunchCharacter(LaunchVel, true, true);
}

void AAdventureCharacter::TryClimb()
{

	const FVector Start = GetActorLocation() + FVector(0.f, 0.f, 140.f);
	const FVector End = Start + GetActorForwardVector() * ClimbCheckDistance;

	FHitResult WallHit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ClimbTrace), true);
	Params.AddIgnoredActor(this);

	if (!GetWorld()->LineTraceSingleByChannel(WallHit, Start, End, ECC_Visibility, Params))
	{
		return;
	}

	// Look for ledge: trace downward from above wall hit.
	const FVector LedgeProbeTop = WallHit.ImpactPoint + FVector(0.f, 0.f, ClimbHeightMax);
	const FVector LedgeProbeDown = WallHit.ImpactPoint + FVector(0.f, 0.f, ClimbHeightMin) - FVector(0.f,0.f,260.f);

	FHitResult LedgeHit;
	if (!GetWorld()->LineTraceSingleByChannel(LedgeHit, LedgeProbeTop, LedgeProbeDown, ECC_Visibility, Params))
	{
		return;
	}

	const float LedgeHeight = LedgeHit.ImpactPoint.Z - GetActorLocation().Z;
	if (LedgeHeight < ClimbHeightMin || LedgeHeight > ClimbHeightMax)
	{
		return;
	}

	// Launch upward + forward. Real climbing uses animation + hand IK.
	const FVector LaunchVel = GetActorForwardVector() * 200.f + FVector(0.f, 0.f, 520.f);
	LaunchCharacter(LaunchVel, true, true);
}
