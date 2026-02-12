// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlashlightComponent.generated.h"

class USpotLightComponent;

/**
 * Flashlight component for the player character.
 *
 * Creates and owns a USpotLightComponent at runtime, attached to the player's
 * camera. Produces a narrow-cone spotlight with deep shadows outside the beam,
 * forcing the player to actively scan their surroundings.
 *
 * Always-on for the demo (no toggle, no battery). The beam attracts the Wendigo
 * when shining directly at it -- detection logic is handled externally via
 * periodic traces in a later plan.
 *
 * Performance: A movable spotlight with shadow casting + Lumen GI costs 2-5ms GPU.
 * VolumetricScatteringIntensity is tunable to reduce ghosting artifacts from
 * temporal reprojection in volumetric fog.
 *
 * No Tick -- the spotlight moves with the camera attachment automatically.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTWALKINGSIM_API UFlashlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFlashlightComponent();

	/** Returns the spotlight component. May be null before BeginPlay. */
	UFUNCTION(BlueprintCallable, Category = "Flashlight")
	USpotLightComponent* GetSpotLight() const { return SpotLight; }

protected:
	virtual void BeginPlay() override;

	// --- Beam Configuration ---

	/** Inner cone angle in degrees. Narrow beam for horror atmosphere. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Beam")
	float InnerConeAngle = 14.0f;

	/** Outer cone angle in degrees. Penumbra falloff region. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Beam")
	float OuterConeAngle = 28.0f;

	/** Light intensity in lumens. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Beam")
	float Intensity = 8000.0f;

	/** Attenuation radius in cm (~15m range). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Beam")
	float AttenuationRadius = 1500.0f;

	/** Cool bluish-white LED color. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Beam")
	FLinearColor LightColor = FLinearColor(0.85f, 0.92f, 1.0f);

	/**
	 * Volumetric fog scattering intensity.
	 * Keep low (0.5-1.0) to avoid ghosting artifacts with fast camera movement.
	 * Set to 0 to disable volumetric scattering entirely.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flashlight|Beam", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float VolumetricScatteringIntensity = 1.0f;

private:
	/** The runtime spot light, created in BeginPlay and attached to the camera. */
	UPROPERTY()
	TObjectPtr<USpotLightComponent> SpotLight;
};
