// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SereneGameMode.generated.h"

/**
 * Game mode for The Juniper Tree.
 *
 * Sets default pawn to ASereneCharacter and default player controller
 * to ASerenePlayerController. All PIE sessions using this game mode
 * will spawn the first-person character with Enhanced Input bindings.
 */
UCLASS()
class PROJECTWALKINGSIM_API ASereneGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASereneGameMode();
};
