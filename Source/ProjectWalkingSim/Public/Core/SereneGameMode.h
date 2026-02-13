// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SereneGameMode.generated.h"

class UUserWidget;

/**
 * Game mode for The Juniper Tree.
 *
 * Sets default pawn to ASereneCharacter and default player controller
 * to ASerenePlayerController. All PIE sessions using this game mode
 * will spawn the first-person character with Enhanced Input bindings.
 *
 * Handles:
 * - Player death (Wendigo grab) -> Game Over screen
 * - Post-level-reload save data application via OnActorsInitialized
 */
UCLASS()
class PROJECTWALKINGSIM_API ASereneGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASereneGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/**
	 * Called when the player dies (e.g., Wendigo grab completes).
	 * Creates the Game Over widget, adds it to viewport, and switches to UI-only input.
	 */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnPlayerDeath();

	/** Returns true if the Game Over screen is currently displayed. */
	UFUNCTION(BlueprintCallable, Category = "Game")
	bool IsGameOver() const { return GameOverWidgetInstance != nullptr; }

protected:
	/** Blueprint subclass of UGameOverWidget to spawn on player death. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	/** Live instance of the Game Over widget (if active). */
	UPROPERTY()
	TObjectPtr<UUserWidget> GameOverWidgetInstance;

private:
	/** Called by OnActorsInitialized delegate after level reload to apply pending save data. */
	void OnActorsReady(const FActorsInitializedParams& Params);
};
