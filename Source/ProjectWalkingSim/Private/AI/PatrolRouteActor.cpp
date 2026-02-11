// Copyright Null Lantern.

#include "AI/PatrolRouteActor.h"
#include "Components/BillboardComponent.h"
#include "DrawDebugHelpers.h"

APatrolRouteActor::APatrolRouteActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

#if WITH_EDITORONLY_DATA
	BillboardComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	if (BillboardComponent)
	{
		BillboardComponent->SetHiddenInGame(true);
		RootComponent = BillboardComponent;
	}
#endif

	// This actor has no gameplay relevance at runtime beyond data; disable
	// replication and other runtime overhead.
	SetReplicates(false);
}

FVector APatrolRouteActor::GetWaypoint(int32 Index) const
{
	if (Waypoints.Num() == 0)
	{
		return GetActorLocation();
	}

	// Clamp index to valid range
	const int32 ClampedIndex = FMath::Clamp(Index, 0, Waypoints.Num() - 1);
	return Waypoints[ClampedIndex];
}

int32 APatrolRouteActor::GetNextWaypointIndex(int32 CurrentIndex) const
{
	const int32 NumWaypoints = Waypoints.Num();
	if (NumWaypoints <= 1)
	{
		return 0;
	}

	if (bLoopRoute)
	{
		// Loop: wrap around from last to first
		return (CurrentIndex + 1) % NumWaypoints;
	}

	// Ping-pong: reverse direction at ends
	const int32 NextIndex = CurrentIndex + PingPongDirection;
	if (NextIndex >= NumWaypoints)
	{
		PingPongDirection = -1;
		return CurrentIndex - 1;
	}
	if (NextIndex < 0)
	{
		PingPongDirection = 1;
		return 1;
	}

	return NextIndex;
}

void APatrolRouteActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if WITH_EDITOR
	const UWorld* World = GetWorld();
	if (!World || !World->IsEditorWorld())
	{
		return;
	}

	const int32 NumWaypoints = Waypoints.Num();
	if (NumWaypoints == 0)
	{
		return;
	}

	const FColor RouteColor = FColor::Green;
	const FColor LoopBackColor = FColor(100, 100, 255); // Light blue for loop-back

	// Draw spheres at each waypoint and lines between consecutive waypoints
	for (int32 i = 0; i < NumWaypoints; ++i)
	{
		const FVector& WaypointPos = Waypoints[i];

		// Sphere at waypoint
		DrawDebugSphere(World, WaypointPos, 25.0f, 8, RouteColor, false, -1.0f, SDPG_World, 2.0f);

		// Line to next waypoint (not from last in non-loop mode)
		if (i < NumWaypoints - 1)
		{
			DrawDebugLine(World, WaypointPos, Waypoints[i + 1], RouteColor, false, -1.0f, SDPG_World, 2.0f);
		}
	}

	// Draw loop-back line from last to first if looping
	if (bLoopRoute && NumWaypoints > 1)
	{
		DrawDebugLine(
			World,
			Waypoints[NumWaypoints - 1],
			Waypoints[0],
			LoopBackColor,
			false, -1.0f, SDPG_World, 1.5f
		);
	}
#endif
}
