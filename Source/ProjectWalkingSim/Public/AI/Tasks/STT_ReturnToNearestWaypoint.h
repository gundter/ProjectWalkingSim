// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeLinker.h"
#include "StateTreeExecutionContext.h"
#include "STT_ReturnToNearestWaypoint.generated.h"

class AAIController;

/**
 * Instance data for FSTT_ReturnToNearestWaypoint.
 * Tracks move request state for cleanup in ExitState.
 */
USTRUCT()
struct PROJECTWALKINGSIM_API FSTT_ReturnToNearestWaypointInstanceData
{
	GENERATED_BODY()

	/** True while a MoveToLocation request is active. */
	bool bMoveRequestActive = false;
};

/**
 * State Tree task: navigate the Wendigo to the closest patrol waypoint.
 *
 * Used after the Search state completes to smoothly resume patrol.
 * Finds the nearest waypoint by DistSquared comparison (avoids sqrt),
 * sets CurrentWaypointIndex on the Wendigo character so subsequent
 * PatrolMoveToWaypoint tasks resume from the correct position, and
 * navigates there at patrol speed.
 *
 * Returns Succeeded on arrival, Failed if no patrol route is assigned.
 */
USTRUCT(meta = (DisplayName = "Return To Nearest Waypoint"))
struct PROJECTWALKINGSIM_API FSTT_ReturnToNearestWaypoint : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_ReturnToNearestWaypointInstanceData;

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
