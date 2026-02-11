// Copyright Null Lantern.

#include "Hiding/HidingComponent.h"
#include "Hiding/HidingSpotActor.h"
#include "Hiding/HidingSpotDataAsset.h"
#include "Hiding/HidingTypes.h"
#include "Interaction/HideableInterface.h"
#include "Visibility/VisibilityScoreComponent.h"
#include "Player/Components/InteractionComponent.h"
#include "Player/Components/HeadBobComponent.h"
#include "Player/Components/LeanComponent.h"
#include "Player/Components/FootstepComponent.h"
#include "Core/SereneLogChannels.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

UHidingComponent::UHidingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

void UHidingComponent::EnterHidingSpot(AHidingSpotActor* Spot)
{
	if (HidingState != EHidingState::Free)
	{
		return;
	}

	if (!Spot)
	{
		return;
	}

	// Verify the spot allows hiding
	if (!IHideable::Execute_CanHide(Spot, GetOwner()))
	{
		return;
	}

	// Begin the Entering transition
	HidingState = EHidingState::Entering;
	CurrentHidingSpot = Spot;

	// Immediately disable player movement and peripheral systems
	DisablePlayerSystems();

	// Get the spot's data asset for montage and camera config
	UHidingSpotDataAsset* Data = IHideable::Execute_GetSpotData(Spot);

	// Play entry montage if available
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Data && Data->EntryMontage && Character && Character->GetMesh())
	{
		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			float Duration = AnimInstance->Montage_Play(Data->EntryMontage);
			if (Duration > 0.f)
			{
				// CRITICAL: Bind end delegate AFTER Montage_Play (UE pitfall #2)
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &UHidingComponent::OnEntryMontageEnded);
				AnimInstance->Montage_SetEndDelegate(EndDelegate, Data->EntryMontage);
			}
			else
			{
				// Montage failed to play -- skip directly to Hidden
				TransitionToHiddenState();
			}
		}
		else
		{
			TransitionToHiddenState();
		}
	}
	else
	{
		// No montage configured -- skip directly to Hidden state
		TransitionToHiddenState();
	}

	// Blend camera to the hiding spot's view target
	if (Character)
	{
		APlayerController* PC = Cast<APlayerController>(Character->GetController());
		if (PC && Data)
		{
			PC->SetViewTargetWithBlend(
				Spot,
				Data->CameraBlendInTime,
				VTBlend_Cubic,
				0.0f,
				false
			);
		}
	}

	// Notify the hiding spot that an actor is entering
	IHideable::Execute_OnEnterHiding(Spot, GetOwner());

	// Broadcast state change
	OnHidingStateChanged.Broadcast(EHidingState::Entering);

	UE_LOG(LogSerene, Log, TEXT("Entering hiding spot [%s]"), *Spot->GetName());
}

void UHidingComponent::ExitHidingSpot()
{
	if (HidingState != EHidingState::Hidden)
	{
		return;
	}

	HidingState = EHidingState::Exiting;

	// Restore look constraints and input before playing exit montage
	RestoreLookConstraints();
	RestoreDefaultInputContext();
	SetPlayerMeshVisibility(true);

	// Clear visibility reduction
	if (UVisibilityScoreComponent* VisComp = GetOwner()->FindComponentByClass<UVisibilityScoreComponent>())
	{
		VisComp->SetHidingReduction(0.0f);
	}

	// Get the spot's data asset for exit montage and camera config
	UHidingSpotDataAsset* Data = nullptr;
	if (CurrentHidingSpot.IsValid())
	{
		Data = IHideable::Execute_GetSpotData(CurrentHidingSpot.Get());
	}

	// Play exit montage if available
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Data && Data->ExitMontage && Character && Character->GetMesh())
	{
		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			float Duration = AnimInstance->Montage_Play(Data->ExitMontage);
			if (Duration > 0.f)
			{
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &UHidingComponent::OnExitMontageEnded);
				AnimInstance->Montage_SetEndDelegate(EndDelegate, Data->ExitMontage);
			}
			else
			{
				TransitionToFreeState();
			}
		}
		else
		{
			TransitionToFreeState();
		}
	}
	else
	{
		TransitionToFreeState();
	}

	// Blend camera back to player character
	if (Character)
	{
		APlayerController* PC = Cast<APlayerController>(Character->GetController());
		if (PC && Data)
		{
			PC->SetViewTargetWithBlend(
				Character,
				Data->CameraBlendOutTime,
				VTBlend_Cubic,
				0.0f,
				false
			);
		}
	}

	// Notify the hiding spot that the actor is exiting
	if (CurrentHidingSpot.IsValid())
	{
		IHideable::Execute_OnExitHiding(CurrentHidingSpot.Get(), GetOwner());
	}

	// Broadcast state change
	OnHidingStateChanged.Broadcast(EHidingState::Exiting);
}

// -----------------------------------------------------------------------------
// Montage Callbacks
// -----------------------------------------------------------------------------

void UHidingComponent::OnEntryMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		// Entry was interrupted -- abort back to Free
		TransitionToFreeState();
		return;
	}

	TransitionToHiddenState();
}

void UHidingComponent::OnExitMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// Whether interrupted or completed normally, transition to Free
	TransitionToFreeState();
}

// -----------------------------------------------------------------------------
// State Transitions
// -----------------------------------------------------------------------------

void UHidingComponent::TransitionToHiddenState()
{
	HidingState = EHidingState::Hidden;

	// Hide the player mesh (invisible while inside the hiding spot)
	SetPlayerMeshVisibility(false);

	// Apply look constraints relative to hiding spot orientation
	ApplyLookConstraints();

	// Switch to hiding-only input context (look + exit)
	SwitchToHidingInputContext();

	// Apply visibility score reduction from hiding spot data
	if (UVisibilityScoreComponent* VisComp = GetOwner()->FindComponentByClass<UVisibilityScoreComponent>())
	{
		if (CurrentHidingSpot.IsValid())
		{
			UHidingSpotDataAsset* Data = IHideable::Execute_GetSpotData(CurrentHidingSpot.Get());
			if (Data)
			{
				VisComp->SetHidingReduction(Data->HidingVisibilityReduction);
			}
		}
	}

	OnHidingStateChanged.Broadcast(EHidingState::Hidden);

	if (CurrentHidingSpot.IsValid())
	{
		UE_LOG(LogSerene, Log, TEXT("Now hidden in [%s]"), *CurrentHidingSpot->GetName());
	}
}

void UHidingComponent::TransitionToFreeState()
{
	HidingState = EHidingState::Free;
	CurrentHidingSpot = nullptr;

	// Re-enable all player systems
	RestorePlayerSystems();

	OnHidingStateChanged.Broadcast(EHidingState::Free);

	UE_LOG(LogSerene, Log, TEXT("Exited hiding spot, now Free"));
}

// -----------------------------------------------------------------------------
// Camera Constraints
// -----------------------------------------------------------------------------

void UHidingComponent::ApplyLookConstraints()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}

	APlayerCameraManager* CamMgr = PC->PlayerCameraManager;

	// Save current limits so they can be restored on exit
	SavedViewPitchMin = CamMgr->ViewPitchMin;
	SavedViewPitchMax = CamMgr->ViewPitchMax;
	SavedViewYawMin = CamMgr->ViewYawMin;
	SavedViewYawMax = CamMgr->ViewYawMax;

	if (!CurrentHidingSpot.IsValid())
	{
		return;
	}

	UHidingSpotDataAsset* Data = IHideable::Execute_GetSpotData(CurrentHidingSpot.Get());
	if (!Data)
	{
		return;
	}

	// Pitch: absolute values (same regardless of spot orientation)
	CamMgr->ViewPitchMin = Data->ViewPitchMin;
	CamMgr->ViewPitchMax = Data->ViewPitchMax;

	// Yaw: RELATIVE to hiding spot's world yaw (Research Pitfall 3 -- critical)
	float SpotYaw = CurrentHidingSpot->GetActorRotation().Yaw;
	CamMgr->ViewYawMin = SpotYaw + Data->ViewYawMin;
	CamMgr->ViewYawMax = SpotYaw + Data->ViewYawMax;
}

void UHidingComponent::RestoreLookConstraints()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}

	APlayerCameraManager* CamMgr = PC->PlayerCameraManager;
	CamMgr->ViewPitchMin = SavedViewPitchMin;
	CamMgr->ViewPitchMax = SavedViewPitchMax;
	CamMgr->ViewYawMin = SavedViewYawMin;
	CamMgr->ViewYawMax = SavedViewYawMax;
}

// -----------------------------------------------------------------------------
// Player System Control
// -----------------------------------------------------------------------------

void UHidingComponent::DisablePlayerSystems()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	// Freeze movement
	Character->GetCharacterMovement()->SetMovementMode(MOVE_None);

	// Disable interaction trace (prevent interacting with other objects during transition)
	if (UInteractionComponent* IC = Character->FindComponentByClass<UInteractionComponent>())
	{
		IC->SetComponentTickEnabled(false);
	}

	// Disable headbob (reads movement state, should be quiet while hiding)
	if (UHeadBobComponent* HB = Character->FindComponentByClass<UHeadBobComponent>())
	{
		HB->SetComponentTickEnabled(false);
	}

	// Disable lean (camera-only lean is not applicable while hiding)
	if (ULeanComponent* Lean = Character->FindComponentByClass<ULeanComponent>())
	{
		Lean->SetComponentTickEnabled(false);
	}

	// Disable footstep detection (no movement while hiding)
	if (UFootstepComponent* FS = Character->FindComponentByClass<UFootstepComponent>())
	{
		FS->SetComponentTickEnabled(false);
	}
}

void UHidingComponent::RestorePlayerSystems()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	// Restore walking movement
	Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	// Re-enable interaction trace
	if (UInteractionComponent* IC = Character->FindComponentByClass<UInteractionComponent>())
	{
		IC->SetComponentTickEnabled(true);
	}

	// Re-enable headbob
	if (UHeadBobComponent* HB = Character->FindComponentByClass<UHeadBobComponent>())
	{
		HB->SetComponentTickEnabled(true);
	}

	// Re-enable lean
	if (ULeanComponent* Lean = Character->FindComponentByClass<ULeanComponent>())
	{
		Lean->SetComponentTickEnabled(true);
	}

	// Re-enable footstep detection
	if (UFootstepComponent* FS = Character->FindComponentByClass<UFootstepComponent>())
	{
		FS->SetComponentTickEnabled(true);
	}
}

// -----------------------------------------------------------------------------
// Mesh Visibility
// -----------------------------------------------------------------------------

void UHidingComponent::SetPlayerMeshVisibility(bool bVisible)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	// Hide/show ALL skeletal mesh components on the character.
	// This covers both the main first-person mesh and the WorldRepresentationMesh
	// (shadow/reflection mesh) without needing direct access to protected members.
	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	Character->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
	for (USkeletalMeshComponent* Mesh : SkeletalMeshes)
	{
		Mesh->SetVisibility(bVisible);
	}
}

// -----------------------------------------------------------------------------
// Input Context Switching
// -----------------------------------------------------------------------------

void UHidingComponent::SwitchToHidingInputContext()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!Subsystem)
	{
		return;
	}

	// Remove the default mapping context to prevent movement/interact input
	if (DefaultMappingContext)
	{
		Subsystem->RemoveMappingContext(DefaultMappingContext);
	}

	// Add hiding mapping context (look + exit only) at higher priority
	if (HidingMappingContext)
	{
		Subsystem->AddMappingContext(HidingMappingContext, HidingInputPriority);
	}
}

void UHidingComponent::RestoreDefaultInputContext()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!Subsystem)
	{
		return;
	}

	// Remove the hiding mapping context
	if (HidingMappingContext)
	{
		Subsystem->RemoveMappingContext(HidingMappingContext);
	}

	// Restore the default mapping context at priority 0
	if (DefaultMappingContext)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}
