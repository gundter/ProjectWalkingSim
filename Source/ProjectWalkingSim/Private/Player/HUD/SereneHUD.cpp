// Copyright Null Lantern.

#include "Player/HUD/SereneHUD.h"

#include "Player/HUD/SereneHUDWidget.h"
#include "Player/HUD/StaminaBarWidget.h"
#include "Player/HUD/InteractionPromptWidget.h"
#include "Player/SereneCharacter.h"
#include "Player/Components/StaminaComponent.h"
#include "Player/Components/InteractionComponent.h"
#include "Core/SereneLogChannels.h"

void ASereneHUD::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<USereneHUDWidget>(GetOwningPlayerController(), HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
			UE_LOG(LogSerene, Log, TEXT("ASereneHUD::BeginPlay - HUDWidget created and added to viewport."));
		}
	}
	else
	{
		UE_LOG(LogSerene, Warning, TEXT("ASereneHUD::BeginPlay - HUDWidgetClass not set. HUD will not display."));
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
	if (HUDWidgetInstance && HUDWidgetInstance->GetStaminaBarWidget())
	{
		HUDWidgetInstance->GetStaminaBarWidget()->SetStaminaPercent(Percent);
	}
	else
	{
		UE_LOG(LogSerene, Warning, TEXT("ASereneHUD::HandleStaminaChanged - HUDWidgetInstance or StaminaBarWidget is null."));
	}
}

void ASereneHUD::HandleInteractableChanged(AActor* NewInteractable, FText InteractionText)
{
	if (HUDWidgetInstance && HUDWidgetInstance->GetInteractionPromptWidget())
	{
		HUDWidgetInstance->GetInteractionPromptWidget()->UpdatePrompt(NewInteractable, InteractionText);
	}
}
