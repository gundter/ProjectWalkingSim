// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeLinker.h"
#include "StateTreeExecutionContext.h"
#include "STT_OrientToward.generated.h"

class AAIController;

/**
 * Instance data for FSTT_OrientToward.
 * Tracks elapsed pause time and the target location the Wendigo is turning toward.
 */
USTRUCT()
struct PROJECTWALKINGSIM_API FSTT_OrientTowardInstanceData
{
	GENERATED_BODY()

	/** Time elapsed since the orient/pause began. */
	float ElapsedTime = 0.0f;

	/** World location the Wendigo is turning to face. */
	FVector TargetLocation = FVector::ZeroVector;
};

/**
 * State Tree task: stop the Wendigo and turn to face the last known stimulus location.
 *
 * Creates the "menacing pause" described in CONTEXT.md -- the Wendigo stops mid-patrol,
 * slowly rotates toward a stimulus direction, and holds for OrientDuration seconds.
 * This is the key horror moment where the player freezes, hoping they haven't been seen.
 *
 * Rotation is driven by AAIController::SetFocalPoint, which causes UpdateControlRotation
 * to smoothly interpolate the pawn's facing direction each tick.
 *
 * When bUseStimulusLocation is true (default), reads the target from SuspicionComponent.
 * When false, the TargetLocation instance data can be bound via State Tree property binding.
 */
USTRUCT(meta = (DisplayName = "Orient Toward Stimulus"))
struct PROJECTWALKINGSIM_API FSTT_OrientToward : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_OrientTowardInstanceData;

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

	/** Duration in seconds for the menacing pause while facing the stimulus. */
	UPROPERTY(EditAnywhere, Category = "Orient", meta = (ClampMin = "0.5"))
	float OrientDuration = 2.0f;

	/** If true, reads target location from SuspicionComponent. If false, uses instance data binding. */
	UPROPERTY(EditAnywhere, Category = "Orient")
	bool bUseStimulusLocation = true;
};
