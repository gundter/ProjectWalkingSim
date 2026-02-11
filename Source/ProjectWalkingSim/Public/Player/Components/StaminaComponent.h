// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StaminaComponent.generated.h"

/** Broadcast when stamina ratio changes. Param is 0.0 (empty) to 1.0 (full). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChanged, float, Percent);

/** Broadcast once when stamina reaches zero. C++ only -- no Blueprint binding needed. */
DECLARE_MULTICAST_DELEGATE(FOnStaminaDepleted);

/** Broadcast once when stamina reaches MaxStamina after being depleted. C++ only -- no Blueprint binding needed. */
DECLARE_MULTICAST_DELEGATE(FOnStaminaFull);

/**
 * Stamina system component for The Juniper Tree.
 *
 * Drains stamina while the owning character is sprinting and regenerates
 * after a configurable delay once sprinting stops. When stamina depletes,
 * broadcasts OnStaminaDepleted so the character can force-stop sprinting.
 * Prevents re-sprint until stamina recovers past ExhaustionThreshold.
 *
 * All tuning values are EditAnywhere for designer adjustment.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTWALKINGSIM_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStaminaComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// --- Public API ---

	/** Called by the character when sprint state changes. */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetSprinting(bool bSprinting);

	/** Returns current stamina as a 0-1 ratio. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina")
	float GetStaminaPercent() const;

	/** True when stamina is zero and hasn't recovered past ExhaustionThreshold. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina")
	bool IsExhausted() const;

	/** True when the component is actively draining stamina. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stamina")
	bool IsSprinting() const;

	// --- Delegates ---

	/** Fires each tick when stamina ratio actually changes (not at idle full). */
	UPROPERTY(BlueprintAssignable, Category = "Stamina")
	FOnStaminaChanged OnStaminaChanged;

	/** Fires once when stamina hits zero. */
	FOnStaminaDepleted OnStaminaDepleted;

	/** Fires once when stamina reaches max after depletion. */
	FOnStaminaFull OnStaminaFull;

protected:
	// --- Tuning ---

	/** Maximum stamina pool. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
	float MaxStamina = 100.0f;

	/** Stamina drained per second while sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
	float DrainRate = 20.0f;

	/** Stamina regenerated per second after regen delay elapses. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
	float RegenRate = 15.0f;

	/** Seconds after sprinting stops before regeneration begins. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
	float RegenDelay = 1.5f;

	/**
	 * Stamina percent (0-100) required before the player can sprint again
	 * after full depletion. Prevents sprint-tap spam at low stamina.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float ExhaustionThreshold = 20.0f;

private:
	/** Current stamina value. Starts at MaxStamina. */
	float CurrentStamina;

	/** True while SetSprinting(true) is active. */
	bool bIsSprinting = false;

	/** True after stamina hits zero until recovered past ExhaustionThreshold. */
	bool bIsExhausted = false;

	/** Accumulates time since the player stopped sprinting. */
	float TimeSinceStoppedSprinting = 0.0f;

	/** Cached stamina ratio from previous tick for change detection. */
	float PreviousStaminaPercent = 1.0f;
};
