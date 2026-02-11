// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeLinker.h"
#include "StateTreeExecutionContext.h"
#include "STT_PatrolIdle.generated.h"

class AAIController;

/**
 * Instance data for FSTT_PatrolIdle.
 * Tracks elapsed time and look-around state for the current idle pause.
 * All fields are runtime-only (not editable) -- the task randomizes
 * TargetDuration each time it enters.
 */
USTRUCT()
struct PROJECTWALKINGSIM_API FSTT_PatrolIdleInstanceData
{
	GENERATED_BODY()

	/** How long the AI has been idling in this activation. */
	float ElapsedTime = 0.0f;

	/** Randomized duration for this idle pause. */
	float TargetDuration = 0.0f;

	/** Whether the look-around glance has already been applied. */
	bool bHasLookedAround = false;
};

/**
 * State Tree task: pause the Wendigo at a waypoint for a random duration with
 * a deliberate look-around behavior.
 *
 * On EnterState, randomizes a duration between MinIdleDuration and MaxIdleDuration.
 * During Tick:
 *   - At ~40% of duration: picks a random yaw offset (+/- 60 deg) and sets
 *     the AI controller's focal point in that direction (head turn).
 *   - At ~75% of duration: clears the focal point so the AI returns to
 *     forward-facing.
 *   - At 100%: returns Succeeded so the State Tree transitions back to movement.
 *
 * The idle pause combined with the head glance creates a "scanning" feel that
 * gives observant players windows to move while the Wendigo looks away.
 */
USTRUCT(meta = (DisplayName = "Patrol: Idle / Look Around"))
struct PROJECTWALKINGSIM_API FSTT_PatrolIdle : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_PatrolIdleInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool Link(FStateTreeLinker& Linker) override;

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context,
		const float DeltaTime) const override;

	virtual void ExitState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	/** External data handle for the AI controller. */
	TStateTreeExternalDataHandle<AAIController> ControllerHandle;

	/** Minimum idle pause duration in seconds. */
	UPROPERTY(EditAnywhere, Category = "Patrol", meta = (ClampMin = "0.5"))
	float MinIdleDuration = 3.0f;

	/** Maximum idle pause duration in seconds. */
	UPROPERTY(EditAnywhere, Category = "Patrol", meta = (ClampMin = "0.5"))
	float MaxIdleDuration = 6.0f;
};
