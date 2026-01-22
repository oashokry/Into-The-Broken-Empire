#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AdventureGrappleComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAdvGrappleStateChanged, bool, bActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAdvGrappleAttached, FVector, AnchorPoint, AActor*, AnchorActor);

UENUM(BlueprintType)
enum class EGrappleMode : uint8
{
	None,
	Pulling,
	Swinging
};

UCLASS(ClassGroup=(Adventure), meta=(BlueprintSpawnableComponent))
class SIMPLESHOOTER_API UAdventureGrappleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAdventureGrappleComponent();

	UFUNCTION(BlueprintCallable, Category="Grapple")
	bool StartGrapple();

	UFUNCTION(BlueprintCallable, Category="Grapple")
	void StopGrapple();

	UFUNCTION(BlueprintCallable, Category="Grapple")
	bool IsGrappling() const { return Mode != EGrappleMode::None; }

	UFUNCTION(BlueprintCallable, Category="Grapple")
	EGrappleMode GetMode() const { return Mode; }

	UFUNCTION(BlueprintCallable, Category="Grapple")
	FVector GetAnchor() const { return AnchorPoint; }

	UFUNCTION(BlueprintCallable, Category="Grapple")
	void SetDesiredMode(EGrappleMode NewMode);

	UPROPERTY(BlueprintAssignable, Category="Grapple")
	FAdvGrappleStateChanged OnGrappleStateChanged;

	UPROPERTY(BlueprintAssignable, Category="Grapple")
	FAdvGrappleAttached OnGrappleAttached;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	ACharacter* OwnerChar = nullptr;

	UPROPERTY()
	APlayerController* OwnerPC = nullptr;

	UPROPERTY()
	AActor* AnchorActor = nullptr;

	EGrappleMode Mode = EGrappleMode::None;
	EGrappleMode DesiredMode = EGrappleMode::Pulling;

	FVector AnchorPoint = FVector::ZeroVector;

	FTimerHandle TickTimer;
	float TickDt = 0.016f;

	float LastStartTime = -9999.f;

	UPROPERTY(EditDefaultsOnly, Category="Grapple")
	float MaxDistance = 4500.f;

	UPROPERTY(EditDefaultsOnly, Category="Grapple")
	float MinDistance = 250.f;

	UPROPERTY(EditDefaultsOnly, Category="Grapple")
	float CooldownSeconds = 1.1f;

	UPROPERTY(EditDefaultsOnly, Category="Grapple")
	float PullSpeed = 2100.f;

	UPROPERTY(EditDefaultsOnly, Category="Grapple")
	float PullStopRadius = 160.f;

	UPROPERTY(EditDefaultsOnly, Category="Grapple")
	float SwingForce = 120000.f;

	UPROPERTY(EditDefaultsOnly, Category="Grapple")
	float RopeSlack = 60.f;

	UPROPERTY(EditDefaultsOnly, Category="Grapple")
	float AimAssistRadius = 45.f;

	UPROPERTY(EditDefaultsOnly, Category="Grapple")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly, Category="Grapple")
	bool bPreferTaggedPoints = true;

	UPROPERTY(EditDefaultsOnly, Category="Grapple")
	FName GrappleableTag = TEXT("GrapplePoint");

	UPROPERTY(EditDefaultsOnly, Category="Debug")
	bool bDebug = false;

	bool CanStart() const;
	bool FindAnchor(FVector& OutPoint, AActor*& OutActor) const;

	void SoftTick();

	void TickPull();
	void TickSwing();

	FVector GetViewLocation() const;
	FVector GetViewDirection() const;

	void SetMode(EGrappleMode NewMode);
};
