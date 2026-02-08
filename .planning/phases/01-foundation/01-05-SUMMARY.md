---
phase: 01-foundation
plan: 05
subsystem: footsteps-hud-integration
tags: [footstep, surface-detection, stamina-bar, hud, widget, integration, component-wiring]

dependency-graph:
  requires:
    - "01-03: StaminaComponent (OnStaminaChanged delegate), HeadBobComponent, LeanComponent"
    - "01-04: InteractionComponent (OnInteractableChanged delegate), InteractionPromptWidget"
  provides:
    - "UFootstepComponent: surface detection via downward trace with physical material query"
    - "UStaminaBarWidget: progress bar with auto-show/hide on stamina change"
    - "ASereneHUD: widget lifecycle manager binding component delegates to UI"
    - "Complete character with all 5 components created in constructor"
    - "GameMode with character, controller, and HUD class defaults"
  affects:
    - "Phase 4/5 (AI noise detection listens to FootstepComponent::OnFootstep)"
    - "Phase 6 (audio playback listens to FootstepComponent::OnFootstep)"
    - "Phase 8 (HUD polish can extend SereneHUD with additional widgets)"

tech-stack:
  added: []
  patterns:
    - "Timer-based footstep with AnimNotify override toggle"
    - "bReturnPhysicalMaterial + DetermineSurfaceType for surface detection"
    - "Widget auto-show/hide with delay timer in NativeTick"
    - "HUD deferred pawn binding via timer for BeginPlay timing safety"
    - "FindComponentByClass for HUD-to-component delegate binding"

key-files:
  created:
    - "Source/ProjectWalkingSim/Public/Player/Components/FootstepComponent.h"
    - "Source/ProjectWalkingSim/Private/Player/Components/FootstepComponent.cpp"
    - "Source/ProjectWalkingSim/Public/Player/HUD/StaminaBarWidget.h"
    - "Source/ProjectWalkingSim/Private/Player/HUD/StaminaBarWidget.cpp"
    - "Source/ProjectWalkingSim/Public/Player/HUD/SereneHUD.h"
    - "Source/ProjectWalkingSim/Private/Player/HUD/SereneHUD.cpp"
  modified:
    - "Source/ProjectWalkingSim/Public/Player/SereneCharacter.h (updated docs, added FootstepComponent)"
    - "Source/ProjectWalkingSim/Private/Player/SereneCharacter.cpp (FootstepComponent creation, component status logging)"
    - "Source/ProjectWalkingSim/Private/Core/SereneGameMode.cpp (added HUDClass = ASereneHUD)"

key-decisions:
  - "01-05-d1: Timer-based footsteps as default with bUseAnimNotify toggle for AnimNotify mode"
  - "01-05-d2: StaminaBarWidget auto-hides 2 seconds after stamina reaches 100%"
  - "01-05-d3: SereneHUD defers pawn binding via 0.1s timer when pawn not yet possessed in BeginPlay"

patterns-established:
  - "Surface detection pattern: bReturnPhysicalMaterial + UPhysicalMaterial::DetermineSurfaceType"
  - "Widget auto-visibility: show on state change, hide with delay after state normalizes"
  - "Central HUD class managing widget creation and delegate binding"

metrics:
  duration: "~3min"
  completed: "2026-02-08"
---

# Phase 1 Plan 05: Footsteps, Stamina HUD, and System Integration Summary

**FootstepComponent with surface detection, StaminaBarWidget with auto-show/hide, SereneHUD managing widget lifecycle, all 5 character components wired**

## Performance

- **Duration:** ~3 min
- **Started:** 2026-02-08T11:17:39Z
- **Completed:** 2026-02-08T11:20:50Z
- **Tasks:** 2/2
- **Files modified:** 6 created, 3 modified

## Accomplishments

- UFootstepComponent: timer-based footstep trigger with downward line trace, bReturnPhysicalMaterial=true for surface detection, sprint/crouch interval multipliers (0.6x/1.3x), volume scaling per movement state (walk=1.0, sprint=1.5, crouch=0.3), OnFootstep delegate broadcasts surface type and volume, bUseAnimNotify toggle for AnimNotify-driven mode
- UStaminaBarWidget: UProgressBar-based fill display, auto-shows when stamina < 100%, auto-hides 2 seconds after stamina reaches 100%, optional FadeAnimation via BindWidgetAnimOptional, NativeTick accumulates hide delay timer
- ASereneHUD: creates StaminaBarWidget and InteractionPromptWidget from TSubclassOf properties, binds StaminaComponent::OnStaminaChanged to stamina bar, binds InteractionComponent::OnInteractableChanged to interaction prompt, deferred pawn binding via timer for BeginPlay timing safety
- All 5 components now created in ASereneCharacter constructor: Stamina, HeadBob, Lean, Interaction, Footstep
- ASereneGameMode sets HUDClass = ASereneHUD (character + controller + HUD all configured)
- Component status logging in BeginPlay confirms all 5 components present

## Task Commits

Each task was committed atomically:

1. **Task 1: Create FootstepComponent with surface detection** - `1fe76b8` (feat)
2. **Task 2: StaminaBarWidget, SereneHUD, and full system wiring** - `3c152c6` (feat)

## Files Created/Modified

- `Source/ProjectWalkingSim/Public/Player/Components/FootstepComponent.h` - UActorComponent with surface trace, timer-based trigger, OnFootstep delegate
- `Source/ProjectWalkingSim/Private/Player/Components/FootstepComponent.cpp` - TickComponent timer logic, PlayFootstepForSurface with bReturnPhysicalMaterial trace
- `Source/ProjectWalkingSim/Public/Player/HUD/StaminaBarWidget.h` - UUserWidget with BindWidget ProgressBar, auto-show/hide logic
- `Source/ProjectWalkingSim/Private/Player/HUD/StaminaBarWidget.cpp` - SetStaminaPercent, ShowBar, HideBar, NativeTick hide delay
- `Source/ProjectWalkingSim/Public/Player/HUD/SereneHUD.h` - AHUD with widget class references and delegate handlers
- `Source/ProjectWalkingSim/Private/Player/HUD/SereneHUD.cpp` - Widget creation, delegate binding, deferred pawn binding
- `Source/ProjectWalkingSim/Public/Player/SereneCharacter.h` - Updated docs for all 5 components, removed "Future" section
- `Source/ProjectWalkingSim/Private/Player/SereneCharacter.cpp` - Added FootstepComponent include + CreateDefaultSubobject, component status logging
- `Source/ProjectWalkingSim/Private/Core/SereneGameMode.cpp` - Added HUDClass = ASereneHUD::StaticClass()

## Decisions Made

| ID | Decision | Rationale |
|----|----------|-----------|
| 01-05-d1 | Timer-based footsteps as default, bUseAnimNotify toggle | No animation assets configured in Phase 1; timer is reliable fallback; AnimNotify ready for later |
| 01-05-d2 | StaminaBarWidget auto-hides 2s after full stamina | Matches CONTEXT.md "stamina bar only shows while sprinting then fades"; 2s delay prevents flicker |
| 01-05-d3 | SereneHUD defers pawn binding via 0.1s timer | BeginPlay timing: pawn may not be possessed yet when HUD initializes; timer is simple and reliable |

## Deviations from Plan

None -- plan executed exactly as written.

## Issues Encountered

None.

## User Setup Required

- Create UMG Blueprint **WBP_StaminaBar** in the editor (reparent to UStaminaBarWidget)
- Add a ProgressBar widget named exactly "StaminaBar" in the UMG layout
- Optionally add a UWidgetAnimation named "FadeAnimation" for smooth show/hide transitions
- Assign WBP_StaminaBar and WBP_InteractionPrompt classes on the BP_SereneHUD defaults (EditDefaultsOnly properties)
- Physical materials must be assigned to floor surfaces for footstep surface detection to return meaningful types

## Next Plan Readiness

Plan 01-06 can proceed immediately:
- All 5 components are created and wired on ASereneCharacter
- Full input-to-interaction pipeline works end to end
- HUD widget system is in place and ready for additional widgets
- Footstep system broadcasts events for future audio/AI integration

No blockers identified.

---
*Phase: 01-foundation*
*Completed: 2026-02-08*
