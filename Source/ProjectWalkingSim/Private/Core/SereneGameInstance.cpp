// Copyright Null Lantern.

#include "Core/SereneGameInstance.h"
#include "Core/SereneLogChannels.h"
#include "Kismet/GameplayStatics.h"

const FString USereneGameInstance::SettingsSlotName = TEXT("AccessibilitySettings");

void USereneGameInstance::Init()
{
	Super::Init();

	LoadAccessibilitySettings();
	UE_LOG(LogSerene, Log, TEXT("SereneGameInstance initialized. HeadBob=%s CrouchToggle=%s"),
		bHeadBobEnabled ? TEXT("ON") : TEXT("OFF"),
		bCrouchToggleMode ? TEXT("Toggle") : TEXT("Hold"));
}

void USereneGameInstance::SaveAccessibilitySettings()
{
	USereneSettingsSave* SaveObject = Cast<USereneSettingsSave>(
		UGameplayStatics::CreateSaveGameObject(USereneSettingsSave::StaticClass()));

	if (!SaveObject)
	{
		UE_LOG(LogSerene, Error, TEXT("Failed to create settings save object."));
		return;
	}

	SaveObject->bHeadBobEnabled = bHeadBobEnabled;
	SaveObject->bCrouchToggleMode = bCrouchToggleMode;

	if (UGameplayStatics::SaveGameToSlot(SaveObject, SettingsSlotName, 0))
	{
		UE_LOG(LogSerene, Log, TEXT("Accessibility settings saved."));
	}
	else
	{
		UE_LOG(LogSerene, Warning, TEXT("Failed to save accessibility settings to slot."));
	}
}

void USereneGameInstance::LoadAccessibilitySettings()
{
	if (!UGameplayStatics::DoesSaveGameExist(SettingsSlotName, 0))
	{
		UE_LOG(LogSerene, Log, TEXT("No saved accessibility settings found. Using defaults."));
		return;
	}

	USereneSettingsSave* SaveObject = Cast<USereneSettingsSave>(
		UGameplayStatics::LoadGameFromSlot(SettingsSlotName, 0));

	if (!SaveObject)
	{
		UE_LOG(LogSerene, Warning, TEXT("Failed to load accessibility settings. Using defaults."));
		return;
	}

	bHeadBobEnabled = SaveObject->bHeadBobEnabled;
	bCrouchToggleMode = SaveObject->bCrouchToggleMode;

	UE_LOG(LogSerene, Log, TEXT("Accessibility settings loaded. HeadBob=%s CrouchToggle=%s"),
		bHeadBobEnabled ? TEXT("ON") : TEXT("OFF"),
		bCrouchToggleMode ? TEXT("Toggle") : TEXT("Hold"));
}
