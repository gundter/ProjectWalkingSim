// Copyright Null Lantern.

#include "AI/NoiseReportingComponent.h"
#include "Player/Components/FootstepComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Core/SereneLogChannels.h"

UNoiseReportingComponent::UNoiseReportingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNoiseReportingComponent::BeginPlay()
{
	Super::BeginPlay();

	// Find FootstepComponent on owner and bind to its delegate
	UFootstepComponent* FootstepComp = GetOwner()->FindComponentByClass<UFootstepComponent>();
	if (FootstepComp)
	{
		FootstepComp->OnFootstep.AddDynamic(this, &UNoiseReportingComponent::HandleFootstep);
		UE_LOG(LogSerene, Log, TEXT("NoiseReportingComponent [%s]: Bound to FootstepComponent"),
			*GetOwner()->GetName());
	}
	else
	{
		UE_LOG(LogSerene, Warning, TEXT("NoiseReportingComponent [%s]: FootstepComponent not found on owner"),
			*GetOwner()->GetName());
	}
}

void UNoiseReportingComponent::HandleFootstep(EPhysicalSurface SurfaceType, float Volume)
{
	// Only report noise for sprint-volume footsteps
	if (Volume <= SprintVolumeThreshold)
	{
		return;
	}

	UAISense_Hearing::ReportNoiseEvent(
		GetWorld(),
		GetOwner()->GetActorLocation(),
		Volume,
		GetOwner(),
		SprintNoiseRange,
		FName()
	);

	UE_LOG(LogSerene, Log, TEXT("NoiseReportingComponent: Sprint noise reported at volume %.1f"), Volume);
}
