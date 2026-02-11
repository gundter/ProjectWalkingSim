// Copyright Null Lantern.

#include "AI/Conditions/STC_SuspicionLevel.h"
#include "AIController.h"
#include "AI/WendigoCharacter.h"
#include "AI/SuspicionComponent.h"
#include "Core/SereneLogChannels.h"

bool FSTC_SuspicionLevel::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(ControllerHandle);
	return true;
}

bool FSTC_SuspicionLevel::TestCondition(FStateTreeExecutionContext& Context) const
{
	const AAIController& Controller = Context.GetExternalData(ControllerHandle);

	APawn* Pawn = Controller.GetPawn();
	const AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Pawn);
	if (!Wendigo)
	{
		UE_LOG(LogSerene, Warning, TEXT("STC_SuspicionLevel: No Wendigo pawn"));
		return false;
	}

	const USuspicionComponent* Suspicion = Wendigo->GetSuspicionComponent();
	if (!Suspicion)
	{
		UE_LOG(LogSerene, Warning, TEXT("STC_SuspicionLevel: No SuspicionComponent"));
		return false;
	}

	const EAlertLevel CurrentLevel = Suspicion->GetAlertLevel();

	// EAlertLevel enum is ordered: Patrol(0) < Suspicious(1) < Alert(2)
	const bool bMeetsLevel = (CurrentLevel >= RequiredLevel);

	return bInvertCondition ? !bMeetsLevel : bMeetsLevel;
}
