// Copyright Null Lantern.

#include "Player/HUD/PauseMenuWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Player/HUD/SaveLoadMenuWidget.h"
#include "Save/SaveSubsystem.h"
#include "Core/SereneLogChannels.h"
#include "Kismet/KismetSystemLibrary.h"

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button handlers
	if (ContinueButton)
	{
		ContinueButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleContinueClicked);
	}
	if (LoadGameButton)
	{
		LoadGameButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleLoadGameClicked);
	}
	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleResumeClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleQuitClicked);
	}

	// Disable Continue and Load Game if no saves exist
	USaveSubsystem* SaveSub = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	const bool bHasSaves = SaveSub && SaveSub->HasAnySave();

	if (ContinueButton)
	{
		ContinueButton->SetIsEnabled(bHasSaves);
	}
	if (LoadGameButton)
	{
		LoadGameButton->SetIsEnabled(bHasSaves);
	}
}

void UPauseMenuWidget::HandleContinueClicked()
{
	USaveSubsystem* SaveSub = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	if (SaveSub)
	{
		SaveSub->LoadLatestSave();
		// LoadLatestSave restarts the level; widget will be destroyed
	}
}

void UPauseMenuWidget::HandleLoadGameClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC || !LoadMenuWidgetClass)
	{
		UE_LOG(LogSerene, Warning, TEXT("UPauseMenuWidget::HandleLoadGameClicked - No player controller or LoadMenuWidgetClass not set"));
		return;
	}

	LoadMenuInstance = CreateWidget<USaveLoadMenuWidget>(PC, LoadMenuWidgetClass);
	if (!LoadMenuInstance)
	{
		UE_LOG(LogSerene, Error, TEXT("UPauseMenuWidget::HandleLoadGameClicked - Failed to create SaveLoadMenuWidget"));
		return;
	}

	LoadMenuInstance->AddToViewport(60);
	LoadMenuInstance->OpenMenu(ESaveLoadMode::Load);

	// Hide pause menu while load menu is showing
	SetVisibility(ESlateVisibility::Collapsed);

	// Bind close callback to re-show pause menu
	LoadMenuInstance->OnMenuClosed.AddDynamic(this, &UPauseMenuWidget::HandleLoadMenuClosed);

	UE_LOG(LogSerene, Log, TEXT("UPauseMenuWidget::HandleLoadGameClicked - Load menu opened"));
}

void UPauseMenuWidget::HandleResumeClicked()
{
	OnPauseMenuClosed.Broadcast();
}

void UPauseMenuWidget::HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UPauseMenuWidget::HandleLoadMenuClosed()
{
	// Remove load menu widget
	if (LoadMenuInstance)
	{
		LoadMenuInstance->RemoveFromParent();
		LoadMenuInstance = nullptr;
	}

	// Show pause menu again
	SetVisibility(ESlateVisibility::Visible);
}
