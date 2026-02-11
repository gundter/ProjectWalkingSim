// Copyright Null Lantern.

#include "Player/Components/HeadBobComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/SereneCharacter.h"
#include "Core/SereneLogChannels.h"

UHeadBobComponent::UHeadBobComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHeadBobComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		CurrentBobOffset = FVector::ZeroVector;
		return;
	}

	const float VelocityMagnitude = OwnerCharacter->GetVelocity().Size();
	const bool bIsMoving = VelocityMagnitude > 10.0f && bEnabled;

	// Smoothly interpolate bob alpha toward target (1 if moving + enabled, 0 otherwise)
	const float TargetAlpha = bIsMoving ? 1.0f : 0.0f;
	CurrentBobAlpha = FMath::FInterpTo(CurrentBobAlpha, TargetAlpha, DeltaTime, BobInterpSpeed);

	// If alpha is effectively zero, zero out the offset and skip computation
	if (CurrentBobAlpha < KINDA_SMALL_NUMBER)
	{
		CurrentBobOffset = FVector::ZeroVector;
		BobTimer = 0.0f;
		return;
	}

	// Accumulate timer (frame-rate independent -- Pitfall 6)
	BobTimer += DeltaTime;

	// Determine amplitude multiplier based on character state
	float AmplitudeMultiplier = 1.0f;
	const UCharacterMovementComponent* CMC = OwnerCharacter->GetCharacterMovement();
	if (CMC && CMC->IsCrouching())
	{
		AmplitudeMultiplier = CrouchBobMultiplier;
	}
	else if (const ASereneCharacter* SereneChar = Cast<ASereneCharacter>(OwnerCharacter))
	{
		if (SereneChar->GetIsSprinting())
		{
			AmplitudeMultiplier = SprintBobMultiplier;
		}
	}

	const float Amplitude = WalkBobAmplitude * AmplitudeMultiplier;

	// Vertical: full sine wave cycle per oscillation
	const float VerticalOffset = FMath::Sin(BobTimer * WalkBobFrequency * 2.0f * UE_PI)
		* Amplitude * CurrentBobAlpha;

	// Horizontal: half-frequency cosine for lateral sway (one sway per two vertical bobs)
	const float HorizontalOffset = FMath::Cos(BobTimer * WalkBobFrequency * UE_PI)
		* Amplitude * HorizontalBobRatio * CurrentBobAlpha;

	// Local space: X = forward (unused), Y = right (horizontal sway), Z = up (vertical bob)
	CurrentBobOffset = FVector(0.0f, HorizontalOffset, VerticalOffset);
}

void UHeadBobComponent::SetEnabled(bool bEnable)
{
	bEnabled = bEnable;

	UE_LOG(LogSerene, Log, TEXT("UHeadBobComponent::SetEnabled(%s)"),
		bEnable ? TEXT("true") : TEXT("false"));

	// When disabling, CurrentBobAlpha will naturally interp to 0 on next ticks
}

bool UHeadBobComponent::IsEnabled() const
{
	return bEnabled;
}

FVector UHeadBobComponent::GetCurrentOffset() const
{
	return CurrentBobOffset;
}
