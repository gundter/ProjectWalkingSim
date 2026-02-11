// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WendigoCharacter.generated.h"

class USuspicionComponent;
class AWendigoAIController;
class APatrolRouteActor;

/**
 * The Wendigo monster character.
 * A tall (~260cm / ~8.5ft), slow-moving AI pawn that patrols the environment.
 * Carries a SuspicionComponent for gradual player detection and a PatrolRoute
 * reference for waypoint-based patrol behavior.
 *
 * No skeletal mesh is assigned in C++ -- a Blueprint subclass will assign
 * the appropriate mesh and animations in a later plan.
 */
UCLASS()
class PROJECTWALKINGSIM_API AWendigoCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AWendigoCharacter();

	/** Get the suspicion tracking component. */
	UFUNCTION(BlueprintCallable, Category = "AI")
	USuspicionComponent* GetSuspicionComponent() const { return SuspicionComponent; }

	/** Get the patrol route assigned to this Wendigo instance. */
	UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
	APatrolRouteActor* GetPatrolRoute() const { return PatrolRoute; }

	/** Current waypoint index — stored on character so it persists across State Tree state re-entries. */
	UPROPERTY(BlueprintReadOnly, Category = "AI|Patrol")
	int32 CurrentWaypointIndex = 0;

protected:
	/** Suspicion component -- tracks detection state and alert levels. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<USuspicionComponent> SuspicionComponent;

	/**
	 * Patrol route for this Wendigo instance.
	 * Set per-instance in the level editor to link a Wendigo to its waypoint route.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI|Patrol")
	TObjectPtr<APatrolRouteActor> PatrolRoute;
};
