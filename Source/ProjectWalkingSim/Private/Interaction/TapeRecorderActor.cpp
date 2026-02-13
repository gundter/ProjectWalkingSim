// Copyright Null Lantern.

#include "Interaction/TapeRecorderActor.h"

#include "Player/HUD/SaveLoadMenuWidget.h"
#include "Save/SaveSubsystem.h"
#include "Hiding/HidingComponent.h"
#include "Hiding/HidingTypes.h"
#include "Core/SereneLogChannels.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

ATapeRecorderActor::ATapeRecorderActor()
{
	InteractionText = NSLOCTEXT("Interaction", "TapeRecorder", "Record");
}

bool ATapeRecorderActor::CanInteract_Implementation(AActor* Interactor) const
{
	if (!bCanBeInteracted)
	{
		return false;
	}

	// Cannot interact while save menu is already open
	if (SaveMenuInstance != nullptr)
	{
		return false;
	}

	// Cannot interact while hiding
	if (Interactor)
	{
		UHidingComponent* HidingComp = Interactor->FindComponentByClass<UHidingComponent>();
		if (HidingComp && HidingComp->GetHidingState() != EHidingState::Free)
		{
			return false;
		}
	}

	return true;
}

FText ATapeRecorderActor::GetInteractionText_Implementation() const
{
	return InteractionText;
}

void ATapeRecorderActor::OnInteract_Implementation(AActor* Interactor)
{
	// Play save sound at tape recorder location
	if (SaveSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SaveSound, GetActorLocation());
	}

	// Tell SaveSubsystem to use tape recorder position for the next save
	USaveSubsystem* SaveSub = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	if (SaveSub)
	{
		SaveSub->SetPendingSaveLocation(GetActorLocation(), GetActorRotation());
	}

	// Create the save menu widget
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC || !SaveMenuWidgetClass)
	{
		UE_LOG(LogSerene, Warning, TEXT("TapeRecorderActor::OnInteract - No player controller or SaveMenuWidgetClass not set"));
		return;
	}

	SaveMenuInstance = CreateWidget<USaveLoadMenuWidget>(PC, SaveMenuWidgetClass);
	if (!SaveMenuInstance)
	{
		UE_LOG(LogSerene, Error, TEXT("TapeRecorderActor::OnInteract - Failed to create SaveLoadMenuWidget"));
		return;
	}

	SaveMenuInstance->AddToViewport(50);
	SaveMenuInstance->OpenMenu(ESaveLoadMode::Save);

	// Set input mode to Game + UI so player can click slots
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(true);

	// Bind close callback
	SaveMenuInstance->OnMenuClosed.AddDynamic(this, &ATapeRecorderActor::HandleSaveMenuClosed);

	UE_LOG(LogSerene, Log, TEXT("TapeRecorderActor::OnInteract - Save menu opened"));
}

void ATapeRecorderActor::HandleSaveMenuClosed()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->SetShowMouseCursor(false);

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
	}

	// Remove widget from viewport
	if (SaveMenuInstance)
	{
		SaveMenuInstance->RemoveFromParent();
		SaveMenuInstance = nullptr;
	}

	// Clear pending save location
	USaveSubsystem* SaveSub = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	if (SaveSub)
	{
		SaveSub->ClearPendingSaveLocation();
	}

	UE_LOG(LogSerene, Log, TEXT("TapeRecorderActor::HandleSaveMenuClosed - Input restored, menu removed"));
}
