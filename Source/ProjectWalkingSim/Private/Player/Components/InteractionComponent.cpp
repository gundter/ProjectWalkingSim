// Copyright Null Lantern.

#include "Player/Components/InteractionComponent.h"

#include "Interaction/InteractableInterface.h"
#include "Core/SereneLogChannels.h"
#include "GameFramework/PlayerController.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	PerformTrace();
}

void UInteractionComponent::PerformTrace()
{
	// Get the player controller from the owning pawn.
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC)
	{
		return;
	}

	// IMPORTANT: Trace from camera (GetPlayerViewPoint), NOT from GetActorLocation (Pitfall 5).
	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * InteractionRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, ViewLocation, TraceEnd, TraceChannel, Params);

	AActor* HitActor = bHit ? Hit.GetActor() : nullptr;

	// Check if hit actor implements IInteractable and can currently be interacted with.
	if (HitActor && HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		if (IInteractable::Execute_CanInteract(HitActor, GetOwner()))
		{
			SetCurrentInteractable(HitActor);
			return;
		}
	}

	SetCurrentInteractable(nullptr);
}

void UInteractionComponent::SetCurrentInteractable(AActor* NewInteractable)
{
	if (NewInteractable == CurrentInteractable.Get())
	{
		return; // No change
	}

	// End focus on old target
	if (CurrentInteractable.IsValid())
	{
		IInteractable::Execute_OnFocusEnd(CurrentInteractable.Get(), GetOwner());
	}

	// Update tracked interactable
	CurrentInteractable = NewInteractable;

	// Begin focus on new target
	if (NewInteractable)
	{
		IInteractable::Execute_OnFocusBegin(NewInteractable, GetOwner());
	}

	// Get interaction text for the delegate broadcast
	FText InteractionText;
	if (NewInteractable)
	{
		InteractionText = IInteractable::Execute_GetInteractionText(NewInteractable);
	}

	// Broadcast to listeners (HUD widget, etc.)
	OnInteractableChanged.Broadcast(NewInteractable, InteractionText);
}

void UInteractionComponent::TryInteract()
{
	if (!CurrentInteractable.IsValid())
	{
		return;
	}

	AActor* Target = CurrentInteractable.Get();

	if (!IInteractable::Execute_CanInteract(Target, GetOwner()))
	{
		return;
	}

	IInteractable::Execute_OnInteract(Target, GetOwner());

	UE_LOG(LogSerene, Log, TEXT("UInteractionComponent::TryInteract - Interacted with: %s"),
		*Target->GetName());

	// After interaction, the interactable's text may have changed (e.g., "Open" -> "Close").
	// Re-broadcast the updated text so the HUD reflects the change.
	const FText UpdatedText = IInteractable::Execute_GetInteractionText(Target);
	OnInteractableChanged.Broadcast(Target, UpdatedText);
}

AActor* UInteractionComponent::GetCurrentInteractable() const
{
	return CurrentInteractable.Get();
}
