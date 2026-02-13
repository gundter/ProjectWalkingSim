---
phase: 07-save-system
plan: 02
subsystem: save
tags: [ISaveable, USereneSaveGame, GameOver, death-system, GameMode, inventory-restore]

# Dependency graph
requires:
  - phase: 07-save-system
    provides: "SaveTypes.h structs, USereneSaveGame, USaveSubsystem with save/load API"
  - phase: 01-foundation
    provides: "DoorActor, PickupActor, DrawerActor, InteractableBase, InventoryComponent"
  - phase: 05-monster-behaviors
    provides: "STT_GrabAttack State Tree task"
provides:
  - "ISaveable expanded with USereneSaveGame* parameter on DoorActor, PickupActor, DrawerActor"
  - "SereneGameMode::OnPlayerDeath death pipeline with Game Over widget"
  - "SereneGameMode::InitGame -> OnActorsInitialized -> ApplyPendingSaveData load flow"
  - "InventoryComponent::RestoreSavedInventory for save-load inventory restoration"
  - "UGameOverWidget with Load Last Save / Restart / Quit buttons"
affects: [07-03, 07-04]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "ISaveable WriteSaveData/ReadSaveData with USereneSaveGame* parameter for per-actor state"
    - "OnActorsInitialized delegate for post-reload save data application"
    - "Game Over widget with dynamic button text based on save existence"

key-files:
  created:
    - "Source/ProjectWalkingSim/Public/Player/HUD/GameOverWidget.h"
    - "Source/ProjectWalkingSim/Private/Player/HUD/GameOverWidget.cpp"
  modified:
    - "Source/ProjectWalkingSim/Public/Interaction/SaveableInterface.h"
    - "Source/ProjectWalkingSim/Public/Interaction/DoorActor.h"
    - "Source/ProjectWalkingSim/Private/Interaction/DoorActor.cpp"
    - "Source/ProjectWalkingSim/Public/Interaction/PickupActor.h"
    - "Source/ProjectWalkingSim/Private/Interaction/PickupActor.cpp"
    - "Source/ProjectWalkingSim/Public/Interaction/DrawerActor.h"
    - "Source/ProjectWalkingSim/Private/Interaction/DrawerActor.cpp"
    - "Source/ProjectWalkingSim/Public/Core/SereneGameMode.h"
    - "Source/ProjectWalkingSim/Private/Core/SereneGameMode.cpp"
    - "Source/ProjectWalkingSim/Public/AI/Tasks/STT_GrabAttack.h"
    - "Source/ProjectWalkingSim/Private/AI/Tasks/STT_GrabAttack.cpp"
    - "Source/ProjectWalkingSim/Public/Inventory/InventoryComponent.h"
    - "Source/ProjectWalkingSim/Private/Inventory/InventoryComponent.cpp"
    - "Source/ProjectWalkingSim/Private/Save/SaveSubsystem.cpp"

key-decisions:
  - "ISaveable GetSaveId returns FName matching GetFName() for stable level-placed actor identification"
  - "Door ReadSaveData snaps mesh rotation immediately (no interpolation on load)"
  - "PickupActor WriteSaveData/ReadSaveData are no-ops -- destruction tracked by SaveSubsystem"
  - "OnActorsInitialized delegate for save data application timing (after all actors spawned)"
  - "RestoreSavedInventory added to InventoryComponent for clean slot replacement on load"

patterns-established:
  - "ISaveable::Execute_WriteSaveData/Execute_ReadSaveData for actor state serialization"
  - "SaveSubsystem::TrackDestroyedPickup before Destroy() for pickup persistence"
  - "GameMode InitGame -> OnActorsInitialized -> ApplyPendingSaveData for load flow"
  - "OnPlayerDeath -> Game Over widget -> LoadLatestSave/Restart for death pipeline"

# Metrics
duration: 8min
completed: 2026-02-12
---

# Phase 7 Plan 02: ISaveable Actors + Death System + Load Flow Summary

**ISaveable interface on doors/pickups/drawers with USereneSaveGame*, Game Over death screen replacing RestartLevel, and GameMode load-flow hook via OnActorsInitialized**

## Performance

- **Duration:** ~8 min
- **Started:** 2026-02-13T00:11:54Z
- **Completed:** 2026-02-13T00:19:31Z
- **Tasks:** 2
- **Files modified:** 16 (2 created, 14 modified)

## Accomplishments
- Expanded ISaveable interface with USereneSaveGame* parameter; implemented on DoorActor, PickupActor, DrawerActor
- Wired SaveSubsystem GatherWorldState/ApplyPendingSaveData through ISaveable interface (replaced placeholders)
- Replaced STT_GrabAttack RestartLevel with GameMode::OnPlayerDeath -> Game Over widget
- GameMode InitGame hooks OnActorsInitialized for post-reload save data application
- InventoryComponent::RestoreSavedInventory for clean inventory state replacement on load
- GameOverWidget with dynamic "Load Last Save" / "Restart" / "Quit" buttons

## Task Commits

Each task was committed atomically:

1. **Task 1: Expand ISaveable interface and implement on DoorActor, PickupActor, DrawerActor** - `eceef54` (feat)
2. **Task 2: Death system + GameMode load flow + GrabAttack replacement** - `336bfe0` (feat)

## Files Created/Modified
- `Source/ProjectWalkingSim/Public/Interaction/SaveableInterface.h` - ISaveable expanded with FName GetSaveId, WriteSaveData/ReadSaveData(USereneSaveGame*)
- `Source/ProjectWalkingSim/Public/Interaction/DoorActor.h` - ISaveable inheritance + override declarations
- `Source/ProjectWalkingSim/Private/Interaction/DoorActor.cpp` - WriteSaveData/ReadSaveData: saves/restores open/locked/angle/direction
- `Source/ProjectWalkingSim/Public/Interaction/PickupActor.h` - ISaveable inheritance + override declarations
- `Source/ProjectWalkingSim/Private/Interaction/PickupActor.cpp` - TrackDestroyedPickup before Destroy(); no-op WriteSaveData/ReadSaveData
- `Source/ProjectWalkingSim/Public/Interaction/DrawerActor.h` - ISaveable inheritance + override declarations
- `Source/ProjectWalkingSim/Private/Interaction/DrawerActor.cpp` - WriteSaveData/ReadSaveData: saves/restores open/slide state
- `Source/ProjectWalkingSim/Public/Core/SereneGameMode.h` - OnPlayerDeath, InitGame override, OnActorsReady, GameOverWidgetClass
- `Source/ProjectWalkingSim/Private/Core/SereneGameMode.cpp` - Full implementation of death + load flow
- `Source/ProjectWalkingSim/Public/AI/Tasks/STT_GrabAttack.h` - Updated comment (no more RestartLevel)
- `Source/ProjectWalkingSim/Private/AI/Tasks/STT_GrabAttack.cpp` - OnPlayerDeath replaces ConsoleCommand RestartLevel
- `Source/ProjectWalkingSim/Public/Inventory/InventoryComponent.h` - RestoreSavedInventory declaration
- `Source/ProjectWalkingSim/Private/Inventory/InventoryComponent.cpp` - RestoreSavedInventory implementation
- `Source/ProjectWalkingSim/Private/Save/SaveSubsystem.cpp` - ISaveable-based GatherWorldState/ApplyPendingSaveData + inventory restore
- `Source/ProjectWalkingSim/Public/Player/HUD/GameOverWidget.h` - UGameOverWidget with BindWidget pattern
- `Source/ProjectWalkingSim/Private/Player/HUD/GameOverWidget.cpp` - Load/Restart/Quit button handlers

## Decisions Made
- **ISaveable GetSaveId returns FName via GetFName():** Stable for level-placed actors; no need for FGuid since demo has no runtime-spawned saveable actors.
- **Door ReadSaveData snaps rotation immediately:** No interpolation on load -- player should see the correct state instantly, not watch doors animate open.
- **PickupActor ISaveable methods are no-ops:** Destruction tracking handled centrally by SaveSubsystem::DestroyedPickupTracker, not per-actor. Simpler and avoids double-tracking.
- **OnActorsInitialized for save data application timing:** InitGame is too early (actors may not have BeginPlay). OnActorsInitialized fires after all actors are spawned and initialized, ensuring save data can be applied reliably.
- **RestoreSavedInventory replaces slots then pads to MaxSlots:** SetNum(MaxSlots) ensures exactly 8 slots regardless of saved data length. Broadcasts OnInventoryChanged for UI refresh.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Moved RestoreSavedInventory from Task 2 to Task 1**
- **Found during:** Task 1 (SaveSubsystem compilation)
- **Issue:** SaveSubsystem::ApplyPendingSaveData calls InventoryComponent::RestoreSavedInventory, which was planned for Task 2. Updating the ISaveable integration in SaveSubsystem during Task 1 required the method to exist.
- **Fix:** Added RestoreSavedInventory declaration and implementation in Task 1 alongside the SaveSubsystem ISaveable updates.
- **Files modified:** InventoryComponent.h, InventoryComponent.cpp
- **Verification:** Project compiles cleanly
- **Committed in:** eceef54 (Task 1 commit)

---

**Total deviations:** 1 auto-fixed (1 blocking)
**Impact on plan:** Minor reordering -- RestoreSavedInventory moved from Task 2 to Task 1 to unblock SaveSubsystem compilation. No scope change.

## Issues Encountered
- Task 2 changes (SereneGameMode, STT_GrabAttack, GameOverWidget) were already committed by a prior agent session as `336bfe0`. The current execution confirmed correctness and verified the code matches plan requirements.

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- ISaveable interface fully functional on all 3 actor types (doors, pickups, drawers)
- Death pipeline wired: GrabAttack -> OnPlayerDeath -> Game Over widget -> LoadLatestSave / Restart
- GameMode load flow hooks correctly apply saved state after level reload
- Ready for 07-03 (Save/Load UI) which creates the save slot picker widget
- Ready for 07-04 (Tape Recorder save point + pause menu integration)
- **User action required for Game Over widget:** Create WBP_GameOver Blueprint in editor, reparent to UGameOverWidget, add required BindWidget elements, and assign to SereneGameMode's GameOverWidgetClass

---
*Phase: 07-save-system*
*Completed: 2026-02-12*
