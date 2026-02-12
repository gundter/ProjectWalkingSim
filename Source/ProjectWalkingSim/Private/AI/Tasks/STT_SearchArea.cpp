// Copyright Null Lantern.

#include "AI/Tasks/STT_SearchArea.h"
#include "AIController.h"
#include "AI/WendigoCharacter.h"
#include "AI/MonsterAITypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Core/SereneLogChannels.h"

bool FSTT_SearchArea::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(ControllerHandle);
	return true;
}

EStateTreeRunStatus FSTT_SearchArea::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController& Controller = Context.GetExternalData(ControllerHandle);

	APawn* Pawn = Controller.GetPawn();
	AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Pawn);
	if (!Wendigo)
	{
		UE_LOG(LogSerene, Warning, TEXT("SearchArea: No Wendigo pawn"));
		return EStateTreeRunStatus::Failed;
	}

	// Set behavior state to Searching
	Wendigo->SetBehaviorState(EWendigoBehaviorState::Searching);

	// Set search speed
	if (UCharacterMovementComponent* MoveComp = Wendigo->GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = SearchSpeed;
	}

	// Build search points array
	InstanceData.SearchPoints.Reset();

	// First point: last-known player location (or current position as fallback)
	FVector SearchOrigin;
	if (Wendigo->bHasLastKnownPlayerLocation)
	{
		SearchOrigin = Wendigo->LastKnownPlayerLocation;
	}
	else
	{
		SearchOrigin = Pawn->GetActorLocation();
		UE_LOG(LogSerene, Verbose, TEXT("SearchArea: No last-known location, using pawn position as fallback"));
	}
	InstanceData.SearchPoints.Add(SearchOrigin);

	// Generate random NavMesh points around the search origin
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Wendigo->GetWorld());
	if (NavSys)
	{
		for (int32 i = 0; i < NumRandomPoints; ++i)
		{
			FNavLocation RandomPoint;
			if (NavSys->GetRandomReachablePointInRadius(SearchOrigin, SearchRadius, RandomPoint))
			{
				InstanceData.SearchPoints.Add(RandomPoint.Location);
			}
			// If a random point fails, skip it (don't pad with duplicates)
		}
	}

	if (InstanceData.SearchPoints.Num() == 0)
	{
		UE_LOG(LogSerene, Warning, TEXT("SearchArea: No search points generated"));
		return EStateTreeRunStatus::Failed;
	}

	// Initialize counters
	InstanceData.CurrentSearchIndex = 0;
	InstanceData.TotalSearchTime = 0.0f;
	InstanceData.TimeAtCurrentPoint = 0.0f;
	InstanceData.TimeSinceLastGlance = 0.0f;

	// Navigate to first search point
	const EPathFollowingRequestResult::Type MoveResult = Controller.MoveToLocation(
		InstanceData.SearchPoints[0],
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
		// Skip to linger phase at current point
		UE_LOG(LogSerene, Verbose, TEXT("SearchArea: MoveToLocation failed for first point, lingering at current position"));
		InstanceData.bMoveRequestActive = false;
	}
	else if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		// Already at first search point -- go straight to linger
		InstanceData.bMoveRequestActive = false;
	}
	else
	{
		InstanceData.bMoveRequestActive = true;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_SearchArea::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController& Controller = Context.GetExternalData(ControllerHandle);

	// Increment total search timer
	InstanceData.TotalSearchTime += DeltaTime;
	if (InstanceData.TotalSearchTime >= MaxSearchDuration)
	{
		UE_LOG(LogSerene, Verbose, TEXT("SearchArea: Max search duration reached (%.0fs), completing"),
			InstanceData.TotalSearchTime);
		return EStateTreeRunStatus::Succeeded;
	}

	// Navigation phase: wait until arrival
	if (InstanceData.bMoveRequestActive)
	{
		const EPathFollowingStatus::Type MoveStatus = Controller.GetMoveStatus();

		if (MoveStatus == EPathFollowingStatus::Moving)
		{
			return EStateTreeRunStatus::Running;
		}

		// Arrived or movement ended -- transition to linger phase
		InstanceData.bMoveRequestActive = false;
		InstanceData.TimeAtCurrentPoint = 0.0f;
		InstanceData.TimeSinceLastGlance = 0.0f;
	}

	// Linger phase: stand at search point and look around
	InstanceData.TimeAtCurrentPoint += DeltaTime;
	InstanceData.TimeSinceLastGlance += DeltaTime;

	// Periodic look-around: change glance direction every ~2 seconds
	if (InstanceData.TimeSinceLastGlance >= 2.0f)
	{
		APawn* Pawn = Controller.GetPawn();
		if (Pawn)
		{
			const float YawOffset = FMath::RandRange(-90.0f, 90.0f);
			const FRotator CurrentRot = Pawn->GetActorRotation();
			const FRotator GlanceRot(CurrentRot.Pitch, CurrentRot.Yaw + YawOffset, 0.0f);
			const FVector GlanceDir = GlanceRot.Vector();
			const FVector FocalPoint = Pawn->GetActorLocation() + GlanceDir * 500.0f;
			Controller.SetFocalPoint(FocalPoint, EAIFocusPriority::Gameplay);
		}
		InstanceData.TimeSinceLastGlance = 0.0f;
	}

	// When linger is complete, advance to next search point
	if (InstanceData.TimeAtCurrentPoint >= LingerDuration)
	{
		// Clear focus from look-around
		Controller.ClearFocus(EAIFocusPriority::Gameplay);

		InstanceData.CurrentSearchIndex++;

		if (InstanceData.CurrentSearchIndex >= InstanceData.SearchPoints.Num())
		{
			// All search points visited
			UE_LOG(LogSerene, Verbose, TEXT("SearchArea: All %d search points visited, completing"),
				InstanceData.SearchPoints.Num());
			return EStateTreeRunStatus::Succeeded;
		}

		// Navigate to next search point
		const FVector& NextPoint = InstanceData.SearchPoints[InstanceData.CurrentSearchIndex];
		const EPathFollowingRequestResult::Type MoveResult = Controller.MoveToLocation(
			NextPoint,
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
			// Skip this point -- advance to linger at current position
			UE_LOG(LogSerene, Verbose, TEXT("SearchArea: MoveToLocation failed for point %d, skipping"),
				InstanceData.CurrentSearchIndex);
			InstanceData.TimeAtCurrentPoint = 0.0f;
			InstanceData.TimeSinceLastGlance = 0.0f;
			InstanceData.bMoveRequestActive = false;
		}
		else if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			// Already at this search point -- go to linger
			InstanceData.TimeAtCurrentPoint = 0.0f;
			InstanceData.TimeSinceLastGlance = 0.0f;
			InstanceData.bMoveRequestActive = false;
		}
		else
		{
			InstanceData.bMoveRequestActive = true;
			InstanceData.TimeAtCurrentPoint = 0.0f;
			InstanceData.TimeSinceLastGlance = 0.0f;
		}
	}

	return EStateTreeRunStatus::Running;
}

void FSTT_SearchArea::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController& Controller = Context.GetExternalData(ControllerHandle);

	// Restore patrol walk speed
	APawn* Pawn = Controller.GetPawn();
	AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Pawn);
	if (Wendigo)
	{
		if (UCharacterMovementComponent* MoveComp = Wendigo->GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = AIConstants::WendigoWalkSpeed;
		}

		// Search complete: wipe last-known location and witnessed hiding spot
		Wendigo->ClearLastKnownPlayerLocation();
		Wendigo->ClearWitnessedHidingSpot();
	}

	// Stop any active movement
	if (InstanceData.bMoveRequestActive)
	{
		Controller.StopMovement();
		InstanceData.bMoveRequestActive = false;
	}

	// Clear focal point from look-around
	Controller.ClearFocus(EAIFocusPriority::Gameplay);
}
