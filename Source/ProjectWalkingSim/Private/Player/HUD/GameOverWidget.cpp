// Copyright Null Lantern.

#include "Player/HUD/GameOverWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Save/SaveSubsystem.h"
#include "Core/SereneLogChannels.h"

void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Determine button text based on whether any saves exist
	USaveSubsystem* SaveSub = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	if (SaveSub && SaveSub->HasAnySave())
	{
		if (LoadLastSaveButtonText)
		{
			LoadLastSaveButtonText->SetText(FText::FromString(TEXT("Load Last Save")));
		}
	}
	else
	{
		if (LoadLastSaveButtonText)
		{
			LoadLastSaveButtonText->SetText(FText::FromString(TEXT("Restart")));
		}
	}

	// Bind button delegates
	if (LoadLastSaveButton)
	{
		LoadLastSaveButton->OnClicked.AddDynamic(this, &UGameOverWidget::HandleLoadClicked);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UGameOverWidget::HandleQuitClicked);
	}
}

void UGameOverWidget::HandleLoadClicked()
{
	USaveSubsystem* SaveSub = GetGameInstance()->GetSubsystem<USaveSubsystem>();
	if (SaveSub && SaveSub->HasAnySave())
	{
		SaveSub->LoadLatestSave();
	}
	else
	{
		// No saves -- restart the current level
		UWorld* World = GetWorld();
		if (World)
		{
			const FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(World);
			UGameplayStatics::OpenLevel(World, FName(*CurrentLevel));
		}
	}
}

void UGameOverWidget::HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}
