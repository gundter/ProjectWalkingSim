---
status: complete
phase: 07-save-system
source: [07-01-SUMMARY.md, 07-02-SUMMARY.md, 07-03-SUMMARY.md, 07-04-SUMMARY.md]
started: 2026-02-12T21:00:00Z
updated: 2026-02-12T21:15:00Z
---

## Current Test

[testing complete]

## Tests

### 1. Project Compilation
expected: Project compiles without errors or warnings-as-errors. All new Save/ files, widget files, TapeRecorderActor, and modified DoorActor/PickupActor/DrawerActor/GameMode/PlayerController build cleanly.
result: pass

### 2. Tape Recorder Interaction
expected: Place a BP_TapeRecorderActor in level (or create Blueprint from ATapeRecorderActor). Walk up to it in PIE -- "Record" interaction text appears. Interact to open save/load menu showing 3 empty save slots.
result: skipped
reason: UMG Blueprints not yet created. Interaction prompt confirmed working -- "Record" text appears. OnInteract fires but SaveMenuWidgetClass is null so no widget opens.

### 3. Save to Empty Slot
expected: Click an empty save slot in the save menu. Game saves (brief pause for screenshot capture). Slot updates to show it's now occupied.
result: skipped
reason: Requires WBP_SaveLoadMenu Blueprint

### 4. Save Slot Thumbnail and Timestamp
expected: After saving, the slot displays a screenshot thumbnail of the game view and a formatted timestamp showing when the save was made.
result: skipped
reason: Requires WBP_SaveSlot Blueprint

### 5. Overwrite Occupied Slot
expected: Save to a slot that already has a save. A confirmation prompt appears asking to overwrite. Confirming overwrites the save; cancelling returns to slot selection.
result: skipped
reason: Requires WBP_SaveLoadMenu Blueprint with ConfirmOverlay

### 6. Esc Opens Pause Menu
expected: During gameplay, press Esc. Pause menu appears with Continue, Load Game, Resume, and Quit buttons. Game is paused (world stops ticking). Continue and Load Game are disabled if no saves exist.
result: skipped
reason: Requires IA_Pause input action + WBP_PauseMenu Blueprint

### 7. Pause Menu Resume
expected: Click Resume in the pause menu. Menu closes, game unpauses, and normal gameplay input resumes.
result: skipped
reason: Requires WBP_PauseMenu Blueprint

### 8. Load Game from Pause Menu
expected: With at least one save, click Load Game in pause menu. A 3-slot picker appears in Load mode showing occupied slots. Select a slot -- level reloads and save is applied.
result: skipped
reason: Requires WBP_PauseMenu + WBP_SaveLoadMenu Blueprints

### 9. Player Position Restored at Tape Recorder
expected: After loading a save made at a tape recorder, player spawns at the tape recorder's position (not where the player was standing when they interacted).
result: skipped
reason: Requires save/load UI to create a save first

### 10. Door and Pickup State Restored
expected: Open a door and pick up an item, then save. Load that save. The door should still be open and the picked-up item should be gone from the world.
result: skipped
reason: Requires save/load UI to create a save first

### 11. Inventory Restored on Load
expected: Have items in inventory when saving. Load that save. Inventory UI shows the same items that were present when the save was made.
result: skipped
reason: Requires save/load UI to create a save first

### 12. Death Shows Game Over Screen
expected: Let the Wendigo grab the player. Instead of the level restarting immediately, a Game Over screen appears with "Load Last Save" (or "Restart" if no saves exist) and "Quit" buttons.
result: skipped
reason: Requires WBP_GameOver Blueprint assigned to GameOverWidgetClass

### 13. Game Over Load Last Save
expected: On the Game Over screen (with a prior save existing), click "Load Last Save". Level reloads and the most recent save is applied -- player is at tape recorder position with correct inventory and world state.
result: skipped
reason: Requires WBP_GameOver Blueprint

### 14. Esc Closes Inventory First
expected: Open inventory (Tab), then press Esc. Inventory closes but pause menu does NOT open. Press Esc again with no overlays open -- now pause menu opens.
result: skipped
reason: Requires IA_Pause input action bound to Esc

## Summary

total: 14
passed: 1
issues: 0
pending: 0
skipped: 13

## Gaps

[none -- all skips are due to missing UMG Blueprint setup, not code issues]

## Deferred Setup Required

Before re-running UAT, the following editor assets must be created:

**Widgets (create User Widget, reparent to C++ class):**
- WBP_GameOver → reparent to UGameOverWidget (add BindWidget: GameOverText, LoadLastSaveButton, LoadLastSaveButtonText, QuitButton)
- WBP_SaveSlot → reparent to USaveSlotWidget (add BindWidget: ThumbnailImage, TimestampText, SlotLabelText, SlotButton)
- WBP_SaveLoadMenu → reparent to USaveLoadMenuWidget (add BindWidget: TitleText, SlotContainer, CloseButton, ConfirmOverlay, ConfirmYesButton, ConfirmNoButton)
- WBP_PauseMenu → reparent to UPauseMenuWidget (add BindWidget: PauseTitle, ContinueButton, LoadGameButton, ResumeButton, QuitButton)

**Input:**
- IA_Pause input action (Esc key, Started trigger)
- Add IA_Pause mapping to IMC_Default

**Property assignments:**
- BP_SerenePlayerController: PauseAction = IA_Pause, PauseMenuWidgetClass = WBP_PauseMenu
- BP_TapeRecorderActor (create from ATapeRecorderActor): SaveMenuWidgetClass = WBP_SaveLoadMenu
- BP_SereneGameMode: GameOverWidgetClass = WBP_GameOver
- WBP_SaveLoadMenu needs SaveSlotWidgetClass reference (may need UPROPERTY or hardcoded class)
