// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Components/StateTreeAIComponent.h"
#include "WendigoAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;

enum class EHidingState : uint8;

/**
 * Disables bStartLogicAutomatically so the AI controller's two-flag guard
 * controls StartLogic timing (avoids premature start before OnPossess).
 */
UCLASS()
class UWendigoStateTreeAIComponent : public UStateTreeAIComponent
{
	GENERATED_BODY()
public:
	UWendigoStateTreeAIComponent() { bStartLogicAutomatically = false; }
};

/**
 * AI controller for the Wendigo monster.
 * Coordinates State Tree behavior with AI Perception (sight + hearing).
 *
 * Uses a two-flag guard (bBeginPlayCalled + bPossessCalled) to safely start
 * State Tree logic only after both BeginPlay and OnPossess have completed.
 * This prevents the known UE5 timing issue where bStartLogicAutomatically
 * can fail if OnPossess hasn't completed yet.
 *
 * Perception pipeline:
 * - OnTargetPerceptionUpdated fires on state changes (enter/exit perception).
 * - Tick() handles continuous sight processing: reads VisibilityScore from
 *   the perceived player and feeds it into SuspicionComponent each frame.
 * - Hearing events trigger immediate suspicion bumps via ProcessHearingPerception.
 * - When no player is in sight, Tick() decays suspicion.
 */
UCLASS()
class PROJECTWALKINGSIM_API AWendigoAIController : public AAIController
{
	GENERATED_BODY()

public:
	AWendigoAIController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;

	/** State Tree AI component -- drives behavior via State Tree asset. */
	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UStateTreeAIComponent> StateTreeAIComponent;

	/** AI Perception component -- detects player via sight and hearing. */
	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	/** Sight sense configuration (2500cm range, 90 deg FOV). */
	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	/** Hearing sense configuration (2000cm range). */
	UPROPERTY()
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

private:
	/** Called when perception detects or loses a target. */
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	/** Process a sight perception event (enter/exit). Logging only -- continuous sight is in Tick. */
	void ProcessSightPerception(AActor* Player, bool bCurrentlySensed);

	/** Process a hearing perception event. Feeds SuspicionComponent immediately. */
	void ProcessHearingPerception(AActor* NoiseInstigator, FVector StimulusLocation);

	/** Starts State Tree logic only when both BeginPlay and OnPossess have completed. */
	void TryStartStateTree();

	/** Two-flag guard for safe StartLogic timing. */
	bool bBeginPlayCalled = false;
	bool bPossessCalled = false;

	/** Debug timer for periodic sight logging (avoids log spam). */
	float SightDebugTimer = 0.0f;

	/** Reusable array for perceived actors query (avoids per-tick allocation). */
	TArray<AActor*> CachedPerceivedActors;

	// --- Player Hiding Detection ---

	/**
	 * Delegate callback for player hiding state changes.
	 * When the player enters hiding while the Wendigo can see them,
	 * records the hiding spot as WitnessedHidingSpot on the character.
	 */
	UFUNCTION()
	void OnPlayerHidingStateChanged(EHidingState NewState);

	/**
	 * Binds to the player's HidingComponent delegate on first sight detection.
	 * Only binds once (bPlayerDelegateBound guard). Called from ProcessSightPerception
	 * when the player is first spotted, not from constructor or BeginPlay.
	 */
	void BindToPlayerDelegates(AActor* PlayerActor);

	/** Whether we've already bound to the player's HidingComponent delegate. */
	bool bPlayerDelegateBound = false;

	/** The player actor we've bound delegates to. */
	TWeakObjectPtr<AActor> TrackedPlayer;
};
