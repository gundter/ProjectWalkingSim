// Copyright Null Lantern.

#include "AI/WendigoCharacter.h"
#include "AI/SuspicionComponent.h"
#include "AI/WendigoAIController.h"
#include "AI/MonsterAITypes.h"
#include "Core/SereneLogChannels.h"
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

void AWendigoCharacter::SetBehaviorState(EWendigoBehaviorState NewState)
{
	if (NewState != BehaviorState)
	{
		const EWendigoBehaviorState PreviousState = BehaviorState;
		BehaviorState = NewState;

		UE_LOG(LogSerene, Log, TEXT("WendigoCharacter [%s]: Behavior state changed %d -> %d"),
			*GetName(),
			static_cast<uint8>(PreviousState),
			static_cast<uint8>(NewState));

		OnBehaviorStateChanged.Broadcast(NewState);
	}
}

void AWendigoCharacter::SetLastKnownPlayerLocation(const FVector& Location)
{
	LastKnownPlayerLocation = Location;
	bHasLastKnownPlayerLocation = true;
}

void AWendigoCharacter::ClearLastKnownPlayerLocation()
{
	LastKnownPlayerLocation = FVector::ZeroVector;
	bHasLastKnownPlayerLocation = false;
}

void AWendigoCharacter::SetWitnessedHidingSpot(AActor* Spot)
{
	WitnessedHidingSpot = Spot;
}

void AWendigoCharacter::ClearWitnessedHidingSpot()
{
	WitnessedHidingSpot = nullptr;
}
