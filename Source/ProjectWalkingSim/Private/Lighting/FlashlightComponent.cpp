// Copyright Null Lantern.

#include "Lighting/FlashlightComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "Core/SereneLogChannels.h"

UFlashlightComponent::UFlashlightComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFlashlightComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogSerene, Warning, TEXT("FlashlightComponent: No owner actor."));
		return;
	}

	// Find the camera component on the owning actor.
	UCameraComponent* Camera = Owner->FindComponentByClass<UCameraComponent>();
	if (!Camera)
	{
		UE_LOG(LogSerene, Warning, TEXT("FlashlightComponent: No UCameraComponent found on %s. Flashlight will not be created."), *Owner->GetName());
		return;
	}

	// Create the spotlight at runtime. Must use NewObject + RegisterComponent,
	// NOT CreateDefaultSubobject (which only works in actor constructors).
	SpotLight = NewObject<USpotLightComponent>(Owner, TEXT("FlashlightSpotLight"));
	if (!SpotLight)
	{
		UE_LOG(LogSerene, Error, TEXT("FlashlightComponent: Failed to create USpotLightComponent."));
		return;
	}

	SpotLight->SetupAttachment(Camera);

	// Apply beam configuration from editable properties.
	SpotLight->SetInnerConeAngle(InnerConeAngle);
	SpotLight->SetOuterConeAngle(OuterConeAngle);
	SpotLight->SetIntensity(Intensity);
	SpotLight->SetAttenuationRadius(AttenuationRadius);
	SpotLight->SetLightColor(LightColor);
	SpotLight->SetVolumetricScatteringIntensity(VolumetricScatteringIntensity);
	SpotLight->SetCastShadows(true);
	SpotLight->SetMobility(EComponentMobility::Movable);

	SpotLight->RegisterComponent();

	UE_LOG(LogSerene, Verbose, TEXT("FlashlightComponent: Spotlight created and attached to camera on %s."), *Owner->GetName());
}
