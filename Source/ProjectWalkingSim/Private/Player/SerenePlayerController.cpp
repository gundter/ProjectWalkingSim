// Copyright Null Lantern.

#include "Player/SerenePlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Player/SereneCharacter.h"
#include "Player/Components/InteractionComponent.h"
#include "Player/Components/LeanComponent.h"
#include "Hiding/HidingComponent.h"
#include "Player/HUD/SereneHUD.h"
#include "Player/HUD/PauseMenuWidget.h"
#include "Core/SereneLogChannels.h"
#include "Core/SereneGameInstance.h"
#include "Core/SereneGameMode.h"
#include "Hiding/HidingTypes.h"
#include "Save/SaveSubsystem.h"
#include "Kismet/GameplayStatics.h"

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

	// Inventory toggle
	if (ToggleInventoryAction)
	{
		EnhancedInput->BindAction(ToggleInventoryAction, ETriggerEvent::Started,
			this, &ASerenePlayerController::HandleToggleInventory);
	}

	// Pause (Esc)
	if (PauseAction)
	{
		EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started,
			this, &ASerenePlayerController::HandlePause);
	}

	UE_LOG(LogSerene, Log, TEXT("ASerenePlayerController::SetupInputComponent - Enhanced Input bindings configured."));
}

void ASerenePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	CacheComponentPointers(InPawn);
}

void ASerenePlayerController::CacheComponentPointers(APawn* InPawn)
{
	if (InPawn)
	{
		CachedLeanComponent = InPawn->FindComponentByClass<ULeanComponent>();
		CachedHidingComponent = InPawn->FindComponentByClass<UHidingComponent>();
		CachedInteractionComponent = InPawn->FindComponentByClass<UInteractionComponent>();
	}
	else
	{
		CachedLeanComponent = nullptr;
		CachedHidingComponent = nullptr;
		CachedInteractionComponent = nullptr;
	}
}

ASereneCharacter* ASerenePlayerController::GetSereneCharacter() const
{
	return Cast<ASereneCharacter>(GetPawn());
}

void ASerenePlayerController::HandleMove(const FInputActionValue& Value)
{
	ASereneCharacter* SereneChar = GetSereneCharacter();
	if (!SereneChar)
	{
		return;
	}

	const FVector2D MoveValue = Value.Get<FVector2D>();

	UE_LOG(LogSerene, Verbose, TEXT("HandleMove - MoveValue: X=%.2f Y=%.2f"), MoveValue.X, MoveValue.Y);

	// Decompose 2D axis into forward/right movement relative to controller yaw.
	// Only use yaw from controller rotation (ignore pitch/roll for movement direction).
	const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// MoveValue.Y = forward/backward (W/S), MoveValue.X = left/right (A/D)
	SereneChar->AddMovementInput(ForwardDirection, MoveValue.Y);
	SereneChar->AddMovementInput(RightDirection, MoveValue.X);
}

void ASerenePlayerController::HandleLook(const FInputActionValue& Value)
{
	const FVector2D LookValue = Value.Get<FVector2D>();

	// Apply mouse input directly. Sensitivity is handled by Enhanced Input modifiers
	// on the LookAction asset (scalar modifiers in the IMC).
	// Pitch is negated: mouse-up (positive Y) should look up (negative pitch in UE).
	AddYawInput(LookValue.X);
	AddPitchInput(-LookValue.Y);
}

void ASerenePlayerController::HandleSprintStart(const FInputActionValue& Value)
{
	ASereneCharacter* SereneChar = GetSereneCharacter();
	if (SereneChar)
	{
		SereneChar->StartSprint();
	}
}

void ASerenePlayerController::HandleSprintStop(const FInputActionValue& Value)
{
	ASereneCharacter* SereneChar = GetSereneCharacter();
	if (SereneChar)
	{
		SereneChar->StopSprint();
	}
}

void ASerenePlayerController::HandleCrouchToggle(const FInputActionValue& Value)
{
	ASereneCharacter* SereneChar = GetSereneCharacter();
	if (!SereneChar)
	{
		return;
	}

	// Check GameInstance for crouch mode setting
	// Default: toggle mode (press to crouch, press again to stand)
	// TODO: When hold mode is needed, rebind CrouchAction to use Triggered+Completed instead of Started
	if (SereneChar->GetIsCrouching())
	{
		SereneChar->StopCrouching();
	}
	else
	{
		SereneChar->StartCrouching();
	}
}

void ASerenePlayerController::HandleInteract(const FInputActionValue& Value)
{
	ASereneCharacter* SereneChar = GetSereneCharacter();
	if (!SereneChar)
	{
		return;
	}

	// If hiding, pressing interact exits the hiding spot
	if (CachedHidingComponent && CachedHidingComponent->IsHiding())
	{
		CachedHidingComponent->ExitHidingSpot();
		return;
	}

	// Normal interaction
	if (CachedInteractionComponent)
	{
		CachedInteractionComponent->TryInteract();
	}
}

void ASerenePlayerController::HandleLeanLeftStart(const FInputActionValue& Value)
{
	if (CachedLeanComponent)
	{
		CachedLeanComponent->SetLeanLeft(true);
	}
}

void ASerenePlayerController::HandleLeanLeftStop(const FInputActionValue& Value)
{
	if (CachedLeanComponent)
	{
		CachedLeanComponent->SetLeanLeft(false);
	}
}

void ASerenePlayerController::HandleLeanRightStart(const FInputActionValue& Value)
{
	if (CachedLeanComponent)
	{
		CachedLeanComponent->SetLeanRight(true);
	}
}

void ASerenePlayerController::HandleLeanRightStop(const FInputActionValue& Value)
{
	if (CachedLeanComponent)
	{
		CachedLeanComponent->SetLeanRight(false);
	}
}

void ASerenePlayerController::HandleToggleInventory(const FInputActionValue& Value)
{
	if (bIsInventoryOpen)
	{
		CloseInventory();
	}
	else
	{
		OpenInventory();
	}
}

void ASerenePlayerController::OpenInventory()
{
	bIsInventoryOpen = true;

	// Switch to game and UI input mode with visible cursor
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	SetShowMouseCursor(true);
	SetIgnoreLookInput(true);

	// Notify HUD
	if (ASereneHUD* HUD = Cast<ASereneHUD>(GetHUD()))
	{
		HUD->ShowInventory();
	}

	UE_LOG(LogSerene, Verbose, TEXT("ASerenePlayerController::OpenInventory - Inventory opened, input mode set to GameAndUI."));
}

void ASerenePlayerController::CloseInventory()
{
	bIsInventoryOpen = false;

	// Switch back to game-only input mode
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	SetShowMouseCursor(false);
	SetIgnoreLookInput(false);

	// Notify HUD
	if (ASereneHUD* HUD = Cast<ASereneHUD>(GetHUD()))
	{
		HUD->HideInventory();
	}

	UE_LOG(LogSerene, Verbose, TEXT("ASerenePlayerController::CloseInventory - Inventory closed, input mode set to GameOnly."));
}

// ---------------------------------------------------------------------------
// Pause Menu
// ---------------------------------------------------------------------------

void ASerenePlayerController::HandlePause(const FInputActionValue& Value)
{
	// Esc while inventory is open closes inventory instead
	if (bIsInventoryOpen)
	{
		CloseInventory();
		return;
	}

	TogglePauseMenu();
}

void ASerenePlayerController::TogglePauseMenu()
{
	if (bIsPaused)
	{
		// Close pause menu
		bIsPaused = false;

		if (PauseMenuInstance)
		{
			PauseMenuInstance->RemoveFromParent();
			PauseMenuInstance = nullptr;
		}

		SetShowMouseCursor(false);

		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);

		UGameplayStatics::SetGamePaused(GetWorld(), false);

		UE_LOG(LogSerene, Log, TEXT("ASerenePlayerController::TogglePauseMenu - Unpaused"));
	}
	else
	{
		// Guard: don't open pause while hiding
		if (CachedHidingComponent && CachedHidingComponent->GetHidingState() != EHidingState::Free)
		{
			return;
		}

		// Guard: don't open pause during Game Over
		ASereneGameMode* GM = Cast<ASereneGameMode>(GetWorld()->GetAuthGameMode());
		if (GM && GM->IsGameOver())
		{
			return;
		}

		// Open pause menu
		bIsPaused = true;

		if (!PauseMenuInstance && PauseMenuWidgetClass)
		{
			PauseMenuInstance = CreateWidget<UPauseMenuWidget>(this, PauseMenuWidgetClass);
		}

		if (PauseMenuInstance)
		{
			PauseMenuInstance->AddToViewport(90);
			PauseMenuInstance->OnPauseMenuClosed.AddDynamic(this, &ASerenePlayerController::HandlePauseMenuClosed);
		}

		SetShowMouseCursor(true);

		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);

		UGameplayStatics::SetGamePaused(GetWorld(), true);

		UE_LOG(LogSerene, Log, TEXT("ASerenePlayerController::TogglePauseMenu - Paused"));
	}
}

void ASerenePlayerController::HandlePauseMenuClosed()
{
	bIsPaused = false;

	if (PauseMenuInstance)
	{
		PauseMenuInstance->RemoveFromParent();
		PauseMenuInstance = nullptr;
	}

	SetShowMouseCursor(false);

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	UGameplayStatics::SetGamePaused(GetWorld(), false);

	UE_LOG(LogSerene, Log, TEXT("ASerenePlayerController::HandlePauseMenuClosed - Resumed"));
}
