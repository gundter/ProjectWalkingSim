// Copyright Null Lantern.

#include "Player/SereneCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/SereneLogChannels.h"

ASereneCharacter::ASereneCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// --- First Person Camera ---
	// Attach to head bone for true first-person view that follows animations.
	// If no mesh is assigned yet, falls back to root component with a warning.
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));

	if (GetMesh())
	{
		FirstPersonCamera->SetupAttachment(GetMesh(), FName(TEXT("head")));
	}
	else
	{
		FirstPersonCamera->SetupAttachment(GetRootComponent());
		UE_LOG(LogSerene, Warning, TEXT("ASereneCharacter: No skeletal mesh available. Camera attached to RootComponent instead of head bone."));
	}

	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->bUseFirstPersonFieldOfView = true;
	FirstPersonCamera->bUseFirstPersonScale = true;

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
	CMC->CrouchedHalfHeight = 44.0f;

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

void ASereneCharacter::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogSerene, Log, TEXT("ASereneCharacter::BeginPlay - Character initialized. WalkSpeed=%.0f, SprintSpeed=%.0f"),
		WalkSpeed, SprintSpeed);
}

void ASereneCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Input is handled on the PlayerController, not here.
	// This override is intentionally empty.
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

	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;

	UE_LOG(LogSerene, Verbose, TEXT("ASereneCharacter::StartSprint - Sprinting at %.0f cm/s."), SprintSpeed);
}

void ASereneCharacter::StopSprint()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

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
