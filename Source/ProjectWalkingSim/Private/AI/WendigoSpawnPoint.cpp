// Copyright Null Lantern.

#include "AI/WendigoSpawnPoint.h"
#include "AI/WendigoCharacter.h"
#include "AI/PatrolRouteActor.h"
#include "Core/SereneLogChannels.h"
#include "Components/BillboardComponent.h"

AWendigoSpawnPoint::AWendigoSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root scene component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

#if WITH_EDITORONLY_DATA
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	BillboardComponent->SetupAttachment(Root);
#endif
}

AWendigoCharacter* AWendigoSpawnPoint::SpawnWendigo()
{
	if (!WendigoClass)
	{
		UE_LOG(LogSerene, Warning, TEXT("AWendigoSpawnPoint [%s]: WendigoClass is not set. Cannot spawn."),
			*GetName());
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogSerene, Warning, TEXT("AWendigoSpawnPoint [%s]: No world. Cannot spawn."), *GetName());
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AWendigoCharacter* SpawnedWendigo = World->SpawnActor<AWendigoCharacter>(
		WendigoClass, GetActorLocation(), GetActorRotation(), SpawnParams);

	if (!SpawnedWendigo)
	{
		UE_LOG(LogSerene, Warning, TEXT("AWendigoSpawnPoint [%s]: SpawnActor failed."), *GetName());
		return nullptr;
	}

	// Assign a random patrol route from available routes
	if (AvailablePatrolRoutes.Num() > 0)
	{
		const int32 RouteIndex = FMath::RandRange(0, AvailablePatrolRoutes.Num() - 1);
		APatrolRouteActor* SelectedRoute = AvailablePatrolRoutes[RouteIndex];

		if (SelectedRoute)
		{
			SpawnedWendigo->SetPatrolRoute(SelectedRoute);

			UE_LOG(LogSerene, Log, TEXT("AWendigoSpawnPoint [%s]: Spawned Wendigo at %s with patrol route %s"),
				*GetName(),
				*GetActorLocation().ToString(),
				*SelectedRoute->GetName());
		}
		else
		{
			UE_LOG(LogSerene, Warning, TEXT("AWendigoSpawnPoint [%s]: Selected patrol route at index %d is null."),
				*GetName(), RouteIndex);
		}
	}
	else
	{
		UE_LOG(LogSerene, Log, TEXT("AWendigoSpawnPoint [%s]: Spawned Wendigo at %s with no patrol route."),
			*GetName(),
			*GetActorLocation().ToString());
	}

	return SpawnedWendigo;
}
