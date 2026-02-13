# Phase 7 Save System -- Code Quality & Performance Audit

**Date:** 2026-02-12
**Auditor:** Claude Opus 4.6
**Verdict:** PASS (2 critical fixed, 4 warnings, 4 notes)

---

## Files Audited

### New Save System Files
- `Public/Save/SaveTypes.h`
- `Public/Save/SereneSaveGame.h` / `Private/Save/SereneSaveGame.cpp`
- `Public/Save/SaveSubsystem.h` / `Private/Save/SaveSubsystem.cpp`
- `Public/Interaction/SaveableInterface.h`
- `Public/Interaction/TapeRecorderActor.h` / `Private/Interaction/TapeRecorderActor.cpp`
- `Public/Player/HUD/GameOverWidget.h` / `Private/Player/HUD/GameOverWidget.cpp`
- `Public/Player/HUD/SaveSlotWidget.h` / `Private/Player/HUD/SaveSlotWidget.cpp`
- `Public/Player/HUD/SaveLoadMenuWidget.h` / `Private/Player/HUD/SaveLoadMenuWidget.cpp`
- `Public/Player/HUD/PauseMenuWidget.h` / `Private/Player/HUD/PauseMenuWidget.cpp`

### Modified Existing Files
- `Public/Interaction/DoorActor.h` / `Private/Interaction/DoorActor.cpp`
- `Public/Interaction/PickupActor.h` / `Private/Interaction/PickupActor.cpp`
- `Public/Interaction/DrawerActor.h` / `Private/Interaction/DrawerActor.cpp`
- `Public/Core/SereneGameMode.h` / `Private/Core/SereneGameMode.cpp`
- `Private/AI/Tasks/STT_GrabAttack.cpp`
- `Public/Inventory/InventoryComponent.h` / `Private/Inventory/InventoryComponent.cpp`
- `Public/Player/SerenePlayerController.h` / `Private/Player/SerenePlayerController.cpp`

---

## Critical Issues (must fix before phase complete)

### C1. DrawerActor::DrawerInitialLocation never cached from actual mesh position [FIXED]

**File:** `DrawerActor.h` / `DrawerActor.cpp`
**Problem:** `DrawerInitialLocation` was declared as `FVector::ZeroVector` with a comment saying "cached on construction" but no BeginPlay override existed to actually cache the DrawerMesh's initial relative location. If a level designer offsets the drawer mesh in the Blueprint (e.g., to align the drawer panel inside a cabinet frame), the Tick interpolation and ReadSaveData restoration would use `(0,0,0)` as the base, causing the drawer to teleport to the wrong position on open/close or load.
**Impact:** Incorrect drawer positioning on interaction and save/load for any drawer with a non-zero Blueprint mesh offset.
**Fix applied:** Added `BeginPlay()` override that caches `DrawerMesh->GetRelativeLocation()` into `DrawerInitialLocation`. Updated comment to say "cached in BeginPlay".

### C2. HandlePauseMenuClosed missing UFUNCTION() specifier [FIXED]

**File:** `SerenePlayerController.h`, line 146
**Problem:** `HandlePauseMenuClosed()` is bound to `OnPauseMenuClosed` dynamic multicast delegate via `AddDynamic` macro in `SerenePlayerController.cpp` line 399. The `AddDynamic` macro requires the target function to be a `UFUNCTION()` so that UHT generates the necessary reflection metadata. The function was declared without `UFUNCTION()`, which would cause a compilation error on clean build (the `STATIC_FUNCTION_FNAME` macro expansion in `AddDynamic` fails to find the function in the reflection system).
**Impact:** Compilation failure on clean build or after UHT regeneration.
**Fix applied:** Added `UFUNCTION()` specifier to `HandlePauseMenuClosed()` declaration in `SerenePlayerController.h`.

---

## Warnings (should fix, not blocking)

### W1. GetSlotInfo / GetLatestSlotIndex load full save games synchronously

**File:** `SaveSubsystem.cpp`, lines 166 and 216
**Problem:** `GetSlotInfo()` calls `LoadGameFromSlot()` for a single slot, and `GetLatestSlotIndex()` calls it for up to 3 slots. Each load deserializes the entire `USereneSaveGame` including the JPEG screenshot byte array (potentially 50-200 KB per slot). `RefreshSlots()` in `SaveLoadMenuWidget` calls `GetSlotInfo()` for all 3 slots, so opening the save/load menu triggers up to 3 full save file loads.
**Impact:** Brief hitch (likely <100ms for 3 small saves) when opening the save/load menu. Acceptable for 3 slots but would not scale.
**Recommendation:** Cache slot metadata in memory after save/load operations to avoid repeated disk reads. For 3 slots this is low priority.

### W2. OnActorsInitialized delegate never unbound

**File:** `SereneGameMode.cpp`, line 33
**Problem:** `OnActorsInitialized.AddUObject(this, &ASereneGameMode::OnActorsReady)` binds without storing the handle. This is a one-shot usage (only fires once per level load) and the GameMode is destroyed on level change, so it does not cause functional issues. However, not storing or unbinding the handle is technically imprecise.
**Impact:** None in practice (GameMode lifetime matches World lifetime). Minor correctness concern.
**Recommendation:** Store the delegate handle and unbind in a cleanup path, or document why it is safe to leave unbound.

### W3. Pause menu re-binds OnPauseMenuClosed on every open

**File:** `SerenePlayerController.cpp`, line 399
**Problem:** Each time the pause menu is opened (TogglePauseMenu), `OnPauseMenuClosed.AddDynamic()` is called on the PauseMenuInstance. Since the widget is destroyed and recreated each time (set to nullptr on close, new instance on open), this is not a functional bug. However, if a code path ever reuses the same widget instance, duplicate bindings could fire the handler multiple times.
**Impact:** None currently due to create-destroy lifecycle. Fragile if pattern changes.
**Recommendation:** Either bind in NativeConstruct (so it's per-instance) or check `IsAlreadyBound()` before adding.

### W4. Screenshot compressed data copied from TArray64 to TArray via raw Memcpy

**File:** `SaveSubsystem.cpp`, lines 458-462
**Problem:** `CompressedData` is `TArray64<uint8>` (64-bit count) but is copied to `TArray<uint8>` (32-bit count) via raw `FMemory::Memcpy`. If the compressed data ever exceeds `INT32_MAX` bytes (2 GB), this would overflow. For JPEG screenshots this is impossible in practice, but the pattern is fragile.
**Impact:** None (JPEG screenshots are typically < 500 KB). Minor code hygiene.
**Recommendation:** Add a size check or use `Append()` with appropriate casting.

---

## Notes (minor style issues, can defer)

### N1. FInventorySlot UPROPERTY fields lack SaveGame specifier

**File:** `InventoryTypes.h`
**Detail:** `FInventorySlot::ItemId` and `Quantity` have `UPROPERTY(BlueprintReadOnly)` but not the `SaveGame` metadata specifier. This is fine because `USereneSaveGame` stores a `TArray<FInventorySlot>` under its own `UPROPERTY()`, and `USaveGame` serialization uses the standard property serializer which captures all `UPROPERTY()` fields regardless of the `SaveGame` flag. The `SaveGame` specifier is only needed for custom `FArchive`-based serialization patterns.

### N2. SaveableInterface WriteSaveData takes non-const USereneSaveGame pointer

**File:** `SaveableInterface.h`, line 36
**Detail:** `WriteSaveData(USereneSaveGame* SaveGame)` takes a non-const pointer, which is correct since callers write into it. `ReadSaveData` also takes non-const, which is technically incorrect (readers should only read), but this is a UE `BlueprintNativeEvent` limitation -- const pass-by-pointer is not well supported in the UHT.

### N3. Hardcoded strings for save slot names

**File:** `SaveSubsystem.cpp`, line 356
**Detail:** `GetSlotName` uses `SaveSlot_%d` format. This is fine for a single-user game with 3 slots, but if user profiles are ever needed, the slot naming would need to include a user identifier.

### N4. Timestamp formatting uses 12-hour format without locale consideration

**File:** `SaveSlotWidget.cpp`, line 37
**Detail:** `Info.Timestamp.ToString(TEXT("%b %d, %Y %I:%M %p"))` uses English month abbreviations and 12-hour AM/PM. This is fine for an English-only horror game but would need localization for international release.

---

## Audit Checklist Summary

| Category | Result | Details |
|---|---|---|
| **Epic coding standards** | PASS | Proper UPROPERTY/UFUNCTION specifiers, TObjectPtr usage, naming conventions followed. Forward declarations used appropriately in headers. |
| **Tick overhead** | PASS | No new Tick functions added by save system. DoorActor and DrawerActor tick only during animation (bStartWithTickEnabled=false, disabled after reaching target). InventoryComponent has tick disabled. |
| **Memory patterns** | PASS | All UObject pointers are UPROPERTY-tagged (PendingSaveObject, PendingSaveData, SaveMenuInstance, PauseMenuInstance, LoadMenuInstance, GameOverWidgetInstance). No raw new/delete. Screenshot delegate properly cleaned up in Deinitialize. |
| **UE5 pitfalls** | PASS | Correct use of USaveGame serialization API. AsyncSaveGameToSlot for writes. Proper level reload via OpenLevel for load flow. FSaveSlotInfo and all saved structs are BlueprintType with UPROPERTY fields. |
| **Thread safety** | PASS | OnScreenshotCaptured fires on game thread (UGameViewportClient delegate). AsyncSaveGameToSlot completion lambda only logs. No cross-thread data access. |
| **Widget lifecycle** | PASS | Widgets created via CreateWidget, added via AddToViewport, removed via RemoveFromParent. Input mode properly toggled (GameAndUI on show, GameOnly on hide). Mouse cursor shown/hidden correctly. |
| **Include hygiene** | PASS | Headers use forward declarations (USereneSaveGame, USaveSubsystem, USaveLoadMenuWidget, UButton, UTextBlock, etc.). Full includes only in .cpp files. Minimal header includes. |

---

## Critical Issues Fixed

1. **C1 -- DrawerActor::DrawerInitialLocation**: Added `BeginPlay()` override to cache `DrawerMesh->GetRelativeLocation()`. Updated comment.
2. **C2 -- SerenePlayerController::HandlePauseMenuClosed**: Added `UFUNCTION()` specifier to fix `AddDynamic` compilation.

Both fixes committed as: `fix(07): cache DrawerInitialLocation in BeginPlay and add missing UFUNCTION on HandlePauseMenuClosed`
