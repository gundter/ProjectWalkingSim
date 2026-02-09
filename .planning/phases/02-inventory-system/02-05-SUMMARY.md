---
phase: 02-inventory-system
plan: 05
subsystem: item-combine-navigation
tags: [ue5, c++, inventory, combine, keyboard-navigation, delegates, ui]

# Dependency graph
requires:
  - phase: 02-03
    provides: PickupActor with inventory integration, DoorActor with key-lock
  - phase: 02-04
    provides: InventoryComponent on character, HUD delegate routing
provides:
  - TryCombineItems method with recipe TMap on InventoryComponent
  - OnCombineFailed delegate for combine feedback
  - Combine mode UI flow (select A -> Combine -> select B)
  - Keyboard navigation (1-8 number keys, arrow keys, Escape, Delete)
  - Complete combine delegate chain from tooltip through HUD to component
affects: [02-06]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Two-step combine flow: EnterCombineMode stores source, HandleSlotClicked completes"
    - "Order-independent recipe lookup: checks both (A,B) and (B,A) permutations"
    - "NativeOnKeyDown with SetIsFocusable for widget keyboard input"
    - "Super::NativeOnKeyDown pass-through for unhandled keys (WASD movement)"

key-files:
  created: []
  modified:
    - Source/ProjectWalkingSim/Public/Inventory/InventoryTypes.h
    - Source/ProjectWalkingSim/Public/Inventory/InventoryComponent.h
    - Source/ProjectWalkingSim/Private/Inventory/InventoryComponent.cpp
    - Source/ProjectWalkingSim/Public/Player/HUD/InventoryWidget.h
    - Source/ProjectWalkingSim/Private/Player/HUD/InventoryWidget.cpp
    - Source/ProjectWalkingSim/Public/Player/HUD/SereneHUD.h
    - Source/ProjectWalkingSim/Private/Player/HUD/SereneHUD.cpp

key-decisions:
  - "Empty recipe TMap for demo: InitCombineRecipes placeholder ready for future expansion"
  - "Order-independent recipes: checks both (A,B) and (B,A) so user order doesn't matter"
  - "Combine mode exits on: same slot click, empty slot click, Escape key"
  - "SetKeyboardFocus in ShowInventory: ensures NativeOnKeyDown receives input"
  - "Pass unhandled keys to Super: WASD movement continues to work while inventory open"

patterns-established:
  - "Two-step combine UI pattern: enter mode, store source, wait for target, broadcast pair"
  - "Recipe map with TPair keys for item combination lookup"

# Metrics
duration: 5min
completed: 2026-02-09
---

# Phase 2 Plan 05: Item Combine Logic Summary

**Two-step combine flow with recipe TMap backend, keyboard navigation via NativeOnKeyDown with number keys 1-8, arrows, Escape, and Delete**

## Performance

- **Duration:** 5 min
- **Started:** 2026-02-09
- **Completed:** 2026-02-09
- **Tasks:** 2
- **Files modified:** 7

## Accomplishments

- InventoryComponent has TryCombineItems with order-independent recipe lookup
- OnCombineFailed delegate broadcasts reason text on failed combination
- InventoryWidget has combine mode (EnterCombineMode/ExitCombineMode/IsInCombineMode)
- OnCombineSlotSelected two-param delegate for slot pair selection
- NativeOnKeyDown handles: number keys 1-8, left/right arrows, Escape, Delete
- Complete combine delegate chain wired through SereneHUD
- WASD movement still works while inventory is open (keys pass through)

## Task Commits

Each task was committed atomically:

1. **Task 1: Combine system backend (InventoryComponent)** - `7454651` (feat)
2. **Task 2: Combine UI flow + keyboard navigation** - `6c47b35` (feat)

## Files Created/Modified

**Modified:**
- `Source/ProjectWalkingSim/Public/Inventory/InventoryTypes.h` - Added FOnCombineFailed delegate
- `Source/ProjectWalkingSim/Public/Inventory/InventoryComponent.h` - Added TryCombineItems, CombineRecipes TMap, InitCombineRecipes, OnCombineFailed
- `Source/ProjectWalkingSim/Private/Inventory/InventoryComponent.cpp` - Implemented InitCombineRecipes and TryCombineItems with order-independent lookup
- `Source/ProjectWalkingSim/Public/Player/HUD/InventoryWidget.h` - Added combine mode state, OnCombineSlotSelected delegate, EnterCombineMode/ExitCombineMode, NativeOnKeyDown override, NavigateSlot
- `Source/ProjectWalkingSim/Private/Player/HUD/InventoryWidget.cpp` - Implemented combine mode flow, keyboard navigation with number keys/arrows/Escape/Delete
- `Source/ProjectWalkingSim/Public/Player/HUD/SereneHUD.h` - Added HandleCombineButtonClicked, HandleCombineSlotSelected
- `Source/ProjectWalkingSim/Private/Player/HUD/SereneHUD.cpp` - Bound combine delegates, implemented handlers that route to InventoryWidget/InventoryComponent

## Decisions Made

- **Empty recipe TMap:** InitCombineRecipes is a placeholder with commented example. Demo has few/zero recipes, but the UI flow works end-to-end.
- **Order-independent recipes:** TryCombineItems checks both (A,B) and (B,A) permutations so player selection order doesn't matter.
- **Combine mode cancellation:** Three ways to cancel: click same slot, click empty slot, or press Escape.
- **SetKeyboardFocus on ShowInventory:** Ensures the widget receives NativeOnKeyDown events when inventory opens.
- **Super::NativeOnKeyDown pass-through:** Unhandled keys (WASD, etc.) fall through to game input, so movement continues while inventory is open.

## Complete Combine Delegate Chain

1. User clicks Combine button in tooltip -> ItemTooltipWidget::OnCombineClicked fires
2. InventoryWidget re-broadcasts as OnCombineRequested (single-param, from Plan 02)
3. SereneHUD::HandleCombineButtonClicked calls InventoryWidget::EnterCombineMode
4. User clicks second slot -> InventoryWidget broadcasts OnCombineSlotSelected (two-param)
5. SereneHUD::HandleCombineSlotSelected calls InventoryComponent::TryCombineItems

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None - both tasks completed and committed successfully.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Combine system UI flow is complete (even with zero recipes)
- Keyboard navigation provides full non-mouse control
- All key events properly consumed or passed through
- Ready for Plan 06 (final testing/polish) or Phase 3

---
*Phase: 02-inventory-system*
*Completed: 2026-02-09*
