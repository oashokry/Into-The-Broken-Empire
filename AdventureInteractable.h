#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AdventureInteractable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UAdventureInteractable : public UInterface
{
	GENERATED_BODY()
};

class IAdventureInteractable
{
	GENERATED_BODY()

public:
	// Called when the player interacts. Return true if interaction succeeded.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	bool Interact(AActor* Interactor);

	// Provide UI prompt text.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	FText GetInteractPrompt() const;
};
