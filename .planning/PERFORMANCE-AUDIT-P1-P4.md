# Performance & Code Quality Audit: Phases 1-4

**Project:** The Juniper Tree (ProjectWalkingSim)
**Engine:** Unreal Engine 5.7.2
**Date:** 2026-02-11
**Auditor:** Claude Code (claude-opus-4-6)
**Scope:** All C++ source code in `Source/ProjectWalkingSim/` (46 headers, 46 source files)

---

## Executive Summary

**Overall Health Score: 8.2 / 10 (Good)**

This is a well-structured codebase with strong fundamentals. The architecture shows thoughtful design -- components are well-separated, interfaces are used appropriately, Epic naming conventions are followed consistently, and dangerous UE5 pitfalls have been proactively avoided (documented in comments). The code reads like it was written by someone who understands the engine.

That said, there are **2 critical issues**, **11 warnings**, and **16 suggestions** identified in this audit. The critical issues relate to a per-frame GPU readback that will stall the render thread in shipping builds, and a footstep logging call that fires every 0.3-0.5 seconds at `Log` verbosity. The warnings address unnecessary ticking, redundant component lookups, and a few missing null checks.

**Breakdown by severity:**
- Critical (must fix): 2
- Warning (should fix): 11
- Suggestion (nice to have): 16

---

## Critical Issues (Must Fix)

### CRIT-01: GPU Readback Stall in VisibilityScoreComponent::ComputeScore

**File:** `Source/ProjectWalkingSim/Private/Visibility/VisibilityScoreComponent.cpp:108-109`

```cpp
TArray<FFloat16Color> Pixels;
Resource->ReadFloat16Pixels(Pixels);
```

`ReadFloat16Pixels()` is a **synchronous GPU-to-CPU readback** that stalls the render thread until the GPU finishes the scene capture. Even at 4 Hz (0.25s interval) with an 8x8 target, this blocks the render thread for the duration of the readback. On consoles or lower-end hardware, this can produce visible hitches.

Additionally, line 108 allocates a `TArray<FFloat16Color>` on every call (64 pixels * sizeof(FFloat16Color) = 512 bytes), which is a heap allocation in a timer callback that fires 4 times per second.

**Recommended fix:** Use `UTextureRenderTarget2D::ReadPixelsAsync()` or the `FRenderCommandFence` pattern for async readback. Cache the `TArray<FFloat16Color>` as a member to avoid repeated allocations.

---

### CRIT-02: Verbose Logging in Hot Path (FootstepComponent)

**File:** `Source/ProjectWalkingSim/Private/Player/Components/FootstepComponent.cpp:130-131`

```cpp
UE_LOG(LogSerene, Log, TEXT("UFootstepComponent::PlayFootstepForSurface - Surface=%d, Volume=%.2f"),
    static_cast<int32>(SurfaceType), Volume);
```

This `UE_LOG` at `Log` verbosity fires **every single footstep** (every 0.3-0.5 seconds while moving). In shipping builds with file logging enabled, this generates continuous disk I/O. The `Log` verbosity level is not gated and will always execute the format string, even performing the `static_cast` operation.

**Recommended fix:** Change to `Verbose` or `VeryVerbose` verbosity, or guard with `#if !UE_BUILD_SHIPPING`.

---

## Warnings (Should Fix)

### WARN-01: PatrolRouteActor Ticks Every Frame at Runtime for Editor-Only Debug Drawing

**File:** `Source/ProjectWalkingSim/Private/AI/PatrolRouteActor.cpp:9-10`

```cpp
PrimaryActorTick.bCanEverTick = true;
PrimaryActorTick.bStartWithTickEnabled = true;
```

The constructor enables tick unconditionally, but `Tick()` at line 70-121 only does work inside `#if WITH_EDITOR` and only when `World->IsEditorWorld()` returns true. In a packaged build, the `#if WITH_EDITOR` block compiles out, meaning the Tick override does `Super::Tick(DeltaTime)` and then returns -- still paying the tick overhead (task scheduling, virtual dispatch) for zero work.

**File:** `Source/ProjectWalkingSim/Private/AI/PatrolRouteActor.cpp:70-79`

```cpp
void APatrolRouteActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
#if WITH_EDITOR
    const UWorld* World = GetWorld();
    if (!World || !World->IsEditorWorld())
    {
        return;
    }
    // ... debug drawing ...
#endif
}
```

**Recommended fix:** Wrap the tick enablement in `#if WITH_EDITOR`:
```cpp
#if WITH_EDITOR
    PrimaryActorTick.bCanEverTick = true;
#else
    PrimaryActorTick.bCanEverTick = false;
#endif
```

### WARN-02: DrawDebug Calls Not Gated for Non-Shipping Builds

**File:** `Source/ProjectWalkingSim/Private/AI/PatrolRouteActor.cpp:99-118`

The `DrawDebugSphere` and `DrawDebugLine` calls are inside `#if WITH_EDITOR`, which is correct for stripping from packaged builds. However, the `#include "DrawDebugHelpers.h"` at line 5 still compiles in all builds. This is minor but technically violates the principle of not including unnecessary headers.

**Recommended fix:** Wrap the include in `#if WITH_EDITOR`.

### WARN-03: Repeated FindComponentByClass Calls in PlayerController Input Handlers

**File:** `Source/ProjectWalkingSim/Private/Player/SerenePlayerController.cpp:203-264`

The lean input handlers call `FindComponentByClass<ULeanComponent>()` on every input event (4 handlers: HandleLeanLeftStart, HandleLeanLeftStop, HandleLeanRightStart, HandleLeanRightStop). Similarly, `HandleInteract()` at line 194 calls `FindComponentByClass<UHidingComponent>()` and `FindComponentByClass<UInteractionComponent>()`.

`FindComponentByClass` iterates the component array linearly. While the character only has ~11 components, this is unnecessary work that could be cached.

**Recommended fix:** Cache the components in a `OnPossess()` override or use the existing `GetSereneCharacter()` helper to access typed members directly (the character already stores all component pointers as UPROPERTY members).

### WARN-04: StaminaComponent Ticks Even When Fully Regenerated and Not Sprinting

**File:** `Source/ProjectWalkingSim/Private/Player/Components/StaminaComponent.cpp:9`

```cpp
PrimaryComponentTick.bCanEverTick = true;
```

The component always ticks. When the player is not sprinting and stamina is full, `TickComponent` executes but does no meaningful work -- it enters the `else if (!bIsSprinting)` branch, checks `TimeSinceStoppedSprinting >= RegenDelay` (true forever), then checks `CurrentStamina < MaxStamina` (false), exits, then computes `CurrentPercent` (unchanged). This wastes ~200ns per tick doing nothing.

**Recommended fix:** Disable tick when stamina is full and the player is not sprinting. Re-enable in `SetSprinting(true)` and when stamina drops below max.

### WARN-05: HeadBobComponent and LeanComponent Tick Even When No Input Changes

**File:** `Source/ProjectWalkingSim/Private/Player/Components/HeadBobComponent.cpp:10` and `Source/ProjectWalkingSim/Private/Player/Components/LeanComponent.cpp:9`

Both components tick every frame. LeanComponent in particular does work only when lean input changes, but ticks perpetually to interpolate `CurrentLeanAlpha`. Once the alpha reaches the target (0.0 when centered), it continues to call `FMath::FInterpTo` with identical input/output values.

HeadBobComponent is similar -- when the player is stationary and `CurrentBobAlpha` is 0, it still executes the owner cast, velocity check, and interp call.

**Recommended fix:** For LeanComponent, disable tick when `CurrentLeanAlpha` is at target and no lean input is held. For HeadBobComponent, the early-return at line 34-39 mitigates most cost, but the owner cast at line 19 and velocity computation at line 26 still execute. Consider disabling tick when the player is standing still and bob alpha is 0.

### WARN-06: FName Construction Every Frame in SereneCharacter::Tick

**File:** `Source/ProjectWalkingSim/Private/Player/SereneCharacter.cpp:193`

```cpp
const FVector HeadWorldPos = GetMesh()->GetSocketLocation(FName(TEXT("head")));
```

`FName(TEXT("head"))` constructs an FName from a string every tick. While FName construction uses a global name table (O(1) amortized lookup), it still involves a string hash and table lookup on every frame.

**Recommended fix:** Use a static FName:
```cpp
static const FName HeadSocketName(TEXT("head"));
const FVector HeadWorldPos = GetMesh()->GetSocketLocation(HeadSocketName);
```

### WARN-07: Missing Null Check Before Dereferencing GetOwner() in NoiseReportingComponent

**File:** `Source/ProjectWalkingSim/Private/AI/NoiseReportingComponent.cpp:18`

```cpp
UFootstepComponent* FootstepComp = GetOwner()->FindComponentByClass<UFootstepComponent>();
```

`GetOwner()` is called without a null check. While in practice this component is always attached to an actor, failing to check violates defensive programming. If `GetOwner()` returns nullptr (e.g., during teardown), this crashes.

**Recommended fix:** Add `if (!GetOwner()) return;` before the call.

### WARN-08: Missing Null Check Before Dereferencing GetOwner() in SuspicionComponent::UpdateAlertLevel

**File:** `Source/ProjectWalkingSim/Private/AI/SuspicionComponent.cpp:99`

```cpp
*GetOwner()->GetName()
```

Inside the `UE_LOG` call, `GetOwner()` is dereferenced without a null check. If the component is being destroyed or the owner is null, this will crash.

**Recommended fix:** Guard the log with `if (GetOwner())` or use `GetOwner() ? GetOwner()->GetName() : TEXT("None")`.

### WARN-09: VisibilityScoreComponent SceneCapture Keeps GI Enabled

**File:** `Source/ProjectWalkingSim/Private/Visibility/VisibilityScoreComponent.cpp:58`

```cpp
SceneCapture->ShowFlags.SetGlobalIllumination(true);
```

Even with an 8x8 render target, keeping Lumen GI enabled for the scene capture means the GPU must evaluate Lumen lighting for an additional view. Depending on the scene complexity, this can add 0.2-0.5ms of GPU time per capture. With captures at 4 Hz, the amortized cost is ~0.05-0.1ms/frame, but the spikes occur on the capture frame.

**Recommended fix:** Test whether disabling GI still provides usable light readings. If direct lighting alone is sufficient for visibility scoring, disable GI on the capture component.

### WARN-10: Synchronous Asset Loading in PickupActor::BeginPlay and InventoryComponent::LoadItemRegistry

**File:** `Source/ProjectWalkingSim/Private/Interaction/PickupActor.cpp:36-38`

```cpp
LoadedObject = AssetPath.TryLoad();
```

**File:** `Source/ProjectWalkingSim/Private/Inventory/InventoryComponent.cpp:43-47`

```cpp
LoadedObject = AssetPath.TryLoad();
```

Both use `TryLoad()` which is synchronous loading. For a small item count this is acceptable, but it runs in BeginPlay which blocks the game thread. If the item count grows to dozens, this could cause a noticeable hitch on level load.

**Recommended fix:** For now, add a comment documenting the synchronous loading decision with a threshold (e.g., "acceptable for <30 items"). Consider async loading via `UAssetManager::LoadPrimaryAssetsAsync()` if item count grows.

### WARN-11: DoorActor and DrawerActor Tick Even When Stationary

**Files:**
- `Source/ProjectWalkingSim/Private/Interaction/DoorActor.cpp:108-126`
- `Source/ProjectWalkingSim/Private/Interaction/DrawerActor.cpp:36-57`

Both actors tick every frame to interpolate their open/close animation. The `FMath::IsNearlyEqual` check at DoorActor:118 and DrawerActor:48 prevents the SetRelativeLocation/Rotation call when at rest, but the tick itself (virtual dispatch, DeltaTime computation, Super::Tick) still executes.

In a level with 20+ doors and drawers, this is ~20 unnecessary tick calls per frame.

**Recommended fix:** Disable tick in the constructor (`bStartWithTickEnabled = false`) and enable tick only in `OnInteract_Implementation` when animation starts. Disable tick again when `IsNearlyEqual` returns true.

---

## Suggestions (Nice to Have)

### SUGG-01: Use TObjectPtr Consistently for All UObject Pointers

**File:** `Source/ProjectWalkingSim/Public/Player/SerenePlayerController.h:96`

```cpp
ASereneCharacter* GetSereneCharacter() const;
```

The return type uses a raw pointer rather than being consistent with the `TObjectPtr<>` pattern used elsewhere. This is a style consistency issue -- raw pointer returns from functions are acceptable in UE5, but the header inconsistency is notable.

Additionally at `Source/ProjectWalkingSim/Public/Player/Components/InteractionComponent.h:58`:
```cpp
TWeakObjectPtr<AActor> CurrentInteractable;
```
Good use of `TWeakObjectPtr` for tracked actors. This pattern is correct.

### SUGG-02: Forward Declarations Could Replace Some Header Includes

**File:** `Source/ProjectWalkingSim/Public/Player/SereneCharacter.h:7-9`

```cpp
#include "Player/Components/StaminaComponent.h"
#include "Player/Components/HeadBobComponent.h"
#include "Player/Components/LeanComponent.h"
```

These three headers are included because the character accesses component methods inline (GetCurrentOffset, GetLeanOffset, etc.) in Tick. However, since these calls are in the .cpp file, these includes could be moved to the .cpp file with forward declarations in the header. The `TObjectPtr<>` member declarations only need forward declarations.

**Counter-argument:** The header is only included in a small number of files, so the compile-time impact is minimal. This is a purity suggestion.

### SUGG-03: Consider Non-Dynamic Delegates for Internal C++ Communication

**File:** `Source/ProjectWalkingSim/Public/Player/Components/StaminaComponent.h:10-16`

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChanged, float, Percent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaDepleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaFull);
```

All component delegates are `DYNAMIC_MULTICAST` which allows Blueprint binding. This is correct if Blueprint binding is planned. However, `OnStaminaDepleted` is only bound in C++ (SereneCharacter::BeginPlay line 124). If Blueprint binding is not needed for specific delegates, using `DECLARE_MULTICAST_DELEGATE` would be ~5x faster invocation.

**This applies to:** OnStaminaDepleted, OnStaminaFull, OnHidingStateChanged, OnAlertLevelChanged. Review each delegate to determine if Blueprint binding is actually needed.

### SUGG-04: InteractionComponent Per-Tick Line Trace Could Use Reduced Frequency

**File:** `Source/ProjectWalkingSim/Private/Player/Components/InteractionComponent.cpp:14-20`

The interaction trace runs every single frame via `TickComponent`. For a 150cm trace, the player would need to move very quickly to miss an interactable between frames. Running the trace at half frequency (every other frame, or on a 0.05s timer) would halve the trace cost with negligible gameplay impact.

### SUGG-05: HeadBobComponent Sprint Detection Uses Magic Number

**File:** `Source/ProjectWalkingSim/Private/Player/Components/HeadBobComponent.cpp:51`

```cpp
else if (CMC && CMC->MaxWalkSpeed > 400.0f)
```

**File:** `Source/ProjectWalkingSim/Private/Player/Components/FootstepComponent.cpp:60`

```cpp
else if (CurrentSpeed > 400.0f)
```

Both files use the magic number `400.0f` to detect sprint state. If WalkSpeed or SprintSpeed changes, these become silently wrong.

**Recommended fix:** Read the sprint state from the character directly (e.g., `ASereneCharacter::GetIsSprinting()`) rather than inferring it from speed thresholds. This eliminates the coupling to specific speed values.

### SUGG-06: Consider Using check() Macros for Invariant Validation

The codebase consistently uses null checks with early returns but never uses `check()`, `ensure()`, or `verify()` macros. For invariants that should never be violated (e.g., components created in the constructor being null), `ensure()` is preferable because it logs a callstack on first failure while still allowing graceful recovery.

**Example:** `Source/ProjectWalkingSim/Private/Player/SereneCharacter.cpp:158`
```cpp
if (!FirstPersonCamera || !GetMesh())
{
    return;
}
```
If the camera or mesh is null after construction, something is fundamentally wrong. This is a good candidate for `ensureMsgf(FirstPersonCamera, TEXT("..."))`.

### SUGG-07: Missing const on Methods That Don't Modify State

**File:** `Source/ProjectWalkingSim/Public/Player/Components/InteractionComponent.h:40-41`

```cpp
UFUNCTION(BlueprintCallable, Category = "Interaction")
AActor* GetCurrentInteractable() const;
```

Good -- this is correctly const. But for consistency:

**File:** `Source/ProjectWalkingSim/Public/Visibility/VisibilityScoreComponent.h:36-37`

```cpp
UFUNCTION(BlueprintCallable, Category = "Visibility")
float GetVisibilityScore() const { return VisibilityScore; }
```

Missing `BlueprintPure` specifier. Since it's const and returns a value, adding `BlueprintPure` would make it a pure function node in Blueprint (no exec pins).

This also applies to `GetRawLightLevel()` on line 40.

### SUGG-08: Hiding Component Includes Many Headers Directly

**File:** `Source/ProjectWalkingSim/Private/Hiding/HidingComponent.cpp:1-20`

This .cpp file includes 16 headers. While all are needed, the breadth of includes indicates the component has wide knowledge of the system. The `DisablePlayerSystems()` and `RestorePlayerSystems()` methods (lines 418-488) use `FindComponentByClass` for 4 different component types, creating implicit coupling.

**Recommendation (long-term):** Consider a "PlayerSystemsInterface" or "ComponentEnableInterface" that components register with, allowing bulk enable/disable without explicit knowledge of each component type. This is a Phase 5+ architecture consideration, not an immediate fix.

### SUGG-09: UE_LOG in GameMode Constructor

**File:** `Source/ProjectWalkingSim/Private/Core/SereneGameMode.cpp:16`

```cpp
UE_LOG(LogSerene, Log, TEXT("ASereneGameMode: DefaultPawnClass=..."));
```

Logging in constructors is technically safe for CDO construction but produces log output during module load, before the game world exists. This clutters the log with diagnostic information that's only useful for debugging class defaults.

**Recommended fix:** Move to BeginPlay or remove entirely.

### SUGG-10: InventoryWidget Stores const UInventoryComponent* as TObjectPtr

**File:** `Source/ProjectWalkingSim/Public/Player/HUD/InventoryWidget.h:154-155`

```cpp
UPROPERTY()
TObjectPtr<const UInventoryComponent> CachedInventoryComp;
```

`TObjectPtr<const T>` is technically valid but unusual. The const qualifier provides no runtime safety benefit with `TObjectPtr` and may cause warnings with some static analysis tools.

### SUGG-11: PickupActor::InitFromItemData Uses LoadSynchronous for WorldMesh

**File:** `Source/ProjectWalkingSim/Private/Interaction/PickupActor.cpp:149`

```cpp
UStaticMesh* Mesh = ItemData->WorldMesh.LoadSynchronous();
```

This synchronous load occurs when discarding items, which is a player-initiated action. It could cause a micro-hitch if the mesh is not already in memory.

**Recommended fix:** Pre-load item meshes during InventoryComponent::LoadItemRegistry or use async loading with a callback.

### SUGG-12: Consider Gating Verbose UE_LOG Calls in StaminaComponent

**File:** `Source/ProjectWalkingSim/Private/Player/Components/StaminaComponent.cpp:73-74, 81-82, 89-90`

Several `UE_LOG(LogSerene, Verbose, ...)` calls in `SetSprinting()` involve string formatting. While `Verbose` level is typically disabled in shipping, the format string construction (float-to-string conversions) still executes before the log macro checks the verbosity level. For truly hot paths, use `UE_LOG_CHECK_VERBOSITY` or the `UE_CLOG` pattern.

### SUGG-13: SereneHUD::HandleInventoryChanged Contains Diagnostic Logging

**File:** `Source/ProjectWalkingSim/Private/Player/HUD/SereneHUD.cpp:98-101`

```cpp
UE_LOG(LogSerene, Log, TEXT("ASereneHUD::HandleInventoryChanged - HUDWidgetInstance=%s, ..."));
```

This diagnostic log fires at `Log` verbosity every time any inventory slot changes (add, remove, combine). It should be `Verbose` or removed before shipping.

### SUGG-14: InventorySlotWidget::SetSlotData Contains Excessive Diagnostic Logging

**File:** `Source/ProjectWalkingSim/Private/Player/HUD/InventorySlotWidget.cpp:28-57`

Lines 28, 34-37, 44, 56 contain multiple `UE_LOG(LogSerene, Log, ...)` calls that fire every time a slot is refreshed. With 8 slots refreshing on every inventory change, this produces 8-24 log lines per inventory operation.

**Recommended fix:** Change all to `Verbose` or remove after debugging is complete.

### SUGG-15: StaminaBarWidget Uses NativeTick for Hide Timer

**File:** `Source/ProjectWalkingSim/Private/Player/HUD/StaminaBarWidget.cpp:79-91`

```cpp
void UStaminaBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
```

The widget ticks every frame but only does work when `bWaitingToHide` is true (a 2-second window after stamina reaches 100%). Using a `FTimerHandle` via `GetWorld()->GetTimerManager()` instead would avoid the per-frame NativeTick overhead during the ~95% of gameplay when the bar is not waiting to hide.

### SUGG-16: ASereneCharacter::Tick Creates Transient FRotator and FVector Every Frame

**File:** `Source/ProjectWalkingSim/Private/Player/SereneCharacter.cpp:154-200`

The Tick function creates several stack-allocated FVector and FRotator objects per frame. This is standard UE practice and the compiler will optimize most away, but the function could be slightly more efficient by avoiding the `RotateVector` call (line 194) when the aggregated offset is zero (no lean, no headbob, no crouch transition).

**Recommended fix:** Add an early-out:
```cpp
if (ActorSpaceOffset.IsNearlyZero() && FMath::IsNearlyZero(CameraRoll))
{
    FirstPersonCamera->SetWorldLocation(HeadWorldPos);
    FirstPersonCamera->SetWorldRotation(GetControlRotation());
    return;
}
```

---

## Per-System Breakdown

### Player System (Phase 1)

| File | Lines | Issues |
|------|-------|--------|
| SereneCharacter.h/cpp | 196 + 285 | WARN-06, SUGG-02, SUGG-06, SUGG-16 |
| SerenePlayerController.h/cpp | 106 + 319 | WARN-03 |
| StaminaComponent.h/cpp | 112 + 112 | WARN-04, SUGG-03, SUGG-12 |
| HeadBobComponent.h/cpp | 86 + 90 | WARN-05, SUGG-05 |
| LeanComponent.h/cpp | 82 + 68 | WARN-05 |
| InteractionComponent.h/cpp | 66 + 128 | SUGG-04 |
| FootstepComponent.h/cpp | 106 + 133 | CRIT-02, SUGG-05 |
| SereneGameMode.h/cpp | 24 + 18 | SUGG-09 |
| SereneGameInstance.h/cpp | 66 + 67 | Clean |
| SereneLogChannels.h/cpp | 8 + 6 | Clean |
| HUD Widgets (7 files) | ~640 total | SUGG-13, SUGG-14, SUGG-15 |

**Verdict:** Solid foundation. The camera offset aggregation pattern in Tick() is clean. Main concerns are unnecessary ticking and diagnostic logging that should be demoted to Verbose.

### Inventory System (Phase 2)

| File | Lines | Issues |
|------|-------|--------|
| InventoryComponent.h/cpp | 180 + 354 | WARN-10, Clean otherwise |
| InventoryTypes.h/cpp | 50 + 4 | Clean |
| ItemDataAsset.h/cpp | 81 + 9 | Clean |
| PickupActor.h/cpp | 60 + 175 | WARN-10, SUGG-11 |
| DoorActor.h/cpp | 79 + 127 | WARN-11 |
| DrawerActor.h/cpp | 53 + 58 | WARN-11 |
| ReadableActor.h/cpp | 37 + 22 | Clean |

**Verdict:** Well-designed inventory system. The `TMap<FName, TObjectPtr<UItemDataAsset>>` registry pattern is efficient. The combine recipe system uses `TPair<FName, FName>` which is functional but could be improved with a canonical key ordering to avoid the double-lookup pattern. Synchronous asset loading is acceptable for the current scale.

### Hiding System (Phase 3)

| File | Lines | Issues |
|------|-------|--------|
| HidingComponent.h/cpp | 163 + 584 | SUGG-08 (coupling concern) |
| HidingSpotActor.h/cpp | 115 + 177 | Clean |
| HidingSpotDataAsset.h/cpp | 90 + 20 | Clean |
| VisibilityScoreComponent.h/cpp | 109 + 152 | CRIT-01, WARN-09 |
| HidingTypes.h/cpp | 22 + 4 | Clean |
| HideableInterface.h/cpp | 65 + 4 | Clean |

**Verdict:** The hiding state machine is robust -- montage-driven transitions with proper cleanup in both interrupted and normal paths. The IHideable interface is well-designed. VisibilityScoreComponent has the most serious performance issue in the codebase (GPU readback stall).

### AI System (Phase 4)

| File | Lines | Issues |
|------|-------|--------|
| WendigoAIController.h/cpp | 80 + 199 | Clean (good BeginPlay/OnPossess guard) |
| WendigoCharacter.h/cpp | 54 + 32 | Clean |
| SuspicionComponent.h/cpp | 129 + 108 | WARN-08 |
| NoiseReportingComponent.h/cpp | 55 + 51 | WARN-07 |
| PatrolRouteActor.h/cpp | 80 + 122 | WARN-01, WARN-02 |
| MonsterAITypes.h/cpp | 63 + 4 | Clean |
| STT_PatrolIdle.h/cpp | 83 + 87 | Clean |
| STT_PatrolMoveToWaypoint.h/cpp | 67 + 115 | Clean |
| STT_InvestigateLocation.h/cpp | 83 + 150 | Clean |
| STT_OrientToward.h/cpp | 76 + 82 | Clean |
| STC_SuspicionLevel.h/cpp | 66 + 41 | Clean |

**Verdict:** The AI system is well-architected. The two-flag guard pattern for State Tree startup timing is excellent and avoids a known UE5 pitfall. The State Tree tasks properly clean up in ExitState (stopping movement, clearing focus, restoring speed). The suspicion system has clean threshold-based alert level transitions.

---

## Component Tick Analysis

| Component | bCanEverTick | bStartWithTickEnabled | Justified? | Notes |
|-----------|-------------|----------------------|------------|-------|
| ASereneCharacter | true | true (default) | **Yes** | Camera offset aggregation every frame |
| UStaminaComponent | true | true (default) | **Partial** | Should disable when idle (full stamina, not sprinting) |
| UHeadBobComponent | true | true (default) | **Partial** | Early returns mitigate, but still ticks when stationary |
| ULeanComponent | true | true (default) | **Partial** | Should disable when centered and no input held |
| UInteractionComponent | true | true (default) | **Yes** | Per-frame trace is the standard approach |
| UFootstepComponent | true | true (default) | **Yes** | Timer-based footstep generation |
| UInventoryComponent | false | N/A | **Yes** | Event-driven only |
| UHidingComponent | true | **false** | **Yes** | Only ticks during Hidden state |
| UVisibilityScoreComponent | false | N/A | **Yes** | Timer-driven capture |
| UNoiseReportingComponent | false | N/A | **Yes** | Event-driven only |
| USuspicionComponent | false | N/A | **Yes** | Fed by AI controller tick |
| AWendigoAIController | true | true (default) | **Yes** | Continuous sight processing, suspicion decay |
| ADoorActor | true | true (default) | **No** | Should only tick during animation |
| ADrawerActor | true | true (default) | **No** | Should only tick during animation |
| APatrolRouteActor | true | true | **No** | Only needed in editor for debug draw |

**Total unnecessary ticks per frame (worst case):** 4 (StaminaComponent when idle, LeanComponent when centered, DoorActor when closed, DrawerActor when closed) + N (for every door/drawer actor in the level)

---

## Architecture Quality Assessment

### Strengths

1. **Clean component separation:** Each component has a single responsibility. StaminaComponent only tracks stamina. HeadBobComponent only computes offsets. The character aggregates.

2. **Interface-driven interaction:** `IInteractable` and `IHideable` are properly used as contracts. The InteractionComponent communicates with actors through the interface, never downcasting to specific types.

3. **Data-driven design:** Item data assets, hiding spot data assets, and patrol routes are all designer-configurable without C++ changes.

4. **Proper UPROPERTY memory management:** `TObjectPtr<>` is used consistently for all UObject member pointers. `TWeakObjectPtr<>` is correctly used for cross-actor references that could become stale (InteractionComponent::CurrentInteractable, HidingSpotActor::OccupantActor, HidingComponent::CurrentHidingSpot).

5. **Good Blueprint boundary:** Tuning values are `EditAnywhere`, state queries are `BlueprintPure`, actions are `BlueprintCallable`. The C++ defines behavior; Blueprint defines values and visuals.

6. **Delegate usage is appropriate:** Dynamic multicast delegates are used for Blueprint-facing events. Non-dynamic delegates (`FOnMontageEnded`) are used for internal C++ communication.

### Concerns

1. **HidingComponent has broad system knowledge:** It directly manipulates InteractionComponent, HeadBobComponent, LeanComponent, FootstepComponent, and VisibilityScoreComponent via FindComponentByClass. This creates implicit coupling.

2. **Sprint detection via magic speed threshold:** Multiple components (HeadBob, Footstep) detect sprint by checking `MaxWalkSpeed > 400.0f` rather than querying the character's sprint state. This is fragile.

3. **BeginPlay ordering dependency:** NoiseReportingComponent::BeginPlay depends on FootstepComponent existing on the owner. Since both are created in ASereneCharacter's constructor, this is safe, but it's an implicit ordering dependency.

---

## Epic Games Coding Standards Compliance

| Standard | Compliance | Notes |
|----------|-----------|-------|
| U/A/F/E/I/b prefixes | **Pass** | All classes, structs, enums, interfaces, and booleans follow conventions |
| UPROPERTY/UFUNCTION specifiers | **Pass** | Proper use of EditAnywhere, BlueprintReadOnly, BlueprintCallable, BlueprintPure |
| const correctness | **Pass** | Getter functions are const. Const references used for FText, FVector params |
| Forward declarations vs includes | **Good** | Forward declarations used in most headers. SUGG-02 notes some opportunities |
| Header include order | **Pass** | Module header first in .cpp, CoreMinimal.h in headers |
| check/ensure/verify macros | **Not used** | SUGG-06 recommends adoption for invariant validation |
| GENERATED_BODY() | **Pass** | Present in all UCLASS, USTRUCT declarations |
| Copyright header | **Pass** | "// Copyright Null Lantern." on every file |

---

## Summary of All Findings

| ID | Severity | System | File | Line | Description |
|----|----------|--------|------|------|-------------|
| CRIT-01 | Critical | Hiding | VisibilityScoreComponent.cpp | 108-109 | Synchronous GPU readback stalls render thread |
| CRIT-02 | Critical | Player | FootstepComponent.cpp | 130-131 | UE_LOG at Log verbosity fires every footstep |
| WARN-01 | Warning | AI | PatrolRouteActor.cpp | 9-10 | Ticks at runtime for editor-only debug draw |
| WARN-02 | Warning | AI | PatrolRouteActor.cpp | 5 | DrawDebugHelpers.h included unconditionally |
| WARN-03 | Warning | Player | SerenePlayerController.cpp | 203-264 | Repeated FindComponentByClass in input handlers |
| WARN-04 | Warning | Player | StaminaComponent.cpp | 9 | Ticks when idle (full stamina, not sprinting) |
| WARN-05 | Warning | Player | HeadBobComponent.cpp:10, LeanComponent.cpp:9 | 10, 9 | Tick when not needed |
| WARN-06 | Warning | Player | SereneCharacter.cpp | 193 | FName constructed from string every frame |
| WARN-07 | Warning | AI | NoiseReportingComponent.cpp | 18 | Missing null check on GetOwner() |
| WARN-08 | Warning | AI | SuspicionComponent.cpp | 99 | Missing null check on GetOwner() in UE_LOG |
| WARN-09 | Warning | Hiding | VisibilityScoreComponent.cpp | 58 | SceneCapture keeps Lumen GI enabled |
| WARN-10 | Warning | Inventory | PickupActor.cpp:36, InventoryComponent.cpp:43 | 36, 43 | Synchronous asset loading in BeginPlay |
| WARN-11 | Warning | Inventory | DoorActor.cpp:10, DrawerActor.cpp:10 | 10, 10 | Tick when stationary (not animating) |
| SUGG-01 | Suggestion | Player | SerenePlayerController.h | 96 | Raw pointer return inconsistency |
| SUGG-02 | Suggestion | Player | SereneCharacter.h | 7-9 | Includes could be forward declarations |
| SUGG-03 | Suggestion | Player | StaminaComponent.h | 10-16 | Dynamic delegates for C++-only binding |
| SUGG-04 | Suggestion | Player | InteractionComponent.cpp | 14-20 | Per-frame trace could use reduced frequency |
| SUGG-05 | Suggestion | Player | HeadBobComponent.cpp:51, FootstepComponent.cpp:60 | 51, 60 | Sprint detection uses magic number 400.0f |
| SUGG-06 | Suggestion | All | Multiple | -- | No use of check/ensure/verify macros |
| SUGG-07 | Suggestion | Hiding | VisibilityScoreComponent.h | 36-37 | Missing BlueprintPure on const getters |
| SUGG-08 | Suggestion | Hiding | HidingComponent.cpp | 1-20 | Broad system coupling via includes/FindComponentByClass |
| SUGG-09 | Suggestion | Core | SereneGameMode.cpp | 16 | UE_LOG in constructor |
| SUGG-10 | Suggestion | Inventory | InventoryWidget.h | 154-155 | TObjectPtr<const T> is unusual |
| SUGG-11 | Suggestion | Inventory | PickupActor.cpp | 149 | Synchronous mesh load on discard |
| SUGG-12 | Suggestion | Player | StaminaComponent.cpp | 73-90 | Verbose logs still format strings |
| SUGG-13 | Suggestion | Inventory | SereneHUD.cpp | 98-101 | Diagnostic log at Log verbosity |
| SUGG-14 | Suggestion | Inventory | InventorySlotWidget.cpp | 28-57 | Excessive diagnostic logging per slot |
| SUGG-15 | Suggestion | Player | StaminaBarWidget.cpp | 79-91 | NativeTick for timer that rarely fires |
| SUGG-16 | Suggestion | Player | SereneCharacter.cpp | 154-200 | Early-out when no offsets to apply |

---

*End of audit. No code changes were made. This document is read-only analysis.*
