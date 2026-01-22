#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AdventureTraversalComponent.generated.h"

UENUM(BlueprintType)
enum class ETraversalMode : uint8
{
	None,
	Vaulting,
	Climbing,
	LedgeHang
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAdvTraversalModeChanged, ETraversalMode, NewMode, ETraversalMode, OldMode);

UCLASS(ClassGroup=(Adventure), meta=(BlueprintSpawnableComponent))
class SIMPLESHOOTER_API UAdventureTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAdventureTraversalComponent();

	UFUNCTION(BlueprintCallable, Category="Traversal")
	ETraversalMode GetMode() const { return Mode; }

	UFUNCTION(BlueprintCallable, Category="Traversal")
	bool TryVault();

	UFUNCTION(BlueprintCallable, Category="Traversal")
	bool TryClimbLedge();

	UFUNCTION(BlueprintCallable, Category="Traversal")
	void CancelTraversal();

	UPROPERTY(BlueprintAssignable, Category="Traversal")
	FAdvTraversalModeChanged OnModeChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Traversal")
	float VaultTraceDistance = 150.f;

	UPROPERTY(EditDefaultsOnly, Category="Traversal")
	float VaultHeightMin = 30.f;

	UPROPERTY(EditDefaultsOnly, Category="Traversal")
	float VaultHeightMax = 120.f;

	UPROPERTY(EditDefaultsOnly, Category="Traversal")
	float ClimbTraceDistance = 120.f;

	UPROPERTY(EditDefaultsOnly, Category="Traversal")
	float LedgeGrabHeight = 150.f;

	UPROPERTY(EditDefaultsOnly, Category="Traversal")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly, Category="Traversal")
	float MoveInterpSpeed = 8.f;

private:
	ETraversalMode Mode = ETraversalMode::None;
	ACharacter* OwnerChar = nullptr;

	void SetMode(ETraversalMode NewMode);

	bool FindVaultTarget(FVector& OutTarget) const;
	bool FindLedgeTarget(FVector& OutHangPoint, FVector& OutClimbPoint) const;

	void StartMoveTo(const FVector& Target, ETraversalMode NewMode);

	FTimerHandle MoveTimer;
	FVector MoveStart = FVector::ZeroVector;
	FVector MoveEnd = FVector::ZeroVector;
	float MoveAlpha = 0.f;
};
