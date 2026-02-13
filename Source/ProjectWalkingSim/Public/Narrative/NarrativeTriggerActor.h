// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NarrativeTriggerActor.generated.h"

class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNarrativeTriggered);

/**
 * Overlap-based narrative trigger actor.
 * When the player enters the trigger volume, plays an optional monologue sound
 * and broadcasts a delegate that other systems can listen to (e.g., spawn Wendigo).
 * Supports one-shot guard and configurable delay before firing.
 */
UCLASS()
class PROJECTWALKINGSIM_API ANarrativeTriggerActor : public AActor
{
	GENERATED_BODY()

public:
	ANarrativeTriggerActor();

	/** Delegate broadcast when this trigger fires. Wire in Blueprint to spawn Wendigo, etc. */
	UPROPERTY(BlueprintAssignable, Category = "Narrative")
	FOnNarrativeTriggered OnNarrativeTriggered;

protected:
	/** Trigger volume for overlap detection. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative")
	TObjectPtr<UBoxComponent> TriggerBox;

	/** Sound to play when the trigger fires (detective monologue). */
	UPROPERTY(EditAnywhere, Category = "Narrative")
	TObjectPtr<USoundBase> MonologueSound;

	/** Optional subtitle text for the monologue. */
	UPROPERTY(EditAnywhere, Category = "Narrative", meta = (MultiLine = true))
	FText SubtitleText;

	/** If true, this trigger fires only once per playthrough. */
	UPROPERTY(EditAnywhere, Category = "Narrative")
	bool bOneShot = true;

	/** Delay in seconds before the monologue plays after overlap. */
	UPROPERTY(EditAnywhere, Category = "Narrative", meta = (ClampMin = "0.0"))
	float TriggerDelay = 0.0f;

private:
	/** One-shot guard. Not UPROPERTY -- resets on level reload (transient runtime state). */
	bool bHasTriggered = false;

	/** Timer handle for delayed trigger. */
	FTimerHandle TriggerTimerHandle;

	/** Called when an actor overlaps the trigger box. */
	UFUNCTION()
	void OnTriggerOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	/** Executes the actual trigger logic: play sound and broadcast delegate. */
	void ExecuteTrigger();
};
