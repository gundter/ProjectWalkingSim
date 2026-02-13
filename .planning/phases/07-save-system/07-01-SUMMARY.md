---
phase: 07-save-system
plan: 01
subsystem: save
tags: [USaveGame, UGameInstanceSubsystem, screenshot, JPEG, async-save, save-slots]

# Dependency graph
requires:
  - phase: 01-foundation
    provides: "SereneCharacter, SereneGameInstance, InventoryComponent, ISaveable stub"
  - phase: 02-inventory
    provides: "FInventorySlot struct, InventoryComponent::GetSlots()"
provides:
  - "SaveTypes.h: FSavedDoorState, FSavedDrawerState, FSaveSlotInfo USTRUCTs"
  - "USereneSaveGame: USaveGame subclass with all save data fields"
  - "USaveSubsystem: UGameInstanceSubsystem with save/load/screenshot/tracking API"
affects: [07-02, 07-03, 07-04]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Flat struct arrays in USaveGame (not per-actor binary serialization)"
    - "Async screenshot capture via OnScreenshotCaptured delegate + JPEG compression"
    - "Level restart load flow: PendingSaveData -> OpenLevel -> ApplyPendingSaveData"
    - "UGameInstanceSubsystem for cross-level persistence"

key-files:
  created:
    - "Source/ProjectWalkingSim/Public/Save/SaveTypes.h"
    - "Source/ProjectWalkingSim/Public/Save/SereneSaveGame.h"
    - "Source/ProjectWalkingSim/Private/Save/SereneSaveGame.cpp"
    - "Source/ProjectWalkingSim/Public/Save/SaveSubsystem.h"
    - "Source/ProjectWalkingSim/Private/Save/SaveSubsystem.cpp"
  modified: []

key-decisions:
  - "Flat struct arrays in USaveGame over per-actor binary serialization -- simpler for known limited state types"
  - "FImageUtils::CompressImage with jpg extension for screenshot compression (CompressImageArray deprecated)"
  - "GatherWorldState iterates typed actors directly (ADoorActor, ADrawerActor) -- ISaveable expansion deferred to 07-02"
  - "GetPendingSaveData() accessor added for character/controller to restore inventory independently"

patterns-established:
  - "Save slot naming: SaveSlot_0 through SaveSlot_2"
  - "Screenshot flow: RequestScreenshot -> OnScreenshotCaptured -> CompressImage -> AsyncSaveGameToSlot"
  - "Load flow: LoadFromSlot -> PendingSaveData + OpenLevel -> GameMode calls ApplyPendingSaveData"

# Metrics
duration: 10min
completed: 2026-02-12
---

# Phase 7 Plan 01: Save Data Foundation Summary

**USaveGame subclass with flat struct arrays + UGameInstanceSubsystem with async save/load/screenshot API across 3 named slots**

## Performance

- **Duration:** ~10 min
- **Started:** 2026-02-12T23:58:09Z
- **Completed:** 2026-02-13T00:08:37Z
- **Tasks:** 2
- **Files created:** 5

## Accomplishments
- Created SaveTypes.h with FSavedDoorState, FSavedDrawerState, FSaveSlotInfo data structures
- Created USereneSaveGame with complete save state fields (player, inventory, doors, drawers, destroyed pickups)
- Created USaveSubsystem with full save/load orchestration, async screenshot capture, and destroyed pickup tracking
- Screenshot capture compresses to JPEG (~85 quality) via FImageUtils::CompressImage

## Task Commits

Each task was committed atomically:

1. **Task 1: Save data types and USaveGame subclass** - `2cce141` (feat)
2. **Task 2: SaveSubsystem UGameInstanceSubsystem** - `c4717cc` (feat)

## Files Created/Modified
- `Source/ProjectWalkingSim/Public/Save/SaveTypes.h` - FSavedDoorState, FSavedDrawerState, FSaveSlotInfo USTRUCTs
- `Source/ProjectWalkingSim/Public/Save/SereneSaveGame.h` - USereneSaveGame : USaveGame with all save data fields
- `Source/ProjectWalkingSim/Private/Save/SereneSaveGame.cpp` - Minimal constructor
- `Source/ProjectWalkingSim/Public/Save/SaveSubsystem.h` - USaveSubsystem : UGameInstanceSubsystem with full API
- `Source/ProjectWalkingSim/Private/Save/SaveSubsystem.cpp` - Complete implementation of all save/load methods

## Decisions Made
- **Flat struct arrays over per-actor binary serialization:** Known limited state types (doors, drawers, pickups, inventory) don't warrant the complexity of FMemoryWriter/FMemoryReader.
- **FImageUtils::CompressImage with "jpg" extension:** CompressImageArray is deprecated in UE5; CompressImage with FImageView is the current API. JPEG at quality 85 for small file size (~20KB).
- **Direct actor iteration for world state gathering:** GatherWorldState uses TActorIterator<ADoorActor> and TActorIterator<ADrawerActor> directly. The ISaveable-based approach will be wired in 07-02 when the interface is expanded.
- **GetPendingSaveData() public accessor:** Allows character/controller to check for and apply pending inventory data independently of the subsystem's ApplyPendingSaveData call, solving the BeginPlay timing issue.
- **BlueprintType on all save structs:** UHT requires BlueprintType for structs returned by BlueprintCallable functions (FSaveSlotInfo is returned by GetSlotInfo).

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Added BlueprintType specifier to save structs**
- **Found during:** Task 2 (SaveSubsystem compilation)
- **Issue:** UHT error: "Type 'FSaveSlotInfo' is not supported by blueprint" -- GetSlotInfo returns FSaveSlotInfo as BlueprintCallable, requiring BlueprintType
- **Fix:** Added BlueprintType to FSavedDoorState, FSavedDrawerState, FSaveSlotInfo
- **Files modified:** SaveTypes.h
- **Verification:** Project compiles cleanly
- **Committed in:** c4717cc (Task 2 commit)

---

**Total deviations:** 1 auto-fixed (1 bug)
**Impact on plan:** Minimal -- standard UHT requirement for Blueprint-exposed return types.

## Issues Encountered
None

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- Save data types and subsystem API ready for 07-02 (ISaveable expansion + actor implementations)
- GatherWorldState and ApplyPendingSaveData have placeholder logic for door/drawer state -- will use ISaveable::WriteSaveData/ReadSaveData once 07-02 expands the interface
- GetPendingSaveData() accessor ready for character inventory restore in 07-02

---
*Phase: 07-save-system*
*Completed: 2026-02-12*
