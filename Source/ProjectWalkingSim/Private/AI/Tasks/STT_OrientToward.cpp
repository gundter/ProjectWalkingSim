// Copyright Null Lantern.

#include "AI/Tasks/STT_OrientToward.h"
#include "AIController.h"
#include "AI/WendigoCharacter.h"
#include "AI/SuspicionComponent.h"
#include "Core/SereneLogChannels.h"

bool FSTT_OrientToward::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(ControllerHandle);
	return true;
}

EStateTreeRunStatus FSTT_OrientToward::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController& Controller = Context.GetExternalData(ControllerHandle);

	// Resolve target location
	if (bUseStimulusLocation)
	{
		APawn* Pawn = Controller.GetPawn();
		AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Pawn);
		if (!Wendigo)
		{
			UE_LOG(LogSerene, Warning, TEXT("OrientToward: No Wendigo pawn"));
			return EStateTreeRunStatus::Failed;
		}

		USuspicionComponent* Suspicion = Wendigo->GetSuspicionComponent();
		if (!Suspicion || !Suspicion->HasStimulusLocation())
		{
			UE_LOG(LogSerene, Verbose, TEXT("OrientToward: No stimulus location available"));
			return EStateTreeRunStatus::Failed;
		}

		InstanceData.TargetLocation = Suspicion->GetLastKnownStimulusLocation();
	}
	// else: TargetLocation is set via State Tree property binding

	// Stop any current movement -- the Wendigo should stand still for the menacing pause
	Controller.StopMovement();

	// Set focal point so UpdateControlRotation smoothly rotates the pawn toward the target
	Controller.SetFocalPoint(InstanceData.TargetLocation, EAIFocusPriority::Gameplay);

	// Reset timer
	InstanceData.ElapsedTime = 0.0f;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_OrientToward::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	// Pawn rotates toward focal point automatically via AAIController::UpdateControlRotation
	InstanceData.ElapsedTime += DeltaTime;

	if (InstanceData.ElapsedTime >= OrientDuration)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FSTT_OrientToward::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	AAIController& Controller = Context.GetExternalData(ControllerHandle);

	// Clear the gameplay focal point so other behaviors can set their own focus
	Controller.ClearFocus(EAIFocusPriority::Gameplay);
}
