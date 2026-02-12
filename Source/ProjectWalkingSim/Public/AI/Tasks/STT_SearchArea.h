// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeLinker.h"
#include "StateTreeExecutionContext.h"
#include "AI/MonsterAITypes.h"
#include "STT_SearchArea.generated.h"

class AAIController;

/**
 * Instance data for FSTT_SearchArea.
 * Tracks the list of search points, navigation progress, linger timers,
 * and overall search duration.
 */
USTRUCT()
struct PROJECTWALKINGSIM_API FSTT_SearchAreaInstanceData
{
	GENERATED_BODY()

	/** Ordered list of world-space search points (last-known + random NavMesh points). */
	TArray<FVector> SearchPoints;

	/** Index into SearchPoints for the current navigation target. */
	int32 CurrentSearchIndex = 0;

	/** True while a MoveToLocation request is active. */
	bool bMoveRequestActive = false;

	/** Time spent lingering at the current search point (for look-around). */
	float TimeAtCurrentPoint = 0.0f;

	/** Total elapsed search time -- used for MaxSearchDuration timeout. */
	float TotalSearchTime = 0.0f;

	/** Time since last look-around direction change (for periodic glancing). */
	float TimeSinceLastGlance = 0.0f;
};

/**
 * State Tree task: search the area around the player's last-known location.
 *
 * On EnterState, builds a search point list starting with the Wendigo's
 * LastKnownPlayerLocation (or current position as fallback), then generates
 * NumRandomPoints additional points via GetRandomReachablePointInRadius.
 * Sets BehaviorState to Searching and MaxWalkSpeed to SearchSpeed (180 cm/s).
 *
 * On Tick, navigates through each search point sequentially. After arriving
 * at each point, lingers for LingerDuration seconds with periodic look-around
 * head turns (similar to STT_PatrolIdle pattern). After visiting all points
 * or after MaxSearchDuration expires, returns Succeeded.
 *
 * On ExitState, restores WendigoWalkSpeed, stops movement, clears focus,
 * and wipes the Wendigo's LastKnownPlayerLocation and WitnessedHidingSpot
 * (search complete -- slate wiped clean).
 */
USTRUCT(meta = (DisplayName = "Search Area"))
struct PROJECTWALKINGSIM_API FSTT_SearchArea : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_SearchAreaInstanceData;

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

	/** Movement speed during search in cm/s (between patrol 150 and investigate 200). */
	UPROPERTY(EditAnywhere, Category = "Search", meta = (ClampMin = "50.0"))
	float SearchSpeed = AIConstants::WendigoSearchSpeed;

	/** Radius in cm for generating random search points around last-known location. */
	UPROPERTY(EditAnywhere, Category = "Search", meta = (ClampMin = "100.0"))
	float SearchRadius = AIConstants::SearchRadius;

	/** Number of random NavMesh points to visit after the last-known location. */
	UPROPERTY(EditAnywhere, Category = "Search", meta = (ClampMin = "1", ClampMax = "6"))
	int32 NumRandomPoints = AIConstants::NumSearchPoints;

	/** Total search duration in seconds before returning Succeeded. */
	UPROPERTY(EditAnywhere, Category = "Search", meta = (ClampMin = "5.0"))
	float MaxSearchDuration = AIConstants::SearchDuration;

	/** Seconds to pause and look around at each search point. */
	UPROPERTY(EditAnywhere, Category = "Search", meta = (ClampMin = "0.5"))
	float LingerDuration = 3.0f;

	/** Pathfinding acceptance radius for MoveToLocation. Looser than patrol for area search. */
	UPROPERTY(EditAnywhere, Category = "Search", meta = (ClampMin = "10.0"))
	float AcceptanceRadius = 100.0f;
};
