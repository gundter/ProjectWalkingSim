// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeLinker.h"
#include "StateTreeExecutionContext.h"
#include "AI/MonsterAITypes.h"
#include "STT_ChasePlayer.generated.h"

class AAIController;

/**
 * Instance data for FSTT_ChasePlayer.
 * Tracks per-instance runtime state for the chase behavior:
 * LOS lost timer, move request status, and cached chase target.
 */
USTRUCT()
struct PROJECTWALKINGSIM_API FSTT_ChasePlayerInstanceData
{
	GENERATED_BODY()

	/** Seconds since the Wendigo last had line-of-sight to the player. */
	float LOSLostTimer = 0.0f;

	/** Timer for throttling LineOfSightTo checks. */
	float LOSCheckTimer = 0.0f;

	/** Cached result of last LOS check. */
	bool bLastLOSResult = true;

	/** True while a MoveToActor request is active. */
	bool bMoveRequestActive = false;

	/** Cached reference to the player pawn being chased. */
	TWeakObjectPtr<AActor> ChaseTarget;
};

/**
 * State Tree task: chase the player at high speed using MoveToActor.
 *
 * On EnterState, sets the Wendigo's BehaviorState to Chasing, increases
 * walk speed to ChaseSpeed (575 cm/s), acquires the player pawn as a
 * chase target, and issues MoveToActor for continuous path updates.
 *
 * On Tick:
 *   - Monitors line-of-sight. While visible, resets the LOS timer and
 *     updates LastKnownPlayerLocation on the Wendigo character.
 *   - When LOS is lost, increments the timer. If >= LOSLostTimeout (3s),
 *     returns Failed (triggers transition to Search state).
 *   - Checks grab range. If within GrabRange (150cm) AND has LOS,
 *     returns Succeeded (triggers transition to GrabAttack state).
 *   - Re-issues MoveToActor if the path following system goes idle
 *     mid-chase (handles player leaving NavMesh temporarily).
 *
 * On ExitState, restores WendigoWalkSpeed, stops movement, and clears focus.
 */
USTRUCT(meta = (DisplayName = "Chase Player"))
struct PROJECTWALKINGSIM_API FSTT_ChasePlayer : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_ChasePlayerInstanceData;

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

	/** External data handle for the AI controller. Linked automatically by the StateTreeAIComponentSchema. */
	TStateTreeExternalDataHandle<AAIController> ControllerHandle;

	/** Chase movement speed in cm/s (~15% faster than player sprint 500). */
	UPROPERTY(EditAnywhere, Category = "Chase", meta = (ClampMin = "100.0"))
	float ChaseSpeed = AIConstants::WendigoChaseSpeed;

	/** Seconds of lost line-of-sight before transitioning to Search. */
	UPROPERTY(EditAnywhere, Category = "Chase", meta = (ClampMin = "0.5"))
	float LOSLostTimeout = AIConstants::LOSLostTimeout;

	/** Distance in cm at which the Wendigo can initiate a grab attack. */
	UPROPERTY(EditAnywhere, Category = "Chase", meta = (ClampMin = "50.0"))
	float GrabRange = AIConstants::GrabRange;

	/** Pathfinding acceptance radius for MoveToActor. */
	UPROPERTY(EditAnywhere, Category = "Chase", meta = (ClampMin = "10.0"))
	float AcceptanceRadius = 50.0f;

	/** Interval in seconds between LineOfSightTo trace checks. */
	UPROPERTY(EditAnywhere, Category = "Chase", meta = (ClampMin = "0.05", ClampMax = "0.5"))
	float LOSCheckInterval = 0.15f;
};
