// Copyright Null Lantern.

#include "Player/Components/StaminaComponent.h"

#include "Core/SereneLogChannels.h"

UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	CurrentStamina = MaxStamina;
}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsSprinting && CurrentStamina > 0.0f)
	{
		// Drain stamina while sprinting
		CurrentStamina = FMath::Max(0.0f, CurrentStamina - DrainRate * DeltaTime);

		if (CurrentStamina <= 0.0f)
		{
			bIsExhausted = true;
			UE_LOG(LogSerene, Log, TEXT("UStaminaComponent: Stamina depleted. Player is exhausted."));
			OnStaminaDepleted.Broadcast();
		}
	}
	else if (!bIsSprinting)
	{
		// Accumulate delay timer
		TimeSinceStoppedSprinting += DeltaTime;

		if (TimeSinceStoppedSprinting >= RegenDelay && CurrentStamina < MaxStamina)
		{
			// Regenerate stamina after delay
			CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + RegenRate * DeltaTime);

			// Clear exhaustion once we pass the threshold
			if (bIsExhausted && GetStaminaPercent() * 100.0f >= ExhaustionThreshold)
			{
				bIsExhausted = false;
				UE_LOG(LogSerene, Log, TEXT("UStaminaComponent: Stamina recovered past exhaustion threshold (%.0f%%). Sprint available."),
					ExhaustionThreshold);
			}

			if (CurrentStamina >= MaxStamina)
			{
				UE_LOG(LogSerene, Verbose, TEXT("UStaminaComponent: Stamina full."));
				OnStaminaFull.Broadcast();
			}
		}
	}

	// Broadcast stamina change only when the ratio actually changed
	const float CurrentPercent = GetStaminaPercent();
	if (!FMath::IsNearlyEqual(CurrentPercent, PreviousStaminaPercent, KINDA_SMALL_NUMBER))
	{
		OnStaminaChanged.Broadcast(CurrentPercent);
		PreviousStaminaPercent = CurrentPercent;
	}
}

void UStaminaComponent::SetSprinting(bool bSprinting)
{
	if (bSprinting)
	{
		// Reject sprint if exhausted (must recover past threshold)
		if (bIsExhausted)
		{
			UE_LOG(LogSerene, Verbose, TEXT("UStaminaComponent::SetSprinting(true) - Rejected: exhausted. Need %.0f%% stamina, have %.0f%%."),
				ExhaustionThreshold, GetStaminaPercent() * 100.0f);
			return;
		}

		bIsSprinting = true;
		TimeSinceStoppedSprinting = 0.0f;

		UE_LOG(LogSerene, Verbose, TEXT("UStaminaComponent::SetSprinting(true) - Sprint started. Stamina: %.1f/%.1f"),
			CurrentStamina, MaxStamina);
	}
	else
	{
		bIsSprinting = false;
		// TimeSinceStoppedSprinting will start accumulating in TickComponent

		UE_LOG(LogSerene, Verbose, TEXT("UStaminaComponent::SetSprinting(false) - Sprint stopped. Stamina: %.1f/%.1f. Regen in %.1fs."),
			CurrentStamina, MaxStamina, RegenDelay);
	}
}

float UStaminaComponent::GetStaminaPercent() const
{
	if (MaxStamina <= 0.0f)
	{
		return 0.0f;
	}
	return CurrentStamina / MaxStamina;
}

bool UStaminaComponent::IsExhausted() const
{
	return bIsExhausted;
}

bool UStaminaComponent::IsSprinting() const
{
	return bIsSprinting;
}
