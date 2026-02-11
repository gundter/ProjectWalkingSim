// Copyright Null Lantern.

#include "Core/SereneGameMode.h"

#include "Player/SereneCharacter.h"
#include "Player/SerenePlayerController.h"
#include "Player/HUD/SereneHUD.h"
#include "Core/SereneLogChannels.h"

ASereneGameMode::ASereneGameMode()
{
	DefaultPawnClass = ASereneCharacter::StaticClass();
	PlayerControllerClass = ASerenePlayerController::StaticClass();
	HUDClass = ASereneHUD::StaticClass();

	UE_LOG(LogSerene, Verbose, TEXT("ASereneGameMode: DefaultPawnClass=ASereneCharacter, PlayerControllerClass=ASerenePlayerController, HUDClass=ASereneHUD"));
}
