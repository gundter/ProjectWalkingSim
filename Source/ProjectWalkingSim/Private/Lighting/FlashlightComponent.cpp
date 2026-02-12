// Copyright Null Lantern.

#include "Lighting/FlashlightComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "AI/WendigoCharacter.h"
#include "AI/SuspicionComponent.h"
#include "Audio/AudioConstants.h"
#include "Kismet/GameplayStatics.h"
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
	SpotLight->bAllowMegaLights = bAllowMegaLights;
	SpotLight->SetCastShadows(true);
	SpotLight->SetMobility(EComponentMobility::Movable);

	SpotLight->RegisterComponent();

	UE_LOG(LogSerene, Verbose, TEXT("FlashlightComponent: Spotlight created and attached to camera on %s."), *Owner->GetName());

	// Start periodic Wendigo detection
	GetWorld()->GetTimerManager().SetTimer(
		DetectionTimerHandle, this,
		&UFlashlightComponent::FlashlightDetectionTrace,
		DetectionInterval, true, DetectionInterval);
}

void UFlashlightComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DetectionTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void UFlashlightComponent::FlashlightDetectionTrace()
{
	if (!SpotLight)
	{
		return;
	}

	// Lazy-cache the Wendigo
	if (!CachedWendigo.IsValid())
	{
		FindWendigo();
	}

	if (!CachedWendigo.IsValid())
	{
		return;
	}

	const FVector FlashlightLocation = SpotLight->GetComponentLocation();
	const FVector FlashlightForward = SpotLight->GetForwardVector();
	const FVector WendigoLocation = CachedWendigo->GetActorLocation();

	// Distance check
	const FVector ToWendigo = WendigoLocation - FlashlightLocation;
	const float Distance = ToWendigo.Size();

	if (Distance > DetectionRange)
	{
		return;
	}

	// Cone angle check
	const FVector DirectionToWendigo = ToWendigo.GetSafeNormal();
	const float DotProduct = FVector::DotProduct(FlashlightForward, DirectionToWendigo);
	const float AngleRadians = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));
	const float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	if (AngleDegrees > DetectionHalfAngle)
	{
		return;
	}

	// Line-of-sight occlusion check (walls between flashlight and Wendigo)
	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(CachedWendigo.Get());

	if (GetWorld()->LineTraceSingleByChannel(
		Hit, FlashlightLocation, WendigoLocation,
		ECC_Visibility, QueryParams))
	{
		// Something between flashlight and Wendigo -- blocked
		return;
	}

	// All checks passed: report flashlight detection to suspicion system
	USuspicionComponent* Suspicion = CachedWendigo->GetSuspicionComponent();
	if (Suspicion)
	{
		Suspicion->SetStimulusLocation(GetOwner()->GetActorLocation());
		Suspicion->ProcessSightStimulus(AudioConstants::FlashlightSuspicionScore, DetectionInterval);

		UE_LOG(LogSerene, Verbose,
			TEXT("FlashlightComponent: Detected Wendigo at distance %.0f cm, angle %.1f deg, suspicion score %.2f"),
			Distance, AngleDegrees, AudioConstants::FlashlightSuspicionScore);
	}
}

void UFlashlightComponent::FindWendigo()
{
	TArray<AActor*> Wendigos;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWendigoCharacter::StaticClass(), Wendigos);

	if (Wendigos.Num() > 0)
	{
		CachedWendigo = Cast<AWendigoCharacter>(Wendigos[0]);
	}
}
