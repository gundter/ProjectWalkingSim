---
phase: 04-monster-ai-core
plan: 05
subsystem: ai
tags: [perception, suspicion, hearing, noise, footsteps, UAISense_Hearing, VisibilityScore]

# Dependency graph
requires:
  - phase: 04-02
    provides: "AWendigoCharacter with SuspicionComponent, EAlertLevel, AIConstants"
  - phase: 04-03
    provides: "AWendigoAIController with AIPerceptionComponent (sight+hearing), OnTargetPerceptionUpdated stub"
  - phase: 01-05
    provides: "UFootstepComponent with OnFootstep delegate (SurfaceType, Volume)"
  - phase: 03-04
    provides: "UVisibilityScoreComponent with GetVisibilityScore()"
provides:
  - "Perception-to-suspicion pipeline: sight feeds VisibilityScore into SuspicionComponent per-frame"
  - "Hearing perception triggers immediate suspicion bumps with stimulus location"
  - "Suspicion decay when player not in sight"
  - "UNoiseReportingComponent: sprint footstep noise bridge to AI hearing"
  - "Sprint-only noise filtering (walk/crouch silent to AI)"
affects: [05-monster-behaviors, 06-light-audio]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Tick-based continuous perception processing (sight accumulation needs DeltaTime)"
    - "Delegate-based event routing for discrete perception events (hearing)"
    - "Volume threshold filtering for noise reporting (sprint > 1.0)"

key-files:
  created:
    - "Source/ProjectWalkingSim/Public/AI/NoiseReportingComponent.h"
    - "Source/ProjectWalkingSim/Private/AI/NoiseReportingComponent.cpp"
  modified:
    - "Source/ProjectWalkingSim/Public/AI/WendigoAIController.h"
    - "Source/ProjectWalkingSim/Private/AI/WendigoAIController.cpp"
    - "Source/ProjectWalkingSim/Public/Player/SereneCharacter.h"
    - "Source/ProjectWalkingSim/Private/Player/SereneCharacter.cpp"

key-decisions:
  - "Task 1 already implemented by parallel 04-04 agent; no duplicate commit needed"
  - "Volume threshold > 1.0 (not >=) ensures walk footsteps at exactly 1.0 are silent"
  - "NoiseReportingComponent lives on player (noise source), not on Wendigo"

patterns-established:
  - "Player-side noise reporting: noise sources report via UAISense_Hearing::ReportNoiseEvent"
  - "AI controller Tick for continuous perception; delegate for discrete events"

# Metrics
duration: 14min
completed: 2026-02-11
---

# Phase 4 Plan 05: Perception-Suspicion Pipeline and Sprint Noise Summary

**Perception-to-suspicion wiring: sight feeds VisibilityScore into per-frame accumulation, hearing triggers suspicion bumps, NoiseReportingComponent bridges sprint footsteps to AI hearing**

## Performance

- **Duration:** ~14 min
- **Started:** 2026-02-11T16:00:43Z
- **Completed:** 2026-02-11T16:14:30Z
- **Tasks:** 2 (Task 1 pre-completed by parallel agent)
- **Files modified:** 6 (2 created, 4 modified)

## Accomplishments
- Perception-to-suspicion pipeline fully wired: sight accumulates suspicion via VisibilityScore, hearing applies immediate bumps
- Suspicion decays automatically when no player is in sight (Tick-based)
- NoiseReportingComponent created as bridge between FootstepComponent and AI hearing
- Sprint-only noise filtering: walk (1.0) and crouch (0.3) are silent to AI; sprint (1.5) generates noise events

## Task Commits

Each task was committed atomically:

1. **Task 1: Perception-to-Suspicion wiring in AI Controller** - `b99d36c` (feat, pre-completed by parallel 04-04 agent)
   - Tick: reads VisibilityScore from perceived actors, feeds ProcessSightStimulus
   - Tick: DecaySuspicion when no player in sight
   - OnTargetPerceptionUpdated routes to ProcessSightPerception / ProcessHearingPerception
   - ProcessHearingPerception calls SuspicionComponent->ProcessHearingStimulus with location
2. **Task 2: Sprint noise reporting component** - `bababb2` (feat)
   - UNoiseReportingComponent: binds to FootstepComponent::OnFootstep, reports noise for sprint volume
   - Integrated on ASereneCharacter as 10th component

**Plan metadata:** (pending)

## Files Created/Modified
- `Source/ProjectWalkingSim/Public/AI/NoiseReportingComponent.h` - Sprint noise bridge component declaration
- `Source/ProjectWalkingSim/Private/AI/NoiseReportingComponent.cpp` - HandleFootstep filters by volume, calls ReportNoiseEvent
- `Source/ProjectWalkingSim/Public/AI/WendigoAIController.h` - Added Tick override, ProcessSightPerception, ProcessHearingPerception
- `Source/ProjectWalkingSim/Private/AI/WendigoAIController.cpp` - Full perception pipeline: Tick sight accumulation, delegate routing, hearing bump
- `Source/ProjectWalkingSim/Public/Player/SereneCharacter.h` - Added NoiseReportingComponent UPROPERTY and forward decl
- `Source/ProjectWalkingSim/Private/Player/SereneCharacter.cpp` - Created NoiseReportingComponent in constructor, updated component log

## Decisions Made
- **Task 1 pre-completed by parallel agent:** The 04-04 plan agent implemented the full perception-to-suspicion wiring as part of its own work (it needed perception active for patrol task testing). No duplicate commit was created; the existing `b99d36c` commit contains all Task 1 changes.
- **Volume threshold uses > not >=:** `Volume > SprintVolumeThreshold` (where threshold=1.0) ensures walk footsteps at exactly 1.0 volume are silent. Only sprint (1.5) generates noise. This matches the plan requirement: "Walking and crouching footsteps do NOT generate AI noise."
- **NoiseReportingComponent on player, not Wendigo:** The player is the noise source. The component lives where the noise originates, reporting outward to the AI perception system.

## Deviations from Plan

### Discovery: Task 1 Pre-completed by Parallel Agent

**Found during:** Task 1 execution
**Issue:** The parallel 04-04 agent (wave 3 peer) had already implemented the full perception-to-suspicion wiring in WendigoAIController as part of commit `b99d36c`. This included all Tick processing, OnTargetPerceptionUpdated routing, ProcessSightPerception, ProcessHearingPerception, and the Instigator->NoiseInstigator rename to avoid AActor::Instigator shadow.
**Resolution:** Verified the committed code matches plan 04-05 Task 1 spec exactly. No duplicate commit needed. Task 1 credited to `b99d36c`.
**Impact:** No negative impact. This is a known pattern with parallel wave execution (documented in STATE.md technical discoveries).

---

**Total deviations:** 1 discovery (parallel agent pre-completion)
**Impact on plan:** No scope change. Task 1 work was done correctly by parallel agent. Task 2 executed as planned.

## Issues Encountered
None.

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- Perception-to-suspicion pipeline is complete: the Wendigo can now see the player (visibility-based suspicion accumulation) and hear sprint footsteps (immediate suspicion bump)
- Suspicion decay works automatically when player is not perceived
- Ready for Phase 5 (Monster Behaviors): chase, investigate, and search behaviors can now read alert levels from SuspicionComponent
- Remaining Phase 4 plans (04-06, 04-07) can build on this pipeline for editor assets and PIE verification

---
*Phase: 04-monster-ai-core*
*Completed: 2026-02-11*
