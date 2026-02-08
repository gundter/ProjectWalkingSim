// Copyright Null Lantern.

#include "Player/HUD/SereneHUD.h"

#include "Player/HUD/StaminaBarWidget.h"
#include "Player/HUD/InteractionPromptWidget.h"
#include "Player/SereneCharacter.h"
#include "Player/Components/StaminaComponent.h"
#include "Player/Components/InteractionComponent.h"
#include "Core/SereneLogChannels.h"

void ASereneHUD::BeginPlay()
{
	Super::BeginPlay();

	// Create stamina bar widget.
	if (StaminaBarWidgetClass)
	{
		StaminaBarInstance = CreateWidget<UStaminaBarWidget>(GetOwningPlayerController(), StaminaBarWidgetClass);
		if (StaminaBarInstance)
		{
			StaminaBarInstance->AddToViewport();
			StaminaBarInstance->SetVisibility(ESlateVisibility::Collapsed); // Hidden until stamina changes
			UE_LOG(LogSerene, Log, TEXT("ASereneHUD::BeginPlay - StaminaBarWidget created and added to viewport."));
		}
	}
	else
	{
		UE_LOG(LogSerene, Warning, TEXT("ASereneHUD::BeginPlay - StaminaBarWidgetClass not set. Stamina bar will not display."));
	}

	// Create interaction prompt widget.
	if (InteractionPromptWidgetClass)
	{
		InteractionPromptInstance = CreateWidget<UInteractionPromptWidget>(GetOwningPlayerController(), InteractionPromptWidgetClass);
		if (InteractionPromptInstance)
		{
			InteractionPromptInstance->AddToViewport();
			UE_LOG(LogSerene, Log, TEXT("ASereneHUD::BeginPlay - InteractionPromptWidget created and added to viewport."));
		}
	}
	else
	{
		UE_LOG(LogSerene, Warning, TEXT("ASereneHUD::BeginPlay - InteractionPromptWidgetClass not set. Interaction prompt will not display."));
	}

	// Bind to the character's component delegates.
	// Handle timing: pawn may not be possessed yet in BeginPlay.
	ASereneCharacter* Character = Cast<ASereneCharacter>(GetOwningPawn());
	if (Character)
	{
		BindToCharacter(Character);
	}
	else
	{
		// Defer binding until pawn is available via a short timer.
		FTimerHandle BindTimerHandle;
		GetWorldTimerManager().SetTimer(BindTimerHandle, [this]()
		{
			ASereneCharacter* DeferredCharacter = Cast<ASereneCharacter>(GetOwningPawn());
			if (DeferredCharacter)
			{
				BindToCharacter(DeferredCharacter);
			}
			else
			{
				UE_LOG(LogSerene, Warning, TEXT("ASereneHUD - Deferred bind failed: no pawn available."));
			}
		}, 0.1f, false);
	}
}

void ASereneHUD::BindToCharacter(ASereneCharacter* Character)
{
	if (!Character)
	{
		return;
	}

	// Bind stamina delegate to HUD handler.
	if (UStaminaComponent* Stamina = Character->FindComponentByClass<UStaminaComponent>())
	{
		Stamina->OnStaminaChanged.AddDynamic(this, &ASereneHUD::HandleStaminaChanged);
		UE_LOG(LogSerene, Log, TEXT("ASereneHUD::BindToCharacter - Bound to StaminaComponent::OnStaminaChanged."));
	}

	// Bind interaction delegate to HUD handler.
	if (UInteractionComponent* Interaction = Character->FindComponentByClass<UInteractionComponent>())
	{
		Interaction->OnInteractableChanged.AddDynamic(this, &ASereneHUD::HandleInteractableChanged);
		UE_LOG(LogSerene, Log, TEXT("ASereneHUD::BindToCharacter - Bound to InteractionComponent::OnInteractableChanged."));
	}
}

void ASereneHUD::HandleStaminaChanged(float Percent)
{
	if (StaminaBarInstance)
	{
		StaminaBarInstance->SetStaminaPercent(Percent);
	}
}

void ASereneHUD::HandleInteractableChanged(AActor* NewInteractable, FText InteractionText)
{
	if (InteractionPromptInstance)
	{
		InteractionPromptInstance->UpdatePrompt(NewInteractable, InteractionText);
	}
}
