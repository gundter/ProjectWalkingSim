// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WendigoSpawnPoint.generated.h"

class AWendigoCharacter;
class APatrolRouteActor;
class UBillboardComponent;

/**
 * Spawn point for Wendigo characters with zone-based patrol route assignment.
 *
 * Place in the level and assign one or more AvailablePatrolRoutes.
 * SpawnWendigo() creates a Wendigo at this actor's transform and assigns
 * a randomly selected patrol route from the available routes.
 *
 * Multiple spawn points with different patrol route sets enable zone-based
 * AI population (WNDG-06).
 */
UCLASS()
class PROJECTWALKINGSIM_API AWendigoSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AWendigoSpawnPoint();

	/**
	 * Spawn a Wendigo at this spawn point's location and assign a random patrol route.
	 * @return The spawned Wendigo character, or nullptr if WendigoClass is not set.
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Spawn")
	AWendigoCharacter* SpawnWendigo();

protected:
	/** Patrol routes available to Wendigos spawned from this point. One is randomly selected per spawn. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI|Spawn")
	TArray<TObjectPtr<APatrolRouteActor>> AvailablePatrolRoutes;

	/** Class to spawn. Typically BP_WendigoCharacter. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Spawn")
	TSubclassOf<AWendigoCharacter> WendigoClass;

#if WITH_EDITORONLY_DATA
	/** Billboard sprite for level-editor placement visibility. */
	UPROPERTY(VisibleAnywhere, Category = "AI|Spawn")
	TObjectPtr<UBillboardComponent> BillboardComponent;
#endif
};
