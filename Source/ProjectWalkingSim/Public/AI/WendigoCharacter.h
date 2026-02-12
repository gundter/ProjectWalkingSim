// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AI/MonsterAITypes.h"
#include "WendigoCharacter.generated.h"

class USuspicionComponent;
class AWendigoAIController;
class APatrolRouteActor;
class AHidingSpotActor;

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

	/** Current waypoint index -- stored on character so it persists across State Tree state re-entries. */
	UPROPERTY(BlueprintReadOnly, Category = "AI|Patrol")
	int32 CurrentWaypointIndex = 0;

	// --- Chase/Search persistent state ---

	/** Last known world location of the player during chase. */
	UPROPERTY(BlueprintReadOnly, Category = "AI|Chase")
	FVector LastKnownPlayerLocation = FVector::ZeroVector;

	/** Whether a last-known player location has been recorded. */
	UPROPERTY(BlueprintReadOnly, Category = "AI|Chase")
	bool bHasLastKnownPlayerLocation = false;

	/**
	 * Hiding spot the Wendigo witnessed the player enter.
	 * AActor* to avoid circular header dependency with Hiding module -- cast at use-site.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AI|Chase")
	TWeakObjectPtr<AActor> WitnessedHidingSpot;

	// --- Behavior state ---

	/** Current behavior state of this Wendigo. */
	UPROPERTY(BlueprintReadOnly, Category = "AI|Behavior")
	EWendigoBehaviorState BehaviorState = EWendigoBehaviorState::Patrol;

	/** Broadcast when the Wendigo's behavior state changes. */
	UPROPERTY(BlueprintAssignable, Category = "AI|Behavior")
	FOnBehaviorStateChanged OnBehaviorStateChanged;

	/** Set the behavior state; broadcasts delegate only if state actually changed. */
	UFUNCTION(BlueprintCallable, Category = "AI|Behavior")
	void SetBehaviorState(EWendigoBehaviorState NewState);

	/** Record the player's last known location for search/investigation. */
	UFUNCTION(BlueprintCallable, Category = "AI|Chase")
	void SetLastKnownPlayerLocation(const FVector& Location);

	/** Clear the last known player location. */
	UFUNCTION(BlueprintCallable, Category = "AI|Chase")
	void ClearLastKnownPlayerLocation();

	/** Record a hiding spot the Wendigo saw the player enter. */
	UFUNCTION(BlueprintCallable, Category = "AI|Chase")
	void SetWitnessedHidingSpot(AActor* Spot);

	/** Clear the witnessed hiding spot reference. */
	UFUNCTION(BlueprintCallable, Category = "AI|Chase")
	void ClearWitnessedHidingSpot();

	/** Assign a patrol route at runtime (e.g., from WendigoSpawnPoint). Resets waypoint index to 0. */
	void SetPatrolRoute(APatrolRouteActor* Route) { PatrolRoute = Route; CurrentWaypointIndex = 0; }

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
