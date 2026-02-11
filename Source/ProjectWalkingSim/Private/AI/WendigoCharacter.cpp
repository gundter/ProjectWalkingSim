// Copyright Null Lantern.

#include "AI/WendigoCharacter.h"
#include "AI/SuspicionComponent.h"
#include "AI/WendigoAIController.h"
#include "AI/MonsterAITypes.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AWendigoCharacter::AWendigoCharacter()
{
	// ---- Capsule: tall character (~260cm / ~8.5ft) ----
	GetCapsuleComponent()->InitCapsuleSize(45.0f, 130.0f);

	// ---- Character Movement: slow, deliberate pace ----
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = AIConstants::WendigoWalkSpeed;      // 150 cm/s
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.0, 120.0, 0.0);

	// ---- Rotation: driven by movement, not controller ----
	bUseControllerRotationYaw = false;

	// ---- AI Controller ----
	AIControllerClass = AWendigoAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// ---- Suspicion Component ----
	SuspicionComponent = CreateDefaultSubobject<USuspicionComponent>(
		TEXT("SuspicionComponent"));
}
