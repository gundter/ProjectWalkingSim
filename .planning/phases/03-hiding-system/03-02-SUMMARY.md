---
phase: 03-hiding-system
plan: 02
subsystem: visibility
tags: [ue5, c++, scene-capture, render-target, light-sampling, visibility, hdr]

# Dependency graph
requires:
  - phase: 01-foundation
    provides: ASereneCharacter with bIsCrouched, LogSerene category
  - phase: 03-hiding-system
    plan: 01
    provides: IHideable interface, hiding types, gameplay tags
provides:
  - UVisibilityScoreComponent with GetVisibilityScore() returning 0.0-1.0
  - Timer-based SceneCaptureComponent2D light sampling pipeline
  - SetHidingReduction() API for HidingComponent integration
  - RenderCore and RHI module dependencies in Build.cs
affects: [03-03, 03-04, 03-05, "Phase 4 (AI perception reads GetVisibilityScore)"]

# Tech tracking
tech-stack:
  added:
    - "RenderCore module"
    - "RHI module"
  patterns:
    - "SceneCaptureComponent2D with tiny HDR render target for light sampling"
    - "ReadFloat16Pixels with Rec.709 luminance for brightness quantification"
    - "Timer-driven periodic capture (0.25s) instead of per-frame"
    - "ShowFlags optimization to minimize scene capture cost"

key-files:
  created:
    - Source/ProjectWalkingSim/Public/Visibility/VisibilityScoreComponent.h
    - Source/ProjectWalkingSim/Private/Visibility/VisibilityScoreComponent.cpp
  modified:
    - Source/ProjectWalkingSim/ProjectWalkingSim.Build.cs

key-decisions:
  - "UActorComponent not USceneComponent -- SceneCapture is a child of the owner actor"
  - "bIsCrouched from ACharacter base avoids circular dependency with Player/ directory"
  - "8x8 HDR render target balances accuracy with minimal GPU cost"
  - "ShowFlags disables Bloom, MotionBlur, Particles, SkeletalMeshes, Fog, PostProcessing"
  - "Lumen GI and ReflectionEnvironment kept enabled for correct indirect light readings"

patterns-established:
  - "Periodic light sampling: CaptureScene() on timer, ReadFloat16Pixels, Rec.709 luminance average"
  - "Modifier stacking: raw light - crouch reduction - hiding reduction = final score"
  - "SetHidingReduction pattern: HidingComponent calls setter, VisibilityComponent owns score calculation"

# Metrics
duration: 3min
completed: 2026-02-10
---

# Phase 3 Plan 02: Visibility Score Component Summary

**SceneCaptureComponent2D light sampling with 8x8 HDR render target, Rec.709 luminance averaging, and crouch/hiding modifiers outputting 0.0-1.0 score**

## Performance

- **Duration:** 3 min
- **Started:** 2026-02-10T19:15:41Z
- **Completed:** 2026-02-10T19:18:18Z
- **Tasks:** 2
- **Files modified:** 3

## Accomplishments

- Created UVisibilityScoreComponent that quantifies player visibility from actual scene lighting
- SceneCaptureComponent2D renders to 8x8 RGBA16f render target on configurable timer (0.25s default)
- ReadFloat16Pixels extracts HDR pixel data, averaged using Rec.709 luminance coefficients
- Crouch reduces visibility by 0.15, hiding spots reduce by configurable amount via SetHidingReduction()
- ShowFlags optimized: disables Bloom, MotionBlur, Particles, SkeletalMeshes, Fog, PostProcessing
- Lumen GI and ReflectionEnvironment kept enabled for accurate indirect light readings
- Added RenderCore and RHI module dependencies to Build.cs

## Task Commits

Each task was committed atomically:

1. **Task 1: VisibilityScoreComponent with SceneCapture light sampling** - `06a62c3` (feat)
2. **Task 2: Add RenderCore and RHI module dependencies to Build.cs** - `08a0f56` (chore)

## Files Created/Modified

**Created:**
- `Source/ProjectWalkingSim/Public/Visibility/VisibilityScoreComponent.h` - UActorComponent with GetVisibilityScore(), GetRawLightLevel(), SetHidingReduction(), configurable capture settings
- `Source/ProjectWalkingSim/Private/Visibility/VisibilityScoreComponent.cpp` - Full light sampling pipeline: BeginPlay creates SceneCapture + RenderTarget, timer drives PerformCapture, ComputeScore reads pixels and applies modifiers

**Modified:**
- `Source/ProjectWalkingSim/ProjectWalkingSim.Build.cs` - Added RenderCore and RHI to PublicDependencyModuleNames

## Decisions Made

- **UActorComponent over USceneComponent:** SceneCaptureComponent2D is created as a child of the owning actor, not as a subobject of this component. UActorComponent is sufficient.
- **ACharacter::bIsCrouched over ASereneCharacter::GetIsCrouching():** Using the engine's built-in crouch state avoids circular dependency between Visibility/ and Player/ directories. bIsCrouched is set by the CMC and is accurate.
- **8x8 capture resolution:** 64 pixels provides enough data for luminance averaging while being trivially cheap to render and read back.
- **ShowFlags optimization:** Disabling Bloom, MotionBlur, Particles, SkeletalMeshes, Fog, and PostProcessing reduces capture cost significantly. Only static geometry and lighting matter for brightness.
- **GI and Reflections kept enabled:** Lumen indirect illumination is critical for correct light readings in a game that uses darkness as a gameplay mechanic.
- **90-degree FOV for capture:** Captures the hemisphere above the player, sampling the general ambient light around their position.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None.

## Next Phase Readiness

- UVisibilityScoreComponent is ready to attach to ASereneCharacter (Plan 03 or later)
- HidingComponent (Plan 03-04) will call SetHidingReduction() when player enters/exits hiding spots
- AI perception (Phase 4) will read GetVisibilityScore() for sight checks
- GetRawLightLevel() available for debug HUD display

---
*Phase: 03-hiding-system*
*Completed: 2026-02-10*
