# Phase 2 Editor Setup Guide: Inventory System

After running the Python asset creation script, complete these manual steps in the Unreal Editor.

The C++ code for the inventory system is complete. This guide covers creating the UMG Blueprint widgets, item data assets, and test map configuration that Claude cannot automate.

---

## Part 1: Run Python Script

### Execute create_phase2_assets.py

1. Open Unreal Editor with the project
2. Go to **Window > Output Log** to monitor progress
3. Run the Python script via one of these methods:

**Method A: Editor Console**
1. Open **Window > Developer Tools > Output Log**
2. At the bottom, enter: `py "Scripts/create_phase2_assets.py"`
3. Press Enter

**Method B: Command Line (outside editor)**
```
"C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "H:\Unreal Projects\ProjectWalkingSim\ProjectWalkingSim.uproject" -ExecutePythonScript="H:\Unreal Projects\ProjectWalkingSim\Scripts\create_phase2_assets.py" -nullrhi -unattended
```

### What the Script Creates/Modifies

- **IA_ToggleInventory** - Input action for Tab key to toggle inventory
- **IMC_Default** updates:
  - Tab -> IA_ToggleInventory (Pressed trigger)
  - IA_Interact rebound from E to F (IA_LeanRight E-hold unchanged)

### Verify Script Results

1. Navigate to `Content/Input/Actions/`
2. Confirm `IA_ToggleInventory` exists
3. Open `Content/Input/Mappings/IMC_Default`
4. Verify:
   - Tab is bound to IA_ToggleInventory
   - F is bound to IA_Interact (was E)
   - E is bound to IA_LeanRight with Hold trigger (unchanged)

---

## Part 2: Create UMG Blueprint Widgets

### WBP_InventorySlot

This widget displays a single inventory slot with item icon and quantity.

1. Content Browser: Right-click in `Content/UI/Inventory/` (create folder if needed)
2. **User Interface > Widget Blueprint**
3. In "Pick Parent Class", search for `InventorySlotWidget` and select it
4. Name it `WBP_InventorySlot`
5. Open in UMG Designer and create this hierarchy:

```
[Canvas Panel] (root)
  +-- [Image] "SlotBackground"
  |     - Anchors: Stretch (fill entire slot)
  |     - Size: Fill parent
  |     - Brush: Solid color, dark gray (#1A1A1AFF)
  |     - Render Opacity: 0.8
  |
  +-- [Image] "ItemIcon"
  |     - Anchors: Center
  |     - Size: 64x64 (or 56x56 for padding)
  |     - Alignment: 0.5, 0.5
  |     - Visibility: Hidden by default (shown when item present)
  |
  +-- [TextBlock] "QuantityText"
  |     - Anchors: Bottom-Right
  |     - Position: -4, -4 offset from anchor
  |     - Font Size: 12
  |     - Color: White
  |     - Visibility: Hidden by default (shown when quantity > 1)
  |
  +-- [Image] "SelectionHighlight" (optional but recommended)
        - Anchors: Stretch
        - Size: Fill parent
        - Brush: Solid color, yellow (#FFCC0044) or light blue
        - Visibility: Hidden by default (shown when slot selected)
```

**Size Hint**: Set Canvas Panel size to 80x80 pixels for consistent slot sizing.

**CRITICAL**: Widget names MUST exactly match the C++ BindWidget names: `SlotBackground`, `ItemIcon`, `QuantityText`, `SelectionHighlight`.

---

### WBP_ItemTooltip

This widget displays item details and action buttons when a slot is selected.

1. Content Browser: Right-click in `Content/UI/Inventory/`
2. **User Interface > Widget Blueprint**
3. In "Pick Parent Class", search for `ItemTooltipWidget` and select it
4. Name it `WBP_ItemTooltip`
5. Open in UMG Designer and create this hierarchy:

```
[Canvas Panel or Overlay] (root)
  +-- [Border] (optional, for background styling)
  |     - Brush: Solid color, dark (#0D0D0DCC)
  |     - Padding: 8
  |
  +-- [Vertical Box]
        +-- [TextBlock] "ItemNameText"
        |     - Font Size: 16
        |     - Font Style: Bold
        |     - Color: White
        |     - Margin Bottom: 4
        |
        +-- [TextBlock] "ItemDescriptionText"
        |     - Font Size: 12
        |     - Color: Light gray (#CCCCCCFF)
        |     - Auto Wrap Text: True
        |     - Margin Bottom: 8
        |
        +-- [Horizontal Box]
              +-- [Button] "UseButton"
              |     - Child: TextBlock with text "Use"
              |     - Style: Dark button
              |     - Padding: 8, 4
              |
              +-- [Spacer] (8px width)
              |
              +-- [Button] "CombineButton"
              |     - Child: TextBlock with text "Combine"
              |     - Style: Dark button
              |     - Padding: 8, 4
              |
              +-- [Spacer] (8px width)
              |
              +-- [Button] "DiscardButton"
                    - Child: TextBlock with text "Discard"
                    - Style: Dark button, red tint for warning
                    - Padding: 8, 4
```

**CRITICAL**: Widget names MUST exactly match: `ItemNameText`, `ItemDescriptionText`, `UseButton`, `CombineButton`, `DiscardButton`.

---

### WBP_InventoryWidget

This is the main inventory container that holds slots and tooltip.

1. Content Browser: Right-click in `Content/UI/Inventory/`
2. **User Interface > Widget Blueprint**
3. In "Pick Parent Class", search for `InventoryWidget` and select it
4. Name it `WBP_InventoryWidget`
5. Open in UMG Designer and create this hierarchy:

```
[Canvas Panel] (root)
  +-- [Vertical Box] (centered at bottom of screen)
  |     - Anchors: Bottom-Center
  |     - Alignment: 0.5, 1.0
  |     - Position: Y offset -20 (above screen edge)
  |
  |     +-- [Widget] "TooltipWidget" (instance of WBP_ItemTooltip)
  |     |     - Visibility: Hidden (shown when item selected)
  |     |     - Margin Bottom: 8
  |     |
  |     +-- [HorizontalBox] "SlotContainer"
  |           - (Slots are added dynamically by C++ code)
  |           - (C++ creates 8 WBP_InventorySlot instances here)
```

**Class Defaults Configuration**:
1. Click on the root widget or go to Class Defaults
2. Find **SlotWidgetClass** property under "Inventory"
3. Set it to `WBP_InventorySlot`

**CRITICAL**: Widget names MUST exactly match: `SlotContainer`, `TooltipWidget`.

---

## Part 3: Update WBP_SereneHUD

Add the inventory widget to the existing HUD.

1. Open `Content/Blueprints/UI/WBP_SereneHUD` (or wherever your HUD is located)
2. In the UMG Designer, add `WBP_InventoryWidget` as a child
3. Name it exactly `InventoryWidget`
4. Configure placement:
   - Anchors: Full screen stretch (or center-bottom if preferred)
   - The inventory widget manages its own positioning internally
5. Set initial Visibility to **Hidden**

**Class Defaults** (if not already set):
1. Go to Class Defaults
2. Find **InventoryWidgetClass** property
3. Set it to `WBP_InventoryWidget`

---

## Part 4: Create Demo Item Data Assets

Create the Content/Data/Items/ folder structure and item data assets.

### Create Folder

1. Content Browser: Navigate to `Content/`
2. Right-click > **New Folder** > Name it `Data`
3. Inside Data, create another folder: `Items`
4. Final path: `Content/Data/Items/`

### Create Item Data Assets

For each item, right-click in `Content/Data/Items/` > **Miscellaneous > Data Asset** > Select `ItemDataAsset` as the class.

**DA_Key_FrontDoor**
- ItemId: `Key_FrontDoor`
- DisplayName: `Front Door Key`
- Description: `A rusty iron key. It looks like it fits an old lock.`
- bIsKeyItem: `True`
- bCanStack: `False`
- ItemType: `KeyItem`
- ItemTag: `Item.Key`
- Icon: (leave empty for now, or assign a placeholder texture)
- WorldMesh: (leave empty for now)

**DA_Key_Basement**
- ItemId: `Key_Basement`
- DisplayName: `Basement Key`
- Description: `A small brass key with a worn handle.`
- bIsKeyItem: `True`
- bCanStack: `False`
- ItemType: `KeyItem`
- ItemTag: `Item.Key`
- Icon: (leave empty)
- WorldMesh: (leave empty)

**DA_Code_Safe**
- ItemId: `Code_Safe`
- DisplayName: `Safe Code`
- Description: `A scrap of paper with the numbers 7-3-4-9 scrawled on it.`
- bIsKeyItem: `True`
- bCanStack: `False`
- ItemType: `KeyItem`
- ItemTag: `Item.Key`
- Icon: (leave empty)
- WorldMesh: (leave empty)

---

## Part 5: Configure Blueprints

### BP_SerenePlayerController

1. Open `Content/Blueprints/Game/BP_SerenePlayerController`
2. In the Details panel, find the **Inventory** category
3. Set **ToggleInventoryAction** to `IA_ToggleInventory`
4. Verify **InteractAction** is still set to `IA_Interact` (unchanged)

### Verify Existing Configuration

These should already be set from Phase 1:
- DefaultMappingContext = IMC_Default
- MoveAction = IA_Move
- LookAction = IA_Look
- SprintAction = IA_Sprint
- CrouchAction = IA_Crouch
- LeanLeftAction = IA_LeanLeft
- LeanRightAction = IA_LeanRight

---

## Part 6: Test Map Setup

### Place Pickup Actors

1. Open `Content/Maps/TestMap`
2. In Place Actors panel, search for `PickupActor`
3. Drag 3-4 PickupActor instances into the level
4. For each PickupActor, set in Details:

**Pickup 1**:
- ItemId: `Key_FrontDoor`
- Quantity: `1`
- Assign a placeholder cube mesh to MeshComponent

**Pickup 2**:
- ItemId: `Key_Basement`
- Quantity: `1`
- Assign a placeholder cube mesh

**Pickup 3**:
- ItemId: `Code_Safe`
- Quantity: `1`
- Assign a placeholder plane mesh (for paper)

### Place a Locked Door

1. Find an existing DoorActor in the level (or place a new one)
2. In Details panel, configure:
   - **bIsLocked**: `True`
   - **RequiredItemId**: `Key_FrontDoor`
   - Assign door mesh to MeshComponent

### Verify Existing Interactables

Ensure your test level has:
- A PlayerStart actor
- At least one unlocked DoorActor (for comparison)
- Good lighting for visibility

---

## Troubleshooting

### Widget BindWidget Errors

**Symptom**: Editor crashes or logs error about BindWidget property not found.

**Cause**: Widget names in UMG don't match C++ UPROPERTY names.

**Fix**: Verify exact name matches:
- `SlotBackground`, `ItemIcon`, `QuantityText`, `SelectionHighlight` (InventorySlot)
- `ItemNameText`, `ItemDescriptionText`, `UseButton`, `CombineButton`, `DiscardButton` (ItemTooltip)
- `SlotContainer`, `TooltipWidget` (InventoryWidget)
- `InventoryWidget` (SereneHUD)

### Asset Manager Not Finding Items

**Symptom**: Items don't load, "Failed to load item" warnings in log.

**Cause**: Asset Manager not configured or items in wrong folder.

**Fix**:
1. Verify `Config/DefaultEngine.ini` has the `[/Script/Engine.AssetManagerSettings]` section
2. Ensure data assets are in `Content/Data/Items/` exactly
3. Restart editor after config changes

### Inventory Doesn't Open

**Symptom**: Pressing Tab does nothing.

**Cause**: IA_ToggleInventory not assigned or input not routed.

**Fix**:
1. Verify IA_ToggleInventory exists in Content/Input/Actions/
2. Open BP_SerenePlayerController and set ToggleInventoryAction = IA_ToggleInventory
3. Verify IMC_Default has Tab bound to IA_ToggleInventory

### Interact Key Changed to F

**Expected behavior**: After running create_phase2_assets.py, interact is bound to F key instead of E.

This frees up E-press for future use while keeping E-hold for lean right.

---

## Verification Checklist (PIE)

After completing all setup steps, run Play In Editor and verify:

**Inventory Toggle**
- [ ] Press Tab: inventory UI appears at bottom of screen, mouse cursor shows
- [ ] Press Tab again: inventory closes, cursor hidden, camera rotates normally
- [ ] WASD movement works while inventory is open

**Item Pickup**
- [ ] Look at PickupActor: "F: Pick up [item name]" prompt appears
- [ ] Press F: item added to inventory, pickup disappears
- [ ] Item icon appears in leftmost empty slot

**Inventory Interaction**
- [ ] Click on slot: item selected, tooltip appears with name/description
- [ ] Click Discard: item drops in front of player
- [ ] Pick up dropped item: returns to inventory

**Keyboard Navigation**
- [ ] Number keys 1-8 select corresponding slot
- [ ] Arrow left/right cycle through slots
- [ ] Escape deselects current slot
- [ ] Delete discards selected item

**Locked Door**
- [ ] Look at locked door: "F: Locked" text appears
- [ ] Pick up front door key
- [ ] Look at locked door again: prompt shows key available
- [ ] Press F: door unlocks and opens, key consumed from inventory

**Full Inventory**
- [ ] Fill all 8 slots with items
- [ ] Try to pick up another item: "Inventory Full" prompt appears
- [ ] Item remains in world (not picked up)

---

## Next Steps

With the inventory system complete, you can:

1. Create more item data assets for your game
2. Add item icons (2D textures) to data assets
3. Implement combine recipes in UInventoryComponent::InitCombineRecipes()
4. Create world meshes for items that drop back into the world
5. Add more locked doors with different key requirements

The C++ code supports:
- Stackable items (set bCanStack=true, MaxStackSize in data asset)
- Item combining (add recipes to the TMap in InitCombineRecipes)
- Use actions (implement in game-specific code)
- Item tags for categorization (GameplayTags)
