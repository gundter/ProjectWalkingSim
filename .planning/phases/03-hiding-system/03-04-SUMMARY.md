---
phase: 03-hiding-system
plan: 04
subsystem: hiding
tags: [state-machine, montage, camera-blend, input-context, component]

requires:
  - phase: 03-hiding-system
    provides: "EHidingState enum, FOnHidingStateChanged delegate, UHidingSpotDataAsset"
  - phase: 03-hiding-system
    provides: "AHidingSpotActor implementing IHideable with HidingCamera"
  - phase: 01-foundation
    provides: "InteractionComponent, HeadBobComponent, LeanComponent, FootstepComponent"
  - phase: 03-hiding-system
    provides: "UVisibilityScoreComponent with SetHidingReduction API"
provides:
  - "UHidingComponent with 4-state machine (Free/Entering/Hidden/Exiting)"
  - "EnterHidingSpot/ExitHidingSpot API for HidingSpotActor integration"
  - "Camera blend via SetViewTargetWithBlend (cubic)"
  - "Look constraints relative to hiding spot world yaw"
  - "Input context switching (default IMC removed, hiding IMC added)"
  - "Player system disable/restore (movement, interaction, headbob, lean, footstep)"
affects:
  - 03-hiding-system (plan 05 integrates HidingComponent with interaction and HUD)
  - 04-monster-ai-core (reads HidingState for perception decisions)
  - 05-monster-behaviors (discovery system interacts with hiding state)

tech-stack:
  added: []
  patterns:
    - "Montage completion delegates for state transitions (not tick polling)"
    - "SetViewTargetWithBlend for camera ownership transfer"
    - "IMC swap pattern: remove default context, add specialized context at higher priority"

key-files:
  created:
    - "Source/ProjectWalkingSim/Public/Hiding/HidingComponent.h"
    - "Source/ProjectWalkingSim/Private/Hiding/HidingComponent.cpp"
  modified: []

key-decisions:
  - "Event-driven state machine (PrimaryComponentTick disabled, montage delegates drive transitions)"
  - "DefaultMappingContext stored as config UPROPERTY on HidingComponent (avoids accessing protected member on controller)"
  - "All skeletal mesh components hidden via GetComponents iteration (covers main mesh and WorldRepresentationMesh)"
  - "Exit input binding always exists but only fires when HidingMappingContext is active (Enhanced Input pattern)"

patterns-established:
  - "Montage_SetEndDelegate called AFTER Montage_Play (UE pitfall avoidance)"
  - "ViewYaw limits computed as SpotWorldYaw + DataAssetYawOffset (world-space relative)"
  - "Player system disable/restore pattern via SetComponentTickEnabled and SetMovementMode"

duration: 4min
completed: 2026-02-10
---

# Phase 3 Plan 4: HidingComponent State Machine Summary

**Event-driven 4-state hiding lifecycle with montage playback, SetViewTargetWithBlend camera transitions, world-yaw-relative look constraints, and IMC context switching**

## Performance

- **Duration:** 4 min
- **Started:** 2026-02-10T19:21:10Z
- **Completed:** 2026-02-10T19:25:35Z
- **Tasks:** 1
- **Files created:** 2

## Accomplishments
- Created UHidingComponent with complete 4-state machine (Free -> Entering -> Hidden -> Exiting -> Free)
- EnterHidingSpot plays entry montage via Montage_Play, blends camera to hiding spot via SetViewTargetWithBlend (cubic), disables movement and peripheral systems
- Montage completion drives state transitions via FOnMontageEnded delegates (not tick polling)
- TransitionToHiddenState hides player mesh, applies look constraints relative to spot world yaw, switches to hiding input context, applies visibility score reduction
- ExitHidingSpot plays exit montage, blends camera back to player character, restores all systems
- DisablePlayerSystems freezes CharacterMovementComponent and disables tick on InteractionComponent, HeadBobComponent, LeanComponent, FootstepComponent
- Input context switching removes default IMC and adds hiding IMC (look + exit only) at higher priority
- BeginPlay binds ExitHidingAction to ExitHidingSpot (always bound, only fires when hiding IMC active)

## Task Commits

Each task was committed atomically:

1. **Task 1: HidingComponent state machine with montage and camera control** - `33d9e44` (feat)

## Files Created/Modified
- `Source/ProjectWalkingSim/Public/Hiding/HidingComponent.h` - UHidingComponent with public API (Enter/Exit/GetState/IsHiding/IsInTransition), delegate, input config, private state machine methods
- `Source/ProjectWalkingSim/Private/Hiding/HidingComponent.cpp` - Full implementation: entry/exit flows, montage callbacks, camera constraints, player system control, mesh visibility, input context switching

## Decisions Made
- PrimaryComponentTick disabled (event-driven via montage delegates, no polling needed)
- DefaultMappingContext stored as EditAnywhere UPROPERTY on HidingComponent rather than accessing protected member on SerenePlayerController
- All skeletal mesh components hidden via GetComponents<USkeletalMeshComponent> iteration to cover both main mesh and WorldRepresentationMesh without needing protected access
- Exit input binding registered in BeginPlay on the input component; only fires when HidingMappingContext is active (standard Enhanced Input pattern)
- Interrupted entry montage aborts to Free state; interrupted exit montage also transitions to Free (safe fallback)

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None.

## User Setup Required

After plan 03-05 (integration), the following Blueprint setup will be needed:
- Create IA_ExitHiding input action asset
- Create IMC_Hiding input mapping context with Look + ExitHiding mappings
- Assign DefaultMappingContext, HidingMappingContext, and ExitHidingAction on HidingComponent in BP

## Next Phase Readiness
- HidingComponent is ready for Plan 03-05 (integration) which wires HidingSpotActor::OnInteract to HidingComponent::EnterHidingSpot
- Plan 03-05 will add HidingComponent to ASereneCharacter and handle HUD feedback
- Plan 03-06 (verification) will validate the full hiding flow end-to-end
- No blockers

---
*Phase: 03-hiding-system*
*Completed: 2026-02-10*
