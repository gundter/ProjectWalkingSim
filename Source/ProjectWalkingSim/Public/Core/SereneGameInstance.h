// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameFramework/SaveGame.h"
#include "SereneGameInstance.generated.h"

/**
 * Lightweight save object for user accessibility/control settings.
 * Persisted via USaveGame slot so preferences survive across sessions.
 */
UCLASS()
class PROJECTWALKINGSIM_API USereneSettingsSave : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	bool bHeadBobEnabled = true;

	UPROPERTY()
	bool bCrouchToggleMode = true;
};

/**
 * Game instance for The Juniper Tree.
 *
 * Stores user-facing accessibility settings (head-bob toggle, crouch mode)
 * and persists them across sessions via a USaveGame slot. Settings are loaded
 * automatically on Init() and can be saved at any time from gameplay code or
 * a settings menu.
 */
UCLASS()
class PROJECTWALKINGSIM_API USereneGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	// --- Accessibility Settings ---

	/** If true, procedural head-bob is active while moving. */
	UPROPERTY(Config, BlueprintReadWrite, Category = "Accessibility")
	bool bHeadBobEnabled = true;

	/** If true, crouch is a toggle (press once to crouch, again to stand). If false, hold-to-crouch. */
	UPROPERTY(Config, BlueprintReadWrite, Category = "Accessibility")
	bool bCrouchToggleMode = true;

	// --- Persistence ---

	/** Save current accessibility settings to disk. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SaveAccessibilitySettings();

	/** Load accessibility settings from disk. Called automatically on Init(). */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadAccessibilitySettings();

private:
	static const FString SettingsSlotName;
};
