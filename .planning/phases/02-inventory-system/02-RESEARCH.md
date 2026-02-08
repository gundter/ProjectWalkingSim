# Phase 2: Inventory System - Research

**Researched:** 2026-02-08
**Domain:** UE5 C++ inventory component, item data assets, UMG inventory UI, input mode management
**Confidence:** HIGH

## Summary

This phase builds an 8-slot inventory system for a first-person horror game (The Juniper Tree) using UE 5.7. The existing codebase follows a clean component-based architecture with C++ base classes and UMG Blueprint subclasses connected via `meta=(BindWidget)`. The interaction system (Phase 1) already provides `APickupActor` with `ItemId`/`Quantity` fields, `UInteractionComponent` with line-trace focus detection, and `IInteractable` interface -- all designed as integration points for this phase.

The standard UE5 approach for inventory systems uses: (1) a `UPrimaryDataAsset` subclass for item definitions with soft references to icons/meshes, (2) a `UActorComponent` subclass as the inventory backend attached to the character, (3) event-driven UI updates via delegates, and (4) data-driven item behavior through item type enums and the item definition assets. The project's existing patterns (delegate-driven HUD, component architecture, BindWidget) align perfectly with this approach.

**Primary recommendation:** Use `UPrimaryDataAsset` for item definitions (not DataTable), `UInventoryComponent` as an `UActorComponent` on the character, `FInventorySlot` struct for runtime slot state, and modular UMG widgets following the existing BindWidget pattern. Keep all item data as assets in `Content/Data/Items/` for designer-friendly editing.

## Standard Stack

### Core (already in Build.cs)
| Library/Module | Version | Purpose | Why Standard |
|----------------|---------|---------|--------------|
| UMG | UE 5.7 | Inventory UI widgets | Already used for HUD; BindWidget pattern established |
| EnhancedInput | UE 5.7 | Toggle inventory, navigate slots | Already used for all input; IMC priority layering for UI mode |
| GameplayTags | UE 5.7 | Item type identification, interaction tags | Already used for SereneTags; extend for item types |
| PhysicsCore | UE 5.7 | Discarded item physics (AddImpulse) | Already in Build.cs; needed for item toss on discard |

### Supporting (no new modules needed)
| Library/Module | Purpose | When to Use |
|----------------|---------|-------------|
| Engine (UPrimaryDataAsset) | Item definition base class | Item data assets with asset bundle support |
| Slate/SlateCore | Already in PrivateDependency | Low-level widget features if needed |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| UPrimaryDataAsset | UDataTable + FTableRowBase | DataTable is better for hundreds of items with CSV import; PrimaryDataAsset is better for this project's small item count (~10-20), provides per-asset editing in Content Browser, inheritance support, soft ref bundles, and no C++ row-lookup boilerplate |
| UPrimaryDataAsset | Plain UDataAsset | PrimaryDataAsset adds GetPrimaryAssetId() and asset bundle support for free; no reason to use plain UDataAsset |
| TArray<FInventorySlot> | TMap<FName, FInventorySlot> | TMap is faster for lookup by ID but 8 slots means TArray iteration is negligible; TArray preserves slot ordering which UI needs |

**No new Build.cs dependencies required.** All needed modules are already present.

## Architecture Patterns

### Recommended Project Structure
```
Source/ProjectWalkingSim/
  Public/
    Inventory/
      ItemDataAsset.h          # UPrimaryDataAsset subclass - item definitions
      InventoryComponent.h     # UActorComponent - inventory backend (8 slots)
      InventoryTypes.h         # FInventorySlot struct, EItemType enum, delegates
    Interaction/
      PickupActor.h            # EXISTING - modified to integrate with inventory
      DoorActor.h              # EXISTING - modified to check for key items
    Player/
      HUD/
        InventoryWidget.h      # Root inventory panel (1x8 horizontal row)
        InventorySlotWidget.h  # Single slot widget (icon, quantity badge)
        ItemTooltipWidget.h    # Tooltip popup (name, description, action buttons)
        SereneHUDWidget.h      # EXISTING - add InventoryWidget as BindWidget child
      Components/
        InteractionComponent.h # EXISTING - no changes needed
      SereneCharacter.h        # EXISTING - add InventoryComponent
      SerenePlayerController.h # EXISTING - add inventory toggle input + input mode switching
    Tags/
      SereneTags.h             # EXISTING - add inventory/item tags
  Private/
    Inventory/
      ItemDataAsset.cpp
      InventoryComponent.cpp
      InventoryTypes.cpp
    Interaction/
      PickupActor.cpp          # MODIFIED
      DoorActor.cpp            # MODIFIED - locked door support
    Player/
      HUD/
        InventoryWidget.cpp
        InventorySlotWidget.cpp
        ItemTooltipWidget.cpp
      SereneCharacter.cpp      # MODIFIED
      SerenePlayerController.cpp # MODIFIED
    Tags/
      SereneTags.cpp           # MODIFIED

Content/
  Data/
    Items/
      DA_Key_FrontDoor.uasset   # ItemDataAsset instances
      DA_Key_Basement.uasset
      DA_Code_Safe.uasset
  UI/
    Inventory/
      WBP_InventoryWidget.uasset    # UMG Blueprint subclasses
      WBP_InventorySlot.uasset
      WBP_ItemTooltip.uasset
  Input/
    Actions/
      IA_ToggleInventory.uasset     # New input action
    Mappings/
      IMC_Default.uasset            # MODIFIED - rebind interact to F, add inventory toggle
```

### Pattern 1: Item Data Asset (UPrimaryDataAsset)
**What:** Each item type is a `UPrimaryDataAsset` instance created in Content Browser. Contains all static data about the item: name, description, icon, item type, max stack size, world mesh, etc. Uses `TSoftObjectPtr` for heavy assets (textures, meshes) to avoid loading everything at startup.
**When to use:** Define any new item type (key, code, fuse, etc.)
**Example:**
```cpp
// Source: UE5 official docs + Tom Looman Asset Manager pattern
UENUM(BlueprintType)
enum class EItemType : uint8
{
    KeyItem,      // Keys, codes - critical path items
    Consumable,   // Future: healing, etc.
    Tool,         // Future: flashlight battery, etc.
    Puzzle        // Fuse, handle, etc.
};

UCLASS(BlueprintType)
class PROJECTWALKINGSIM_API UItemDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;

    /** Unique identifier matching APickupActor::ItemId. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FName ItemId;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText Description;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemType ItemType = EItemType::KeyItem;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    bool bIsStackable = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item",
        meta = (EditCondition = "bIsStackable", ClampMin = "1", ClampMax = "99"))
    int32 MaxStackSize = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    bool bIsKeyItem = false;

    /** Icon displayed in inventory slot. Soft ref to avoid loading all icons at startup. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Visuals")
    TSoftObjectPtr<UTexture2D> Icon;

    /** Mesh to spawn when item is discarded into world. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Visuals")
    TSoftObjectPtr<UStaticMesh> WorldMesh;

    /** GameplayTag for item-specific logic (e.g., Item.Key.FrontDoor). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FGameplayTag ItemTag;
};
```

### Pattern 2: Inventory Component (UActorComponent)
**What:** `UInventoryComponent` manages an 8-slot TArray of `FInventorySlot` structs. Exposes `TryAddItem`, `RemoveItem`, `UseItem`, `GetSlots`, `HasItem`, `IsFull` etc. Broadcasts `OnInventoryChanged` delegate for UI updates. Holds a reference to a registry of all item data assets for lookup by `FName ItemId`.
**When to use:** Attached to `ASereneCharacter` in constructor, same pattern as `UStaminaComponent`, `UInteractionComponent`.
**Example:**
```cpp
USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FName ItemId = NAME_None;

    UPROPERTY(BlueprintReadOnly)
    int32 Quantity = 0;

    bool IsEmpty() const { return ItemId == NAME_None || Quantity <= 0; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTWALKINGSIM_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    /** Try to add item. Returns true if successful, false if inventory full. */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryAddItem(FName ItemId, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(int32 SlotIndex, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool HasItem(FName ItemId) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool IsFull() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    const TArray<FInventorySlot>& GetSlots() const { return Slots; }

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    const UItemDataAsset* GetItemData(FName ItemId) const;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryChanged OnInventoryChanged;

    static constexpr int32 MaxSlots = 8;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<FInventorySlot> Slots;
};
```

### Pattern 3: Delegate-Driven UI Updates (Existing Project Pattern)
**What:** The inventory component broadcasts `OnInventoryChanged` whenever slots change. `ASereneHUD::BindToCharacter` subscribes to this delegate (same pattern as `StaminaComponent::OnStaminaChanged` and `InteractionComponent::OnInteractableChanged`). The HUD handler calls into the `InventoryWidget` to refresh.
**When to use:** Any time inventory state changes (add, remove, use, combine).
**Example:**
```cpp
// In ASereneHUD::BindToCharacter (existing pattern)
if (UInventoryComponent* Inventory = Character->FindComponentByClass<UInventoryComponent>())
{
    Inventory->OnInventoryChanged.AddDynamic(this, &ASereneHUD::HandleInventoryChanged);
}

// Handler routes to widget
void ASereneHUD::HandleInventoryChanged()
{
    if (HUDWidgetInstance && HUDWidgetInstance->GetInventoryWidget())
    {
        HUDWidgetInstance->GetInventoryWidget()->RefreshSlots();
    }
}
```

### Pattern 4: Input Mode Switching for Real-Time Inventory
**What:** When inventory opens, switch to `FInputModeGameAndUI` + `bShowMouseCursor = true` + disable look input via `SetIgnoreLookInput(true)`. Player retains WASD movement but cannot rotate camera. When inventory closes, restore `FInputModeGameOnly` + `bShowMouseCursor = false` + `SetIgnoreLookInput(false)`.
**When to use:** Toggle inventory open/close via key press.
**Example:**
```cpp
// In ASerenePlayerController
void ASerenePlayerController::OpenInventory()
{
    bIsInventoryOpen = true;

    // Show cursor, allow UI interaction, keep game input for WASD
    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);
    SetShowMouseCursor(true);
    SetIgnoreLookInput(true);

    // Notify HUD to show inventory
    if (ASereneHUD* HUD = Cast<ASereneHUD>(GetHUD()))
    {
        HUD->ShowInventory();
    }
}

void ASerenePlayerController::CloseInventory()
{
    bIsInventoryOpen = false;

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    SetShowMouseCursor(false);
    SetIgnoreLookInput(false);

    if (ASereneHUD* HUD = Cast<ASereneHUD>(GetHUD()))
    {
        HUD->HideInventory();
    }
}
```

### Pattern 5: Discard Item -> Spawn in World
**What:** When player discards an item, spawn an `APickupActor` in front of the player with a slight forward impulse. Calculate spawn point ~100cm in front of player at eye height, apply small forward+downward impulse so item arcs and lands ~1m ahead.
**When to use:** Discard action from inventory tooltip.
**Example:**
```cpp
void UInventoryComponent::DiscardItem(int32 SlotIndex)
{
    if (!Slots.IsValidIndex(SlotIndex) || Slots[SlotIndex].IsEmpty()) return;

    const FInventorySlot& Slot = Slots[SlotIndex];
    const UItemDataAsset* ItemData = GetItemData(Slot.ItemId);
    if (!ItemData) return;

    // Spawn pickup in front of player
    AActor* Owner = GetOwner();
    FVector EyeLocation;
    FRotator EyeRotation;
    if (APawn* Pawn = Cast<APawn>(Owner))
    {
        Pawn->GetController()->GetPlayerViewPoint(EyeLocation, EyeRotation);
    }

    const FVector SpawnLocation = EyeLocation + EyeRotation.Vector() * 80.0f;
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    APickupActor* DroppedItem = GetWorld()->SpawnActor<APickupActor>(
        APickupActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);

    if (DroppedItem)
    {
        // Configure the spawned pickup with this item's data
        DroppedItem->InitFromItemData(Slot.ItemId, Slot.Quantity, ItemData);

        // Apply small forward + downward impulse
        if (UPrimitiveComponent* PrimComp = DroppedItem->FindComponentByClass<UPrimitiveComponent>())
        {
            PrimComp->SetSimulatePhysics(true);
            const FVector Impulse = EyeRotation.Vector() * 150.0f + FVector(0, 0, 50.0f);
            PrimComp->AddImpulse(Impulse);
        }
    }

    // Remove from inventory
    RemoveItem(SlotIndex, Slot.Quantity);
}
```

### Pattern 6: Locked Door Integration
**What:** Extend `ADoorActor` with an optional `RequiredItemId` (FName). Override `CanInteract_Implementation` to check the player's `UInventoryComponent` for the required item. If the player has the key, consume it and unlock. Change `InteractionText` to "Locked" when locked, "Open" when unlocked.
**When to use:** Any door that requires a key item.
**Example:**
```cpp
// In ADoorActor (extended)
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
FName RequiredItemId = NAME_None;

UPROPERTY(BlueprintReadOnly, Category = "Door")
bool bIsLocked = false; // Set to true in editor when RequiredItemId is set

bool ADoorActor::CanInteract_Implementation(AActor* Interactor) const
{
    if (!bCanBeInteracted) return false;
    if (!bIsLocked) return true;

    // Check if player has the required key
    if (UInventoryComponent* Inv = Interactor->FindComponentByClass<UInventoryComponent>())
    {
        return Inv->HasItem(RequiredItemId);
    }
    return false;
}

void ADoorActor::OnInteract_Implementation(AActor* Interactor)
{
    if (bIsLocked)
    {
        // Consume the key and unlock
        if (UInventoryComponent* Inv = Interactor->FindComponentByClass<UInventoryComponent>())
        {
            Inv->RemoveItemByName(RequiredItemId);
        }
        bIsLocked = false;
        // Fall through to open
    }
    // Existing open/close toggle logic
    Super::OnInteract_Implementation(Interactor);
}
```

### Anti-Patterns to Avoid
- **Inventory logic in the widget:** Keep all state mutation in `UInventoryComponent`. Widgets read state and display it -- they never modify the TArray directly. Widgets call component functions, component broadcasts delegate, widget refreshes.
- **Hard references to item textures:** Use `TSoftObjectPtr<UTexture2D>` in `UItemDataAsset`. Loading every item icon into memory at startup defeats the purpose.
- **Tick-based UI updates:** Use the `OnInventoryChanged` delegate, not per-frame polling. The inventory changes at discrete moments.
- **Per-widget visibility toggling with SetVisibility:** Per the project's UMG lessons, use `SetRenderOpacity(0.0f/1.0f)` for child UserWidgets inside Canvas Panels. Individual elements within a widget (TextBlock, Image) can use `SetVisibility` fine.
- **Putting inventory state in the controller or HUD:** The inventory is gameplay state -- it belongs on the character as a component, not on the controller or HUD. The HUD only reflects state, never owns it.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Item definition storage | Custom serialization/JSON loader | `UPrimaryDataAsset` subclass | Engine handles serialization, Content Browser editing, soft references, asset bundles |
| Item registry/lookup | Manual TMap<FName, UItemDataAsset*> with hard-coded paths | `UAssetManager::GetPrimaryAssetIdList` + engine scanning | Engine discovers assets by type automatically; configure in Project Settings > Asset Manager |
| Widget show/hide animation | Manual alpha interpolation in Tick | `UWidgetAnimation` with `BindWidgetAnimOptional` | UMG animation editor is visual and tunable; project already uses this pattern (StaminaBarWidget) |
| Input mode switching | Custom input blocking/routing | `FInputModeGameAndUI` / `FInputModeGameOnly` + `SetIgnoreLookInput` | Engine built-in; handles cursor, focus, input routing correctly |
| Slot selection/hover | Custom hit testing | UMG button/widget `OnClicked`, `OnHovered`, `OnUnhovered` events | UMG handles hit testing, cursor feedback, focus navigation natively |

**Key insight:** The item count for this demo is very small (under 20 items). Over-engineering the data pipeline (DataTables, async streaming, custom asset managers) adds complexity without benefit. Simple `UPrimaryDataAsset` instances in the Content Browser are the right scale.

## Common Pitfalls

### Pitfall 1: Initialization Order - Delegate Binding vs Widget Creation
**What goes wrong:** `BindToCharacter` runs during `PossessedBy` (before `BeginPlay`), but widgets are created in `ASereneHUD::BeginPlay`. If a delegate fires between these two events, the widget pointer is null.
**Why it happens:** UE initialization order: PossessedBy -> BeginPlay. The project already handles this correctly (delegates bind early but only fire during gameplay), but adding inventory delegates follows the same pattern.
**How to avoid:** Same pattern as existing code. Bind delegate in `BindToCharacter`, null-check `HUDWidgetInstance` in handlers. Delegates will not fire before BeginPlay completes because no inventory operations happen during initialization.
**Warning signs:** Null pointer crash in HandleInventoryChanged on first frame.

### Pitfall 2: Input Mode Conflicts with Look Input
**What goes wrong:** In `FInputModeGameAndUI`, mouse movement drives both cursor position AND camera rotation simultaneously, causing the camera to spin when the player tries to click UI elements.
**Why it happens:** Game input is still active in GameAndUI mode, so mouse delta feeds into look input.
**How to avoid:** Call `SetIgnoreLookInput(true)` when opening inventory, `SetIgnoreLookInput(false)` when closing. This disables camera rotation from mouse while keeping WASD movement active.
**Warning signs:** Camera spinning while inventory is open; unable to click inventory slots because the view keeps moving.

### Pitfall 3: TSoftObjectPtr Not Loaded When Needed
**What goes wrong:** Calling `Icon.Get()` returns nullptr because the soft reference has not been loaded yet.
**Why it happens:** `TSoftObjectPtr` is a lazy reference -- the asset is not in memory until explicitly loaded.
**How to avoid:** For this project's small item count, call `Icon.LoadSynchronous()` when creating the slot widget. Synchronous loading is acceptable for small textures. For larger projects, use `FStreamableManager::RequestAsyncLoad`. In the item data asset, ensure icons are small (128x128 or 256x256 max).
**Warning signs:** Empty/invisible slot icons in the inventory UI.

### Pitfall 4: BindWidget Naming Mismatch
**What goes wrong:** UMG Blueprint fails to compile or crashes because a `BindWidget` pointer name doesn't match the widget name in the Blueprint.
**Why it happens:** The `meta=(BindWidget)` system requires exact name matching between the C++ UPROPERTY and the UMG Blueprint child widget name.
**How to avoid:** Follow the existing project convention: C++ property `TObjectPtr<UImage> ItemIcon` must match a widget named "ItemIcon" in the UMG Blueprint. Document expected names in class docstrings.
**Warning signs:** Editor error "Required widget 'X' not bound" when opening the UMG Blueprint.

### Pitfall 5: Discarded Item Spawns Inside Geometry
**What goes wrong:** Spawned pickup actor clips through floor or walls because spawn location overlaps with world geometry.
**Why it happens:** Spawn point calculated from eye position + forward offset may be inside a wall if the player is looking at a wall closely.
**How to avoid:** Use `ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn` and spawn slightly below eye height (not at eye level). Optionally do a short line trace from camera to find clear space. Enable physics simulation on the spawned mesh so it settles naturally.
**Warning signs:** Items disappearing into walls or falling through the floor after discard.

### Pitfall 6: Inventory Full Check Not Reflected in Interaction Prompt
**What goes wrong:** Player can see "F: Pick Up" even when inventory is full, leading to confusion when pressing F does nothing.
**Why it happens:** `InteractionComponent` reads `GetInteractionText()` from the interactable, but the interactable doesn't know about the player's inventory state.
**How to avoid:** In `APickupActor::CanInteract_Implementation`, check the interactor's `UInventoryComponent::IsFull()`. If full, modify `GetInteractionText` to return "Inventory Full" and/or return false from `CanInteract`. The `InteractionComponent` already handles null/invalid interactables gracefully.
**Warning signs:** Silent failure when trying to pick up items with a full inventory.

## Code Examples

### Item Data Asset GetPrimaryAssetId
```cpp
// Source: UE5 official documentation + Tom Looman pattern
FPrimaryAssetId UItemDataAsset::GetPrimaryAssetId() const
{
    return FPrimaryAssetId("Item", GetFName());
}
```

### Register Item Asset Type in DefaultEngine.ini (or Project Settings)
```ini
; In Config/DefaultEngine.ini, under [/Script/Engine.AssetManagerSettings]
; Or configure via Project Settings > Game > Asset Manager > Primary Asset Types to Scan
; Add entry:
;   PrimaryAssetType: "Item"
;   Asset Base Class: /Script/ProjectWalkingSim.ItemDataAsset
;   Directories: /Game/Data/Items
;   Has Blueprint Classes: False
```

### Creating a Slot Widget (BindWidget pattern matching project conventions)
```cpp
// Source: Existing project pattern (StaminaBarWidget, InteractionPromptWidget)
UCLASS()
class PROJECTWALKINGSIM_API UInventorySlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Update slot display from inventory data. */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetSlotData(const FInventorySlot& SlotData, const UItemDataAsset* ItemData);

    /** Clear slot to empty state. */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ClearSlot();

    /** Set visual selected state. */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetSelected(bool bSelected);

    /** Slot index in the inventory array (0-7). */
    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    int32 SlotIndex = -1;

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> SlotBackground;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ItemIcon;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> QuantityText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> SelectionHighlight;
};
```

### Inventory Widget with Dynamic Slot Creation
```cpp
// In InventoryWidget - creates 8 slot widgets at construction
void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Start hidden
    SetRenderOpacity(0.0f);

    // Create 8 slot widgets and add to horizontal box
    if (SlotContainer && SlotWidgetClass)
    {
        for (int32 i = 0; i < UInventoryComponent::MaxSlots; i++)
        {
            UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(
                GetOwningPlayer(), SlotWidgetClass);
            if (SlotWidget)
            {
                SlotWidget->SlotIndex = i;
                SlotContainer->AddChildToHorizontalBox(SlotWidget);
                SlotWidgets.Add(SlotWidget);
            }
        }
    }
}

void UInventoryWidget::RefreshSlots(const TArray<FInventorySlot>& Slots,
    const UInventoryComponent* InventoryComp)
{
    for (int32 i = 0; i < SlotWidgets.Num() && i < Slots.Num(); i++)
    {
        if (Slots[i].IsEmpty())
        {
            SlotWidgets[i]->ClearSlot();
        }
        else
        {
            const UItemDataAsset* ItemData = InventoryComp->GetItemData(Slots[i].ItemId);
            SlotWidgets[i]->SetSlotData(Slots[i], ItemData);
        }
    }
}
```

### PickupActor Integration with Inventory
```cpp
// Modified APickupActor::OnInteract_Implementation
void APickupActor::OnInteract_Implementation(AActor* Interactor)
{
    UInventoryComponent* Inventory = Interactor->FindComponentByClass<UInventoryComponent>();
    if (!Inventory)
    {
        UE_LOG(LogSerene, Warning, TEXT("APickupActor::OnInteract - No InventoryComponent on interactor."));
        return;
    }

    if (Inventory->TryAddItem(ItemId, Quantity))
    {
        UE_LOG(LogSerene, Log, TEXT("APickupActor::OnInteract - Added %s x%d to inventory."),
            *ItemId.ToString(), Quantity);

        // TODO: Play pickup sound

        if (bDestroyOnPickup)
        {
            Destroy();
        }
    }
    else
    {
        UE_LOG(LogSerene, Log, TEXT("APickupActor::OnInteract - Inventory full, cannot pick up %s."),
            *ItemId.ToString());
    }
}
```

### Inventory Toggle Input Binding
```cpp
// In ASerenePlayerController::SetupInputComponent (add to existing)
if (ToggleInventoryAction)
{
    EnhancedInput->BindAction(ToggleInventoryAction, ETriggerEvent::Started,
        this, &ASerenePlayerController::HandleToggleInventory);
}

void ASerenePlayerController::HandleToggleInventory(const FInputActionValue& Value)
{
    if (bIsInventoryOpen)
    {
        CloseInventory();
    }
    else
    {
        OpenInventory();
    }
}
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| UDataTable for item defs | UPrimaryDataAsset for small item sets | UE 4.17+ (Asset Manager) | Per-asset editing in Content Browser; soft reference bundles; no CSV boilerplate |
| Blueprint tick-based UI | Event-driven delegates | Long-standing best practice | No per-frame cost; cleaner data flow |
| Set Input Mode UI Only (for menus) | FInputModeGameAndUI (for real-time overlays) | Always available | Allows game input to continue while UI is active |
| Hard UTexture2D* references | TSoftObjectPtr<UTexture2D> | UE 4.x+ | Prevents loading all item icons at asset load time |

**Deprecated/outdated:**
- `ConstructorHelpers::FObjectFinder` for data table hard-coding: Use asset manager scanning or UPROPERTY soft refs instead. The wiki example uses this but it's considered fragile for production.
- `UDataTable` for item definitions in small projects: Overkill when you have fewer than ~50 items; PrimaryDataAsset is simpler and more designer-friendly.

## Open Questions

1. **Item Registry Loading Strategy**
   - What we know: `UAssetManager` can scan directories for `UPrimaryDataAsset` subclasses and provide a list of all registered items. For the demo's small item count (<20), this is the simplest approach.
   - What's unclear: Whether to load all item data assets at game startup (simplest), or load on-demand per item. With <20 items and small data assets, eager loading is likely fine.
   - Recommendation: Eager-load all `UItemDataAsset` assets via Asset Manager at game startup (in `USereneGameInstance::Init`). Cache in a TMap<FName, UItemDataAsset*> for O(1) lookup. This is simple and the data is small.

2. **Combine System Complexity**
   - What we know: Context specifies two-step combine (select A -> click Combine -> click B). Demo items are keys and codes -- no obvious combine recipes needed for demo scope.
   - What's unclear: How many combine recipes will exist. If only 0-2 recipes, a simple hardcoded check may suffice vs. a data-driven recipe table.
   - Recommendation: Implement the UI flow for combine (two-step selection), but keep the recipe system extremely simple: a static function or small TMap<TPair<FName,FName>, FName> of {ItemA, ItemB} -> ResultItem. Can be expanded later if needed. For demo, may have zero actual recipes but the UI flow should work.

3. **Keyboard Navigation Specifics**
   - What we know: Context says "both mouse cursor and keyboard navigation supported simultaneously." Number keys 1-8 for direct slot selection is standard for horror games.
   - What's unclear: Exact key bindings for navigating between slots, triggering actions.
   - Recommendation: Number keys 1-8 for direct slot selection. Arrow keys or Q/E for cycling selected slot. Enter for "Use", Delete for "Discard". These can be IA_ input actions in the IMC_Default or a separate IMC_Inventory added at higher priority when inventory is open.

4. **PickupActor Initialization for Discarded Items**
   - What we know: `APickupActor` currently has `ItemId`, `Quantity`, `MeshComponent`, `bDestroyOnPickup`. When discarding, need to spawn a PickupActor with the right mesh and data.
   - What's unclear: Whether to subclass PickupActor or add an `InitFromItemData` method.
   - Recommendation: Add a public `InitFromItemData(FName InItemId, int32 InQuantity, const UItemDataAsset* ItemData)` method to `APickupActor` that sets ItemId, Quantity, and loads the WorldMesh from the data asset onto MeshComponent.

## Sources

### Primary (HIGH confidence)
- [UE5 Official: Data Assets](https://dev.epicgames.com/documentation/en-us/unreal-engine/data-assets-in-unreal-engine) - UPrimaryDataAsset, GetPrimaryAssetId pattern
- [UE5 Official: Asset Management](https://dev.epicgames.com/documentation/en-us/unreal-engine/asset-management-in-unreal-engine) - PrimaryAssetTypesToScan, FPrimaryAssetId
- [UE5 Official: UMG Best Practices](https://dev.epicgames.com/documentation/en-us/unreal-engine/umg-best-practices-in-unreal-engine) - Event-driven UI, widget reusability
- [UE5 Official: Set Input Mode Game And UI](https://dev.epicgames.com/documentation/en-us/unreal-engine/BlueprintAPI/Input/SetInputModeGameAndUI) - Input mode API
- Existing codebase: All C++ source files read directly (`PickupActor.h/.cpp`, `InteractableBase.h/.cpp`, `SereneCharacter.h/.cpp`, `SerenePlayerController.h/.cpp`, `SereneHUD.h/.cpp`, `SereneHUDWidget.h`, `InteractionPromptWidget.h/.cpp`, `StaminaBarWidget.h/.cpp`, `InteractionComponent.h`, `SereneTags.h/.cpp`)
- Project memory files: `patterns.md`, `umg-lessons.md`

### Secondary (MEDIUM confidence)
- [Tom Looman: Asset Manager & Async Loading](https://tomlooman.com/unreal-engine-asset-manager-async-loading/) - PrimaryDataAsset patterns with code examples
- [UE Community Wiki: C++ Inventory](https://unrealcommunity.wiki/cpp-inventory-0z7lrwqv) - FInventoryItem struct, DataTable integration, AddItem pattern
- [Spongehammer: UE5 Inventory System C++ Guide](https://www.spongehammer.com/unreal-engine-5-inventory-system-cpp-guide/) - Component architecture, soft references, delegate patterns
- [Epic Forums: DataAsset vs DataTables](https://forums.unrealengine.com/t/dataasset-vs-datatables-for-an-item-system/828711) - Community consensus on DataAsset vs DataTable

### Tertiary (LOW confidence)
- [Epic Forums: Input mode cursor issues](https://forums.unrealengine.com/t/show-and-hide-mouse-cursor/2521011) - SetShowMouseCursor quirks (verified against official docs)

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - No new dependencies needed; all modules already in Build.cs; patterns verified against UE5 official docs
- Architecture: HIGH - Follows existing codebase patterns (component-based, BindWidget, delegate-driven HUD); verified against multiple sources
- Item data approach: HIGH - UPrimaryDataAsset is officially documented and well-suited for small item counts; Tom Looman pattern verified
- UI patterns: HIGH - BindWidget, CreateWidget, HorizontalBox dynamic children all verified in UE5 docs and match existing project conventions
- Input mode management: MEDIUM - FInputModeGameAndUI is well-documented but cursor/look interaction nuances may need tuning during implementation
- Combine system: LOW - No concrete example found for two-step combine UI; architecture is sound but implementation details are original design work
- Pitfalls: HIGH - All pitfalls derived from existing codebase analysis (initialization order) or verified UE5 behaviors (soft refs, BindWidget naming)

**Research date:** 2026-02-08
**Valid until:** 2026-03-08 (stable domain; UE5 patterns are well-established)
