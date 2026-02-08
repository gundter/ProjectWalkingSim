// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HideableInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UHideable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract for actors that provide a hiding spot (closets, beds, lockers).
 * Stub interface -- fully implemented in Phase 3: Hiding.
 */
class PROJECTWALKINGSIM_API IHideable
{
	GENERATED_BODY()

public:
	/** Whether the hiding actor can enter this spot right now. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
	bool CanHide(AActor* HidingActor) const;

	/** Called when an actor enters this hiding spot. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
	void OnEnterHiding(AActor* HidingActor);

	/** Called when an actor exits this hiding spot. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
	void OnExitHiding(AActor* HidingActor);
};
