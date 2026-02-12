# Phase 7: Save System - Research

**Researched:** 2026-02-12
**Domain:** UE5.7 USaveGame persistence, world state serialization, screenshot capture, Game Over UI
**Confidence:** HIGH

## Summary

The save system for The Juniper Tree follows the classic survival horror save-point pattern: manual saves at world-placed tape recorder objects, 3 save slots with screenshot thumbnails, Game Over screen on death, and full world state restoration on load. The existing project already uses USaveGame for settings persistence (USereneSettingsSave in SereneGameInstance.h), and the ISaveable interface exists as a stub from Phase 1.

The recommended approach uses UE5's built-in USaveGame with the `SaveGame` UPROPERTY specifier and FMemoryWriter/FMemoryReader binary serialization for per-actor state. This is the established pattern used by Tom Looman's widely-referenced C++ save system and matches Epic's official documentation. For screenshot thumbnails, OnScreenshotCaptured delegate captures viewport pixels, FImageUtils compresses to JPEG, and the compressed TArray<uint8> stores directly in the USaveGame object. The load flow uses level restart via OpenLevel with a URL option indicating which save slot to load from, with GameMode::InitGame parsing and applying saved state before actors call BeginPlay.

**Primary recommendation:** Use USaveGame with per-actor binary serialization via SaveGame UPROPERTY specifier. Expand the existing ISaveable interface to pass a save data struct. Store compressed JPEG screenshots (~10-30KB each) directly in the save file. Manage load flow through GameInstance since it persists across level transitions.

## Standard Stack

### Core
| Library/API | Version | Purpose | Why Standard |
|-------------|---------|---------|--------------|
| USaveGame | UE5.7 built-in | Save file container class | Epic's official persistence mechanism, already used in project |
| UGameplayStatics Save/Load | UE5.7 built-in | SaveGameToSlot, LoadGameFromSlot, DoesSaveGameExist, DeleteGameInSlot | Standard API for disk persistence |
| FMemoryWriter / FMemoryReader | UE5.7 built-in | Binary serialization of actor state | Efficient per-actor state capture via Serialize() |
| FObjectAndNameAsStringProxyArchive | UE5.7 built-in | Proxy archive that converts UObject refs to strings | Safe serialization that handles object references |
| FImageUtils | UE5.7 built-in (Engine module) | CompressImageArray for screenshot thumbnails | Built-in image compression, no extra dependencies |
| UGameViewportClient::OnScreenshotCaptured | UE5.7 built-in | Delegate for viewport pixel capture | Engine-level screenshot capture callback |
| FScreenshotRequest | UE5.7 built-in | Trigger screenshot capture | Standard screenshot request API |

### Supporting
| Library/API | Version | Purpose | When to Use |
|-------------|---------|---------|-------------|
| AsyncSaveGameToSlot | UE5.7 built-in | Non-blocking save to prevent frame hitch | Use for the actual disk write during gameplay |
| IImageWrapperModule | UE5.7 built-in (ImageWrapper module) | JPEG/PNG compression | Alternative to FImageUtils for more control |
| FImageUtils::ImportBufferAsTexture2D | UE5.7 built-in | Create UTexture2D from compressed buffer | Reconstruct thumbnail textures from saved JPEG bytes |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| USaveGame (chosen) | Custom binary/JSON serialization | USaveGame handles platform-specific paths, versioning scaffolding, and integrates with UE tooling. Custom serialization adds complexity for no benefit at demo scope |
| SaveGame UPROPERTY + Serialize() | Manual struct packing per actor | Serialize() with SaveGame specifier is less code, handles nested USTRUCTs automatically, and is the Epic-recommended approach |
| Storing screenshots in save file | Saving screenshots as separate PNG files | In-file storage is simpler (one file per slot), no orphaned files on deletion, small overhead (~20KB JPEG per slot) |
| OpenLevel + InitGame flow | Seamless travel | Single-level demo has no streaming; OpenLevel is simpler and fully resets world state cleanly |

### No New Dependencies Required

All APIs come from Engine and core modules already in the project's Build.cs. The ImageWrapper module may need adding to PrivateDependencyModuleNames if FImageUtils does not cover all compression needs, but FImageUtils::CompressImageArray (in Engine module) should suffice.

## Architecture Patterns

### Recommended Project Structure
```
Source/ProjectWalkingSim/
  Public/
    Save/
      SereneSaveGame.h          # USaveGame subclass with all save data
      SaveTypes.h               # Save-related structs (FActorSaveData, FSaveSlotInfo)
      SaveSubsystem.h           # UGameInstanceSubsystem managing save/load flow
    Interaction/
      TapeRecorderActor.h       # Save point actor (IInteractable)
      SaveableInterface.h       # Expanded ISaveable (already exists)
    Player/
      HUD/
        GameOverWidget.h        # Game Over screen widget
        SaveSlotWidget.h        # Individual save slot display
        SaveLoadMenuWidget.h    # Slot picker for Load Game menu
  Private/
    Save/
      SereneSaveGame.cpp
      SaveSubsystem.cpp
    Interaction/
      TapeRecorderActor.cpp
    Player/
      HUD/
        GameOverWidget.cpp
        SaveSlotWidget.cpp
        SaveLoadMenuWidget.cpp
```

### Pattern 1: USaveGame with Flat Struct Arrays (NOT Per-Actor Binary Serialization)

**What:** For this project's simple scope (inventory, doors, pickups, player position), use a flat USaveGame class with explicit UPROPERTY fields rather than the full FMemoryWriter/FMemoryReader per-actor binary serialization pattern.

**Why flat over binary:** The project has only ~4 types of saveable state (inventory slots, door states, destroyed pickup IDs, player transform). Per-actor binary serialization with Serialize()/FMemoryWriter is powerful but overkill -- it introduces complexity around actor matching, requires the SaveGame UPROPERTY specifier on every field, and makes save file debugging harder. Flat structs in the USaveGame are simpler to implement, debug, and version.

**When to use:** Games with a known, limited set of saveable state types and a single level.

**Example:**
```cpp
// SaveTypes.h
USTRUCT()
struct FSavedDoorState
{
    GENERATED_BODY()

    UPROPERTY()
    FName DoorId;  // Actor name in level

    UPROPERTY()
    bool bIsOpen = false;

    UPROPERTY()
    bool bIsLocked = false;

    UPROPERTY()
    float CurrentAngle = 0.0f;

    UPROPERTY()
    float OpenDirection = 1.0f;
};

USTRUCT()
struct FSaveSlotInfo
{
    GENERATED_BODY()

    UPROPERTY()
    FDateTime Timestamp;

    UPROPERTY()
    TArray<uint8> ScreenshotData;  // Compressed JPEG

    UPROPERTY()
    int32 ScreenshotWidth = 0;

    UPROPERTY()
    int32 ScreenshotHeight = 0;
};

// SereneSaveGame.h
UCLASS()
class USereneSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    // --- Versioning ---
    UPROPERTY()
    int32 SaveVersion = 1;

    // --- Slot Metadata ---
    UPROPERTY()
    FSaveSlotInfo SlotInfo;

    // --- Player State ---
    UPROPERTY()
    FVector PlayerLocation;

    UPROPERTY()
    FRotator PlayerRotation;

    UPROPERTY()
    TArray<FInventorySlot> InventorySlots;

    // --- World State ---
    UPROPERTY()
    TArray<FSavedDoorState> DoorStates;

    UPROPERTY()
    TArray<FName> DestroyedPickupIds;  // FName = actor name
};
```

### Pattern 2: Game Instance Subsystem for Save Management

**What:** A UGameInstanceSubsystem that owns save/load logic, persists across level transitions, and coordinates the flow.

**Why:** The GameInstance persists across OpenLevel calls. A subsystem on it naturally survives the level reload that loading triggers. The GameMode does not persist. The existing USereneGameInstance is the right owner for game-wide state.

**When to use:** Any save system that involves level reloading.

**Example:**
```cpp
// SaveSubsystem.h
UCLASS()
class USaveSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // --- Save API ---
    UFUNCTION(BlueprintCallable, Category = "Save")
    void SaveToSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Save")
    void LoadFromSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Save")
    void LoadLatestSave();

    UFUNCTION(BlueprintCallable, Category = "Save")
    bool HasAnySave() const;

    UFUNCTION(BlueprintCallable, Category = "Save")
    FSaveSlotInfo GetSlotInfo(int32 SlotIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Save")
    void DeleteSlot(int32 SlotIndex);

    // --- Internal ---
    /** Called by GameMode after level reload to apply pending save data. */
    void ApplyPendingSaveData(UWorld* World);

    /** Whether a load is pending (set before OpenLevel, read after). */
    bool IsPendingLoad() const { return PendingSaveData != nullptr; }

private:
    static constexpr int32 MaxSlots = 3;

    FString GetSlotName(int32 SlotIndex) const;

    // Pending data to apply after level reload
    UPROPERTY()
    TObjectPtr<USereneSaveGame> PendingSaveData;

    // Screenshot capture
    void CaptureScreenshot(TFunction<void(TArray<uint8>&&, int32, int32)> Callback);
};
```

### Pattern 3: Load Flow via Level Restart

**What:** Loading a save restarts the level (OpenLevel on the current map), then applies saved state in GameMode::InitGame or HandleStartingNewPlayer.

**Why:** For a single-level demo, OpenLevel is the cleanest way to get a fresh world state. It destroys all actors and recreates them from the map, then the subsystem applies the saved deltas (door states, destroyed pickups, inventory, player position).

**Flow:**
```
1. Player selects "Load Game" -> slot
2. SaveSubsystem stores USereneSaveGame in PendingSaveData
3. SaveSubsystem calls UGameplayStatics::OpenLevel(CurrentMap)
4. Level reloads -- all actors reset to map defaults
5. GameMode::InitGame calls SaveSubsystem->ApplyPendingSaveData(World)
6. ApplyPendingSaveData iterates world actors:
   a. Find all ADoorActor instances, match by FName, restore states
   b. Find all APickupActor instances, destroy those in DestroyedPickupIds
   c. Set player position to saved location
   d. Populate inventory from saved slots
7. Clear PendingSaveData
8. Normal gameplay resumes
```

### Pattern 4: ISaveable Interface Expansion

**What:** Expand the existing ISaveable stub to work with the flat save data pattern. Rather than having ISaveable actors serialize themselves, the SaveSubsystem queries their state directly through the interface.

**Example:**
```cpp
// Expanded SaveableInterface.h
class ISaveable
{
    GENERATED_BODY()

public:
    /** Return a unique, stable identifier for this actor instance. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
    FName GetSaveId() const;

    /** Write this actor's state into the save game object. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
    void WriteSaveData(USereneSaveGame* SaveGame);

    /** Restore this actor's state from saved data. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
    void ReadSaveData(USereneSaveGame* SaveGame);
};
```

**Note:** The existing ISaveable has no-param WriteSaveData/ReadSaveData. These signatures need to change. Since ISaveable is a stub with no implementations anywhere (Phase 1 scaffolding), this is a non-breaking change -- no callers exist yet.

### Pattern 5: Actor Identification via FName (GetFName)

**What:** Use the actor's FName (GetFName()) as its save identifier. This works because all saveable actors (doors, pickups) are level-placed with stable names assigned by the editor.

**Why:** FName is stable across level loads for editor-placed actors. No need for FGuid or custom IDs -- those are for runtime-spawned actors, which this project does not save.

**Constraint:** Do NOT rename saveable actors in the editor after saves exist, or saves will fail to match. This is acceptable for demo scope.

### Anti-Patterns to Avoid
- **Saving everything:** Do NOT serialize the entire world. Only save the delta from default state: door open/closed/locked, destroyed pickups, inventory, player position.
- **Saving pointers/references:** UObject pointers cannot be serialized. Save FName identifiers and re-resolve after load.
- **Loading before world is ready:** Save data must be applied AFTER actors exist in the world (after InitGame/BeginPlay). Using a pending-load flag on the subsystem prevents premature application.
- **Blocking the game thread on save:** Use AsyncSaveGameToSlot to prevent frame hitches during the save animation.
- **Storing raw FColor arrays:** A 1920x1080 screenshot as FColor is ~8MB uncompressed. Compress to JPEG (~20KB) before storing in the save file.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Save file serialization | Custom binary format | USaveGame + UGameplayStatics::SaveGameToSlot | Handles platform paths, file I/O, error handling. Already used in project for settings. |
| Save file location/paths | Manual file path construction | UGameplayStatics slot name system | Platform-specific (Windows, consoles) save directories handled automatically |
| Screenshot compression | Manual pixel encoding | FImageUtils::CompressImageArray or IImageWrapperModule | Compression codecs are complex; engine provides tested implementations |
| Texture reconstruction from bytes | Manual texture creation | FImageUtils::ImportBufferAsTexture2D | Handles format detection, mip creation, GPU upload |
| Async file I/O | Custom threading | AsyncSaveGameToSlot / AsyncLoadGameFromSlot | Engine handles thread safety, callbacks, platform differences |
| Save file existence check | Manual file system queries | UGameplayStatics::DoesSaveGameExist | Abstracts platform-specific file system access |
| Save file deletion | Manual file deletion | UGameplayStatics::DeleteGameInSlot | Platform-safe deletion |

**Key insight:** UE5's save system is deceptively complete. The GameplayStatics API covers creation, saving, loading, existence checking, and deletion. The only custom work is defining what data to save and when to trigger save/load.

## Common Pitfalls

### Pitfall 1: Screenshot Capture Timing
**What goes wrong:** Requesting a screenshot and trying to use the data immediately. The screenshot is captured asynchronously at end-of-frame.
**Why it happens:** FScreenshotRequest::RequestScreenshot is not synchronous. The data arrives via OnScreenshotCaptured delegate.
**How to avoid:** Request screenshot first, then continue save flow in the delegate callback. The tape recorder save animation (~1-2 seconds) provides natural time for capture.
**Warning signs:** Black or corrupt thumbnail images in save slots.

### Pitfall 2: Applying Save Data Before World is Ready
**What goes wrong:** Trying to find actors and apply saved state before they exist.
**Why it happens:** GameInstance persists across level loads, so the subsystem may try to apply data before the new level's actors are spawned.
**How to avoid:** Use the pending-load pattern: store save data in subsystem, call ApplyPendingSaveData from GameMode after level initialization (InitGame or a post-BeginPlay callback).
**Warning signs:** Null actor pointers during load, missing door states, items not destroyed.

### Pitfall 3: Saving During Active Transitions
**What goes wrong:** Player saves while hiding, opening a door, or mid-interaction, causing inconsistent state on load.
**Why it happens:** The tape recorder is an IInteractable -- the player must be in a normal free state to interact with it.
**How to avoid:** TapeRecorderActor::CanInteract checks that the player is not hiding (HidingComponent::GetHidingState() == Free) and not in any transition. This is a natural guard since the player must walk up to the tape recorder.
**Warning signs:** Loading into a hiding state with no hiding spot reference, or loading into a half-open door.

### Pitfall 4: Inventory Slot Data Without Registry Context
**What goes wrong:** Saving FInventorySlot (FName ItemId, int32 Quantity) works, but on load the InventoryComponent's ItemRegistry may not be populated yet.
**Why it happens:** InventoryComponent::BeginPlay loads the ItemRegistry from Asset Manager. If save data is applied before BeginPlay, the registry is empty.
**How to avoid:** Apply inventory save data AFTER BeginPlay has run on the character. Use a two-step approach: save the slot data in the subsystem, then apply it when the character is fully initialized (e.g., via a delegate from character's BeginPlay or PossessedBy).
**Warning signs:** Items show up with correct IDs but no display names or icons.

### Pitfall 5: GrabAttack RestartLevel vs Save System Load
**What goes wrong:** The current GrabAttack task calls RestartLevel, which bypasses the save system entirely.
**Why it happens:** RestartLevel was placeholder death behavior (comment in STT_GrabAttack.h says "Phase 8 Polish will replace").
**How to avoid:** Replace RestartLevel in GrabAttack with a delegate/event that triggers the Game Over screen. The Game Over screen then calls SaveSubsystem->LoadLatestSave() or restarts the level if no save exists.
**Warning signs:** Death always restarts from beginning even when saves exist.

### Pitfall 6: Destroyed Pickup Tracking After Discard
**What goes wrong:** Player discards an item (spawns a new PickupActor at runtime), then saves. On load, the original level-placed pickup is incorrectly destroyed, and the discarded one doesn't exist.
**Why it happens:** Discarded items are runtime-spawned actors with generated names, not matching any level-placed actor FName.
**How to avoid:** Only track level-placed pickups that were picked up (destroyed). Discarded items are temporary and will not persist across saves -- this is acceptable and matches survival horror conventions (items return to original locations if you die/reload).
**Warning signs:** Items duplicating on load (both original and discarded versions appear).

### Pitfall 7: DrawerActor State
**What goes wrong:** DrawerActor has bIsOpen and CurrentSlide state but may be forgotten in save implementation.
**Why it happens:** Drawers are environmental furniture, easy to overlook.
**How to avoid:** Include DrawerActor in ISaveable implementation. Save bIsOpen and CurrentSlide. Or, if drawers are purely cosmetic and not progression-relevant, explicitly decide to not save them (acceptable for demo scope).
**Warning signs:** Drawers reset to closed on load despite being opened before save.

## Code Examples

### Screenshot Capture and Compression
```cpp
// Source: UGameViewportClient API docs + FImageUtils API docs
// Capture screenshot for save slot thumbnail

void USaveSubsystem::CaptureScreenshot(TFunction<void(TArray<uint8>&&, int32, int32)> Callback)
{
    // Store callback for use in delegate
    PendingScreenshotCallback = MoveTemp(Callback);

    // Bind to the viewport's screenshot delegate
    if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
    {
        ViewportClient->OnScreenshotCaptured().AddUObject(
            this, &USaveSubsystem::OnScreenshotCaptured);
    }

    // Request the screenshot (captured at end of frame)
    FScreenshotRequest::RequestScreenshot(false);
}

void USaveSubsystem::OnScreenshotCaptured(int32 Width, int32 Height, const TArray<FColor>& Bitmap)
{
    // Remove delegate binding (one-shot)
    if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
    {
        ViewportClient->OnScreenshotCaptured().RemoveAll(this);
    }

    // Downscale for thumbnail (optional - reduce from full res)
    // For demo, store at captured resolution; UI widget will scale

    // Compress to JPEG (much smaller than PNG for photos)
    TArray<uint8> CompressedData;
    FImageUtils::CompressImageArray(Width, Height, Bitmap, CompressedData);

    // Deliver to callback
    if (PendingScreenshotCallback)
    {
        PendingScreenshotCallback(MoveTemp(CompressedData), Width, Height);
        PendingScreenshotCallback = nullptr;
    }
}
```

### Reconstructing Thumbnail Texture from Saved Data
```cpp
// Source: FImageUtils API docs
// Create UTexture2D from compressed bytes stored in save game

UTexture2D* USaveSubsystem::CreateThumbnailTexture(const TArray<uint8>& CompressedData)
{
    if (CompressedData.Num() == 0)
    {
        return nullptr;
    }

    // ImportBufferAsTexture2D handles format detection and texture creation
    TArrayView64<const uint8> BufferView(CompressedData.GetData(), CompressedData.Num());
    return FImageUtils::ImportBufferAsTexture2D(BufferView);
}
```

### Saving World State (Flat Struct Pattern)
```cpp
// Source: Tom Looman save system pattern adapted for flat structs
void USaveSubsystem::GatherWorldState(USereneSaveGame* SaveGame, UWorld* World)
{
    SaveGame->DoorStates.Empty();
    SaveGame->DestroyedPickupIds.Empty();

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor->IsPendingKillPending())
        {
            continue;
        }

        // Doors
        if (ADoorActor* Door = Cast<ADoorActor>(Actor))
        {
            if (Door->Implements<USaveable>())
            {
                ISaveable::Execute_WriteSaveData(Door, SaveGame);
            }
        }
    }

    // Pickups: track which level-placed pickups have been destroyed
    // (They were Destroy()ed on pickup -- we saved their IDs to a runtime set)
    SaveGame->DestroyedPickupIds = DestroyedPickupTracker;
}
```

### Tracking Destroyed Pickups
```cpp
// Pattern: Hook into PickupActor::OnInteract to record destruction
// The SaveSubsystem maintains a TSet<FName> of destroyed level-placed pickups.
// When a PickupActor is picked up and destroyed, it notifies the subsystem.

// In PickupActor (or via delegate):
void APickupActor::OnInteract_Implementation(AActor* Interactor)
{
    // ... existing pickup logic ...
    if (bDestroyOnPickup)
    {
        // Notify save subsystem before destruction
        if (UGameInstance* GI = GetGameInstance())
        {
            if (USaveSubsystem* SaveSys = GI->GetSubsystem<USaveSubsystem>())
            {
                SaveSys->TrackDestroyedPickup(GetFName());
            }
        }
        Destroy();
    }
}
```

### Load Flow (Level Restart + Apply)
```cpp
// Source: Tom Looman / Epic official docs
void USaveSubsystem::LoadFromSlot(int32 SlotIndex)
{
    FString SlotName = GetSlotName(SlotIndex);

    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        UE_LOG(LogSerene, Warning, TEXT("No save in slot %d"), SlotIndex);
        return;
    }

    PendingSaveData = Cast<USereneSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0));

    if (!PendingSaveData)
    {
        UE_LOG(LogSerene, Error, TEXT("Failed to load save from slot %d"), SlotIndex);
        return;
    }

    // Restart the level -- world resets to map defaults
    UWorld* World = GetWorld();
    FString MapName = World->GetMapName();
    MapName.RemoveFromStart(World->StreamingLevelsPrefix);
    UGameplayStatics::OpenLevel(World, FName(*MapName));
}

// Called from GameMode after level restart
void USaveSubsystem::ApplyPendingSaveData(UWorld* World)
{
    if (!PendingSaveData)
    {
        return;
    }

    // 1. Restore door states
    for (const FSavedDoorState& DoorState : PendingSaveData->DoorStates)
    {
        // Find door by FName in world
        for (TActorIterator<ADoorActor> It(World); It; ++It)
        {
            if ((*It)->GetFName() == DoorState.DoorId)
            {
                ISaveable::Execute_ReadSaveData(*It, PendingSaveData);
                break;
            }
        }
    }

    // 2. Destroy picked-up items
    for (TActorIterator<APickupActor> It(World); It; ++It)
    {
        if (PendingSaveData->DestroyedPickupIds.Contains((*It)->GetFName()))
        {
            (*It)->Destroy();
        }
    }

    // 3. Restore player state (position + inventory)
    // Done via character's BeginPlay or PossessedBy callback

    // 4. Repopulate destroyed pickup tracker
    DestroyedPickupTracker.Empty();
    for (const FName& Id : PendingSaveData->DestroyedPickupIds)
    {
        DestroyedPickupTracker.Add(Id);
    }

    PendingSaveData = nullptr;
}
```

### Game Over Screen Integration
```cpp
// Replace GrabAttack's RestartLevel with death event
// In STT_GrabAttack::Tick, instead of ConsoleCommand("RestartLevel"):

// Option: Use a delegate on the GameMode or a global event
if (ASereneGameMode* GM = Cast<ASereneGameMode>(World->GetAuthGameMode()))
{
    GM->OnPlayerDeath();  // New method that shows Game Over widget
}

// GameMode::OnPlayerDeath()
void ASereneGameMode::OnPlayerDeath()
{
    // Show Game Over widget
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        // Create and show Game Over widget
        // Widget has two buttons: "Load Last Save" and "Quit"
        // "Load Last Save" calls SaveSubsystem->LoadLatestSave()
        // If no saves exist, button shows "Restart" and calls OpenLevel
        PC->SetShowMouseCursor(true);
        PC->SetInputMode(FInputModeUIOnly());
    }
}
```

### Tape Recorder Save Point Actor
```cpp
// TapeRecorderActor inherits AInteractableBase (same as Door, Pickup, etc.)
UCLASS()
class ATapeRecorderActor : public AInteractableBase
{
    GENERATED_BODY()

public:
    ATapeRecorderActor();

protected:
    virtual void OnInteract_Implementation(AActor* Interactor) override;
    virtual bool CanInteract_Implementation(AActor* Interactor) const override;
    virtual FText GetInteractionText_Implementation() const override;

    // Save slot selection handled by UI widget shown on interact
    // OR: tape recorder auto-picks next available slot / shows slot picker

    /** Sound played during save animation. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
    TObjectPtr<USoundBase> SaveSound;

    /** Duration of save animation (tape rolling). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
    float SaveAnimDuration = 1.5f;
};
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| SaveGameToSlot (sync) | AsyncSaveGameToSlot | UE4.20+ | Prevents frame hitch during save |
| FImageUtils::PNGCompressImageArray | FImageUtils::CompressImageArray (format-agnostic) | UE5.x | PNGCompressImageArray deprecated; use CompressImageArray or CompressImage instead |
| Custom file path management | UGameplayStatics slot system | Always | Platform-independent, handles Saved/SaveGames directory |
| Manual FViewport::ReadPixels | OnScreenshotCaptured delegate | UE4+ | Cleaner async pattern, no direct viewport access needed |

**Deprecated/outdated:**
- FImageUtils::PNGCompressImageArray: Deprecated in favor of CompressImage / CompressImageArray
- Manual save file paths: Use UGameplayStatics slot naming exclusively

## Open Questions

1. **DrawerActor save state**
   - What we know: DrawerActor has bIsOpen and CurrentSlide state. It exists in the codebase.
   - What's unclear: Whether drawers contain progression-relevant items or are purely cosmetic.
   - Recommendation: Include in ISaveable for completeness; it's minimal additional work (2 fields).

2. **Screenshot resolution and quality**
   - What we know: OnScreenshotCaptured captures at viewport resolution. FImageUtils::CompressImageArray produces compressed output.
   - What's unclear: Whether CompressImageArray produces JPEG or PNG by default (docs say it "compresses to thumbnail format"). CompressImage allows format specification.
   - Recommendation: Use CompressImage with "jpg" extension for smaller file size. If CompressImageArray output format is unclear at implementation time, use IImageWrapperModule with EImageFormat::JPEG explicitly.

3. **Save slot UI integration with pause menu**
   - What we know: Pause menu needs "Continue" and "Load Game". "Continue" loads latest save; "Load Game" opens slot picker.
   - What's unclear: Whether pause menu already exists or needs to be created from scratch.
   - Recommendation: Phase 7 should create both the save/load slot picker widget and wire it into a pause menu. If no pause menu exists, create a minimal one. This is UI work that can be done in Blueprint subclasses of C++ widget base classes.

4. **Exact timing of ApplyPendingSaveData**
   - What we know: Must happen after actors exist but ideally before gameplay starts.
   - What's unclear: Whether InitGame is early enough (actors may not have BeginPlay called yet) or if we need a later hook.
   - Recommendation: Use GameMode::HandleStartingNewPlayer or bind to the World's OnWorldBeginPlay delegate. Test empirically -- if InitGame is too early, move to a post-BeginPlay callback via GetWorld()->OnActorsInitialized.

## Sources

### Primary (HIGH confidence)
- [Epic Official Docs: Saving and Loading Your Game](https://dev.epicgames.com/documentation/en-us/unreal-engine/saving-and-loading-your-game-in-unreal-engine) - USaveGame API, async save/load, slot management
- [Epic Official Docs: FImageUtils API](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/FImageUtils) - CompressImageArray, CompressImage, ImportBufferAsTexture2D signatures
- [Epic Official Docs: UGameViewportClient](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/UGameViewportClient) - OnScreenshotCaptured delegate
- [Epic Official Docs: FImageUtils::CompressImage](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/FImageUtils/CompressImage) - Format-agnostic compression with quality parameter
- Existing codebase analysis: SereneGameInstance.h (USereneSettingsSave pattern), SaveableInterface.h (ISaveable stub), all actor headers

### Secondary (MEDIUM confidence)
- [Tom Looman: Unreal Engine C++ Save System](https://tomlooman.com/unreal-engine-cpp-save-system/) - Per-actor binary serialization with FMemoryWriter/FMemoryReader, FActorSaveData pattern, SaveGame UPROPERTY specifier usage
- [Epic Forums: OnScreenshotCaptured delegate usage](https://forums.unrealengine.com/t/how-use-onscreenshotcaptured-delegate/562068) - Delegate binding pattern, callback signature (int32, int32, TArray<FColor>)
- [Epic Forums: Stable actor IDs across executions](https://forums.unrealengine.com/t/is-there-a-unique-id-for-actors-placed-in-the-level-that-is-stable-across-executions-and-modifications-to-the-level/777971) - FName stability for level-placed actors

### Tertiary (LOW confidence)
- [Epic Serialization Best Practices Talk](https://dev.epicgames.com/community/learning/talks-and-demos/4ORW/unreal-engine-serialization-best-practices-and-techniques) - Referenced but not directly fetched
- [SPUD Save Library](https://github.com/sinbad/SPUD) - Comprehensive save persistence library, confirms destroyed actor tracking pattern

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - All APIs are built-in UE5 Engine module, verified against official docs
- Architecture (flat struct USaveGame): HIGH - Simpler variant of established pattern, well-suited to project scope
- Architecture (screenshot capture): MEDIUM - OnScreenshotCaptured delegate confirmed in docs and forums, but exact CompressImageArray output format needs runtime verification
- Architecture (load flow): MEDIUM - OpenLevel + apply pattern is well-documented, exact timing hook (InitGame vs post-BeginPlay) needs empirical testing
- Pitfalls: HIGH - Based on codebase analysis and established community knowledge
- Game Over / death integration: HIGH - Current GrabAttack code analyzed, replacement path is straightforward

**Research date:** 2026-02-12
**Valid until:** 2026-03-12 (stable domain, UE5.7 release cycle)
