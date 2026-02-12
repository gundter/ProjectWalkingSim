// Copyright Null Lantern.

#include "AI/Tasks/STT_ReturnToNearestWaypoint.h"
#include "AIController.h"
#include "AI/WendigoCharacter.h"
#include "AI/PatrolRouteActor.h"
#include "AI/MonsterAITypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Core/SereneLogChannels.h"

bool FSTT_ReturnToNearestWaypoint::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(ControllerHandle);
	return true;
}

EStateTreeRunStatus FSTT_ReturnToNearestWaypoint::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController& Controller = Context.GetExternalData(ControllerHandle);

	APawn* Pawn = Controller.GetPawn();
	AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Pawn);
	if (!Wendigo)
	{
		UE_LOG(LogSerene, Warning, TEXT("ReturnToNearestWaypoint: No Wendigo pawn"));
		return EStateTreeRunStatus::Failed;
	}

	APatrolRouteActor* PatrolRoute = Wendigo->GetPatrolRoute();
	if (!PatrolRoute || PatrolRoute->GetNumWaypoints() == 0)
	{
		UE_LOG(LogSerene, Warning, TEXT("ReturnToNearestWaypoint: No patrol route or empty waypoints"));
		return EStateTreeRunStatus::Failed;
	}

	// Set patrol walk speed
	if (UCharacterMovementComponent* MoveComp = Wendigo->GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = AIConstants::WendigoWalkSpeed;
	}

	// Find nearest waypoint by DistSquared (avoids sqrt)
	const FVector PawnLocation = Pawn->GetActorLocation();
	int32 NearestIndex = 0;
	float NearestDistSq = MAX_FLT;

	const int32 NumWaypoints = PatrolRoute->GetNumWaypoints();
	for (int32 i = 0; i < NumWaypoints; ++i)
	{
		const FVector WaypointLocation = PatrolRoute->GetWaypoint(i);
		const float DistSq = FVector::DistSquared(PawnLocation, WaypointLocation);

		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			NearestIndex = i;
		}
	}

	// Set waypoint index on character so patrol resumes from here
	Wendigo->CurrentWaypointIndex = NearestIndex;

	UE_LOG(LogSerene, Verbose, TEXT("ReturnToNearestWaypoint: Nearest waypoint %d at distance %.0f cm"),
		NearestIndex, FMath::Sqrt(NearestDistSq));

	// Navigate to nearest waypoint
	const FVector TargetLocation = PatrolRoute->GetWaypoint(NearestIndex);
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
		UE_LOG(LogSerene, Warning, TEXT("ReturnToNearestWaypoint: MoveToLocation failed for waypoint %d"),
			NearestIndex);
		return EStateTreeRunStatus::Failed;
	}

	if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		// Already at nearest waypoint -- succeed immediately
		InstanceData.bMoveRequestActive = false;
		return EStateTreeRunStatus::Succeeded;
	}

	InstanceData.bMoveRequestActive = true;
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_ReturnToNearestWaypoint::Tick(
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

	// Arrived or movement ended
	InstanceData.bMoveRequestActive = false;
	return EStateTreeRunStatus::Succeeded;
}

void FSTT_ReturnToNearestWaypoint::ExitState(
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
