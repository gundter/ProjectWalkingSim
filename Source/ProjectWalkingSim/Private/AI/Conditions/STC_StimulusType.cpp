// Copyright Null Lantern.

#include "AI/Conditions/STC_StimulusType.h"
#include "AIController.h"
#include "AI/WendigoCharacter.h"
#include "AI/SuspicionComponent.h"
#include "Core/SereneLogChannels.h"

bool FSTC_StimulusType::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(ControllerHandle);
	return true;
}

bool FSTC_StimulusType::TestCondition(FStateTreeExecutionContext& Context) const
{
	const AAIController& Controller = Context.GetExternalData(ControllerHandle);

	APawn* Pawn = Controller.GetPawn();
	const AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Pawn);
	if (!Wendigo)
	{
		UE_LOG(LogSerene, Warning, TEXT("STC_StimulusType: No Wendigo pawn"));
		return false;
	}

	const USuspicionComponent* Suspicion = Wendigo->GetSuspicionComponent();
	if (!Suspicion)
	{
		UE_LOG(LogSerene, Warning, TEXT("STC_StimulusType: No SuspicionComponent"));
		return false;
	}

	const EStimulusType LastType = Suspicion->GetLastStimulusType();
	const bool bResult = (LastType == RequiredType);

	return bInvertCondition ? !bResult : bResult;
}
