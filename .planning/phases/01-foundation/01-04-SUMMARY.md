---
phase: 01-foundation
plan: 04
subsystem: interaction-system
tags: [interaction, line-trace, interactable, door, pickup, readable, drawer, hud-widget]

dependency-graph:
  requires:
    - "01-01: IInteractable interface, SereneTags (Interaction.Door/Pickup/Readable/Drawer), LogSerene"
    - "01-02: ASereneCharacter (InteractionComponent forward decl + UPROPERTY slot), ASerenePlayerController (HandleInteract)"
  provides:
    - "UInteractionComponent: camera-center line trace detection + focus management + delegate"
    - "UInteractionPromptWidget: C++ base with BindWidget slots for UMG prompt"
    - "AInteractableBase: abstract base implementing IInteractable with common properties"
    - "ADoorActor: animated door with player-side-aware swing direction"
    - "APickupActor: generic pickup placeholder for Phase 2 inventory"
    - "AReadableActor: document/note actor with title + content"
    - "ADrawerActor: sliding drawer with smooth interpolation"
  affects:
    - "Phase 2 (PickupActor::OnInteract will add inventory integration)"
    - "Phase 3 (hideable actors can extend AInteractableBase pattern)"
    - "01-06 (footstep surface detection uses similar line trace pattern)"

tech-stack:
  added: []
  patterns:
    - "Camera-center line trace for interaction (GetPlayerViewPoint, not GetActorLocation)"
    - "IInteractable Execute_ wrapper calls for interface polymorphism"
    - "FOnInteractableChanged delegate pattern for HUD decoupling"
    - "FInterpTo tick-based animation for doors and drawers"
    - "NSLOCTEXT for all player-facing interaction text"
    - "Abstract base class with virtual _Implementation overrides"

key-files:
  created:
    - "Source/ProjectWalkingSim/Public/Player/Components/InteractionComponent.h"
    - "Source/ProjectWalkingSim/Private/Player/Components/InteractionComponent.cpp"
    - "Source/ProjectWalkingSim/Public/Player/HUD/InteractionPromptWidget.h"
    - "Source/ProjectWalkingSim/Private/Player/HUD/InteractionPromptWidget.cpp"
    - "Source/ProjectWalkingSim/Public/Interaction/InteractableBase.h"
    - "Source/ProjectWalkingSim/Private/Interaction/InteractableBase.cpp"
    - "Source/ProjectWalkingSim/Public/Interaction/DoorActor.h"
    - "Source/ProjectWalkingSim/Private/Interaction/DoorActor.cpp"
    - "Source/ProjectWalkingSim/Public/Interaction/PickupActor.h"
    - "Source/ProjectWalkingSim/Private/Interaction/PickupActor.cpp"
    - "Source/ProjectWalkingSim/Public/Interaction/ReadableActor.h"
    - "Source/ProjectWalkingSim/Private/Interaction/ReadableActor.cpp"
    - "Source/ProjectWalkingSim/Public/Interaction/DrawerActor.h"
    - "Source/ProjectWalkingSim/Private/Interaction/DrawerActor.cpp"
  modified:
    - "Source/ProjectWalkingSim/Private/Player/SereneCharacter.cpp (added InteractionComponent creation)"
    - "Source/ProjectWalkingSim/Private/Player/SerenePlayerController.cpp (wired HandleInteract to TryInteract)"

key-decisions:
  - "01-04-d1: InteractionComponent re-broadcasts text after TryInteract for HUD refresh"
  - "01-04-d2: InteractionPromptWidget uses fixed screen-space layout, not world-space widget component"
  - "01-04-d3: Door swing direction determined by dot product of player-to-door forward vector"

patterns-established:
  - "Interaction actors inherit AInteractableBase, override _Implementation methods"
  - "Tick-based FInterpTo animation for physical state changes (door angle, drawer slide)"
  - "Gameplay tags on each interactable for type identification (SereneTags::TAG_Interaction_*)"

metrics:
  duration: "~5min"
  completed: "2026-02-08"
---

# Phase 1 Plan 04: Interaction System Summary

**Camera-center line trace interaction component with HUD prompt widget and four interactable actor types (door, pickup, readable, drawer)**

## Performance

- **Duration:** ~5 min
- **Started:** 2026-02-08T17:09:03Z
- **Completed:** 2026-02-08T17:13:45Z
- **Tasks:** 2/2
- **Files modified:** 14 created, 2 modified

## Accomplishments

- UInteractionComponent: per-tick line trace from camera center at 150cm range, detects IInteractable actors, manages focus state (OnFocusBegin/End), broadcasts OnInteractableChanged delegate with actor and text
- UInteractionPromptWidget: C++ base class with BindWidget meta slots (PromptText, ReticleText) for UMG Blueprint layout, formats prompt as "E: {Action}"
- AInteractableBase: abstract base actor implementing IInteractable with InteractionText, InteractionTag, MeshComponent, bCanBeInteracted
- ADoorActor: DoorMesh rotates via FInterpTo around Z axis, player-side-aware swing direction using dot product, toggles "Open"/"Close" text
- APickupActor: logs "Picked up {ItemId} x{Quantity}", destroys self on pickup (Phase 2 inventory stub)
- AReadableActor: logs "Reading: {Title}" on interaction (future text display UI deferred)
- ADrawerActor: DrawerMesh slides along local X axis via FInterpTo, toggles "Open"/"Close" text
- Wired InteractionComponent into ASereneCharacter constructor and HandleInteract in PlayerController

## Task Commits

Each task was committed atomically:

1. **Task 1: Create InteractionComponent and InteractionPromptWidget** - `b64c9ff` (feat)
2. **Task 2: Create InteractableBase and four interactable actor types** - `d3dfef2` (feat)

## Files Created/Modified

- `Source/ProjectWalkingSim/Public/Player/Components/InteractionComponent.h` - UActorComponent with per-tick trace, focus management, OnInteractableChanged delegate
- `Source/ProjectWalkingSim/Private/Player/Components/InteractionComponent.cpp` - PerformTrace (GetPlayerViewPoint), SetCurrentInteractable (focus begin/end), TryInteract
- `Source/ProjectWalkingSim/Public/Player/HUD/InteractionPromptWidget.h` - UUserWidget with BindWidget PromptText + ReticleText
- `Source/ProjectWalkingSim/Private/Player/HUD/InteractionPromptWidget.cpp` - UpdatePrompt formats "E: {Text}", HidePrompt collapses
- `Source/ProjectWalkingSim/Public/Interaction/InteractableBase.h` - Abstract base: IInteractable defaults, InteractionText, InteractionTag, MeshComponent
- `Source/ProjectWalkingSim/Private/Interaction/InteractableBase.cpp` - Default implementations (text, can-interact, empty focus/interact)
- `Source/ProjectWalkingSim/Public/Interaction/DoorActor.h` - DoorMesh, OpenAngle, OpenSpeed, bOpensTowardPlayer
- `Source/ProjectWalkingSim/Private/Interaction/DoorActor.cpp` - Dot product direction detection, FInterpTo rotation tick
- `Source/ProjectWalkingSim/Public/Interaction/PickupActor.h` - ItemId (FName), Quantity, bDestroyOnPickup
- `Source/ProjectWalkingSim/Private/Interaction/PickupActor.cpp` - Log + Destroy on interact
- `Source/ProjectWalkingSim/Public/Interaction/ReadableActor.h` - ReadableTitle, ReadableContent (MultiLine)
- `Source/ProjectWalkingSim/Private/Interaction/ReadableActor.cpp` - Log title on interact
- `Source/ProjectWalkingSim/Public/Interaction/DrawerActor.h` - DrawerMesh, OpenDistance, OpenSpeed
- `Source/ProjectWalkingSim/Private/Interaction/DrawerActor.cpp` - FInterpTo slide along local X axis
- `Source/ProjectWalkingSim/Private/Player/SereneCharacter.cpp` - Added InteractionComponent include and CreateDefaultSubobject
- `Source/ProjectWalkingSim/Private/Player/SerenePlayerController.cpp` - Replaced HandleInteract stub with FindComponentByClass + TryInteract

## Decisions Made

| ID | Decision | Rationale |
|----|----------|-----------|
| 01-04-d1 | Re-broadcast interaction text after TryInteract | Door text changes "Open" to "Close" after interaction; HUD needs immediate refresh |
| 01-04-d2 | Screen-space prompt widget, not world-space UWidgetComponent | Cleaner rendering, no lighting artifacts, still feels world-space via positioning |
| 01-04-d3 | Door swing direction via dot product | Natural behavior: door always opens away from the player's approach side |

## Deviations from Plan

None -- plan executed exactly as written.

## Issues Encountered

None.

## User Setup Required

- Create UMG Blueprint **WBP_InteractionPrompt** in the editor (reparent to UInteractionPromptWidget)
- Add two TextBlock widgets named exactly "PromptText" and "ReticleText" in the UMG layout
- Bind InteractionComponent::OnInteractableChanged delegate to widget UpdatePrompt in the HUD setup
- Place interactable actors in a test level with placeholder cube/plane meshes for verification

## Next Plan Readiness

Plan 01-05 (Lean Component) can proceed immediately:
- ASereneCharacter has TObjectPtr slot for LeanComponent (forward declared)
- Camera offset aggregation pattern is ready for lean offsets
- PlayerController has HandleLeanLeft/Right stubs ready to wire

No blockers identified.

---
*Phase: 01-foundation*
*Completed: 2026-02-08*
