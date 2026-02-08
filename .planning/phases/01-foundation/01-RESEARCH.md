# Phase 1: Foundation - Research

**Researched:** 2026-02-08
**Domain:** Unreal Engine 5.7.2 First-Person Character Architecture (C++)
**Confidence:** HIGH

## Summary

Phase 1 establishes the core player character for a psychological horror game: a first-person controller with visible body, movement system (walk/crouch/sprint/stamina), head-bob camera with accessibility toggle, lean mechanic, interaction system, surface-dependent footsteps, and obstacle transitions. The project already has a UE 5.7.2 workspace with Enhanced Input configured, Lumen GI enabled, Virtual Shadow Maps active, ray tracing on, and static lighting disabled -- all prerequisites for UE5's native First Person Rendering system.

The standard approach uses `ACharacter` with `UCharacterMovementComponent` (not a custom CMC subclass for this scope), component-based architecture with separate `UActorComponent` subclasses for each concern (stamina, head-bob, interaction, lean, footsteps), UE5's native First Person Rendering pipeline (introduced in 5.5) for full-body visibility, and Enhanced Input with `UInputAction` / `UInputMappingContext` assets. Interfaces (`UINTERFACE` / `IInterface`) define contracts for future systems (IInteractable, IHideable, ISaveable).

**Primary recommendation:** Build a component-based character where each system (movement, stamina, head-bob, interaction, lean, footsteps) is a separate `UActorComponent` attached to a single `ACharacter` subclass. Use UE5's native First Person Rendering for body visibility. Use Enhanced Input for all input binding. Define core interfaces as UINTERFACE/IInterface pairs.

## Standard Stack

### Core

| Module | Purpose | Why Standard |
|--------|---------|--------------|
| `Engine` | ACharacter, UCharacterMovementComponent, UCameraComponent | Built-in character framework |
| `EnhancedInput` | UInputAction, UInputMappingContext, UEnhancedInputComponent | UE5 standard input system (already configured in project) |
| `UMG` | UUserWidget for HUD (stamina bar, interaction prompts) | Built-in UI framework |
| `PhysicsCore` | Physical materials, surface types | Footstep surface detection |
| `GameplayTags` | Native gameplay tags for state management | Type-safe tag system for interaction types, movement states |

### Supporting

| Module | Purpose | When to Use |
|--------|---------|-------------|
| `Slate`, `SlateCore` | Low-level UI (required by UMG in C++) | Must add to Build.cs for UMG C++ widgets |
| `AIModule` | EQS, AI Perception (future phases) | NOT needed in Phase 1 -- but interfaces should be designed knowing AI will query them |
| `NavigationSystem` | NavMesh (future phases) | NOT needed in Phase 1 |

### Build.cs Module Dependencies Required for Phase 1

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core", "CoreUObject", "Engine", "InputCore",
    "EnhancedInput",
    "UMG",
    "GameplayTags",
    "PhysicsCore"
});

PrivateDependencyModuleNames.AddRange(new string[] {
    "Slate", "SlateCore"
});
```

## Architecture Patterns

### Recommended Project Structure

```
Source/ProjectWalkingSim/
├── Public/
│   ├── Core/
│   │   ├── SereneGameMode.h
│   │   └── SereneGameInstance.h           // Game settings (head-bob toggle, crouch mode)
│   ├── Player/
│   │   ├── SereneCharacter.h              // ACharacter subclass, owns components
│   │   ├── SerenePlayerController.h       // APlayerController, input mapping context
│   │   ├── Components/
│   │   │   ├── StaminaComponent.h         // Stamina drain/regen/state
│   │   │   ├── HeadBobComponent.h         // Procedural head-bob with toggle
│   │   │   ├── InteractionComponent.h     // Line trace detection + prompt management
│   │   │   ├── LeanComponent.h            // Camera-only lean (Q/E)
│   │   │   └── FootstepComponent.h        // Surface detection + audio dispatch
│   │   └── HUD/
│   │       ├── SereneHUD.h                // AHUD subclass
│   │       ├── StaminaBarWidget.h         // UUserWidget for stamina display
│   │       └── InteractionPromptWidget.h  // UUserWidget for "E: Open" prompts
│   ├── Interaction/
│   │   ├── InteractableInterface.h        // IInteractable UINTERFACE
│   │   ├── HideableInterface.h            // IHideable UINTERFACE (stub for Phase 3)
│   │   ├── SaveableInterface.h            // ISaveable UINTERFACE (stub for Phase 7)
│   │   ├── InteractableBase.h             // AActor base with IInteractable
│   │   ├── DoorActor.h                    // Animated door
│   │   ├── PickupActor.h                  // Generic pickup (feeds into Phase 2 inventory)
│   │   ├── ReadableActor.h                // Notes/documents
│   │   └── DrawerActor.h                  // Openable drawer/cabinet
│   └── Tags/
│       └── SereneTags.h                   // Native gameplay tag declarations
├── Private/
│   ├── Core/
│   ├── Player/
│   │   ├── Components/
│   │   └── HUD/
│   ├── Interaction/
│   └── Tags/
│       └── SereneTags.cpp                 // Native gameplay tag definitions
```

### Pattern 1: Component-Based Character

**What:** Each gameplay system is a separate `UActorComponent` subclass attached to the character, rather than monolithic logic in the character class.

**When to use:** Always for this project. The character class orchestrates but delegates to components.

**Why:** Testable in isolation, reusable across actors, avoids god-class character, easy to add/remove features per phase.

```cpp
// SereneCharacter.h
UCLASS()
class ASereneCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASereneCharacter();

protected:
    // Camera
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<UCameraComponent> FirstPersonCamera;

    // Components (each owns its own logic)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaminaComponent> StaminaComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UHeadBobComponent> HeadBobComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UInteractionComponent> InteractionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<ULeanComponent> LeanComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UFootstepComponent> FootstepComponent;

    // Skeletal mesh is inherited from ACharacter (GetMesh())
    // Configure as FirstPerson primitive type in constructor

    // Optional: World Space Representation mesh for shadow casting
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
    TObjectPtr<USkeletalMeshComponent> WorldRepresentationMesh;

    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void BeginPlay() override;
};
```

### Pattern 2: UE5 Native First Person Rendering (Full Body)

**What:** UE5.5+ introduced a native First Person Rendering pipeline that morphs geometry in clip space to achieve custom FOV and anti-clipping for first-person meshes, without a separate render pass.

**When to use:** For rendering the player's visible body (arms, torso, legs, feet) in first person.

**Setup steps:**
1. Character's `SkeletalMeshComponent` (GetMesh()) -- set `FirstPersonPrimitiveType` to `EFirstPersonPrimitiveType::FirstPerson`
2. Camera component -- enable `bUseFirstPersonFieldOfView` and `bUseFirstPersonScale`
3. Create a second `USkeletalMeshComponent` as `WorldRepresentationMesh` -- set `FirstPersonPrimitiveType` to `EFirstPersonPrimitiveType::WorldSpaceRepresentation` for shadow casting
4. Both meshes share the same skeletal mesh and animation -- align foot positions

**Project prerequisites (already met):**
- `r.AllowStaticLighting=False` (confirmed in DefaultEngine.ini)
- `r.Shadow.Virtual.Enable=1` (confirmed -- Virtual Shadow Maps active)
- Deferred rendering (default, not forward/mobile)

**Critical constraint:** First Person self-shadow requires GBuffer bit marking. This is incompatible with forward rendering and mobile renderer. The project uses deferred rendering, so this is fine.

```cpp
// In ASereneCharacter constructor
FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("head")); // Attach to head bone
FirstPersonCamera->bUsePawnControlRotation = true;
FirstPersonCamera->bUseFirstPersonFieldOfView = true;
FirstPersonCamera->bUseFirstPersonScale = true;

// Main mesh: visible to player in first person
GetMesh()->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);

// Shadow/reflection mesh: invisible to owning camera, casts shadows
WorldRepresentationMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WorldRepMesh"));
WorldRepresentationMesh->SetupAttachment(GetMesh());
WorldRepresentationMesh->SetFirstPersonPrimitiveType(
    EFirstPersonPrimitiveType::WorldSpaceRepresentation);
WorldRepresentationMesh->SetLeaderPoseComponent(GetMesh()); // Follow main mesh animations
```

### Pattern 3: Enhanced Input Binding

**What:** Input Actions are data assets; Input Mapping Contexts group actions with key bindings. Contexts are added/removed at runtime for state changes.

**When to use:** All input. The project already has Enhanced Input configured as default.

```cpp
// SerenePlayerController.h
UCLASS()
class ASerenePlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    // Movement
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;       // Axis2D: WASD
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> LookAction;       // Axis2D: Mouse
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> SprintAction;     // Bool: Shift (hold)
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> CrouchAction;     // Bool: Ctrl (toggle/hold per setting)
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> InteractAction;   // Bool: E
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> LeanLeftAction;   // Bool: Q (hold)
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> LeanRightAction;  // Bool: E... wait, E is interact
    // Note: Lean = Q/E is a design choice. If E is interact, lean right
    // needs a different key or lean is only Q/E when NOT looking at interactable.
    // CONTEXT.md says "Lean left/right (Q/E)" -- so E is both interact and lean right.
    // Recommendation: Interact on E (press), Lean on Q/E (hold). Use triggers to differentiate.

    virtual void BeginPlay() override;
};
```

**Input action differentiation for E key (interact vs lean right):**
Use Enhanced Input triggers. Interact = `Pressed` trigger (tap). Lean = `Hold` trigger with a threshold (e.g., 0.2s). Or use separate actions with the same key but different trigger types in the mapping context. The mapping context priority system handles conflicts.

### Pattern 4: UINTERFACE Definition for Core Contracts

**What:** Define `IInteractable`, `IHideable`, `ISaveable` as UINTERFACE/IInterface pairs. Phase 1 fully implements IInteractable; IHideable and ISaveable are stubs that compile and can be implemented by later phases.

```cpp
// InteractableInterface.h
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
    GENERATED_BODY()
};

class PROJECTWALKINGSIM_API IInteractable
{
    GENERATED_BODY()

public:
    // Return the text shown in the interaction prompt (e.g., "Open", "Pick Up", "Read")
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    FText GetInteractionText() const;

    // Whether interaction is currently available
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    bool CanInteract(AActor* Interactor) const;

    // Perform the interaction
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnInteract(AActor* Interactor);

    // Called when the player's reticle enters/exits this interactable
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnFocusBegin(AActor* Interactor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnFocusEnd(AActor* Interactor);
};
```

```cpp
// HideableInterface.h (stub for Phase 3)
UINTERFACE(MinimalAPI, Blueprintable)
class UHideable : public UInterface
{
    GENERATED_BODY()
};

class PROJECTWALKINGSIM_API IHideable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
    bool CanHide(AActor* HidingActor) const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
    void OnEnterHiding(AActor* HidingActor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
    void OnExitHiding(AActor* HidingActor);
};
```

```cpp
// SaveableInterface.h (stub for Phase 7)
UINTERFACE(MinimalAPI, Blueprintable)
class USaveable : public UInterface
{
    GENERATED_BODY()
};

class PROJECTWALKINGSIM_API ISaveable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
    FString GetSaveId() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
    void WriteSaveData(/* future: save struct ref */);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
    void ReadSaveData(/* future: save struct ref */);
};
```

### Pattern 5: Procedural Head-Bob Component

**What:** Sine-wave-based camera offset applied per-tick, scaling with movement speed, with a toggle setting.

**When to use:** Always active when enabled; intensity scales with velocity magnitude.

```cpp
// HeadBobComponent.h
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UHeadBobComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHeadBobComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    void SetEnabled(bool bEnable);

protected:
    // Tuning parameters
    UPROPERTY(EditAnywhere, Category = "Head Bob")
    float WalkBobAmplitude = 1.5f;    // cm vertical displacement at walk speed

    UPROPERTY(EditAnywhere, Category = "Head Bob")
    float WalkBobFrequency = 8.0f;    // oscillations per second at walk speed

    UPROPERTY(EditAnywhere, Category = "Head Bob")
    float SprintBobMultiplier = 1.8f; // amplitude multiplier when sprinting

    UPROPERTY(EditAnywhere, Category = "Head Bob")
    float HorizontalBobRatio = 0.5f;  // horizontal sway as fraction of vertical

    UPROPERTY(EditAnywhere, Category = "Head Bob")
    float BobInterpSpeed = 6.0f;      // how fast bob fades in/out

private:
    bool bEnabled = true;
    float BobTimer = 0.0f;
    float CurrentBobAlpha = 0.0f;     // 0 when stationary, 1 when walking
    FVector CurrentBobOffset = FVector::ZeroVector;

    // In Tick: calculate offset using FMath::Sin(BobTimer * Frequency * 2PI)
    // Apply as camera relative offset (not world space)
};
```

### Pattern 6: Interaction Component (Line Trace + Screen Prompt)

**What:** Per-tick line trace from camera center, detect IInteractable actors, manage focus state and prompt display.

```cpp
// InteractionComponent.h
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    void TryInteract(); // Called by input binding

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableChanged, AActor*, Interactable);
    UPROPERTY(BlueprintAssignable)
    FOnInteractableChanged OnInteractableChanged;

protected:
    UPROPERTY(EditAnywhere, Category = "Interaction")
    float InteractionRange = 150.0f;  // ~1.5m as specified in context

    UPROPERTY(EditAnywhere, Category = "Interaction")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

private:
    TWeakObjectPtr<AActor> CurrentInteractable;

    void PerformTrace();
    void SetCurrentInteractable(AActor* NewInteractable);
};
```

### Anti-Patterns to Avoid

- **God Character class:** Do NOT put stamina logic, head-bob math, interaction traces, and lean calculations all inside ASereneCharacter. Use components.
- **Subclassing UCharacterMovementComponent for sprint:** Sprint is just changing `MaxWalkSpeed`. A custom CMC subclass is overkill for this scope. Use the default CMC and modify its properties from the character or a stamina component.
- **Dual mesh approach (old FPS style):** Do NOT create separate "arms-only" and "full body" meshes. UE5's native First Person Rendering handles full-body visibility with a single mesh + world space representation.
- **Camera shake for head-bob:** The `UCameraShakeBase` system is designed for one-off shakes (explosions, impacts), not continuous rhythmic bobbing. Use procedural sine-wave in a component instead.
- **Tick-based interaction prompts on interactable actors:** The InteractionComponent on the player should manage traces and UI, not each interactable actor polling for nearby players.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Input binding | Custom key polling | Enhanced Input (UInputAction + UInputMappingContext) | Already configured; supports remapping, triggers, modifiers |
| First-person body rendering | Custom render pass, dual mesh | UE5 First Person Rendering (`EFirstPersonPrimitiveType`) | Native engine feature since 5.5; handles FOV, clipping, shadows |
| Shadow casting for FP body | Manual shadow mesh management | World Space Representation primitive type + Virtual Shadow Maps | Automatic; engine handles visibility, shadow casting |
| Character movement | Custom movement code | UCharacterMovementComponent | Handles collision, gravity, slopes, step-up, crouch capsule resize |
| Crouch capsule resize | Manual capsule height changes | `ACharacter::Crouch()` / `UnCrouch()` + CMC `NavAgentProps` | Built-in with interpolation, half-height config |
| Physical surface detection | Custom material tagging system | `UPhysicalMaterial` + `EPhysicalSurface` enum in Project Settings | Engine-standard; integrates with collision results |
| Gameplay state tags | String comparisons for states | `FGameplayTag` with `UE_DECLARE_GAMEPLAY_TAG_EXTERN` | Type-safe, hierarchical, editor-integrated |
| Settings persistence | Custom file I/O for settings | `UGameUserSettings` subclass or `USaveGame` | Engine-standard save/load with ini serialization |

## Common Pitfalls

### Pitfall 1: Camera Attached to Root Instead of Head Bone

**What goes wrong:** Camera placed as child of capsule root or spring arm, causing disconnect between body animations and camera view. Body sways but camera stays rigid.
**Why it happens:** Third-person template habit. First-person camera should be on the head socket/bone.
**How to avoid:** Attach `UCameraComponent` to `GetMesh()` at the head bone socket. Set `bUsePawnControlRotation = true` on the camera.
**Warning signs:** Camera doesn't follow crouch animation smoothly, head-bob feels disconnected from body.

### Pitfall 2: First Person Rendering Without Static Lighting Disabled

**What goes wrong:** Advanced FP features (self-shadow, world space representation shadows, IsFirstPerson scene texture) silently fail because the GBuffer bit needed is used by static lighting.
**Why it happens:** `r.AllowStaticLighting` defaults to true in some templates.
**How to avoid:** Verify `r.AllowStaticLighting=False` in DefaultEngine.ini. This project already has it disabled.
**Warning signs:** Player body casts no shadow despite WorldSpaceRepresentation being set.

### Pitfall 3: Sprint as Boolean Toggle Instead of Continuous Input

**What goes wrong:** Sprint implemented as toggle (press once to sprint, press again to stop) when the design calls for hold-to-sprint. Or sprint overrides `MaxWalkSpeed` permanently.
**Why it happens:** Misusing Enhanced Input trigger types.
**How to avoid:** Use `ETriggerEvent::Triggered` (fires every tick while held) and `ETriggerEvent::Completed` (fires when released) for sprint input. Store original `MaxWalkSpeed` and restore on release.
**Warning signs:** Player stays at sprint speed after releasing shift.

### Pitfall 4: Stamina Regen Starts Immediately

**What goes wrong:** Stamina starts regenerating the frame after the player stops sprinting, feeling unrealistically instant.
**Why it happens:** Missing the "regen delay" requirement from context decisions.
**How to avoid:** Track time since last sprint ended. Only begin regen after delay (e.g., 1-2 seconds). Use a timer or accumulate delta time.
**Warning signs:** Stamina bar bounces rapidly during intermittent sprinting.

### Pitfall 5: Interaction Trace From Capsule Center Instead of Camera

**What goes wrong:** Interaction trace originates from character capsule center (waist height), not from where the player is actually looking. Objects above/below camera center register differently than expected.
**Why it happens:** Using `GetActorLocation()` instead of camera world position/rotation.
**How to avoid:** Get trace start/direction from `PlayerController->GetPlayerViewPoint()` or directly from the camera component's world transform.
**Warning signs:** Player can interact with objects behind walls, or can't interact with objects they're clearly looking at.

### Pitfall 6: Head-Bob Not Frame-Rate Independent

**What goes wrong:** Head-bob oscillation speed changes with frame rate. At 30 FPS the bob is slow; at 144 FPS it's frantic.
**Why it happens:** Using frame count or unscaled increment for the sine wave timer.
**How to avoid:** Accumulate `DeltaTime` into the bob timer: `BobTimer += DeltaTime`. The sine function then naturally scales with real time.
**Warning signs:** Bob feels different at different frame rates, or is too fast on high-refresh monitors.

### Pitfall 7: Crouch Mode Setting Not Persisted

**What goes wrong:** Player sets crouch to "hold" mode in settings, but it resets to "toggle" on restart.
**Why it happens:** Setting stored only in memory, not serialized.
**How to avoid:** Store accessibility/control settings in a `USaveGame` subclass or `UGameUserSettings` subclass. Load on startup.
**Warning signs:** Settings don't persist across sessions.

### Pitfall 8: E Key Conflict Between Interact and Lean Right

**What goes wrong:** Pressing E to interact also triggers lean right, or lean right blocks interaction.
**Why it happens:** Same key bound to two actions without proper trigger differentiation.
**How to avoid:** Use Enhanced Input trigger types. Interact = `Pressed` (fires on initial press). Lean = `Hold` with threshold (fires after held 0.15-0.2s). The pressed event fires immediately; hold fires only if the key stays down. Alternatively, use a different key for lean right (e.g., E for lean, F for interact).
**Warning signs:** Unexpected behavior when pressing E near interactables.

### Pitfall 9: Mesh Clipping When Looking Straight Down

**What goes wrong:** Looking straight down (pitch -90) causes the first-person mesh to clip through itself or show inside geometry.
**Why it happens:** No pitch clamp on camera rotation, or First Person Scale not configured correctly.
**How to avoid:** Clamp camera pitch (e.g., -80 to +80 degrees). Adjust `FirstPersonScale` on the camera component to prevent near-plane clipping. Use material-based vertex interpolation for lower body (world space at feet, FP space at torso).
**Warning signs:** Visual artifacts when looking down at feet, or mesh disappears at extreme angles.

## Code Examples

### Stamina Component Core Logic

```cpp
// StaminaComponent.cpp - Tick logic
void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsSprinting && CurrentStamina > 0.0f)
    {
        // Drain stamina while sprinting
        CurrentStamina = FMath::Max(0.0f, CurrentStamina - DrainRate * DeltaTime);
        TimeSinceStoppedSprinting = 0.0f;

        if (CurrentStamina <= 0.0f)
        {
            OnStaminaDepleted.Broadcast();
            // Force stop sprint -- character should switch to walk
        }
    }
    else if (!bIsSprinting)
    {
        // Regen after delay
        TimeSinceStoppedSprinting += DeltaTime;
        if (TimeSinceStoppedSprinting >= RegenDelay && CurrentStamina < MaxStamina)
        {
            CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + RegenRate * DeltaTime);
            if (CurrentStamina >= MaxStamina)
            {
                OnStaminaFull.Broadcast();
            }
        }
    }

    // Broadcast ratio for UI
    OnStaminaChanged.Broadcast(CurrentStamina / MaxStamina);
}
```

### Interaction Trace Logic

```cpp
// InteractionComponent.cpp - Per-tick trace
void UInteractionComponent::PerformTrace()
{
    APlayerController* PC = Cast<APlayerController>(
        Cast<APawn>(GetOwner())->GetController());
    if (!PC) return;

    FVector ViewLocation;
    FRotator ViewRotation;
    PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

    FVector TraceEnd = ViewLocation + ViewRotation.Vector() * InteractionRange;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit, ViewLocation, TraceEnd, TraceChannel, Params);

    AActor* HitActor = bHit ? Hit.GetActor() : nullptr;

    // Check if hit actor implements IInteractable
    if (HitActor && HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
    {
        if (IInteractable::Execute_CanInteract(HitActor, GetOwner()))
        {
            SetCurrentInteractable(HitActor);
            return;
        }
    }

    SetCurrentInteractable(nullptr);
}
```

### Footstep Surface Detection

```cpp
// FootstepComponent.cpp - Called from AnimNotify
void UFootstepComponent::PlayFootstepForSurface(const FVector& FootLocation)
{
    FHitResult Hit;
    FVector TraceStart = FootLocation;
    FVector TraceEnd = FootLocation - FVector(0, 0, 50.0f); // Trace downward

    FCollisionQueryParams Params;
    Params.bReturnPhysicalMaterial = true;  // CRITICAL: must request phys mat
    Params.AddIgnoredActor(GetOwner());

    if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd,
        ECC_Visibility, Params))
    {
        if (UPhysicalMaterial* PhysMat = Hit.PhysMaterial.Get())
        {
            EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(PhysMat);
            // Use SurfaceType to select appropriate sound from a TMap or DataTable
            PlayFootstepSound(SurfaceType, Hit.ImpactPoint);
        }
    }
}
```

### Lean Component Logic

```cpp
// LeanComponent.cpp - Smooth camera-only lean
void ULeanComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Target: -1 (left), 0 (center), 1 (right)
    float TargetLean = 0.0f;
    if (bLeaningLeft) TargetLean = -1.0f;
    else if (bLeaningRight) TargetLean = 1.0f;

    // Smooth interpolation
    CurrentLeanAlpha = FMath::FInterpTo(CurrentLeanAlpha, TargetLean, DeltaTime, LeanSpeed);

    // Apply as camera offset (no collision change)
    FVector LeanOffset = FVector(0, CurrentLeanAlpha * LeanDistance, 0); // Right vector
    float LeanRoll = CurrentLeanAlpha * LeanRollAngle; // Slight camera roll

    // Apply to camera component relative location/rotation
    // The character controller handles this via AddRelativeLocation on camera
}
```

### Native Gameplay Tags

```cpp
// SereneTags.h
#pragma once
#include "NativeGameplayTags.h"

namespace SereneTags
{
    // Interaction types
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Interaction_Door);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Interaction_Pickup);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Interaction_Readable);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Interaction_Drawer);

    // Movement states
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_Walking);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_Sprinting);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Movement_Crouching);

    // Player states
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_Exhausted);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_Leaning);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_InTransition);
}

// SereneTags.cpp
#include "Tags/SereneTags.h"

namespace SereneTags
{
    UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Door, "Interaction.Door");
    UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Pickup, "Interaction.Pickup");
    UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Readable, "Interaction.Readable");
    UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Drawer, "Interaction.Drawer");

    UE_DEFINE_GAMEPLAY_TAG(TAG_Movement_Walking, "Movement.Walking");
    UE_DEFINE_GAMEPLAY_TAG(TAG_Movement_Sprinting, "Movement.Sprinting");
    UE_DEFINE_GAMEPLAY_TAG(TAG_Movement_Crouching, "Movement.Crouching");

    UE_DEFINE_GAMEPLAY_TAG(TAG_Player_Exhausted, "Player.Exhausted");
    UE_DEFINE_GAMEPLAY_TAG(TAG_Player_Leaning, "Player.Leaning");
    UE_DEFINE_GAMEPLAY_TAG(TAG_Player_InTransition, "Player.InTransition");
}
```

### Stamina Bar Widget (C++ Base + UMG)

```cpp
// StaminaBarWidget.h
UCLASS()
class UStaminaBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void SetStaminaPercent(float Percent);

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void SetVisibility(bool bVisible, bool bAnimate = true);

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UProgressBar> StaminaBar;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> FadeAnimation;
};
```

### Movement Setup in Character Constructor

```cpp
// SereneCharacter constructor - Movement feel (Outlast-inspired, heavy/grounded)
UCharacterMovementComponent* CMC = GetCharacterMovement();

// Walking
CMC->MaxWalkSpeed = 250.0f;          // Deliberate walk pace
CMC->MaxWalkSpeedCrouched = 130.0f;  // Significantly slower crouch

// Sprint handled dynamically: MaxWalkSpeed set to ~500 when sprinting

// Acceleration/deceleration for weighty feel
CMC->MaxAcceleration = 1200.0f;       // Moderate acceleration (not instant)
CMC->BrakingDecelerationWalking = 1400.0f;  // Moderate deceleration
CMC->GroundFriction = 6.0f;           // Moderate friction for slight slide feel
CMC->BrakingFrictionFactor = 1.0f;    // Lower than default 2.0 for heavier feel

// Crouch
CMC->NavAgentProps.bCanCrouch = true;
CMC->CrouchedHalfHeight = 44.0f;     // Default capsule half-height when crouched

// No jump -- disable
CMC->JumpZVelocity = 0.0f;
CMC->NavAgentProps.bCanJump = false;

// No air control (grounded game)
CMC->AirControl = 0.0f;
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Dual mesh (FP arms + hidden TP body) | UE5 First Person Rendering with single mesh | UE 5.5 (2024) | Single mesh for FP view + world rep for shadows. No manual mesh swapping. |
| Legacy Input (Axis Mappings) | Enhanced Input System | UE 5.1 (2023, default) | Data-driven, supports triggers/modifiers, runtime remapping |
| Behavior Trees for AI | State Trees | UE 5.4+ | More modern, on-demand evaluation (relevant for Phase 4, but interfaces should be designed with awareness) |
| UCameraShake for head-bob | Procedural per-tick sine-wave component | Ongoing best practice | Camera shake is for impacts, not rhythmic motion |
| String-based state checking | Native Gameplay Tags | UE 4.27+ | Compile-time safe, hierarchical, editor support |
| Custom input class for settings | UGameUserSettings subclass | Stable since UE 4 | Engine-integrated settings persistence |

**Deprecated/outdated:**
- `ADefaultPawn`: Do not use. Use `ACharacter` for full movement component support.
- Legacy Axis/Action Mappings: Deprecated in favor of Enhanced Input. Do not define input in DefaultInput.ini axis mappings.
- `UMatineeCameraShake`: Still exists but consider `UCameraShakeBase` if camera shake is needed. For head-bob, use procedural approach instead.
- `bUseControllerRotationYaw` on Character: For first-person with head-bone camera, let the controller rotation drive the camera directly via `bUsePawnControlRotation` on the camera component, and set character `bUseControllerRotationYaw = true`.

## Open Questions

1. **E key conflict with lean right**
   - What we know: CONTEXT.md specifies "Lean left/right (Q/E)" and "Interact (E)". Both use the E key.
   - What's unclear: Whether the user intends E to serve dual purpose (tap = interact, hold = lean) or if this is an oversight.
   - Recommendation: Implement tap-E for interact, hold-E for lean right using Enhanced Input trigger differentiation. If this feels bad during testing, reassign lean right to a different key. Document this as a design decision to validate early.

2. **Obstacle transition system scope**
   - What we know: CONTEXT.md mentions "Context-sensitive obstacle interactions (climbing over debris, squeezing through gaps)" with "first-person animated sequences."
   - What's unclear: Whether Phase 1 needs fully working obstacle transitions or just the framework/interface. No success criteria mention obstacle transitions explicitly.
   - Recommendation: Build the framework (obstacle trigger volumes, transition state on player, montage playback system) but defer actual obstacle content to when level design exists. The interface should be ready so obstacles can be placed later.

3. **World-space vs screen-space interaction prompts**
   - What we know: CONTEXT.md says "world-space prompt ('E: Open') appears once dot is fully visible."
   - What's unclear: Whether to use `UWidgetComponent` on each interactable (world space, affected by lighting/pixelation) or project to screen space (cleaner but not truly world-space).
   - Recommendation: Use screen-space HUD widget positioned via `ProjectWorldLocationToScreen()` from the interactable's location. This gives the "world-space feel" (prompt moves with object) without the rendering issues of `UWidgetComponent`. The reticle dot is a HUD-space element regardless.

4. **Animation assets for Phase 1**
   - What we know: UE5 Mannequin is the stand-in. Need walk, sprint, crouch walk, idle animations. Footstep AnimNotifies need to be on animations.
   - What's unclear: Whether the default UE5 Mannequin animations include crouch walk and have appropriate footstep notify timing.
   - Recommendation: Use Mannequin animations where available. Add custom AnimNotify events for footsteps if not present. This is asset setup, not code architecture.

## Sources

### Primary (HIGH confidence)
- [First Person Rendering | UE 5.7 Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/first-person-rendering) -- Full FP rendering pipeline setup, primitive types, shadow system, material nodes, constraints
- [Enhanced Input | UE 5.7 Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine) -- Input Actions, Mapping Contexts, Triggers, Modifiers, C++ binding
- [Interfaces | UE 5.7 Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/interfaces-in-unreal-engine) -- UINTERFACE declaration, I-prefix pattern, Execute_ wrappers, gotchas
- [CharacterMovementComponent Python API](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/CharacterMovementComponent?application_version=5.4) -- All movement properties with descriptions
- [Configure Character Movement C++ | UE 5.7 Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/configure-character-movement-with-cplusplus-in-unreal-engine) -- Input binding, AddMovementInput pattern
- [Lyra Interaction System | UE 5.7 Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/lyra-sample-game-interaction-system-in-unreal-engine) -- Interaction architecture reference (adapted for simpler non-GAS use)

### Secondary (MEDIUM confidence)
- [UE Tip: Native Gameplay Tags](https://unrealdirective.com/tips/declare-define-native-gameplay-tags) -- UE_DECLARE/DEFINE macro usage
- [GitHub: Head Bobbing Component Gist](https://gist.github.com/iUltimateLP/d1f9c0408f84086401d909b38054db3c) -- Component pattern using camera shake states (adapted to procedural approach)
- [UE 5.5 Native First-Person Rendering Support (80.lv)](https://80.lv/articles/unreal-engine-5-5-received-native-first-person-rendering-support) -- Confirmation of 5.5 release timeline
- Project DefaultEngine.ini -- Verified Lumen GI, VSM, static lighting disabled, ray tracing enabled
- Project DefaultInput.ini -- Verified Enhanced Input configured as default

### Tertiary (LOW confidence)
- Forum discussions on lean mechanics -- No authoritative C++ implementation found; lean is straightforward camera offset + roll
- Forum discussions on obstacle transitions / montage in FP -- General patterns, no definitive standard
- Stamina system patterns from forum posts -- Common pattern but no official recommendation

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH -- All modules are core UE5, verified in official documentation
- Architecture (component-based character): HIGH -- Standard UE5 pattern, widely documented
- First Person Rendering: HIGH -- Official UE 5.7 documentation, project prerequisites verified
- Enhanced Input: HIGH -- Official documentation, already configured in project
- Interfaces (UINTERFACE): HIGH -- Official documentation, standard pattern
- Movement tuning values: MEDIUM -- Values are reasonable estimates inspired by Outlast feel; will need playtesting
- Head-bob implementation: MEDIUM -- Procedural approach is well-established, specific amplitude/frequency values need tuning
- Lean mechanic: MEDIUM -- Straightforward camera offset, but no official reference implementation
- Obstacle transitions: LOW -- Framework concept only; depends on level content and animation availability
- E key conflict resolution: LOW -- Design assumption that needs user validation

**Research date:** 2026-02-08
**Valid until:** 2026-04-08 (60 days -- UE5 core APIs are stable)
