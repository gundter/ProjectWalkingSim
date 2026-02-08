// Copyright Null Lantern.

#include "Player/Components/FootstepComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/SereneLogChannels.h"

UFootstepComponent::UFootstepComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFootstepComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// If using AnimNotify mode, the timer is disabled.
	if (bUseAnimNotify)
	{
		return;
	}

	const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	const UCharacterMovementComponent* CMC = OwnerCharacter->GetCharacterMovement();
	if (!CMC)
	{
		return;
	}

	// Only generate footsteps when on ground and moving.
	if (!CMC->IsMovingOnGround())
	{
		FootstepTimer = 0.0f;
		return;
	}

	const float SpeedSq = OwnerCharacter->GetVelocity().SizeSquared();
	constexpr float MinSpeedThreshold = 10.0f; // cm/s
	if (SpeedSq < MinSpeedThreshold * MinSpeedThreshold)
	{
		FootstepTimer = 0.0f;
		return;
	}

	// Determine current interval based on movement state.
	float CurrentInterval = FootstepInterval;
	const float CurrentSpeed = CMC->MaxWalkSpeed;

	if (CMC->IsCrouching())
	{
		CurrentInterval = FootstepInterval * CrouchIntervalMultiplier;
	}
	else if (CurrentSpeed > 400.0f) // Sprint detection: MaxWalkSpeed > 400 matches Plan 03 convention
	{
		CurrentInterval = FootstepInterval * SprintIntervalMultiplier;
	}

	// Accumulate timer.
	FootstepTimer += DeltaTime;

	if (FootstepTimer >= CurrentInterval)
	{
		FootstepTimer = 0.0f;

		// Foot location: bottom of the capsule (actor origin is capsule center).
		const FVector FootLocation = OwnerCharacter->GetActorLocation();
		PlayFootstepForSurface(FootLocation);
	}
}

void UFootstepComponent::PlayFootstepForSurface(const FVector& FootLocation)
{
	if (!GetWorld())
	{
		return;
	}

	const FVector TraceStart = FootLocation;
	const FVector TraceEnd = FootLocation - FVector(0.0f, 0.0f, TraceDistance);

	FCollisionQueryParams Params;
	Params.bReturnPhysicalMaterial = true; // CRITICAL: must request physical material
	Params.AddIgnoredActor(GetOwner());

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

	// Determine surface type from the trace result.
	EPhysicalSurface SurfaceType = EPhysicalSurface::SurfaceType_Default;

	if (bHit)
	{
		if (const UPhysicalMaterial* PhysMat = Hit.PhysMaterial.Get())
		{
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(PhysMat);
		}
	}

	LastSurfaceType = SurfaceType;

	// Determine volume based on movement state.
	float Volume = 1.0f;

	if (const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (const UCharacterMovementComponent* CMC = OwnerCharacter->GetCharacterMovement())
		{
			if (CMC->IsCrouching())
			{
				Volume = CrouchVolumeMultiplier;
			}
			else if (CMC->MaxWalkSpeed > 400.0f)
			{
				Volume = SprintVolumeMultiplier;
			}
		}
	}

	// Broadcast to listeners (Phase 6 audio, Phase 4/5 AI noise).
	OnFootstep.Broadcast(SurfaceType, Volume);

	UE_LOG(LogSerene, Log, TEXT("UFootstepComponent::PlayFootstepForSurface - Surface=%d, Volume=%.2f"),
		static_cast<int32>(SurfaceType), Volume);
}
