// Copyright Null Lantern.

#include "Player/SereneCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/Components/InteractionComponent.h"
#include "Player/Components/FootstepComponent.h"
#include "Player/HUD/SereneHUD.h"
#include "Core/SereneLogChannels.h"
#include "Core/SereneGameInstance.h"

ASereneCharacter::ASereneCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// --- First Person Camera ---
	// Attached to root component. Position is driven manually in Tick() by
	// reading the head bone world location and adding actor-space offsets
	// (crouch, head-bob, lean). This avoids bone-local-space axis confusion.
	// Rotation is also set manually from the controller + lean roll.
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetRootComponent());
	FirstPersonCamera->bUsePawnControlRotation = false;

	// --- First Person Rendering ---
	// Main mesh: visible to the owning player in first-person (full body).
	GetMesh()->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);

	// --- World Representation Mesh (Shadow Casting) ---
	// Invisible to owning camera, but casts shadows and appears in reflections.
	// Follows the main mesh animation via SetLeaderPoseComponent.
	WorldRepresentationMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WorldRepMesh"));
	WorldRepresentationMesh->SetupAttachment(GetMesh());
	WorldRepresentationMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::WorldSpaceRepresentation);
	WorldRepresentationMesh->SetLeaderPoseComponent(GetMesh());

	// --- Plan 03 Components ---
	StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));
	HeadBobComponent = CreateDefaultSubobject<UHeadBobComponent>(TEXT("HeadBobComponent"));
	LeanComponent = CreateDefaultSubobject<ULeanComponent>(TEXT("LeanComponent"));

	// --- Plan 04/05 Components ---
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
	FootstepComponent = CreateDefaultSubobject<UFootstepComponent>(TEXT("FootstepComponent"));

	// --- Character Movement Component Configuration ---
	// Grounded, deliberate horror movement. No jump, no air control.
	UCharacterMovementComponent* CMC = GetCharacterMovement();

	// Walking speeds
	CMC->MaxWalkSpeed = WalkSpeed;            // 250 cm/s - deliberate walk pace
	CMC->MaxWalkSpeedCrouched = 130.0f;       // Significantly slower crouch

	// Sprint is handled dynamically via StartSprint/StopSprint modifying MaxWalkSpeed.

	// Acceleration/deceleration for weighty, grounded feel
	CMC->MaxAcceleration = 1200.0f;           // Moderate acceleration (not instant)
	CMC->BrakingDecelerationWalking = 1400.0f; // Moderate deceleration
	CMC->GroundFriction = 6.0f;               // Moderate friction for slight slide feel
	CMC->BrakingFrictionFactor = 1.0f;        // Lower than default 2.0 for heavier feel

	// Crouch configuration
	CMC->NavAgentProps.bCanCrouch = true;
	CMC->SetCrouchedHalfHeight(44.0f);

	// No jump -- grounded game
	CMC->JumpZVelocity = 0.0f;
	CMC->NavAgentProps.bCanJump = false;

	// No air control
	CMC->AirControl = 0.0f;

	// --- Controller Rotation ---
	// Yaw controlled by controller (mouse horizontal turns character).
	// Pitch and roll handled by camera component via bUsePawnControlRotation.
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
}

void ASereneCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		if (ASereneHUD* HUD = Cast<ASereneHUD>(PC->GetHUD()))
		{
			HUD->BindToCharacter(this);
		}
	}
}

void ASereneCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Bind stamina depletion to force stop sprint
	if (StaminaComponent)
	{
		StaminaComponent->OnStaminaDepleted.AddDynamic(this, &ASereneCharacter::OnStaminaDepleted);
	}

	// Read GameInstance head-bob accessibility setting
	if (HeadBobComponent)
	{
		if (const USereneGameInstance* GI = Cast<USereneGameInstance>(GetGameInstance()))
		{
			HeadBobComponent->SetEnabled(GI->bHeadBobEnabled);
			UE_LOG(LogSerene, Log, TEXT("ASereneCharacter::BeginPlay - HeadBob enabled: %s"),
				GI->bHeadBobEnabled ? TEXT("true") : TEXT("false"));
		}
	}

	UE_LOG(LogSerene, Log, TEXT("ASereneCharacter::BeginPlay - Character initialized. WalkSpeed=%.0f, SprintSpeed=%.0f"),
		WalkSpeed, SprintSpeed);

	// Log all 5 component creation status
	UE_LOG(LogSerene, Log, TEXT("ASereneCharacter::BeginPlay - Components: Stamina=%s, HeadBob=%s, Lean=%s, Interaction=%s, Footstep=%s"),
		StaminaComponent ? TEXT("OK") : TEXT("MISSING"),
		HeadBobComponent ? TEXT("OK") : TEXT("MISSING"),
		LeanComponent ? TEXT("OK") : TEXT("MISSING"),
		InteractionComponent ? TEXT("OK") : TEXT("MISSING"),
		FootstepComponent ? TEXT("OK") : TEXT("MISSING"));
}

void ASereneCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!FirstPersonCamera || !GetMesh())
	{
		return;
	}

	// --- Crouch Camera Offset ---
	const float TargetCrouchOffset = bIsCrouching ? CrouchCameraZOffset : 0.0f;
	CurrentCrouchCameraOffset = FMath::FInterpTo(CurrentCrouchCameraOffset, TargetCrouchOffset, DeltaTime, CrouchCameraInterpSpeed);

	// --- Camera Offset Aggregation ---
	// All offsets are in actor space (X=forward, Y=right, Z=up).
	FVector ActorSpaceOffset(0.0f, 0.0f, CurrentCrouchCameraOffset);
	float CameraRoll = 0.0f;

	if (HeadBobComponent)
	{
		ActorSpaceOffset += HeadBobComponent->GetCurrentOffset();
	}

	if (LeanComponent)
	{
		ActorSpaceOffset += LeanComponent->GetLeanOffset();
		CameraRoll += LeanComponent->GetLeanRoll();
	}

	// --- Position: head bone world location + actor-space offsets ---
	const FVector HeadWorldPos = GetMesh()->GetSocketLocation(FName(TEXT("head")));
	const FVector WorldOffset = GetActorRotation().RotateVector(ActorSpaceOffset);
	FirstPersonCamera->SetWorldLocation(HeadWorldPos + WorldOffset);

	// --- Rotation: controller rotation + lean roll ---
	FRotator CameraRotation = GetControlRotation();
	CameraRotation.Roll = CameraRoll;
	FirstPersonCamera->SetWorldRotation(CameraRotation);
}

void ASereneCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Input is handled on the PlayerController, not here.
	// This override is intentionally empty.
}

void ASereneCharacter::OnStaminaDepleted()
{
	UE_LOG(LogSerene, Log, TEXT("ASereneCharacter::OnStaminaDepleted - Forcing sprint stop due to stamina depletion."));
	StopSprint();
}

void ASereneCharacter::StartSprint()
{
	// Cannot sprint while crouching
	if (bIsCrouching)
	{
		UE_LOG(LogSerene, Verbose, TEXT("ASereneCharacter::StartSprint - Cannot sprint while crouching."));
		return;
	}

	// Only sprint if there is movement velocity
	if (GetVelocity().SizeSquared() <= 0.0f)
	{
		UE_LOG(LogSerene, Verbose, TEXT("ASereneCharacter::StartSprint - Cannot sprint with zero velocity."));
		return;
	}

	// Check stamina: exhaustion prevents sprinting
	if (StaminaComponent && StaminaComponent->IsExhausted())
	{
		UE_LOG(LogSerene, Verbose, TEXT("ASereneCharacter::StartSprint - Cannot sprint while exhausted."));
		return;
	}

	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;

	if (StaminaComponent)
	{
		StaminaComponent->SetSprinting(true);
	}

	UE_LOG(LogSerene, Verbose, TEXT("ASereneCharacter::StartSprint - Sprinting at %.0f cm/s."), SprintSpeed);
}

void ASereneCharacter::StopSprint()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	if (StaminaComponent)
	{
		StaminaComponent->SetSprinting(false);
	}

	UE_LOG(LogSerene, Verbose, TEXT("ASereneCharacter::StopSprint - Restored walk speed %.0f cm/s."), WalkSpeed);
}

void ASereneCharacter::StartCrouching()
{
	// If currently sprinting, stop sprint first (no crouch-sprint)
	if (bIsSprinting)
	{
		StopSprint();
	}

	Crouch();
	bIsCrouching = true;

	UE_LOG(LogSerene, Verbose, TEXT("ASereneCharacter::StartCrouching - Crouched."));
}

void ASereneCharacter::StopCrouching()
{
	UnCrouch();
	bIsCrouching = false;

	UE_LOG(LogSerene, Verbose, TEXT("ASereneCharacter::StopCrouching - Uncrouched."));
}
