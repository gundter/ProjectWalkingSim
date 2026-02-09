---
phase: 02-inventory-system
plan: 03
subsystem: inventory
tags: [ue5, c++, inventory, interaction, pickup, door, locked-door]

# Dependency graph
requires:
  - phase: 01-foundation
    provides: IInteractable interface, AInteractableBase, APickupActor, ADoorActor
  - phase: 02-01
    provides: UInventoryComponent, UItemDataAsset, InitFromItemData minimal version
provides:
  - APickupActor with full inventory integration (TryAddItem, IsFull check)
  - APickupActor::InitFromItemData extended with mesh loading and text update
  - ADoorActor with RequiredItemId, bIsLocked, key consumption on unlock
  - "Inventory Full" interaction text when player inventory is full
  - "Locked" interaction text for locked doors
affects: [02-04, 02-05, 02-06]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "CanInteract with mutable state cache for GetInteractionText"
    - "Key-lock mechanics with item consumption on unlock"
    - "Graceful fallback when no InventoryComponent present"

key-files:
  created: []
  modified:
    - Source/ProjectWalkingSim/Public/Interaction/PickupActor.h
    - Source/ProjectWalkingSim/Private/Interaction/PickupActor.cpp
    - Source/ProjectWalkingSim/Public/Interaction/DoorActor.h
    - Source/ProjectWalkingSim/Private/Interaction/DoorActor.cpp

key-decisions:
  - "mutable bInventoryFullOnLastCheck for const CanInteract to communicate state to GetInteractionText"
  - "Allow interaction attempt on locked doors for player feedback (show 'Locked' text)"
  - "Key consumed immediately on unlock, door opens in same interaction"

patterns-established:
  - "CanInteract always returns true for locked interactables to allow player feedback"
  - "GetInteractionText override pattern for dynamic interaction prompts"

# Metrics
duration: 6min
completed: 2026-02-09
---

# Phase 2 Plan 03: Interactable Actor Integration Summary

**PickupActor calls TryAddItem with "Inventory Full" feedback, DoorActor supports key-lock mechanics with key consumption on unlock**

## Performance

- **Duration:** 6 min
- **Started:** 2026-02-09T16:44:24Z
- **Completed:** 2026-02-09T16:50:24Z
- **Tasks:** 2
- **Files modified:** 4

## Accomplishments

- PickupActor now fully integrates with inventory: calls TryAddItem, only destroys on success
- PickupActor shows "Inventory Full" when player cannot pick up (inventory full and item not stackable)
- PickupActor::InitFromItemData extended to load WorldMesh and set interaction text from item data
- DoorActor supports optional key-lock with RequiredItemId/bIsLocked properties
- Locked doors show "Locked" text, consume key item on unlock via RemoveItemByName

## Task Commits

Each task was committed atomically:

1. **Task 1: PickupActor inventory integration** - `3aa5eb4` (feat)
2. **Task 2: DoorActor locked-door support** - `63fece5` (feat)

## Files Created/Modified

**Modified:**
- `Source/ProjectWalkingSim/Public/Interaction/PickupActor.h` - Added CanInteract, GetInteractionText overrides, bInventoryFullOnLastCheck
- `Source/ProjectWalkingSim/Private/Interaction/PickupActor.cpp` - Full inventory integration, extended InitFromItemData
- `Source/ProjectWalkingSim/Public/Interaction/DoorActor.h` - Added RequiredItemId, bIsLocked, LockedText, interface overrides
- `Source/ProjectWalkingSim/Private/Interaction/DoorActor.cpp` - Lock check and key consumption in OnInteract

## Decisions Made

- **mutable cache for CanInteract state:** CanInteract is const but needs to communicate "inventory full" state to GetInteractionText. Used mutable bool bInventoryFullOnLastCheck to cache the state from CanInteract (which runs every tick via InteractionComponent).
- **Allow locked door interaction attempts:** Player can attempt interaction on locked doors for feedback. GetInteractionText returns "Locked" and OnInteract logs the requirement. This matches horror game convention where players can "try" locked doors.
- **Key consumed immediately on unlock:** When player has the key, it's consumed and door opens in the same interaction. No separate "unlock" then "open" steps.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Fixed InventoryWidget TooltipWidget shadowing**
- **Found during:** Task 1 build verification
- **Issue:** UInventoryWidget::TooltipWidget shadowed UWidget::ToolTipWidget (base class member with similar name)
- **Fix:** Renamed member variable from `TooltipWidget` to `ItemTooltip` in both header and cpp
- **Files modified:** InventoryWidget.h, InventoryWidget.cpp
- **Verification:** Build succeeds after rename
- **Note:** This was a pre-existing issue from 02-02-PLAN execution, fixed as blocking issue for this plan's build

**2. [Rule 3 - Blocking] Fixed InventoryWidget Slot variable shadowing**
- **Found during:** Task 1 build verification
- **Issue:** Local variable `Slot` in RefreshSlots loop shadowed UWidget::Slot base class member
- **Fix:** Renamed loop variable from `Slot` to `SlotData`
- **Files modified:** InventoryWidget.cpp
- **Verification:** Build succeeds after rename
- **Note:** This was a pre-existing issue from 02-02-PLAN execution, fixed as blocking issue for this plan's build

---

**Total deviations:** 2 auto-fixed (2 blocking - pre-existing issues from prior plan)
**Impact on plan:** Both fixes were for shadowing issues that blocked compilation. No scope creep.

## Issues Encountered

- Build path in plan was `C:\Program Files\Epic Games\UE_5.7` but actual path is `E:\Epic Games\UE_5.7`. Used correct path from 02-01-SUMMARY.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- PickupActor and DoorActor fully integrated with inventory system
- Ready for 02-04 (Item Combine Logic) - inventory item actions can now interact with world
- Key items can be placed in world and used to unlock doors
- Discarded items will spawn as proper pickups with mesh from item data

---
*Phase: 02-inventory-system*
*Completed: 2026-02-09*
