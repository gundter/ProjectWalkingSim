// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolRouteActor.generated.h"

class UBillboardComponent;

/**
 * Container for patrol waypoints placed in the level.
 * Each Wendigo instance references one PatrolRouteActor to define its patrol path.
 *
 * Waypoints are world-space positions editable in the Details panel.
 * The MakeEditWidget meta on the Waypoints array enables visual dragging
 * of waypoint handles directly in the viewport.
 *
 * Editor-only debug visualization draws spheres at each waypoint and
 * connecting lines between them (green for sequential, blue dashed for
 * the loop-back connection).
 */
UCLASS()
class PROJECTWALKINGSIM_API APatrolRouteActor : public AActor
{
	GENERATED_BODY()

public:
	APatrolRouteActor();

	// --- Properties ---

	/**
	 * Ordered list of patrol waypoints in world space.
	 * MakeEditWidget enables viewport drag handles for each waypoint.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol", meta = (MakeEditWidget))
	TArray<FVector> Waypoints;

	/**
	 * When true, after reaching the last waypoint, return to the first (loop).
	 * When false, reverse direction at each end (ping-pong).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol")
	bool bLoopRoute = true;

	// --- Public API ---

	/** Get the waypoint at the given index, clamped to valid range. */
	FVector GetWaypoint(int32 Index) const;

	/** Get the total number of waypoints. */
	int32 GetNumWaypoints() const { return Waypoints.Num(); }

	/**
	 * Get the next waypoint index after CurrentIndex, handling loop/ping-pong.
	 * For loop mode: wraps around to 0 after the last waypoint.
	 * For ping-pong mode: reverses direction at each end.
	 */
	int32 GetNextWaypointIndex(int32 CurrentIndex) const;

#if WITH_EDITOR
	virtual void Tick(float DeltaTime) override;
#endif

protected:
#if WITH_EDITORONLY_DATA
	/** Billboard sprite for level-editor placement visibility. */
	UPROPERTY(VisibleAnywhere, Category = "Patrol")
	TObjectPtr<UBillboardComponent> BillboardComponent;
#endif

private:
	/**
	 * Ping-pong direction tracker.
	 * +1 = forward through waypoints, -1 = backward.
	 * Mutable because GetNextWaypointIndex is logically a query
	 * but must track direction state for ping-pong mode.
	 */
	mutable int32 PingPongDirection = 1;
};
