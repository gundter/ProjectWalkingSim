---
phase: 08-demo-polish
plan: 02
subsystem: ui, interaction
tags: [umg, widget, document-reader, inspectable, environmental-storytelling]

# Dependency graph
requires:
  - phase: 01-foundation
    provides: InteractableBase actor framework, interaction system
  - phase: 08-demo-polish (plan 01)
    provides: SereneTags TAG_Interaction_Inspectable, TAG_Narrative_Trigger
provides:
  - UDocumentReaderWidget: full-screen document/inspection reader with E/Esc close
  - AInspectableActor: story object examination with image, text, one-time monologue
affects: [08-demo-polish plan 03 (controller wiring, ReadableActor integration)]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Standalone viewport widget pattern (CreateWidget per-use, AddToViewport, RemoveFromParent on close)"
    - "TSoftObjectPtr<UTexture2D> with LoadSynchronous for designer-assigned inspection images"
    - "One-shot boolean guard for first-inspect monologue"

key-files:
  created:
    - Source/ProjectWalkingSim/Public/Player/HUD/DocumentReaderWidget.h
    - Source/ProjectWalkingSim/Private/Player/HUD/DocumentReaderWidget.cpp
    - Source/ProjectWalkingSim/Public/Interaction/InspectableActor.h
    - Source/ProjectWalkingSim/Private/Interaction/InspectableActor.cpp
  modified: []

key-decisions:
  - "Widget created per-use by triggering actors, not pre-created by controller"
  - "E and Esc both close the document reader via NativeOnKeyDown"
  - "No mouse cursor shown -- keyboard-driven close only"

patterns-established:
  - "Per-use widget creation: actors create fresh widget instance, widget removes itself on close"
  - "Dual-mode widget: ShowDocument() for text, ShowInspection() for image+text"

# Metrics
duration: 3min
completed: 2026-02-13
---

# Phase 8 Plan 02: Storytelling UI (DocumentReaderWidget + InspectableActor) Summary

**Full-screen DocumentReaderWidget with dual document/inspection modes and InspectableActor for story object examination with one-time monologue**

## Performance

- **Duration:** ~3 min
- **Started:** 2026-02-13T18:59:44Z
- **Completed:** 2026-02-13T19:03:07Z
- **Tasks:** 2/2
- **Files created:** 4

## Accomplishments
- DocumentReaderWidget supports two modes: document (title + body text) and inspection (image + description)
- NativeOnKeyDown handles E and Esc keys to close, with FInputModeUIOnly/FInputModeGameOnly lifecycle
- InspectableActor creates widget on interaction, loads TSoftObjectPtr texture, plays optional one-shot monologue
- FOnDocumentClosed delegate allows actors to respond to document close events

## Task Commits

Each task was committed atomically:

1. **Task 1: DocumentReaderWidget with full-screen text display and keyboard close** - `e165ef6` (feat)
2. **Task 2: InspectableActor for story object examination** - `acac273` (feat)

## Files Created/Modified
- `Source/ProjectWalkingSim/Public/Player/HUD/DocumentReaderWidget.h` - Full-screen document/inspection reader widget class
- `Source/ProjectWalkingSim/Private/Player/HUD/DocumentReaderWidget.cpp` - ShowDocument, ShowInspection, CloseDocument, NativeOnKeyDown implementation
- `Source/ProjectWalkingSim/Public/Interaction/InspectableActor.h` - Story object examination actor (photos, toys, clues)
- `Source/ProjectWalkingSim/Private/Interaction/InspectableActor.cpp` - OnInteract creates widget, loads image, plays one-time monologue

## Decisions Made
- Widget created per-use by triggering actors (not pre-created by controller) -- avoids stale state, each interaction gets fresh instance
- Both E and Esc close the document reader -- E matches the interaction key, Esc is universal close
- No mouse cursor shown while document is open -- keyboard-driven close only, consistent with horror immersion
- FOnDocumentClosed delegate declared before UCLASS -- follows PauseMenuWidget pattern for dynamic multicast delegates

## Deviations from Plan

None -- plan executed exactly as written. TAG_Interaction_Inspectable was already defined by 08-01.

## Issues Encountered

None

## User Setup Required

After execution, create UMG Blueprint in editor:
- WBP_DocumentReader (reparent to UDocumentReaderWidget)
  - Add UTextBlock named "TitleText"
  - Add UTextBlock named "ContentText"
  - Add UImage named "InspectionImage"
- Set DocumentReaderWidgetClass on InspectableActor instances placed in levels

## Next Phase Readiness
- DocumentReaderWidget ready for ReadableActor integration (Plan 03 controller wiring)
- InspectableActor ready for level placement with per-instance image, text, and monologue
- ReadableActor still uses log-only OnInteract -- needs Plan 03 to wire up DocumentReaderWidget

---
*Phase: 08-demo-polish*
*Completed: 2026-02-13*
