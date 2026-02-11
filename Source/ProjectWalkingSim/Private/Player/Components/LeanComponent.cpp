// Copyright Null Lantern.

#include "Player/Components/LeanComponent.h"

#include "Core/SereneLogChannels.h"

ULeanComponent::ULeanComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void ULeanComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Determine target lean direction
	float TargetLean = 0.0f;
	if (bLeaningLeft)
	{
		TargetLean = -1.0f;
	}
	else if (bLeaningRight)
	{
		TargetLean = 1.0f;
	}

	// Smooth interpolation toward target
	CurrentLeanAlpha = FMath::FInterpTo(CurrentLeanAlpha, TargetLean, DeltaTime, LeanSpeed);

	// Disable tick when we've reached the target and no input is held
	if (!bLeaningLeft && !bLeaningRight && FMath::IsNearlyEqual(CurrentLeanAlpha, TargetLean, KINDA_SMALL_NUMBER))
	{
		CurrentLeanAlpha = TargetLean; // Snap to exact value
		SetComponentTickEnabled(false);
	}
}

void ULeanComponent::SetLeanLeft(bool bLean)
{
	bLeaningLeft = bLean;
	SetComponentTickEnabled(true);

	UE_LOG(LogSerene, Verbose, TEXT("ULeanComponent::SetLeanLeft(%s)"),
		bLean ? TEXT("true") : TEXT("false"));
}

void ULeanComponent::SetLeanRight(bool bLean)
{
	bLeaningRight = bLean;
	SetComponentTickEnabled(true);

	UE_LOG(LogSerene, Verbose, TEXT("ULeanComponent::SetLeanRight(%s)"),
		bLean ? TEXT("true") : TEXT("false"));
}

bool ULeanComponent::IsLeaning() const
{
	return !FMath::IsNearlyZero(CurrentLeanAlpha, KINDA_SMALL_NUMBER);
}

float ULeanComponent::GetLeanAlpha() const
{
	return CurrentLeanAlpha;
}

FVector ULeanComponent::GetLeanOffset() const
{
	// Local space: Y = right vector. Negative Y = left, positive Y = right.
	return FVector(0.0f, CurrentLeanAlpha * LeanDistance, 0.0f);
}

float ULeanComponent::GetLeanRoll() const
{
	return CurrentLeanAlpha * LeanRollAngle;
}
