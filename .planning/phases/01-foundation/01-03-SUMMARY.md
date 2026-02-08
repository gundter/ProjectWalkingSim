---
phase: 01-foundation
plan: 03
subsystem: player-movement-components
tags: [stamina, head-bob, lean, camera-offset, component-architecture, delegates]

dependency-graph:
  requires:
    - "01-02: ASereneCharacter with FP camera, sprint/crouch state, CMC config"
    - "01-02: ASerenePlayerController with lean input handler stubs"
    - "01-01: LogSerene, USereneGameInstance with bHeadBobEnabled"
  provides:
    - "UStaminaComponent with drain/regen/exhaustion and three delegates"
    - "UHeadBobComponent with procedural sine-wave bob and accessibility toggle"
    - "ULeanComponent with lateral offset and camera roll"
    - "Camera offset aggregation pattern in ASereneCharacter::Tick()"
    - "Stamina depletion forces sprint stop via OnStaminaDepleted delegate"
    - "Lean input wired from PlayerController to LeanComponent"
  affects:
    - "01-04 (InteractionComponent already wired; no conflict)"
    - "01-06 (FootstepComponent can read bIsSprinting, bIsCrouching for audio variation)"
    - "Future HUD plans (OnStaminaChanged delegate ready for stamina bar widget)"
    - "Phase 4+ (AI can query IsExhausted for player vulnerability detection)"

tech-stack:
  added: []
  patterns:
    - "Component offset aggregation: components compute, character applies combined camera transform"
    - "DeltaTime sine-wave accumulation for frame-rate-independent oscillation"
    - "Dynamic multicast delegate binding for component-to-character communication"
    - "Exhaustion threshold to prevent sprint-tap spam at low stamina"

key-files:
  created:
    - "Source/ProjectWalkingSim/Public/Player/Components/StaminaComponent.h"
    - "Source/ProjectWalkingSim/Private/Player/Components/StaminaComponent.cpp"
    - "Source/ProjectWalkingSim/Public/Player/Components/HeadBobComponent.h"
    - "Source/ProjectWalkingSim/Private/Player/Components/HeadBobComponent.cpp"
    - "Source/ProjectWalkingSim/Public/Player/Components/LeanComponent.h"
    - "Source/ProjectWalkingSim/Private/Player/Components/LeanComponent.cpp"
  modified:
    - "Source/ProjectWalkingSim/Public/Player/SereneCharacter.h"
    - "Source/ProjectWalkingSim/Private/Player/SereneCharacter.cpp"
    - "Source/ProjectWalkingSim/Private/Player/SerenePlayerController.cpp"

key-decisions:
  - "01-03-d1: Camera offset aggregation pattern -- components compute offsets, character sums and applies in Tick()"
  - "01-03-d2: ExhaustionThreshold (20%) prevents sprint-tap spam after depletion"
  - "01-03-d3: Sprint detection in HeadBob via MaxWalkSpeed > 400 check (reads CMC state, no tight coupling)"
  - "01-03-d4: OnStaminaChanged only broadcasts when ratio actually changes (KINDA_SMALL_NUMBER threshold)"

patterns-established:
  - "Component offset pattern: HeadBob/Lean store offsets, character aggregates in Tick()"
  - "Delegate-driven component communication: OnStaminaDepleted -> StopSprint"
  - "GameInstance accessibility reads in BeginPlay for component configuration"

metrics:
  duration: "~5min"
  completed: "2026-02-08"
---

# Phase 1 Plan 03: Stamina, Head-Bob, and Lean Components Summary

**Stamina drain/regen with 20% exhaustion threshold, procedural sine-wave head-bob with accessibility toggle, and 30cm lateral lean with 5-degree camera roll -- all using component offset aggregation pattern**

## Performance

- **Duration:** ~5 min
- **Started:** 2026-02-08T17:08:57Z
- **Completed:** 2026-02-08T17:14:07Z
- **Tasks:** 2/2
- **Files modified:** 6 created, 3 modified

## Accomplishments

- UStaminaComponent: drains at 20/s during sprint, waits 1.5s regen delay, regens at 15/s, exhaustion state prevents re-sprint until 20% recovered, three delegates (OnStaminaChanged, OnStaminaDepleted, OnStaminaFull)
- UHeadBobComponent: sine-wave vertical bob + horizontal sway, sprint/crouch amplitude multipliers, smooth fade in/out, DeltaTime-based timer for frame-rate independence, toggleable via GameInstance setting
- ULeanComponent: 30cm lateral offset with 5-degree roll, smooth FInterpTo transitions, Q/E input wired from PlayerController
- Camera offset aggregation: character Tick() reads HeadBob and Lean offsets, sums them, applies once to camera relative transform with BaseCameraLocation reference
- StaminaComponent OnStaminaDepleted bound to ASereneCharacter::StopSprint in BeginPlay
- StartSprint now checks StaminaComponent::IsExhausted() before allowing sprint

## Task Commits

Each task was committed atomically:

1. **Task 1: Create StaminaComponent with drain, regen delay, and delegates** - `18d6c03` (feat)
2. **Task 2: Create HeadBobComponent, LeanComponent, and wire all three to character** - `5ddd973` (feat)

## Files Created/Modified

- `Source/ProjectWalkingSim/Public/Player/Components/StaminaComponent.h` - UStaminaComponent with drain/regen/exhaustion, 3 delegates, all EditAnywhere tuning
- `Source/ProjectWalkingSim/Private/Player/Components/StaminaComponent.cpp` - TickComponent drain/regen logic, SetSprinting with exhaustion check, change-only broadcasting
- `Source/ProjectWalkingSim/Public/Player/Components/HeadBobComponent.h` - UHeadBobComponent with amplitude/frequency/multiplier tuning, GetCurrentOffset()
- `Source/ProjectWalkingSim/Private/Player/Components/HeadBobComponent.cpp` - Sine-wave computation, sprint/crouch detection via CMC, smooth alpha interpolation
- `Source/ProjectWalkingSim/Public/Player/Components/LeanComponent.h` - ULeanComponent with distance/roll/speed tuning, GetLeanOffset(), GetLeanRoll()
- `Source/ProjectWalkingSim/Private/Player/Components/LeanComponent.cpp` - FInterpTo lean transitions, SetLeanLeft/Right, IsLeaning check
- `Source/ProjectWalkingSim/Public/Player/SereneCharacter.h` - Added Tick() override, BaseCameraLocation, OnStaminaDepleted callback, replaced forward decls with includes
- `Source/ProjectWalkingSim/Private/Player/SereneCharacter.cpp` - Constructor creates 3 components, BeginPlay binds delegate + reads GameInstance, Tick() aggregates camera offsets, StartSprint/StopSprint call StaminaComponent
- `Source/ProjectWalkingSim/Private/Player/SerenePlayerController.cpp` - Lean handlers now call LeanComponent SetLeanLeft/SetLeanRight instead of logging stubs

## Decisions Made

| ID | Decision | Rationale |
|----|----------|-----------|
| 01-03-d1 | Camera offset aggregation pattern (components compute, character applies) | Prevents HeadBob and Lean from fighting over camera SetRelativeLocation; single point of application |
| 01-03-d2 | ExhaustionThreshold at 20% before allowing re-sprint | Prevents sprint-tap spam when stamina is near zero; creates genuine tension in chase scenarios |
| 01-03-d3 | Sprint detection via MaxWalkSpeed > 400 check in HeadBob | Avoids direct dependency on character class; reads CMC state which is already set by sprint logic |
| 01-03-d4 | OnStaminaChanged only fires on actual change (KINDA_SMALL_NUMBER check) | Avoids unnecessary delegate broadcasts every tick when idle at full stamina |

## Deviations from Plan

None -- plan executed exactly as written. Note: the plan listed LeanComponent as "Created in Plan 05" in the original character header, but the plan itself (01-03-PLAN.md) specifies creating all three components in Plan 03. The header comments were updated accordingly.

## Issues Encountered

None.

## User Setup Required

None -- no external service configuration required. All components are automatically created in the character constructor. Head-bob respects the GameInstance bHeadBobEnabled setting which defaults to true.

## Next Plan Readiness

Plan 01-04 (Interaction System) can proceed immediately:
- InteractionComponent already exists and is wired to the character constructor
- HandleInteract in PlayerController already calls TryInteract on InteractionComponent
- The character has all three movement-feel components operational (stamina, head-bob, lean)
- LogSerene available for all component logging

No blockers identified.

---
*Phase: 01-foundation*
*Completed: 2026-02-08*
