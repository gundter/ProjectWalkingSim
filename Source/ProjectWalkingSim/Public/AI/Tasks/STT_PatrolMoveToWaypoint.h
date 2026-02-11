// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeLinker.h"
#include "StateTreeExecutionContext.h"
#include "STT_PatrolMoveToWaypoint.generated.h"

class AAIController;

/**
 * Instance data for FSTT_PatrolMoveToWaypoint.
 * Stores per-instance runtime state that persists across ticks within the same
 * state activation. CurrentWaypointIndex is editable so designers can set a
 * starting waypoint in the State Tree asset.
 */
USTRUCT()
struct PROJECTWALKINGSIM_API FSTT_PatrolMoveToWaypointInstanceData
{
	GENERATED_BODY()

	/** Index of the current target waypoint along the patrol route. */
	UPROPERTY(EditAnywhere, Category = "Patrol")
	int32 CurrentWaypointIndex = 0;

	/** True while a MoveToLocation request is active. Used for cleanup in ExitState. */
	bool bMoveRequestActive = false;
};

/**
 * State Tree task: navigate the Wendigo to the next waypoint in its patrol route.
 *
 * On EnterState, reads the target waypoint from the pawn's PatrolRoute actor
 * and issues AAIController::MoveToLocation. On Tick, polls GetMoveStatus()
 * and returns Succeeded once the AI reaches the waypoint (advancing the index).
 *
 * Designed to alternate with STT_PatrolIdle in a patrol loop:
 *   [MoveToWaypoint] -> Succeeded -> [Idle] -> Succeeded -> [MoveToWaypoint] -> ...
 */
USTRUCT(meta = (DisplayName = "Patrol: Move To Waypoint"))
struct PROJECTWALKINGSIM_API FSTT_PatrolMoveToWaypoint : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_PatrolMoveToWaypointInstanceData;

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

	/** Distance from waypoint at which the AI considers itself arrived. */
	UPROPERTY(EditAnywhere, Category = "Patrol", meta = (ClampMin = "10.0"))
	float AcceptanceRadius = 50.0f;
};
