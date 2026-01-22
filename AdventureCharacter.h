#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AdventureCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class AAdventureWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVitalsChanged, float, HealthPct, float, StaminaPct);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRelicsChanged, int32, NewRelics);


UCLASS()
class SIMPLESHOOTER_API AAdventureCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAdventureCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Combat
	void PullTrigger();
	void ReleaseTrigger();
	void Reload();

	bool HasWeaponEquipped() const { return EquippedWeapon != nullptr; }
	void SetAimOverride(AActor* InActor, const FVector& InPoint);

	// Pickups
	void AddSpareAmmo(int32 Amount);
	void RestoreStamina(float Amount);
	void AddRelic(int32 Amount);

	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnVitalsChanged OnVitalsChanged;

	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnRelicsChanged OnRelicsChanged;

	// Damage/death
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	                         class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	// Components
	UPROPERTY(VisibleAnywhere, Category="Components")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category="Components")
	UCameraComponent* Camera;

	// Weapon
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	TSubclassOf<AAdventureWeapon> StarterWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	FName WeaponSocket = TEXT("WeaponSocket");

	UPROPERTY()
	AAdventureWeapon* EquippedWeapon = nullptr;

	// Vitals
	UPROPERTY(EditDefaultsOnly, Category="Vitals")
	float MaxHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, Category="Vitals")
	float MaxStamina = 100.f;

	UPROPERTY(EditDefaultsOnly, Category="Vitals")
	float StaminaRegenPerSecond = 18.f;

	UPROPERTY(EditDefaultsOnly, Category="Vitals")
	float SprintStaminaCostPerSecond = 22.f;

	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float WalkSpeed = 450.f;

	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float SprintSpeed = 720.f;

	float Health = 100.f;
	float Stamina = 100.f;
	bool bSprinting = false;

	// Adventure bits
	UPROPERTY(EditDefaultsOnly, Category="Interaction")
	float InteractDistance = 260.f;

	UPROPERTY(EditDefaultsOnly, Category="Interaction")
	TEnumAsByte<ECollisionChannel> InteractTraceChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly, Category="Traversal")
	float VaultCheckDistance = 140.f;

	UPROPERTY(EditDefaultsOnly, Category="Traversal")
	float VaultHeightMin = 50.f;

	UPROPERTY(EditDefaultsOnly, Category="Traversal")
	float VaultHeightMax = 120.f;

	UPROPERTY(EditDefaultsOnly, Category="Traversal")
	float ClimbCheckDistance = 120.f;

	UPROPERTY(EditDefaultsOnly, Category="Traversal")
	float ClimbHeightMin = 140.f;

	UPROPERTY(EditDefaultsOnly, Category="Traversal")
	float ClimbHeightMax = 240.f;

	// “Inventory”
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	int32 SpareAmmoPool = 60;

	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	int32 Relics = 0;

	// Aim override (AI uses this too)
	UPROPERTY()
	AActor* AimActorOverride = nullptr;

	FVector AimPointOverride = FVector::ZeroVector;

	// Input handlers
	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void Turn(float Value);

	void StartSprint();
	void StopSprint();

	void Interact();
	AActor* FindInteractable(FHitResult& OutHit) const;

	void TryVault();
	void TryClimb();

	// Helpers
	FVector GetTraceStart() const;
	FVector GetAimPoint() const;

	void UpdateMovementSpeed();
	void TickVitals(float DeltaSeconds);
	void BroadcastVitals();

	void Die(class AController* Killer);
};
