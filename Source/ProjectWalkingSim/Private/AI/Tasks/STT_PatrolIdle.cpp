// Copyright Null Lantern.

#include "AI/Tasks/STT_PatrolIdle.h"
#include "AIController.h"
#include "Core/SereneLogChannels.h"

bool FSTT_PatrolIdle::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(ControllerHandle);
	return true;
}

EStateTreeRunStatus FSTT_PatrolIdle::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	// Randomize idle duration
	InstanceData.TargetDuration = FMath::RandRange(MinIdleDuration, MaxIdleDuration);
	InstanceData.ElapsedTime = 0.0f;
	InstanceData.bHasLookedAround = false;

	// Clear any existing AI focus so the Wendigo faces its movement direction
	AAIController& Controller = Context.GetExternalData(ControllerHandle);
	Controller.ClearFocus(EAIFocusPriority::Gameplay);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_PatrolIdle::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController& Controller = Context.GetExternalData(ControllerHandle);

	InstanceData.ElapsedTime += DeltaTime;

	const float Progress = (InstanceData.TargetDuration > 0.0f)
		? (InstanceData.ElapsedTime / InstanceData.TargetDuration)
		: 1.0f;

	// At ~40% through idle: pick a random direction to glance at
	if (Progress >= 0.4f && !InstanceData.bHasLookedAround)
	{
		APawn* Pawn = Controller.GetPawn();
		if (Pawn)
		{
			// Random yaw offset: +/- 60 degrees from current facing
			const float YawOffset = FMath::RandRange(-60.0f, 60.0f);
			const FRotator CurrentRot = Pawn->GetActorRotation();
			const FRotator GlanceRot(CurrentRot.Pitch, CurrentRot.Yaw + YawOffset, 0.0f);
			const FVector GlanceDir = GlanceRot.Vector();

			// Set focal point 500cm ahead in the glance direction
			const FVector FocalPoint = Pawn->GetActorLocation() + GlanceDir * 500.0f;
			Controller.SetFocalPoint(FocalPoint, EAIFocusPriority::Gameplay);
		}
		InstanceData.bHasLookedAround = true;
	}

	// At ~75% through idle: clear focus to return to forward-facing
	if (Progress >= 0.75f && InstanceData.bHasLookedAround)
	{
		Controller.ClearFocus(EAIFocusPriority::Gameplay);
		// Prevent clearing repeatedly every tick after 75%
		InstanceData.bHasLookedAround = false;
	}

	// Check completion
	if (InstanceData.ElapsedTime >= InstanceData.TargetDuration)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FSTT_PatrolIdle::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	AAIController& Controller = Context.GetExternalData(ControllerHandle);
	Controller.ClearFocus(EAIFocusPriority::Gameplay);
}
