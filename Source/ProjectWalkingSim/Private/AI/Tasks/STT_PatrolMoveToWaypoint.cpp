// Copyright Null Lantern.

#include "AI/Tasks/STT_PatrolMoveToWaypoint.h"
#include "AIController.h"
#include "AI/WendigoCharacter.h"
#include "AI/PatrolRouteActor.h"
#include "Navigation/PathFollowingComponent.h"
#include "Core/SereneLogChannels.h"

bool FSTT_PatrolMoveToWaypoint::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(ControllerHandle);
	return true;
}

EStateTreeRunStatus FSTT_PatrolMoveToWaypoint::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController& Controller = Context.GetExternalData(ControllerHandle);

	APawn* Pawn = Controller.GetPawn();
	AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Pawn);
	if (!Wendigo)
	{
		UE_LOG(LogSerene, Warning, TEXT("PatrolMoveToWaypoint: No Wendigo pawn"));
		return EStateTreeRunStatus::Failed;
	}

	APatrolRouteActor* PatrolRoute = Wendigo->GetPatrolRoute();
	if (!PatrolRoute || PatrolRoute->GetNumWaypoints() == 0)
	{
		UE_LOG(LogSerene, Warning, TEXT("PatrolMoveToWaypoint: No patrol route or empty waypoints"));
		return EStateTreeRunStatus::Failed;
	}

	const FVector TargetLocation = PatrolRoute->GetWaypoint(InstanceData.CurrentWaypointIndex);

	// Issue move request. Do NOT set bLockAILogic (prevents State Tree transitions).
	const EPathFollowingRequestResult::Type MoveResult = Controller.MoveToLocation(
		TargetLocation,
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
		UE_LOG(LogSerene, Warning, TEXT("PatrolMoveToWaypoint: MoveToLocation failed for waypoint %d at %s"),
			InstanceData.CurrentWaypointIndex, *TargetLocation.ToString());
		return EStateTreeRunStatus::Failed;
	}

	if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		// Already at waypoint -- advance index and succeed immediately
		InstanceData.CurrentWaypointIndex = PatrolRoute->GetNextWaypointIndex(InstanceData.CurrentWaypointIndex);
		InstanceData.bMoveRequestActive = false;
		return EStateTreeRunStatus::Succeeded;
	}

	InstanceData.bMoveRequestActive = true;
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_PatrolMoveToWaypoint::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController& Controller = Context.GetExternalData(ControllerHandle);

	const EPathFollowingStatus::Type MoveStatus = Controller.GetMoveStatus();

	if (MoveStatus == EPathFollowingStatus::Moving)
	{
		return EStateTreeRunStatus::Running;
	}

	// Idle or reached destination -- advance waypoint index
	APawn* Pawn = Controller.GetPawn();
	AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Pawn);
	if (Wendigo)
	{
		APatrolRouteActor* PatrolRoute = Wendigo->GetPatrolRoute();
		if (PatrolRoute)
		{
			InstanceData.CurrentWaypointIndex = PatrolRoute->GetNextWaypointIndex(InstanceData.CurrentWaypointIndex);
		}
	}

	InstanceData.bMoveRequestActive = false;
	return EStateTreeRunStatus::Succeeded;
}

void FSTT_PatrolMoveToWaypoint::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (InstanceData.bMoveRequestActive)
	{
		AAIController& Controller = Context.GetExternalData(ControllerHandle);
		Controller.StopMovement();
		InstanceData.bMoveRequestActive = false;
	}
}
