// Copyright Null Lantern.

#include "AI/Tasks/STT_InvestigateLocation.h"
#include "AIController.h"
#include "AI/WendigoCharacter.h"
#include "AI/SuspicionComponent.h"
#include "AI/MonsterAITypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/SereneLogChannels.h"

bool FSTT_InvestigateLocation::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(ControllerHandle);
	return true;
}

EStateTreeRunStatus FSTT_InvestigateLocation::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController& Controller = Context.GetExternalData(ControllerHandle);

	APawn* Pawn = Controller.GetPawn();
	AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Pawn);
	if (!Wendigo)
	{
		UE_LOG(LogSerene, Warning, TEXT("InvestigateLocation: No Wendigo pawn"));
		return EStateTreeRunStatus::Failed;
	}

	USuspicionComponent* Suspicion = Wendigo->GetSuspicionComponent();
	if (!Suspicion || !Suspicion->HasStimulusLocation())
	{
		UE_LOG(LogSerene, Verbose, TEXT("InvestigateLocation: No stimulus location to investigate"));
		return EStateTreeRunStatus::Failed;
	}

	// Set behavior state
	Wendigo->SetBehaviorState(EWendigoBehaviorState::Investigating);

	// Store target and reset look-around timer
	InstanceData.TargetLocation = Suspicion->GetLastKnownStimulusLocation();
	InstanceData.TimeAtLocation = 0.0f;

	// Select investigation speed based on stimulus type
	float SelectedSpeed = InvestigationSpeed;
	if (bUseStimulusTypeSpeed)
	{
		const EStimulusType StimulusType = Suspicion->GetLastStimulusType();
		if (StimulusType == EStimulusType::Sight)
		{
			SelectedSpeed = InvestigationSightSpeed;
		}
	}

	if (UCharacterMovementComponent* MoveComp = Wendigo->GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = SelectedSpeed;
	}

	// Issue move request
	const EPathFollowingRequestResult::Type MoveResult = Controller.MoveToLocation(
		InstanceData.TargetLocation,
		AcceptanceRadius,
		/*bStopOnOverlap=*/ true,
		/*bUsePathfinding=*/ true,
		/*bProjectDestinationToNavigation=*/ true,
		/*bCanStrafe=*/ false,
		/*FilterClass=*/ nullptr,
		/*bAllowPartialPath=*/ true
	);

	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogSerene, Warning, TEXT("InvestigateLocation: MoveToLocation failed for %s"),
			*InstanceData.TargetLocation.ToString());
		return EStateTreeRunStatus::Failed;
	}

	if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		// Already at the stimulus location -- skip directly to look-around
		InstanceData.bMoveRequestActive = false;
		return EStateTreeRunStatus::Running;
	}

	InstanceData.bMoveRequestActive = true;
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_InvestigateLocation::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController& Controller = Context.GetExternalData(ControllerHandle);

	// Navigation phase: wait until arrival
	if (InstanceData.bMoveRequestActive)
	{
		const EPathFollowingStatus::Type MoveStatus = Controller.GetMoveStatus();

		if (MoveStatus == EPathFollowingStatus::Moving)
		{
			return EStateTreeRunStatus::Running;
		}

		// Arrived or movement ended -- transition to look-around phase
		InstanceData.bMoveRequestActive = false;
	}

	// Look-around phase: stand at location and scan
	InstanceData.TimeAtLocation += DeltaTime;

	if (InstanceData.TimeAtLocation >= LookAroundDuration)
	{
		// Investigation complete -- clear stimulus so patrol can resume
		APawn* Pawn = Controller.GetPawn();
		AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Pawn);
		if (Wendigo)
		{
			if (USuspicionComponent* Suspicion = Wendigo->GetSuspicionComponent())
			{
				Suspicion->ClearStimulusLocation();
			}
		}
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FSTT_InvestigateLocation::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController& Controller = Context.GetExternalData(ControllerHandle);

	// Restore patrol walk speed and behavior state
	APawn* Pawn = Controller.GetPawn();
	AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Pawn);
	if (Wendigo)
	{
		if (UCharacterMovementComponent* MoveComp = Wendigo->GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = AIConstants::WendigoWalkSpeed;
		}
		Wendigo->SetBehaviorState(EWendigoBehaviorState::Patrol);
	}

	// Stop any active movement
	if (InstanceData.bMoveRequestActive)
	{
		Controller.StopMovement();
		InstanceData.bMoveRequestActive = false;
	}

	// Clear focal point
	Controller.ClearFocus(EAIFocusPriority::Gameplay);
}
