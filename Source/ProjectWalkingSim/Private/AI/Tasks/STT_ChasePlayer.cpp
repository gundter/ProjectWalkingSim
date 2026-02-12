// Copyright Null Lantern.

#include "AI/Tasks/STT_ChasePlayer.h"
#include "AIController.h"
#include "AI/WendigoCharacter.h"
#include "AI/SuspicionComponent.h"
#include "AI/MonsterAITypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Core/SereneLogChannels.h"

bool FSTT_ChasePlayer::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(ControllerHandle);
	return true;
}

EStateTreeRunStatus FSTT_ChasePlayer::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController& Controller = Context.GetExternalData(ControllerHandle);

	APawn* Pawn = Controller.GetPawn();
	AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Pawn);
	if (!Wendigo)
	{
		UE_LOG(LogSerene, Warning, TEXT("ChasePlayer: No Wendigo pawn"));
		return EStateTreeRunStatus::Failed;
	}

	// Find the player pawn as chase target
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(Wendigo->GetWorld(), 0);
	if (!PlayerPawn)
	{
		UE_LOG(LogSerene, Warning, TEXT("ChasePlayer: No player pawn found"));
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.ChaseTarget = PlayerPawn;

	// Set behavior state to Chasing
	Wendigo->SetBehaviorState(EWendigoBehaviorState::Chasing);

	// Set chase speed
	if (UCharacterMovementComponent* MoveComp = Wendigo->GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = ChaseSpeed;
	}

	// Set focus on chase target for head tracking
	Controller.SetFocus(PlayerPawn, EAIFocusPriority::Gameplay);

	// Issue MoveToActor -- auto-updates destination as player moves
	const EPathFollowingRequestResult::Type MoveResult = Controller.MoveToActor(
		PlayerPawn,
		AcceptanceRadius,
		/*bStopOnOverlap=*/ true,
		/*bUsePathfinding=*/ true,
		/*bCanStrafe=*/ true,
		/*FilterClass=*/ nullptr,
		/*bAllowPartialPath=*/ true
	);

	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogSerene, Warning, TEXT("ChasePlayer: MoveToActor failed, storing last-known location"));
		// Fall back: store current player location so Search can use it
		Wendigo->SetLastKnownPlayerLocation(PlayerPawn->GetActorLocation());
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.bMoveRequestActive = (MoveResult == EPathFollowingRequestResult::RequestSuccessful);
	InstanceData.LOSLostTimer = 0.0f;

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_ChasePlayer::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController& Controller = Context.GetExternalData(ControllerHandle);

	// Validate chase target still exists
	AActor* Target = InstanceData.ChaseTarget.Get();
	if (!Target)
	{
		UE_LOG(LogSerene, Verbose, TEXT("ChasePlayer: Chase target lost (expired weak ptr)"));
		return EStateTreeRunStatus::Failed;
	}

	APawn* Pawn = Controller.GetPawn();
	AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Pawn);
	if (!Wendigo)
	{
		return EStateTreeRunStatus::Failed;
	}

	// Check line-of-sight
	const bool bCanSeePlayer = Controller.LineOfSightTo(Target);

	if (bCanSeePlayer)
	{
		// Reset LOS timer and update last-known position
		InstanceData.LOSLostTimer = 0.0f;
		Wendigo->SetLastKnownPlayerLocation(Target->GetActorLocation());
	}
	else
	{
		// Increment LOS lost timer
		InstanceData.LOSLostTimer += DeltaTime;

		if (InstanceData.LOSLostTimer >= LOSLostTimeout)
		{
			UE_LOG(LogSerene, Verbose, TEXT("ChasePlayer: LOS lost for %.1fs, transitioning to Search"),
				InstanceData.LOSLostTimer);
			return EStateTreeRunStatus::Failed;
		}
	}

	// Check grab range (only if we can see the player -- no blind grabs)
	const float DistToTarget = FVector::Dist(Pawn->GetActorLocation(), Target->GetActorLocation());
	if (DistToTarget <= GrabRange && bCanSeePlayer)
	{
		UE_LOG(LogSerene, Verbose, TEXT("ChasePlayer: Within grab range (%.0f cm), initiating grab"),
			DistToTarget);
		return EStateTreeRunStatus::Succeeded;
	}

	// Re-issue MoveToActor if path following went idle mid-chase
	// (handles player temporarily leaving NavMesh or path invalidation)
	const EPathFollowingStatus::Type MoveStatus = Controller.GetMoveStatus();
	if (MoveStatus != EPathFollowingStatus::Moving && InstanceData.bMoveRequestActive)
	{
		const EPathFollowingRequestResult::Type MoveResult = Controller.MoveToActor(
			Target,
			AcceptanceRadius,
			/*bStopOnOverlap=*/ true,
			/*bUsePathfinding=*/ true,
			/*bCanStrafe=*/ true,
			/*FilterClass=*/ nullptr,
			/*bAllowPartialPath=*/ true
		);

		InstanceData.bMoveRequestActive = (MoveResult == EPathFollowingRequestResult::RequestSuccessful);
	}

	return EStateTreeRunStatus::Running;
}

void FSTT_ChasePlayer::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController& Controller = Context.GetExternalData(ControllerHandle);

	// Always restore baseline walk speed (not a variable -- the constant)
	APawn* Pawn = Controller.GetPawn();
	AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Pawn);
	if (Wendigo)
	{
		if (UCharacterMovementComponent* MoveComp = Wendigo->GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = AIConstants::WendigoWalkSpeed;
		}
	}

	// Stop any active movement
	if (InstanceData.bMoveRequestActive)
	{
		Controller.StopMovement();
		InstanceData.bMoveRequestActive = false;
	}

	// Clear focus
	Controller.ClearFocus(EAIFocusPriority::Gameplay);
}
