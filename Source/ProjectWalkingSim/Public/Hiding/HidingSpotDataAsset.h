// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "HidingSpotDataAsset.generated.h"

class UAnimMontage;
class UMaterialInterface;

/**
 * Per-type configuration for hiding spots.
 * Create one data asset per hiding spot type (locker, closet, under bed).
 * Stores montages, camera limits, blend times, and visual overlay settings.
 */
UCLASS(BlueprintType)
class PROJECTWALKINGSIM_API UHidingSpotDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHidingSpotDataAsset();

	// --- Animation ---

	/** Montage played when the player enters this hiding spot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> EntryMontage;

	/** Montage played when the player exits this hiding spot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ExitMontage;

	// --- Camera ---

	/** Time (seconds) to blend from player camera to the hiding camera. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0.0"))
	float CameraBlendInTime;

	/** Time (seconds) to blend from hiding camera back to player camera. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0.0"))
	float CameraBlendOutTime;

	/** Minimum pitch (degrees) the player can look while hidden. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float ViewPitchMin;

	/** Maximum pitch (degrees) the player can look while hidden. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float ViewPitchMax;

	/** Minimum yaw offset (degrees) from spot forward direction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float ViewYawMin;

	/** Maximum yaw offset (degrees) from spot forward direction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float ViewYawMax;

	// --- Visual ---

	/** Post-process material for peek overlay (locker slats, closet crack, under-bed framing). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UMaterialInterface> PeekOverlayMaterial;

	// --- Interaction ---

	/** Text shown when player can enter this spot (e.g. "Hide in Locker"). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	FText InteractionText;

	/** Text shown when player can exit this spot (e.g. "Exit Locker"). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	FText ExitText;

	// --- Classification ---

	/** Gameplay tag identifying this spot type (HidingSpot.Locker, HidingSpot.Closet, etc.). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classification")
	FGameplayTag SpotTypeTag;

	// --- Gameplay ---

	/** How much this spot reduces visibility score while hidden (0.0 = no change, 1.0 = invisible). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HidingVisibilityReduction;
};
