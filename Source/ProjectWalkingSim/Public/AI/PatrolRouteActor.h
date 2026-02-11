// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolRouteActor.generated.h"

/**
 * Container for patrol waypoints placed in the level.
 * Each Wendigo instance references one PatrolRouteActor to define its patrol path.
 *
 * Stub: waypoints and editor visualization will be implemented in a later plan.
 */
UCLASS()
class PROJECTWALKINGSIM_API APatrolRouteActor : public AActor
{
	GENERATED_BODY()

public:
	APatrolRouteActor();

	/** Ordered list of patrol waypoints in world space. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol")
	TArray<FVector> Waypoints;

	/** Get the waypoint at the given index, wrapping around if necessary. */
	FVector GetWaypoint(int32 Index) const;

	/** Get the total number of waypoints. */
	int32 GetNumWaypoints() const { return Waypoints.Num(); }
};
