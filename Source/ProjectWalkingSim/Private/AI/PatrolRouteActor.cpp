// Copyright Null Lantern.

#include "AI/PatrolRouteActor.h"

APatrolRouteActor::APatrolRouteActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

FVector APatrolRouteActor::GetWaypoint(int32 Index) const
{
	if (Waypoints.Num() == 0)
	{
		return GetActorLocation();
	}

	// Wrap index to valid range
	const int32 WrappedIndex = ((Index % Waypoints.Num()) + Waypoints.Num()) % Waypoints.Num();
	return Waypoints[WrappedIndex];
}
