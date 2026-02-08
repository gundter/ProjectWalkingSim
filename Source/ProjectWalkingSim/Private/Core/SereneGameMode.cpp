// Copyright Null Lantern.

#include "Core/SereneGameMode.h"

#include "Player/SereneCharacter.h"
#include "Player/SerenePlayerController.h"
#include "Core/SereneLogChannels.h"

ASereneGameMode::ASereneGameMode()
{
	DefaultPawnClass = ASereneCharacter::StaticClass();
	PlayerControllerClass = ASerenePlayerController::StaticClass();

	UE_LOG(LogSerene, Log, TEXT("ASereneGameMode: DefaultPawnClass=ASereneCharacter, PlayerControllerClass=ASerenePlayerController"));
}
