// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SereneCharacter.generated.h"

// Components created in Plans 03-05
#include "Player/Components/StaminaComponent.h"
#include "Player/Components/HeadBobComponent.h"
#include "Player/Components/LeanComponent.h"

class UInteractionComponent;   // Created in Plan 04
class UFootstepComponent;      // Created in Plan 05

class UCameraComponent;
class USkeletalMeshComponent;

/**
 * First-person player character for The Juniper Tree.
 *
 * Uses UE5 First Person Rendering for full-body visibility (arms, torso, legs, feet)
 * with a WorldRepresentationMesh for shadow casting. Camera is attached to the head
 * bone of the skeletal mesh. Movement is grounded and deliberate (no jump).
 *
 * Attached components (all 5 created in constructor):
 * - StaminaComponent (Plan 03): Stamina drain/regen during sprint
 * - HeadBobComponent (Plan 03): Procedural sine-wave camera bob
 * - LeanComponent (Plan 03): Camera-only lean (Q/E)
 * - InteractionComponent (Plan 04): Line trace interaction detection
 * - FootstepComponent (Plan 05): Surface-dependent footstep detection
 *
 * Camera offset aggregation: Tick() reads offsets from HeadBobComponent and
 * LeanComponent, sums them, and applies the combined result to the camera.
 * Components compute offsets only -- they never directly modify the camera.
 */
UCLASS()
class PROJECTWALKINGSIM_API ASereneCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASereneCharacter();

	// --- Movement State Queries ---

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool GetIsSprinting() const { return bIsSprinting; }

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool GetIsCrouching() const { return bIsCrouching; }

	// --- Movement Actions (called by PlayerController) ---

	/** Begin sprinting. Only if not crouching and has movement velocity. */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSprint();

	/** Stop sprinting and restore walk speed. */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSprint();

	/** Begin crouching. Stops sprint if active. Wraps ACharacter::Crouch(). */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartCrouching();

	/** Stop crouching. Wraps ACharacter::UnCrouch(). */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopCrouching();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// --- Camera ---

	/** First-person camera attached to head bone. Uses FP FOV and FP scale. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	// --- Mesh ---

	/**
	 * Shadow/reflection mesh. Invisible to the owning camera but casts shadows
	 * and appears in reflections. Follows the main mesh via SetLeaderPoseComponent.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<USkeletalMeshComponent> WorldRepresentationMesh;

	// --- Components ---

	/** Stamina drain/regen with exhaustion state. Drains during sprint. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaminaComponent> StaminaComponent;

	/** Procedural sine-wave camera bob. Scales with speed, toggleable. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHeadBobComponent> HeadBobComponent;

	/** Camera-only lean mechanic. Q/E for peeking around corners. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<ULeanComponent> LeanComponent;

	// --- Plan 04/05 Components ---

	/** Per-tick camera line trace for detecting IInteractable actors. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInteractionComponent> InteractionComponent;

	/** Surface detection via downward trace. Broadcasts footstep events. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFootstepComponent> FootstepComponent;

	// --- Movement Configuration ---

	/** Base walking speed (cm/s). Deliberate pace for horror atmosphere. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 250.0f;

	/** Sprint speed (cm/s). ~2x walk speed for urgent bursts. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed = 500.0f;

	// --- Movement State ---

	/** True while the player is holding sprint and actively sprinting. */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;

	/**
	 * Tracked separately from CMC crouch state for component notification purposes.
	 * Other components (StaminaComponent, FootstepComponent) read this directly.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsCrouching = false;

private:
	// --- Camera Offset Aggregation ---

	/**
	 * Initial camera relative location stored in BeginPlay.
	 * HeadBob and Lean offsets are added to this base each tick.
	 */
	FVector BaseCameraLocation = FVector::ZeroVector;

	/** Callback bound to StaminaComponent::OnStaminaDepleted. Forces sprint stop. */
	UFUNCTION()
	void OnStaminaDepleted();
};
