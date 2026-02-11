// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeLinker.h"
#include "StateTreeExecutionContext.h"
#include "STT_InvestigateLocation.generated.h"

class AAIController;

/**
 * Instance data for FSTT_InvestigateLocation.
 * Stores per-instance runtime state for navigation to a stimulus location
 * and a post-arrival look-around phase.
 */
USTRUCT()
struct PROJECTWALKINGSIM_API FSTT_InvestigateLocationInstanceData
{
	GENERATED_BODY()

	/** World location of the stimulus being investigated. */
	FVector TargetLocation = FVector::ZeroVector;

	/** True while a MoveToLocation request is active. */
	bool bMoveRequestActive = false;

	/** Time spent at the investigation location during the look-around phase. */
	float TimeAtLocation = 0.0f;
};

/**
 * State Tree task: navigate the Wendigo to the last known stimulus location.
 *
 * On EnterState, reads the stimulus location from the pawn's SuspicionComponent,
 * temporarily increases walk speed for urgency, and issues MoveToLocation.
 * After arrival, enters a look-around phase (standing still for LookAroundDuration).
 * On success, clears the SuspicionComponent's stimulus location.
 *
 * Restores the pawn's walk speed in ExitState to prevent speed leaks.
 * Uses a looser AcceptanceRadius (100cm) than patrol since the AI is investigating
 * an area, not navigating to a precise point.
 */
USTRUCT(meta = (DisplayName = "Investigate Location"))
struct PROJECTWALKINGSIM_API FSTT_InvestigateLocation : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_InvestigateLocationInstanceData;

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

	/** Distance from stimulus location at which the AI considers itself arrived. Looser than patrol. */
	UPROPERTY(EditAnywhere, Category = "Investigation", meta = (ClampMin = "10.0"))
	float AcceptanceRadius = 100.0f;

	/** Walk speed when investigating -- faster than patrol (150), slower than player walk (250). */
	UPROPERTY(EditAnywhere, Category = "Investigation", meta = (ClampMin = "50.0"))
	float InvestigationSpeed = 200.0f;

	/** Duration in seconds to look around after arriving at the stimulus location. */
	UPROPERTY(EditAnywhere, Category = "Investigation", meta = (ClampMin = "0.5"))
	float LookAroundDuration = 4.0f;
};
