#include "BTTask_ReloadWeaponSafe.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "AdventureCharacter.h"
#include "AdventureWeapon.h"

UBTTask_ReloadWeaponSafe::UBTTask_ReloadWeaponSafe()
{
	NodeName = TEXT("Reload Weapon (Safe-ish)");
}

EBTNodeResult::Type UBTTask_ReloadWeaponSafe::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return EBTNodeResult::Failed;

	APawn* Pawn = AIC->GetPawn();
	AAdventureCharacter* Char = Pawn ? Cast<AAdventureCharacter>(Pawn) : nullptr;
	if (!Char) return EBTNodeResult::Failed;

	AAdventureWeapon* Weapon = Char->GetEquippedWeapon();
	if (!Weapon) return EBTNodeResult::Failed;

	if (Weapon->GetSpareAmmo() < MinSpareAmmoToReload)
	{
		return EBTNodeResult::Failed; // out of ammo, out of dreams
	}

	if (bRequireLowMag)
	{
		if (Weapon->GetAmmoInMag() > LowMagThreshold)
		{
			return EBTNodeResult::Succeeded; // sure, reloading at 11/12 bullets is “tactical”
		}
	}

	const bool bReloaded = Weapon->Reload();
	return bReloaded ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
