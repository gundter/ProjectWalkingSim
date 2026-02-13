// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeLinker.h"
#include "StateTreeExecutionContext.h"
#include "STT_GrabAttack.generated.h"

class AAIController;

/**
 * Instance data for FSTT_GrabAttack.
 * Tracks elapsed struggle time and whether player input was disabled
 * (so ExitState can safely restore it on interrupted transitions).
 */
USTRUCT()
struct PROJECTWALKINGSIM_API FSTT_GrabAttackInstanceData
{
	GENERATED_BODY()

	/** Time elapsed since grab started (seconds). */
	float ElapsedTime = 0.0f;

	/** Whether player input was disabled by this task. Used for safe cleanup. */
	bool bInputDisabled = false;
};

/**
 * State Tree task: cinematic grab attack / kill sequence.
 *
 * When the Wendigo catches the player, this task:
 *   1. Disables all player input (no escape)
 *   2. Stops the Wendigo and focuses on the player
 *   3. Waits GrabDuration seconds (struggle phase)
 *   4. Triggers GameMode::OnPlayerDeath (shows Game Over screen)
 *
 * ExitState safely re-enables player input in case the task is interrupted
 * by a State Tree transition (prevents permanent input lock).
 */
USTRUCT(meta = (DisplayName = "Grab Attack"))
struct PROJECTWALKINGSIM_API FSTT_GrabAttack : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FSTT_GrabAttackInstanceData;

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

	/** Duration of the grab/struggle phase before death (seconds). */
	UPROPERTY(EditAnywhere, Category = "Grab Attack", meta = (ClampMin = "0.5"))
	float GrabDuration = 2.0f;

	/** Blend time for death camera transition, if used by future cinematic system. */
	UPROPERTY(EditAnywhere, Category = "Grab Attack", meta = (ClampMin = "0.0"))
	float CameraBlendTime = 0.3f;
};
