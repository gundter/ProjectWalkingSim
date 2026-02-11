// Copyright Null Lantern.

#include "Hiding/HidingSpotActor.h"
#include "Hiding/HidingSpotDataAsset.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/SereneLogChannels.h"

#include "Hiding/HidingComponent.h"

AHidingSpotActor::AHidingSpotActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// --- Scene Root ---
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// --- Spot Mesh ---
	SpotMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpotMesh"));
	SpotMesh->SetupAttachment(SceneRoot);
	SpotMesh->SetCollisionProfileName(TEXT("BlockAll"));

	// --- Hiding Camera ---
	HidingCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("HidingCamera"));
	HidingCamera->SetupAttachment(SceneRoot);
	HidingCamera->bAutoActivate = false;
}

void AHidingSpotActor::BeginPlay()
{
	Super::BeginPlay();

	// Apply peek overlay material to the hiding camera's post-process settings.
	if (SpotData && SpotData->PeekOverlayMaterial && HidingCamera)
	{
		FPostProcessSettings& PP = HidingCamera->PostProcessSettings;
		FWeightedBlendable Blendable;
		Blendable.Weight = 1.0f;
		Blendable.Object = SpotData->PeekOverlayMaterial;
		PP.WeightedBlendables.Array.Add(Blendable);
	}

	if (SpotData)
	{
		UE_LOG(LogSerene, Log, TEXT("HidingSpot [%s] initialized with SpotData [%s], TypeTag [%s]"),
			*GetName(),
			*SpotData->GetName(),
			*SpotData->SpotTypeTag.ToString());
	}
	else
	{
		UE_LOG(LogSerene, Warning, TEXT("HidingSpot [%s] has no SpotData assigned"), *GetName());
	}
}

// =============================================================================
// IInteractable
// =============================================================================

FText AHidingSpotActor::GetInteractionText_Implementation() const
{
	if (bIsOccupied)
	{
		return FText::GetEmpty();
	}

	if (SpotData)
	{
		return SpotData->InteractionText;
	}

	return NSLOCTEXT("HidingSpot", "FallbackInteraction", "Hide");
}

bool AHidingSpotActor::CanInteract_Implementation(AActor* Interactor) const
{
	if (bIsOccupied || !SpotData)
	{
		return false;
	}

	if (UHidingComponent* HidingComp = Interactor->FindComponentByClass<UHidingComponent>())
	{
		if (HidingComp->GetHidingState() != EHidingState::Free)
		{
			return false;
		}
	}

	return true;
}

void AHidingSpotActor::OnInteract_Implementation(AActor* Interactor)
{
	if (bIsOccupied)
	{
		return;
	}

	if (!Interactor)
	{
		return;
	}

	UHidingComponent* HidingComp = Interactor->FindComponentByClass<UHidingComponent>();
	if (!HidingComp)
	{
		UE_LOG(LogSerene, Warning, TEXT("HidingSpot [%s]: Interactor [%s] has no HidingComponent"),
			*GetName(), *Interactor->GetName());
		return;
	}
	HidingComp->EnterHidingSpot(this);
}

void AHidingSpotActor::OnFocusBegin_Implementation(AActor* Interactor)
{
	// No highlight effect for hiding spots -- they are environmental furniture.
}

void AHidingSpotActor::OnFocusEnd_Implementation(AActor* Interactor)
{
	// No highlight effect for hiding spots -- they are environmental furniture.
}

// =============================================================================
// IHideable
// =============================================================================

bool AHidingSpotActor::CanHide_Implementation(AActor* HidingActor) const
{
	return !bIsOccupied;
}

void AHidingSpotActor::OnEnterHiding_Implementation(AActor* HidingActor)
{
	bIsOccupied = true;
	OccupantActor = HidingActor;

	UE_LOG(LogSerene, Log, TEXT("HidingSpot [%s] occupied by [%s]"),
		*GetName(), *HidingActor->GetName());
}

void AHidingSpotActor::OnExitHiding_Implementation(AActor* HidingActor)
{
	bIsOccupied = false;
	OccupantActor = nullptr;

	UE_LOG(LogSerene, Log, TEXT("HidingSpot [%s] vacated"), *GetName());
}

UCameraComponent* AHidingSpotActor::GetHidingCamera_Implementation() const
{
	return HidingCamera;
}

UHidingSpotDataAsset* AHidingSpotActor::GetSpotData_Implementation() const
{
	return SpotData;
}

bool AHidingSpotActor::IsOccupied_Implementation() const
{
	return bIsOccupied;
}

void AHidingSpotActor::MarkDiscovered_Implementation()
{
	bDiscoveredByMonster = true;

	UE_LOG(LogSerene, Log, TEXT("HidingSpot [%s] marked as discovered by monster"), *GetName());
}

bool AHidingSpotActor::WasDiscovered_Implementation() const
{
	return bDiscoveredByMonster;
}
