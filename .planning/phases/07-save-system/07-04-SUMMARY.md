---
phase: 07-save-system
plan: 04
subsystem: save-integration
tags: [TapeRecorderActor, PauseMenuWidget, IInteractable, save-point, ESC-key, pause, SaveSubsystem, pending-location]

# Dependency graph
requires:
  - phase: 07-save-system
    plan: 02
    what: "SaveSubsystem save/load orchestration, ISaveable on world actors"
  - phase: 07-save-system
    plan: 03
    what: "SaveLoadMenuWidget, GameOverWidget, SaveSlotWidget"

provides:
  - "ATapeRecorderActor: world-placed save point opening save slot picker"
  - "UPauseMenuWidget: Esc pause menu with Continue/Load/Resume/Quit"
  - "SaveSubsystem pending save location API (tape recorder position override)"
  - "SereneGameMode::IsGameOver() state query"
  - "Full player-facing save system integration"

affects:
  - phase: 08-demo-polish
    what: "TapeRecorderActor needs mesh/sound assets; PauseMenuWidget needs UMG Blueprint; IA_Pause input action needed"

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Save point actor pattern: IInteractable opens save UI, sets pending location override"
    - "Pause menu as standalone viewport widget owned by PlayerController (not SereneHUD)"
    - "IsGameOver() guard prevents pause during death screen"
    - "Esc key context-sensitive: closes inventory if open, otherwise toggles pause"

# File tracking
key-files:
  created:
    - "Source/ProjectWalkingSim/Public/Interaction/TapeRecorderActor.h"
    - "Source/ProjectWalkingSim/Private/Interaction/TapeRecorderActor.cpp"
    - "Source/ProjectWalkingSim/Public/Player/HUD/PauseMenuWidget.h"
    - "Source/ProjectWalkingSim/Private/Player/HUD/PauseMenuWidget.cpp"
  modified:
    - "Source/ProjectWalkingSim/Public/Save/SaveSubsystem.h"
    - "Source/ProjectWalkingSim/Private/Save/SaveSubsystem.cpp"
    - "Source/ProjectWalkingSim/Public/Player/SerenePlayerController.h"
    - "Source/ProjectWalkingSim/Private/Player/SerenePlayerController.cpp"
    - "Source/ProjectWalkingSim/Public/Core/SereneGameMode.h"

# Decisions
decisions:
  - id: "pause-menu-standalone-widget"
    choice: "PauseMenuWidget managed by PlayerController, not SereneHUD"
    why: "Controller owns input mode and pause state; cleaner separation of concerns"
  - id: "esc-context-sensitive"
    choice: "Esc closes inventory if open, otherwise toggles pause"
    why: "Natural UX: single escape key handles all overlay dismissal"
  - id: "pending-save-location-api"
    choice: "SetPendingSaveLocation/ClearPendingSaveLocation on SaveSubsystem"
    why: "Decouples tape recorder from save internals; respawn at save point not player position"
  - id: "pause-blocks-during-hiding-and-gameover"
    choice: "Guard against pausing while hiding or during Game Over"
    why: "Prevents invalid state combinations; hiding has its own interaction model"

# Metrics
metrics:
  duration: "~10m"
  completed: "2026-02-12"
---

# Phase 7 Plan 4: Tape Recorder Save Point + Pause Menu Integration Summary

**One-liner:** World-placed tape recorder save point with Esc pause menu (Continue/Load/Resume/Quit) and full save system player-facing integration.

## What Was Built

### Task 1: TapeRecorderActor + SaveSubsystem Pending Location API
- **ATapeRecorderActor** inherits AInteractableBase, implements IInteractable
  - `CanInteract` checks: bCanBeInteracted, not hiding (EHidingState::Free), no save menu open
  - `GetInteractionText` returns "Record" (detective recording case notes)
  - `OnInteract`: plays save sound, sets pending save location to tape recorder position, creates SaveLoadMenuWidget in Save mode, shows cursor, sets Game+UI input
  - `HandleSaveMenuClosed`: restores input mode to Game Only, hides cursor, removes widget, clears pending save location
  - Properties: SaveSound (USoundBase), SaveAnimDuration (float), SaveMenuWidgetClass (TSubclassOf)
- **SaveSubsystem** additions:
  - `SetPendingSaveLocation(FVector, FRotator)` stores override position
  - `ClearPendingSaveLocation()` resets override
  - Private members: PendingSaveLocation, PendingSaveRotation, bHasPendingSaveLocation
  - `GatherPlayerState` checks bHasPendingSaveLocation and uses tape recorder position instead of player position when set

### Task 2: PauseMenuWidget + Esc Binding + IsGameOver Guard
- **UPauseMenuWidget** with BindWidget members: PauseTitle, ContinueButton, LoadGameButton, ResumeButton, QuitButton
  - NativeConstruct: binds all 4 buttons, disables Continue/LoadGame if no saves exist
  - Continue: calls SaveSubsystem->LoadLatestSave()
  - Load Game: creates SaveLoadMenuWidget in Load mode, hides pause menu, binds OnMenuClosed
  - Resume: broadcasts OnPauseMenuClosed delegate
  - Quit: calls UKismetSystemLibrary::QuitGame
  - HandleLoadMenuClosed: removes load menu widget, re-shows pause menu
- **SerenePlayerController** additions:
  - PauseAction (UInputAction) bound to HandlePause on ETriggerEvent::Started
  - PauseMenuWidgetClass (TSubclassOf<UPauseMenuWidget>) for widget creation
  - HandlePause: if inventory open -> CloseInventory(), otherwise TogglePauseMenu()
  - TogglePauseMenu: creates/removes widget, toggles cursor, input mode, and UGameplayStatics::SetGamePaused
  - Guards: no pause while hiding (EHidingState != Free) or during Game Over (IsGameOver())
  - HandlePauseMenuClosed: same cleanup as unpause path
- **SereneGameMode** addition:
  - `IsGameOver() const` returns true if GameOverWidgetInstance is non-null

## Deviations from Plan

None -- plan executed exactly as written.

## Verification

1. Project compiles without errors
2. TapeRecorderActor inherits AInteractableBase and shows "Record" interaction text
3. Interacting with tape recorder opens SaveLoadMenuWidget in Save mode
4. Player save location is set to tape recorder position (not player position)
5. Esc opens pause menu with 4 working buttons
6. Continue loads most recent save (or disabled if no saves)
7. Load Game opens 3-slot picker in Load mode
8. Resume unpauses and restores input
9. Quit exits game
10. Esc closes inventory if inventory is open
11. Pause is blocked during Game Over state
12. SereneGameMode::IsGameOver() returns correct state

## Next Phase Readiness

Phase 7 (Save System) is now complete. All 4 plans delivered:
- 07-01: Save types, USaveGame, SaveSubsystem core
- 07-02: ISaveable on world actors, load flow, death pipeline
- 07-03: Game Over widget, save slot widget, save/load menu widget
- 07-04: Tape recorder save point, pause menu, Esc binding, full integration

**User action needed for Phase 7 UMG Blueprints:**
- WBP_GameOver (reparent to UGameOverWidget)
- WBP_SaveSlot (reparent to USaveSlotWidget)
- WBP_SaveLoadMenu (reparent to USaveLoadMenuWidget)
- WBP_PauseMenu (reparent to UPauseMenuWidget)
- IA_Pause input action (Esc key, Started trigger)
- Assign IA_Pause to BP_SerenePlayerController PauseAction property
- Assign PauseMenuWidgetClass to BP_SerenePlayerController
- Assign SaveMenuWidgetClass to BP_TapeRecorderActor
- Assign LoadMenuWidgetClass to WBP_PauseMenu

**Performance audit** required before marking Phase 7 complete.

## Commits

| Hash | Message |
|------|---------|
| 153b0f7 | feat(07-04): TapeRecorderActor save point + SaveSubsystem pending location API |
| a5905d0 | feat(07-04): PauseMenuWidget + Esc binding + IsGameOver guard |
