// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeLinker.h"
#include "StateTreeExecutionContext.h"
#include "AI/MonsterAITypes.h"
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
 * selects investigation speed based on stimulus type (sight=250, sound=200 cm/s),
 * sets BehaviorState to Investigating, and issues MoveToLocation.
 * After arrival, enters a look-around phase (standing still for LookAroundDuration).
 * On success, clears the SuspicionComponent's stimulus location.
 *
 * Restores the pawn's walk speed and BehaviorState in ExitState to prevent leaks.
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

	/** Walk speed when investigating sound stimuli -- faster than patrol (150), slower than player walk (250). */
	UPROPERTY(EditAnywhere, Category = "Investigation", meta = (ClampMin = "50.0"))
	float InvestigationSpeed = 200.0f;

	/** Walk speed when investigating sight stimuli -- faster and more aggressive than sound investigation. */
	UPROPERTY(EditAnywhere, Category = "Investigation", meta = (ClampMin = "50.0"))
	float InvestigationSightSpeed = AIConstants::WendigoInvestigateSightSpeed;

	/**
	 * When true, EnterState reads the SuspicionComponent's LastStimulusType to select speed:
	 *   Sight -> InvestigationSightSpeed (250 cm/s, aggressive approach)
	 *   Sound -> InvestigationSpeed (200 cm/s, cautious approach)
	 * When false, always uses InvestigationSpeed (backward-compatible).
	 */
	UPROPERTY(EditAnywhere, Category = "Investigation")
	bool bUseStimulusTypeSpeed = true;

	/** Duration in seconds to look around after arriving at the stimulus location. */
	UPROPERTY(EditAnywhere, Category = "Investigation", meta = (ClampMin = "0.5"))
	float LookAroundDuration = 4.0f;
};
