// Copyright Null Lantern.

#include "Player/SerenePlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Player/SereneCharacter.h"
#include "Player/Components/InteractionComponent.h"
#include "Player/Components/LeanComponent.h"
#include "Core/SereneLogChannels.h"
#include "Core/SereneGameInstance.h"

void ASerenePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Add default mapping context at priority 0
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			UE_LOG(LogSerene, Log, TEXT("ASerenePlayerController::BeginPlay - Added DefaultMappingContext at priority 0."));
		}
		else
		{
			UE_LOG(LogSerene, Warning, TEXT("ASerenePlayerController::BeginPlay - DefaultMappingContext is not assigned. Input will not work until assigned."));
		}
	}
	else
	{
		UE_LOG(LogSerene, Error, TEXT("ASerenePlayerController::BeginPlay - Failed to get EnhancedInputLocalPlayerSubsystem."));
	}

	// Set input mode to game only
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	// Clamp camera pitch to prevent extreme angles
	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMin = -80.0f;
		PlayerCameraManager->ViewPitchMax = 80.0f;
	}
}

void ASerenePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInput)
	{
		UE_LOG(LogSerene, Error, TEXT("ASerenePlayerController::SetupInputComponent - InputComponent is not UEnhancedInputComponent. Enhanced Input may not be configured as default."));
		return;
	}

	// Movement
	if (MoveAction)
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASerenePlayerController::HandleMove);
	}

	// Look
	if (LookAction)
	{
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASerenePlayerController::HandleLook);
	}

	// Sprint (hold)
	if (SprintAction)
	{
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ASerenePlayerController::HandleSprintStart);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASerenePlayerController::HandleSprintStop);
	}

	// Crouch (toggle by default)
	if (CrouchAction)
	{
		EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &ASerenePlayerController::HandleCrouchToggle);
	}

	// Interact (tap)
	if (InteractAction)
	{
		EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ASerenePlayerController::HandleInteract);
	}

	// Lean Left (hold)
	if (LeanLeftAction)
	{
		EnhancedInput->BindAction(LeanLeftAction, ETriggerEvent::Triggered, this, &ASerenePlayerController::HandleLeanLeftStart);
		EnhancedInput->BindAction(LeanLeftAction, ETriggerEvent::Completed, this, &ASerenePlayerController::HandleLeanLeftStop);
	}

	// Lean Right (hold)
	if (LeanRightAction)
	{
		EnhancedInput->BindAction(LeanRightAction, ETriggerEvent::Triggered, this, &ASerenePlayerController::HandleLeanRightStart);
		EnhancedInput->BindAction(LeanRightAction, ETriggerEvent::Completed, this, &ASerenePlayerController::HandleLeanRightStop);
	}

	UE_LOG(LogSerene, Log, TEXT("ASerenePlayerController::SetupInputComponent - Enhanced Input bindings configured."));
}

ASereneCharacter* ASerenePlayerController::GetSereneCharacter() const
{
	return Cast<ASereneCharacter>(GetPawn());
}

void ASerenePlayerController::HandleMove(const FInputActionValue& Value)
{
	ASereneCharacter* Character = GetSereneCharacter();
	if (!Character)
	{
		return;
	}

	const FVector2D MoveValue = Value.Get<FVector2D>();

	// Decompose 2D axis into forward/right movement relative to controller yaw.
	// Only use yaw from controller rotation (ignore pitch/roll for movement direction).
	const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// MoveValue.Y = forward/backward (W/S), MoveValue.X = left/right (A/D)
	Character->AddMovementInput(ForwardDirection, MoveValue.Y);
	Character->AddMovementInput(RightDirection, MoveValue.X);
}

void ASerenePlayerController::HandleLook(const FInputActionValue& Value)
{
	const FVector2D LookValue = Value.Get<FVector2D>();

	// Apply mouse input directly. Sensitivity is handled by Enhanced Input modifiers
	// on the LookAction asset (scalar modifiers in the IMC).
	AddYawInput(LookValue.X);
	AddPitchInput(LookValue.Y);
}

void ASerenePlayerController::HandleSprintStart(const FInputActionValue& Value)
{
	ASereneCharacter* Character = GetSereneCharacter();
	if (Character)
	{
		Character->StartSprint();
	}
}

void ASerenePlayerController::HandleSprintStop(const FInputActionValue& Value)
{
	ASereneCharacter* Character = GetSereneCharacter();
	if (Character)
	{
		Character->StopSprint();
	}
}

void ASerenePlayerController::HandleCrouchToggle(const FInputActionValue& Value)
{
	ASereneCharacter* Character = GetSereneCharacter();
	if (!Character)
	{
		return;
	}

	// Check GameInstance for crouch mode setting
	// Default: toggle mode (press to crouch, press again to stand)
	// TODO: When hold mode is needed, rebind CrouchAction to use Triggered+Completed instead of Started
	if (Character->GetIsCrouching())
	{
		Character->StopCrouching();
	}
	else
	{
		Character->StartCrouching();
	}
}

void ASerenePlayerController::HandleInteract(const FInputActionValue& Value)
{
	ASereneCharacter* Character = GetSereneCharacter();
	if (Character)
	{
		UInteractionComponent* InteractionComp = Character->FindComponentByClass<UInteractionComponent>();
		if (InteractionComp)
		{
			InteractionComp->TryInteract();
		}
	}
}

void ASerenePlayerController::HandleLeanLeftStart(const FInputActionValue& Value)
{
	ASereneCharacter* Character = GetSereneCharacter();
	if (Character)
	{
		if (ULeanComponent* LeanComp = Character->FindComponentByClass<ULeanComponent>())
		{
			LeanComp->SetLeanLeft(true);
		}
	}
}

void ASerenePlayerController::HandleLeanLeftStop(const FInputActionValue& Value)
{
	ASereneCharacter* Character = GetSereneCharacter();
	if (Character)
	{
		if (ULeanComponent* LeanComp = Character->FindComponentByClass<ULeanComponent>())
		{
			LeanComp->SetLeanLeft(false);
		}
	}
}

void ASerenePlayerController::HandleLeanRightStart(const FInputActionValue& Value)
{
	ASereneCharacter* Character = GetSereneCharacter();
	if (Character)
	{
		if (ULeanComponent* LeanComp = Character->FindComponentByClass<ULeanComponent>())
		{
			LeanComp->SetLeanRight(true);
		}
	}
}

void ASerenePlayerController::HandleLeanRightStop(const FInputActionValue& Value)
{
	ASereneCharacter* Character = GetSereneCharacter();
	if (Character)
	{
		if (ULeanComponent* LeanComp = Character->FindComponentByClass<ULeanComponent>())
		{
			LeanComp->SetLeanRight(false);
		}
	}
}
