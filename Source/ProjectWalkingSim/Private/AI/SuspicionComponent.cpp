// Copyright Null Lantern.

#include "AI/SuspicionComponent.h"
#include "Core/SereneLogChannels.h"

USuspicionComponent::USuspicionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USuspicionComponent::ProcessSightStimulus(float PlayerVisibilityScore, float DeltaTime)
{
	// Below threshold: player is invisible to this AI
	if (PlayerVisibilityScore < VisibilityThreshold)
	{
		return;
	}

	// Normalize effective visibility to 0-1 range above threshold
	const float EffectiveVisibility = (PlayerVisibilityScore - VisibilityThreshold)
		/ (1.0f - VisibilityThreshold);

	// Accumulate suspicion scaled by visibility
	const float SuspicionGain = BaseSuspicionRate * EffectiveVisibility * DeltaTime;
	CurrentSuspicion = FMath::Clamp(CurrentSuspicion + SuspicionGain, 0.0f, 1.0f);

	UpdateAlertLevel();
}

void USuspicionComponent::ProcessHearingStimulus(const FVector& StimulusLocation)
{
	// Record stimulus location for investigation
	LastKnownStimulusLocation = StimulusLocation;
	bHasStimulusLocation = true;

	// Apply fixed hearing suspicion bump
	CurrentSuspicion = FMath::Clamp(CurrentSuspicion + HearingSuspicionBump, 0.0f, 1.0f);

	UpdateAlertLevel();
}

void USuspicionComponent::DecaySuspicion(float DeltaTime)
{
	if (CurrentSuspicion <= 0.0f)
	{
		return;
	}

	CurrentSuspicion = FMath::Max(0.0f, CurrentSuspicion - SuspicionDecayRate * DeltaTime);

	UpdateAlertLevel();
}

void USuspicionComponent::ClearStimulusLocation()
{
	LastKnownStimulusLocation = FVector::ZeroVector;
	bHasStimulusLocation = false;
}

void USuspicionComponent::ResetSuspicion()
{
	CurrentSuspicion = 0.0f;
	bHasStimulusLocation = false;
	LastKnownStimulusLocation = FVector::ZeroVector;

	const EAlertLevel PreviousLevel = CurrentAlertLevel;
	CurrentAlertLevel = EAlertLevel::Patrol;

	if (PreviousLevel != EAlertLevel::Patrol)
	{
		UE_LOG(LogSerene, Log, TEXT("SuspicionComponent [%s]: Alert level reset to Patrol"),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
		OnAlertLevelChanged.Broadcast(EAlertLevel::Patrol);
	}
}

void USuspicionComponent::UpdateAlertLevel()
{
	EAlertLevel NewLevel;

	if (CurrentSuspicion >= AlertThreshold)
	{
		NewLevel = EAlertLevel::Alert;
	}
	else if (CurrentSuspicion >= SuspiciousThreshold)
	{
		NewLevel = EAlertLevel::Suspicious;
	}
	else
	{
		NewLevel = EAlertLevel::Patrol;
	}

	if (NewLevel != CurrentAlertLevel)
	{
		const EAlertLevel PreviousLevel = CurrentAlertLevel;
		CurrentAlertLevel = NewLevel;

		UE_LOG(LogSerene, Log, TEXT("SuspicionComponent [%s]: Alert level changed %d -> %d (Suspicion: %.3f)"),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
			static_cast<uint8>(PreviousLevel),
			static_cast<uint8>(NewLevel),
			CurrentSuspicion);

		OnAlertLevelChanged.Broadcast(NewLevel);
	}
}
