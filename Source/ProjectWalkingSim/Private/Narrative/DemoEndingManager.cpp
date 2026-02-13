// Copyright Null Lantern.

#include "Narrative/DemoEndingManager.h"

#include "Blueprint/UserWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Core/SereneLogChannels.h"

UDemoEndingManager::UDemoEndingManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDemoEndingManager::TriggerEnding()
{
	if (bEndingTriggered)
	{
		UE_LOG(LogSerene, Warning, TEXT("DemoEndingManager: TriggerEnding called again -- already triggered, ignoring"));
		return;
	}

	bEndingTriggered = true;

	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		UE_LOG(LogSerene, Error, TEXT("DemoEndingManager: No PlayerController found, cannot trigger ending"));
		return;
	}

	UE_LOG(LogSerene, Log, TEXT("DemoEndingManager: Triggering demo ending sequence"));

	// 1. Disable gameplay input
	PC->DisableInput(PC);

	// 2. Play optional final monologue
	if (FinalMonologue)
	{
		UGameplayStatics::PlaySound2D(this, FinalMonologue);
		UE_LOG(LogSerene, Log, TEXT("DemoEndingManager: Playing final monologue"));
	}

	// 3. Fade to black
	if (PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(
			0.0f,                   // FromAlpha
			1.0f,                   // ToAlpha
			FadeDuration,           // Duration
			FLinearColor::Black,    // Color
			true,                   // bShouldFadeAudio
			true);                  // bHoldWhenFinished
		UE_LOG(LogSerene, Log, TEXT("DemoEndingManager: Camera fade started (%.1fs)"), FadeDuration);
	}

	// 4. Set timer for title card display after fade + delay
	const float TotalDelay = FadeDuration + TitleCardDelay;
	GetWorld()->GetTimerManager().SetTimer(
		TitleCardTimerHandle,
		this,
		&UDemoEndingManager::ShowTitleCard,
		TotalDelay,
		false);

	UE_LOG(LogSerene, Log, TEXT("DemoEndingManager: Title card scheduled in %.1fs"), TotalDelay);
}

void UDemoEndingManager::ShowTitleCard()
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		UE_LOG(LogSerene, Error, TEXT("DemoEndingManager: No PlayerController for title card display"));
		return;
	}

	if (TitleCardWidgetClass)
	{
		TitleCardInstance = CreateWidget<UUserWidget>(PC, TitleCardWidgetClass);
		if (TitleCardInstance)
		{
			TitleCardInstance->AddToViewport(200);
			UE_LOG(LogSerene, Log, TEXT("DemoEndingManager: Title card displayed"));
		}
	}

	// Set input mode to UI only so the player can interact with any title card buttons
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;
}
