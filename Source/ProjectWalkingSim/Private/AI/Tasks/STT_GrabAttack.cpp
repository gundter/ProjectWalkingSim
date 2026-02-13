// Copyright Null Lantern.

#include "AI/Tasks/STT_GrabAttack.h"
#include "AIController.h"
#include "AI/WendigoCharacter.h"
#include "AI/MonsterAITypes.h"
#include "Core/SereneGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/SereneLogChannels.h"

bool FSTT_GrabAttack::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(ControllerHandle);
	return true;
}

EStateTreeRunStatus FSTT_GrabAttack::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController& Controller = Context.GetExternalData(ControllerHandle);

	// Reset instance state
	InstanceData.ElapsedTime = 0.0f;
	InstanceData.bInputDisabled = false;

	// Set Wendigo behavior state
	APawn* Pawn = Controller.GetPawn();
	AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Pawn);
	if (!Wendigo)
	{
		UE_LOG(LogSerene, Warning, TEXT("GrabAttack: No Wendigo pawn"));
		return EStateTreeRunStatus::Failed;
	}

	Wendigo->SetBehaviorState(EWendigoBehaviorState::GrabAttack);

	// Stop Wendigo movement
	Controller.StopMovement();

	// Get player references
	UWorld* World = Wendigo->GetWorld();
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);

	if (!PlayerPawn || !PlayerController)
	{
		UE_LOG(LogSerene, Warning, TEXT("GrabAttack: No player pawn or controller"));
		return EStateTreeRunStatus::Failed;
	}

	// Disable player input
	PlayerController->DisableInput(PlayerController);
	InstanceData.bInputDisabled = true;

	// Face the player
	Controller.SetFocus(PlayerPawn);

	UE_LOG(LogSerene, Log, TEXT("Wendigo grabbed player!"));
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_GrabAttack::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	InstanceData.ElapsedTime += DeltaTime;

	if (InstanceData.ElapsedTime >= GrabDuration)
	{
		// Trigger death via GameMode -- shows Game Over screen instead of restarting
		AAIController& Controller = Context.GetExternalData(ControllerHandle);
		APawn* Pawn = Controller.GetPawn();
		if (Pawn)
		{
			UWorld* World = Pawn->GetWorld();
			if (ASereneGameMode* GM = Cast<ASereneGameMode>(World->GetAuthGameMode()))
			{
				GM->OnPlayerDeath();
			}
		}
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FSTT_GrabAttack::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	AAIController& Controller = Context.GetExternalData(ControllerHandle);

	APawn* Pawn = Controller.GetPawn();

	// Safety: re-enable player input if we disabled it (handles interrupted transitions)
	if (InstanceData.bInputDisabled)
	{
		if (Pawn)
		{
			UWorld* World = Pawn->GetWorld();
			APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
			if (PlayerController)
			{
				PlayerController->EnableInput(PlayerController);
			}
		}
		InstanceData.bInputDisabled = false;
	}

	// Clear focus
	Controller.ClearFocus(EAIFocusPriority::Gameplay);

	// Restore patrol walk speed
	AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Pawn);
	if (Wendigo)
	{
		if (UCharacterMovementComponent* MoveComp = Wendigo->GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = AIConstants::WendigoWalkSpeed;
		}
	}
}
