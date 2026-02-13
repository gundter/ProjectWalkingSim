// Copyright Null Lantern.

#include "Core/SereneGameMode.h"

#include "Player/SereneCharacter.h"
#include "Player/SerenePlayerController.h"
#include "Player/HUD/SereneHUD.h"
#include "Save/SaveSubsystem.h"
#include "Core/SereneLogChannels.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

ASereneGameMode::ASereneGameMode()
{
	DefaultPawnClass = ASereneCharacter::StaticClass();
	PlayerControllerClass = ASerenePlayerController::StaticClass();
	HUDClass = ASereneHUD::StaticClass();

	UE_LOG(LogSerene, Verbose, TEXT("ASereneGameMode: DefaultPawnClass=ASereneCharacter, PlayerControllerClass=ASerenePlayerController, HUDClass=ASereneHUD"));
}

void ASereneGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Check if a save load is pending (set by SaveSubsystem before OpenLevel)
	UGameInstance* GI = GetGameInstance();
	USaveSubsystem* SaveSub = GI ? GI->GetSubsystem<USaveSubsystem>() : nullptr;

	if (SaveSub && SaveSub->IsPendingLoad())
	{
		// Bind to OnActorsInitialized so we apply save data after all actors exist
		GetWorld()->OnActorsInitialized.AddUObject(this, &ASereneGameMode::OnActorsReady);

		UE_LOG(LogSerene, Log, TEXT("SereneGameMode::InitGame - Pending load detected, bound OnActorsInitialized"));
	}
}

void ASereneGameMode::OnActorsReady(const FActorsInitializedParams& Params)
{
	USaveSubsystem* SaveSub = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	if (SaveSub)
	{
		SaveSub->ApplyPendingSaveData(GetWorld());
		UE_LOG(LogSerene, Log, TEXT("SereneGameMode::OnActorsReady - Pending save data applied"));
	}
}

void ASereneGameMode::OnPlayerDeath()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		UE_LOG(LogSerene, Warning, TEXT("SereneGameMode::OnPlayerDeath - No player controller"));
		return;
	}

	// Create and show Game Over widget
	if (GameOverWidgetClass)
	{
		GameOverWidgetInstance = CreateWidget<UUserWidget>(PC, GameOverWidgetClass);
		if (GameOverWidgetInstance)
		{
			GameOverWidgetInstance->AddToViewport(100); // High Z-order to be on top

			// Switch to UI-only input mode and show mouse cursor
			PC->SetShowMouseCursor(true);
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(GameOverWidgetInstance->TakeWidget());
			PC->SetInputMode(InputMode);

			UE_LOG(LogSerene, Log, TEXT("SereneGameMode::OnPlayerDeath - Game Over widget displayed"));
		}
	}
	else
	{
		UE_LOG(LogSerene, Warning, TEXT("SereneGameMode::OnPlayerDeath - GameOverWidgetClass not set, cannot show Game Over screen"));
	}
}
