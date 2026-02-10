# Phase 3: Hiding System - Research

**Researched:** 2026-02-10
**Domain:** UE5 hiding mechanics, camera management, animation montages, light-based visibility
**Confidence:** HIGH (core architecture) / MEDIUM (light sampling specifics)

## Summary

This phase builds a hiding system where the player can enter designated hiding spots (lockers, closets, under-beds) via the existing interaction system, experience an animated entry/exit with camera blending, look around in a constrained view while hidden, and benefit from a gradient visibility score that AI (Phase 4) will later read. The project already has IHideable (stub), IInteractable (complete), InteractionComponent (complete), and a component-based character architecture with 7 existing components.

The standard UE5 approach for hiding systems uses: (1) animation montages for entry/exit with FOnMontageEnded delegates for sequencing, (2) APlayerCameraManager ViewPitch/ViewYaw limits for constraining look-around while hidden, (3) a separate UCameraComponent on the hiding spot actor as the view target during hiding via SetViewTargetWithBlend, and (4) SceneCaptureComponent2D with small render targets for light sampling to compute visibility scores. This project's existing camera system (head bone tracking in Tick) requires careful coordination -- during hiding, the character's normal Tick camera logic must be bypassed in favor of the hiding spot's camera.

**Primary recommendation:** Build a UHidingComponent on the player character that manages the full hide state machine (Free -> Entering -> Hidden -> Exiting -> Free), coordinates with a base AHidingSpotActor (implementing both IInteractable and IHideable), and owns a UVisibilityScoreComponent that samples ambient light. The hiding spot actor holds per-type data: camera position, look constraints, entry/exit montages, and peek post-process material.

## Standard Stack

### Core (already in project)
| Library/API | Version | Purpose | Why Standard |
|-------------|---------|---------|--------------|
| UAnimInstance::Montage_Play | UE 5.7.2 | Play entry/exit animation montages | Engine-native montage playback with delegate callbacks |
| APlayerController::SetViewTargetWithBlend | UE 5.7.2 | Smooth camera blend to hiding spot camera | Engine-native camera transition with configurable blend |
| APlayerCameraManager | UE 5.7.2 | ViewPitchMin/Max, ViewYawMin/Max for constrained look | Engine-native rotation clamping, no custom math needed |
| UCharacterMovementComponent::SetMovementMode | UE 5.7.2 | MOVE_None to freeze player during hide | Engine-native movement disable/enable |
| Enhanced Input (UInputMappingContext) | UE 5.7.2 | Swap to hiding-specific input context | Already used; add/remove context by priority |
| FGameplayTag | UE 5.7.2 | Player.Hiding, Player.InTransition states | Already established tag pattern in SereneTags |
| USceneCaptureComponent2D | UE 5.7.2 | Light sampling for visibility score | Engine-native; small render target for brightness detection |

### Supporting (new for this phase)
| Library/API | Purpose | When to Use |
|-------------|---------|-------------|
| UAnimMontage | Data asset for entry/exit animations | Per hiding spot type (locker, closet, under-bed) |
| UTextureRenderTarget2D | Render target for light capture | 8x8 or 16x16 pixel target for visibility sampling |
| UMaterialInterface (Post-Process) | Peek overlay effects (slats, cracks) | Applied via hiding spot's PostProcessSettings on camera |
| FOnMontageEnded / FOnMontageBlendingOut | Delegate for montage completion | Sequence state transitions after animation completes |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| SceneCaptureComponent2D for light | LXR-Flux plugin (GPU compute shader) | More accurate, zero CPU cost, but adds external dependency; overkill for single-player with one sample point |
| SceneCaptureComponent2D for light | LightAwareness plugin | Simpler API but another dependency; does similar thing internally |
| SetViewTargetWithBlend | Manual camera lerp in Tick | More control but reinvents what the engine does natively |
| APlayerCameraManager limits | Manual clamp in UpdateRotation override | Works but ViewPitch/ViewYaw limits already exist for this purpose |
| Animation Montages | Level Sequences | Overkill for character animations; montages are the standard for gameplay-driven anims |

## Architecture Patterns

### Recommended Project Structure
```
Source/ProjectWalkingSim/
  Public/
    Hiding/
      HidingComponent.h              # Player component: state machine, camera control
      HidingSpotActor.h              # Base hiding spot (IInteractable + IHideable)
      HidingSpotDataAsset.h          # Per-type config: montages, camera, look limits
    Visibility/
      VisibilityScoreComponent.h     # Light sampling, score computation
  Private/
    Hiding/
      HidingComponent.cpp
      HidingSpotActor.cpp
    Visibility/
      VisibilityScoreComponent.cpp
```

### Pattern 1: Hiding State Machine on UHidingComponent

**What:** A UActorComponent on ASereneCharacter that manages the full hiding lifecycle as a state machine with 4 states: Free, Entering, Hidden, Exiting. The component orchestrates montage playback, camera transitions, movement locking, and input context switching.

**When to use:** Always -- this is the central controller for all hiding behavior.

**State machine:**
```
Free -> Entering (on interact with hiding spot)
  Actions: disable movement, play entry montage, blend camera to spot
Entering -> Hidden (on montage end)
  Actions: apply look constraints, switch input context, enable exit action
Hidden -> Exiting (on exit input)
  Actions: play exit montage, blend camera back
Exiting -> Free (on montage end)
  Actions: restore movement, restore input context, clear look constraints
```

**Key design decisions:**
- The component stores a `CurrentHidingSpot` weak pointer to the AHidingSpotActor
- State stored as enum `EHidingState { Free, Entering, Hidden, Exiting }`
- GameplayTags set/cleared on state changes: `Player.Hiding`, `Player.InTransition`
- Other components (HeadBob, Lean, Footstep) check the hiding state and disable themselves
- The character's Tick camera logic checks `HidingComponent->IsHiding()` and skips normal camera updates when true

### Pattern 2: Hiding Spot Actor as Dual-Interface Actor

**What:** AHidingSpotActor inherits from AActor and implements both IInteractable and IHideable. It does NOT inherit from AInteractableBase because hiding spots need skeletal mesh (for animated doors/lids) rather than static mesh, and the interaction pattern differs significantly.

**When to use:** For all placeable hiding spots in the world.

**Structure:**
```cpp
UCLASS(Abstract)
class AHidingSpotActor : public AActor, public IInteractable, public IHideable
{
    // Root scene component
    UPROPERTY() USceneComponent* Root;

    // Visual mesh (skeletal for animated doors, or static for beds)
    UPROPERTY() USkeletalMeshComponent* SpotMesh;  // Optional
    UPROPERTY() UStaticMeshComponent* StaticSpotMesh;  // Optional

    // Trigger volume for interaction range
    UPROPERTY() UBoxComponent* InteractionTrigger;

    // Camera position while hiding (placed by designer in editor)
    UPROPERTY() UCameraComponent* HidingCamera;

    // Data asset with all config
    UPROPERTY(EditAnywhere) UHidingSpotDataAsset* SpotData;

    // State
    bool bIsOccupied = false;
    TWeakObjectPtr<AActor> OccupantActor;
};
```

**Per-type subclasses:** ALockerHidingSpot, AClosetHidingSpot, AUnderBedHidingSpot -- these override specific behavior (door swing animation, peek overlay, camera angle) while sharing the base entry/exit flow.

### Pattern 3: Data-Driven Hiding Spot Configuration

**What:** UHidingSpotDataAsset (UDataAsset) stores per-type configuration: montage references, camera look constraints, peek post-process material, blend times, and audio cues. This avoids duplicating config across every instance.

**When to use:** One data asset per hiding spot type (Locker, Closet, UnderBed).

**Structure:**
```cpp
UCLASS()
class UHidingSpotDataAsset : public UDataAsset
{
    // Animations
    UPROPERTY(EditDefaultsOnly) UAnimMontage* EntryMontage;
    UPROPERTY(EditDefaultsOnly) UAnimMontage* ExitMontage;

    // Camera
    UPROPERTY(EditDefaultsOnly) float CameraBlendInTime = 0.5f;
    UPROPERTY(EditDefaultsOnly) float CameraBlendOutTime = 0.3f;
    UPROPERTY(EditDefaultsOnly) float ViewPitchMin = -30.0f;
    UPROPERTY(EditDefaultsOnly) float ViewPitchMax = 30.0f;
    UPROPERTY(EditDefaultsOnly) float ViewYawMin = -45.0f;
    UPROPERTY(EditDefaultsOnly) float ViewYawMax = 45.0f;

    // Peek visual
    UPROPERTY(EditDefaultsOnly) UMaterialInterface* PeekOverlayMaterial;  // Post-process

    // Interaction text
    UPROPERTY(EditDefaultsOnly) FText InteractionText;  // "Hide in Locker"
    UPROPERTY(EditDefaultsOnly) FText ExitText;         // "Exit Locker"

    // Gameplay tag for type identification
    UPROPERTY(EditDefaultsOnly) FGameplayTag SpotTypeTag;
};
```

### Pattern 4: Visibility Score Component (Decoupled Light Sampling)

**What:** UVisibilityScoreComponent on ASereneCharacter that periodically samples ambient light and outputs a float 0.0-1.0. AI (Phase 4) reads this value from the component. The component is self-contained and does not know about AI.

**When to use:** Always active on the player character.

**Key design:**
- Uses a USceneCaptureComponent2D with a tiny render target (8x8 pixels)
- Captures at low frequency (every 0.2-0.5 seconds, NOT every frame)
- CaptureSource set to FinalColor (includes Lumen GI)
- Reads pixels back on CPU (acceptable for single capture at low res)
- Computes average luminance across all pixels
- Applies modifiers: crouching (-0.15), hiding spot (-0.5 to override), flashlight (+0.3 in Phase 6)
- Exposes `GetVisibilityScore()` for AI and debug

### Anti-Patterns to Avoid
- **Teleporting the player character to the hiding spot:** The character stays at the hiding spot entrance position. Only the camera transitions. The character mesh is hidden (SetActorHiddenInGame or visibility flags) during hiding.
- **Using the same camera component for first-person and hiding:** The hiding spot has its own UCameraComponent. SetViewTargetWithBlend handles the blend. The character's FirstPersonCamera resumes when exiting.
- **Polling for montage completion:** Use FOnMontageEnded delegate, never tick-check `Montage_IsPlaying()`.
- **Putting hiding logic on the hiding spot actor:** The player's UHidingComponent owns the state machine. The hiding spot actor is passive data -- it provides camera, montages, constraints, but does not drive the flow.
- **Hand-rolling camera interpolation:** Use SetViewTargetWithBlend with blend time parameter. Do not manually lerp camera transforms in Tick.
- **Modifying ViewPitchMin/Max on the default camera manager:** Create ASerenePlayerCameraManager subclass to avoid polluting the default. Set it on ASerenePlayerController constructor.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Camera blend between two views | Manual FMath::VInterpTo in Tick | APlayerController::SetViewTargetWithBlend | Handles position, rotation, FOV blend with configurable curve; engine-tested |
| Constrained look-around | Clamping controller rotation manually each tick | APlayerCameraManager ViewPitchMin/ViewPitchMax/ViewYawMin/ViewYawMax | Built-in, works with ProcessViewRotation pipeline, supports runtime changes |
| Disable movement during animation | Setting MaxWalkSpeed to 0 | CMC->SetMovementMode(MOVE_None) / CMC->DisableMovement() | Clean state; 0 speed still allows rotation and other side effects |
| Animation completion callback | Tick-polling Montage_IsPlaying() | FOnMontageEnded delegate via Montage_SetEndDelegate | Event-driven, no wasted tick cycles, handles interruption |
| Input context switching | Disabling individual actions | Add/Remove UInputMappingContext with priority | Engine-native, clean context isolation, no action-by-action toggling |
| Screen-space peek overlay | UMG widget with alpha mask | Post-Process Material on hiding camera | GPU-efficient, integrates with scene rendering, no widget tick overhead |
| Light level sampling | Raycasting to each light source | SceneCaptureComponent2D + ReadPixels | Captures ALL light (direct, indirect, Lumen GI, emissive) in one pass |

**Key insight:** UE5's camera management pipeline (APlayerCameraManager -> ViewTarget -> SetViewTargetWithBlend) is specifically designed for the exact scenario of "smoothly transition view to another camera and constrain look-around." Fighting this system with custom camera math is the most common mistake in hiding system implementations.

## Common Pitfalls

### Pitfall 1: Camera Tick Conflict During Hiding
**What goes wrong:** The character's Tick() sets camera position/rotation from head bone every frame, overriding the hiding spot camera that SetViewTargetWithBlend is trying to blend to.
**Why it happens:** ASereneCharacter::Tick already has custom camera logic reading head bone position.
**How to avoid:** Add an early-out check in Tick: `if (HidingComponent && HidingComponent->GetHidingState() != EHidingState::Free) return;` before any camera offset computation. When hiding, the view target IS the hiding spot actor, so the engine's camera pipeline handles everything.
**Warning signs:** Camera jitters or snaps back to character head during hide transition.

### Pitfall 2: Montage Delegate Binding Order
**What goes wrong:** Montage end delegate never fires because it was bound before the montage started playing.
**Why it happens:** FOnMontageEnded requires an active montage instance which only exists after Montage_Play.
**How to avoid:** Always call `AnimInstance->Montage_Play()` FIRST, then `AnimInstance->Montage_SetEndDelegate()` AFTER. The montage must be playing before delegates can be bound.
**Warning signs:** State machine gets stuck in Entering or Exiting state forever.

### Pitfall 3: ViewYaw Limits Are World-Space
**What goes wrong:** Yaw constraints work fine when hiding spot faces north, but break for other orientations.
**Why it happens:** APlayerCameraManager ViewYawMin/ViewYawMax clamp in world-space yaw, not relative to the hiding spot's forward direction.
**How to avoid:** When entering hiding, compute ViewYawMin/Max relative to the hiding spot actor's world yaw. E.g., if spot faces yaw 90 and constraint is +/-45, set ViewYawMin=45, ViewYawMax=135. Or override ProcessViewRotation in a custom camera manager to clamp relative to a reference rotation.
**Warning signs:** Look range works for one hiding spot orientation but not others.

### Pitfall 4: SceneCapture Lumen GI Not Rendering
**What goes wrong:** Visibility score always reads near-zero because SceneCaptureComponent2D doesn't receive Lumen indirect lighting.
**Why it happens:** SceneCaptureComponent2D does not automatically inherit Lumen GI settings from the world. Must be explicitly configured.
**How to avoid:** On the SceneCaptureComponent2D, explicitly set: Global Illumination Method = Lumen, Reflection Method = Lumen. Set CaptureSource to FinalColor (not SceneColor). Set `bUseRayTracingIfEnabled = true`.
**Warning signs:** Light sampling works under direct point lights but not in areas lit only by bounced light.

### Pitfall 5: Player Character Visible Inside Hiding Spot
**What goes wrong:** Player arms/body clip through the hiding spot mesh, visible from outside.
**Why it happens:** Character mesh is still rendering at the hiding spot location.
**How to avoid:** On enter hiding: set main mesh and WorldRepresentationMesh visibility to hidden. On exit: restore visibility after exit montage completes. The entry/exit montages should be played on a separate skeletal mesh on the hiding spot itself (e.g., arms opening locker from inside), not on the player character mesh.
**Warning signs:** Player body visible poking through locker door.

### Pitfall 6: Input Stacking During Transitions
**What goes wrong:** Player can interact with another object or re-enter hiding while already in entry/exit transition.
**Why it happens:** Interaction component still traces and accepts input during animation.
**How to avoid:** UHidingComponent should disable the InteractionComponent during Entering/Exiting states (set component tick disabled, or have CanInteract return false while Player.InTransition tag is active). Re-enable on return to Free state.
**Warning signs:** Double-hiding, stuck states, animation interruption crashes.

### Pitfall 7: SceneCapture Performance Hit
**What goes wrong:** Significant FPS drop from SceneCaptureComponent2D rendering every frame.
**Why it happens:** SceneCapture is effectively rendering the entire scene a second time.
**How to avoid:** Set `bCaptureEveryFrame = false`. Call `CaptureScene()` manually on a timer (every 0.2-0.5 seconds). Use tiny render target (8x8 pixels). Set ShowFlags to disable expensive features not needed for brightness detection (no post-process bloom, no particle effects, no skeletal mesh). Consider using `ShowOnlyActors` to limit what is rendered to just nearby geometry.
**Warning signs:** 20-30 FPS drop when visibility component is active.

## Code Examples

### Entry Flow: Interaction Triggers Hiding
```cpp
// In AHidingSpotActor::OnInteract_Implementation
void AHidingSpotActor::OnInteract_Implementation(AActor* Interactor)
{
    if (bIsOccupied) return;

    ASereneCharacter* Character = Cast<ASereneCharacter>(Interactor);
    if (!Character) return;

    UHidingComponent* HidingComp = Character->FindComponentByClass<UHidingComponent>();
    if (!HidingComp) return;

    // The hiding spot tells the player's hiding component to begin hiding
    HidingComp->EnterHidingSpot(this);
}
```

### UHidingComponent::EnterHidingSpot
```cpp
void UHidingComponent::EnterHidingSpot(AHidingSpotActor* Spot)
{
    if (HidingState != EHidingState::Free) return;
    if (!Spot || !IHideable::Execute_CanHide(Spot, GetOwner())) return;

    CurrentHidingSpot = Spot;
    HidingState = EHidingState::Entering;

    // 1. Disable movement
    ASereneCharacter* Character = Cast<ASereneCharacter>(GetOwner());
    Character->GetCharacterMovement()->SetMovementMode(MOVE_None);

    // 2. Disable interaction trace
    Character->GetInteractionComponent()->SetComponentTickEnabled(false);

    // 3. Set transition tag
    // (Gameplay tag management via AbilitySystemComponent or manual tag container)

    // 4. Play entry montage
    UHidingSpotDataAsset* Data = Spot->GetSpotData();
    if (Data && Data->EntryMontage)
    {
        UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
        float Duration = AnimInstance->Montage_Play(Data->EntryMontage);

        if (Duration > 0.f)
        {
            // Bind completion delegate AFTER play
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &UHidingComponent::OnEntryMontageEnded);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, Data->EntryMontage);
        }
    }

    // 5. Blend camera to hiding spot
    APlayerController* PC = Cast<APlayerController>(Character->GetController());
    if (PC)
    {
        PC->SetViewTargetWithBlend(
            Spot,                          // Target actor (has UCameraComponent)
            Data->CameraBlendInTime,       // Blend time (e.g., 0.5s)
            VTBlend_Cubic,                 // Smooth ease-in/out
            0.0f,                          // Exponent (0 = use default)
            false                          // Don't lock outgoing
        );
    }

    // 6. Notify the hiding spot
    IHideable::Execute_OnEnterHiding(Spot, GetOwner());
}
```

### Montage Completion -> Hidden State
```cpp
void UHidingComponent::OnEntryMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (bInterrupted)
    {
        // Handle interruption: return to Free state
        ExitToFreeState();
        return;
    }

    HidingState = EHidingState::Hidden;

    ASereneCharacter* Character = Cast<ASereneCharacter>(GetOwner());

    // 1. Hide player mesh
    Character->GetMesh()->SetVisibility(false);

    // 2. Apply look constraints
    APlayerController* PC = Cast<APlayerController>(Character->GetController());
    if (PC && PC->PlayerCameraManager)
    {
        UHidingSpotDataAsset* Data = CurrentHidingSpot->GetSpotData();
        float SpotYaw = CurrentHidingSpot->GetActorRotation().Yaw;

        PC->PlayerCameraManager->ViewPitchMin = Data->ViewPitchMin;
        PC->PlayerCameraManager->ViewPitchMax = Data->ViewPitchMax;
        // Adjust yaw limits relative to hiding spot orientation
        PC->PlayerCameraManager->ViewYawMin = SpotYaw + Data->ViewYawMin;
        PC->PlayerCameraManager->ViewYawMax = SpotYaw + Data->ViewYawMax;
    }

    // 3. Switch to hiding input context (only look + exit)
    SwitchToHidingInputContext();

    // 4. Broadcast state change
    OnHidingStateChanged.Broadcast(EHidingState::Hidden);
}
```

### Constrained Camera Look (Handled by Engine)
```cpp
// NO custom code needed. APlayerCameraManager::ProcessViewRotation automatically
// clamps rotation to ViewPitchMin/Max and ViewYawMin/Max.
// The hiding spot's UCameraComponent serves as the view target.
// Player mouse input still goes through AddPitchInput/AddYawInput on the controller.
// The camera manager clamps the result.
```

### Visibility Score Sampling
```cpp
// UVisibilityScoreComponent - attached to ASereneCharacter
void UVisibilityScoreComponent::BeginPlay()
{
    Super::BeginPlay();

    // Create small render target
    RenderTarget = NewObject<UTextureRenderTarget2D>(this);
    RenderTarget->InitAutoFormat(8, 8);  // Tiny: 8x8 pixels
    RenderTarget->RenderTargetFormat = RTF_RGBA16f;  // HDR for accurate light values

    // Configure scene capture (created in constructor, attached to character root)
    SceneCapture->TextureTarget = RenderTarget;
    SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorHDR;
    SceneCapture->bCaptureEveryFrame = false;  // Manual capture only
    SceneCapture->bCaptureOnMovement = false;
    SceneCapture->bAlwaysPersistRenderingState = true;

    // Lumen GI support -- CRITICAL
    SceneCapture->ShowFlags.SetGlobalIllumination(true);
    SceneCapture->ShowFlags.SetReflectionEnvironment(true);

    // Disable expensive features not needed for brightness
    SceneCapture->ShowFlags.SetBloom(false);
    SceneCapture->ShowFlags.SetMotionBlur(false);
    SceneCapture->ShowFlags.SetParticles(false);
    SceneCapture->ShowFlags.SetSkeletalMeshes(false);

    // Start periodic capture timer
    GetWorld()->GetTimerManager().SetTimer(
        CaptureTimerHandle,
        this,
        &UVisibilityScoreComponent::PerformCapture,
        CaptureInterval,  // 0.2 - 0.5 seconds
        true              // Looping
    );
}

void UVisibilityScoreComponent::PerformCapture()
{
    if (!SceneCapture || !RenderTarget) return;

    // Manual capture
    SceneCapture->CaptureScene();

    // Read pixels (CPU readback - acceptable at 8x8 every 0.2s)
    FTextureRenderTargetResource* Resource = RenderTarget->GameThread_GetRenderTargetResource();
    if (!Resource) return;

    TArray<FFloat16Color> Pixels;
    Resource->ReadFloat16Pixels(Pixels);

    // Compute average luminance
    float TotalLuminance = 0.0f;
    for (const FFloat16Color& Pixel : Pixels)
    {
        // Rec.709 luminance: 0.2126R + 0.7152G + 0.0722B
        float R = Pixel.R.GetFloat();
        float G = Pixel.G.GetFloat();
        float B = Pixel.B.GetFloat();
        TotalLuminance += 0.2126f * R + 0.7152f * G + 0.0722f * B;
    }

    float AvgLuminance = (Pixels.Num() > 0) ? TotalLuminance / Pixels.Num() : 0.0f;

    // Normalize to 0.0 - 1.0 range
    // These thresholds need tuning based on actual scene lighting
    RawLightLevel = FMath::Clamp(AvgLuminance / MaxExpectedLuminance, 0.0f, 1.0f);

    // Apply modifiers
    float Score = RawLightLevel;

    ASereneCharacter* Character = Cast<ASereneCharacter>(GetOwner());
    if (Character)
    {
        if (Character->GetIsCrouching())
        {
            Score -= CrouchReduction;  // e.g., 0.15
        }

        UHidingComponent* HidingComp = Character->FindComponentByClass<UHidingComponent>();
        if (HidingComp && HidingComp->IsHidden())
        {
            Score -= HidingReduction;  // e.g., 0.5
        }
    }

    VisibilityScore = FMath::Clamp(Score, 0.0f, 1.0f);
}

float UVisibilityScoreComponent::GetVisibilityScore() const
{
    return VisibilityScore;
}
```

### Enhanced Input Context Switching for Hiding
```cpp
void UHidingComponent::SwitchToHidingInputContext()
{
    ASereneCharacter* Character = Cast<ASereneCharacter>(GetOwner());
    APlayerController* PC = Cast<APlayerController>(Character->GetController());
    if (!PC) return;

    UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
    if (!Subsystem) return;

    // Remove default movement context
    Subsystem->RemoveMappingContext(DefaultMappingContext);

    // Add hiding-specific context (Look + Exit only)
    // Priority higher than default to ensure it takes precedence if both active
    Subsystem->AddMappingContext(HidingMappingContext, 1);
}

void UHidingComponent::RestoreDefaultInputContext()
{
    // Reverse: remove hiding context, add back default
    // ...
}
```

## IHideable Interface Expansion

The existing IHideable stub needs additional methods for the full system:

```cpp
class PROJECTWALKINGSIM_API IHideable
{
    GENERATED_BODY()

public:
    /** Whether the hiding actor can enter this spot right now. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
    bool CanHide(AActor* HidingActor) const;

    /** Called when an actor enters this hiding spot. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
    void OnEnterHiding(AActor* HidingActor);

    /** Called when an actor exits this hiding spot. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
    void OnExitHiding(AActor* HidingActor);

    // --- NEW methods needed for Phase 3 ---

    /** Get the camera component used while hiding in this spot. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
    UCameraComponent* GetHidingCamera() const;

    /** Get the data asset describing this spot's configuration. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
    UHidingSpotDataAsset* GetSpotData() const;

    /** Whether this spot is currently occupied by another actor. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
    bool IsOccupied() const;

    /** Mark this spot as discovered by the monster (for memory system). */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
    void MarkDiscovered();

    /** Whether the monster has previously found someone here. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hiding")
    bool WasDiscovered() const;
};
```

Note: Whether to add GetHidingCamera/GetSpotData to the interface or keep them as concrete methods on AHidingSpotActor is a judgment call. If only AHidingSpotActor implements IHideable, concrete methods are simpler. If other actor types might implement IHideable (e.g., environmental hiding behind curtains), interface methods provide flexibility.

## Peek Visual Effects per Hiding Spot Type

### Locker: Slat Overlay
- Post-process material with horizontal black bars (slats) masking the view
- Material uses screen UV to draw alternating opaque/transparent horizontal bands
- Parameters: slat width, gap width, edge softness
- Applied via the hiding camera's PostProcessSettings.WeightedBlendables

### Closet: Door Crack
- Post-process material with heavy vignette (dark edges, thin vertical bright strip)
- Mimics viewing through a narrow door crack
- Parameters: crack width, crack position, darkness falloff

### Under-Bed: Floor-Level Framing
- Post-process material with heavy top mask (dark across upper 60% of screen)
- Camera positioned low, looking outward at floor level
- Optionally mask with bed-frame silhouette texture

All three are simple post-process materials -- they do not need custom render passes. A single material with different parameter values (material instances) can handle all three if parameterized well, but separate materials give more artistic control.

## Monster Memory Storage

For the "monster remembers spots where it found the player" feature:

```cpp
// Simplest approach: TSet on a future UMonsterMemoryComponent (Phase 4/5)
// Or store directly on the hiding spot via MarkDiscovered()/WasDiscovered()

// On the hiding spot actor:
UPROPERTY(SaveGame)  // ISaveable integration
bool bDiscoveredByMonster = false;

// This flag persists with save system. Monster AI (Phase 4) queries
// nearby hiding spots and prioritizes ones where bDiscoveredByMonster == true.
```

This is the simplest approach for now. A more sophisticated approach (blackboard key, knowledge graph) can be introduced in Phase 4/5 if needed. The important thing is that the hiding spot stores the flag and the interface exposes it.

## New Gameplay Tags Needed

```cpp
// Add to SereneTags.h/cpp:
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_Hiding);        // "Player.Hiding"
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Interaction_HidingSpot); // "Interaction.HidingSpot"
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_HidingSpot_Locker);    // "HidingSpot.Locker"
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_HidingSpot_Closet);    // "HidingSpot.Closet"
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_HidingSpot_UnderBed);  // "HidingSpot.UnderBed"
```

The existing `TAG_Player_InTransition` is already available for the Entering/Exiting states.

## Integration with Existing Character Tick

The most critical integration point is ASereneCharacter::Tick, which currently unconditionally updates camera from head bone + component offsets. This must be made conditional:

```cpp
void ASereneCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!FirstPersonCamera || !GetMesh()) return;

    // --- NEW: Skip camera updates while hiding ---
    if (HidingComponent && HidingComponent->GetHidingState() != EHidingState::Free)
    {
        // Camera is controlled by SetViewTargetWithBlend / hiding spot camera
        // Do NOT update FirstPersonCamera position/rotation
        return;
    }

    // --- Existing camera offset aggregation ---
    // ... (crouch, headbob, lean offsets as before)
}
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Sequencer/Matinee for camera transitions | SetViewTargetWithBlend + UCameraComponent on actors | UE4+ | Simpler, data-driven, no timeline assets needed |
| Raycasting to individual lights for visibility | SceneCaptureComponent2D sampling all light at once | UE5 (Lumen) | Captures indirect light, GI, emissive -- impossible with raycasting |
| Behavior Tree for hiding state | State machine on player component | Architecture choice | Component-based pattern matches project's existing architecture |
| Single IMC with disabled actions | Multiple IMC with priority switching | Enhanced Input (UE5.1+) | Cleaner state isolation, no per-action enable/disable |
| Custom camera manager for all blending | SetViewTargetWithBlend handles most cases | UE4+ | Built-in handles 90% of cases; custom manager only for edge cases |

**Deprecated/outdated:**
- Matinee: Fully removed in UE5. Use Sequencer or SetViewTargetWithBlend.
- InputComponent (legacy): Project already uses Enhanced Input, continue with that.
- Volumetric Lightmap sampling for visibility: Works only with baked lighting. Lumen is fully dynamic, so SceneCapture is the correct approach.

## Open Questions

1. **Entry/Exit Montage Source Skeleton**
   - What we know: The player character has a skeletal mesh. Entry montages (reaching for locker handle, crawling under bed) need to match the player's skeleton.
   - What's unclear: Whether to play entry montages on the player character's skeleton (requires matching anims) or use a separate animation on the hiding spot itself (arms-only mesh on the locker, for example). Outlast-style suggests the player's own arms are animated.
   - Recommendation: Play entry/exit montages on the player character's AnimInstance. This means montages must be compatible with the player's skeleton. The hiding spot's mesh (locker door, bed skirt) animates independently via a simple Timeline or its own AnimBP. Two separate animations play simultaneously: player reaches + door swings.

2. **SceneCapture Lumen Accuracy at Low Resolution**
   - What we know: LXR-Flux uses 32x32 and works well. The UE5 Light Detector uses 4x4-16x16.
   - What's unclear: Whether 8x8 is sufficient to get meaningful Lumen GI data, or if the capture resolution is too low for Lumen's surface cache to populate correctly.
   - Recommendation: Start with 8x8, test in actual lit environments. If values are inconsistent, try 16x16. The performance difference between 8x8 and 16x16 is negligible.

3. **Camera Blend During Montage Timing**
   - What we know: SetViewTargetWithBlend starts immediately. Montage plays simultaneously.
   - What's unclear: Exact timing sync between camera blend and animation. If the montage is 1.5 seconds but camera blend is 0.5 seconds, the camera arrives at the hiding position before the animation shows the player getting in.
   - Recommendation: Tune camera blend time to match the portion of the montage where the character is moving to the hiding position. Or use an AnimNotify in the montage to trigger the camera blend at the exact right moment rather than starting both simultaneously.

4. **Custom PlayerCameraManager vs Runtime Property Modification**
   - What we know: ViewPitchMin/Max and ViewYawMin/Max are public UPROPERTY on APlayerCameraManager and can be modified at runtime.
   - What's unclear: Whether modifying these at runtime on the existing manager works reliably, or if a custom subclass is needed.
   - Recommendation: Start by modifying the existing camera manager's properties at runtime through `PC->PlayerCameraManager->ViewPitchMin = ...`. Only create ASerenePlayerCameraManager if runtime modification causes issues or if ProcessViewRotation override is needed for relative yaw clamping.

## Sources

### Primary (HIGH confidence)
- UE5.7 Documentation: SetViewTargetWithBlend API - [Epic Developer Docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/BlueprintAPI/Game/Player/SetViewTargetwithBlend)
- UE5.7 Documentation: APlayerCameraManager API - [Epic Developer Docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/APlayerCameraManager)
- UE5.7 Documentation: Root Motion - [Epic Developer Docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/root-motion-in-unreal-engine)
- UE5.7 Documentation: Animation Montage - [Epic Developer Docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-montage-in-unreal-engine)
- UE5.7 Documentation: Enhanced Input - [Epic Developer Docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine)
- UE5.7 Documentation: Post-Process Materials - [Epic Developer Docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/post-process-materials-in-unreal-engine)
- UE5.7 Documentation: SceneCaptureComponent2D - [Epic Developer Docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/USceneCaptureComponent2D)
- Existing project source code: HideableInterface.h, InteractableInterface.h, SereneCharacter.h/cpp, InteractionComponent.h/cpp, SereneTags.h/cpp

### Secondary (MEDIUM confidence)
- UE Community Wiki: Player Camera Manager - [Wiki](https://unrealcommunity.wiki/player-camera-manager-227ae5)
- Montage C++ delegate patterns - [Forum Discussion](https://forums.unrealengine.com/t/play-montage-in-c-with-onblendout-oninterrupted-etc/447184)
- Montage control patterns - [zomgmoz](https://zomgmoz.tv/unreal/Animation/Controlling-animation-montages-from-C++)
- Enhanced Input C++ patterns - [UE Community Wiki](https://unrealcommunity.wiki/using-the-enhancedinput-system-in-c++-74b72b)
- Lumen + SceneCapture2D workaround - [Forum Discussion](https://forums.unrealengine.com/t/lumen-and-scenecapture2d-in-ue-5-3-2/1765188)

### Tertiary (LOW confidence)
- UE5 Light Detector (teella) - [GitHub](https://github.com/teella/UE5-Light-Detector) - Verified SceneCaptureComponent2D approach works for light detection
- LXR-Flux light detection - [GitHub](https://github.com/zurra/LXR-Flux) - GPU compute approach, confirms 32x32 render target is viable
- LightAwareness plugin - [GitHub](https://github.com/cem-akkaya/LightAwareness) - Scene buffer approach, confirms Lumen compatibility
- Horror game hiding system forum threads - [Forum](https://forums.unrealengine.com/t/hiding-under-in-furniture-stealth-in-horror-game/413876) - Community approaches, not authoritative

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - All APIs are engine-native UE5, well-documented
- Architecture (hiding component + spot actor): HIGH - Follows project's established component pattern, standard UE5 actor/component design
- Animation montage integration: HIGH - Well-documented UE5 API with known delegate patterns
- Camera management (SetViewTargetWithBlend + look constraints): HIGH - Engine-native, documented
- Light sampling / visibility score: MEDIUM - Approach is proven by multiple open-source projects, but Lumen-specific behavior at very low render target resolutions needs runtime validation
- Peek visual effects (post-process): MEDIUM - Standard post-process material approach, but specific material implementations are art-dependent
- Monster memory storage: HIGH - Simple bool flag, straightforward

**Research date:** 2026-02-10
**Valid until:** 2026-03-10 (stable APIs, unlikely to change within UE5.7 lifecycle)
