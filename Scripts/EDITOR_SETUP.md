# Phase 1 Editor Setup Guide

After running the Python asset creation script, complete these manual steps in the Unreal Editor.

The Input Action assets and IMC_Default have been created automatically with full key bindings. The remaining setup requires the Unreal Editor UI because Blueprint subclasses, UMG widget layouts, and TestMap configuration cannot be created programmatically for custom C++ parent classes.

---

## 1. Blueprint Subclasses

### BP_SereneCharacter

1. Content Browser: Right-click in `Content/Blueprints/Game/`
2. **Blueprint Class** > **All Classes** > Search "SereneCharacter"
3. Select `SereneCharacter` as parent class
4. Name it `BP_SereneCharacter`
5. Open it and configure:
   - **Mesh component**: Assign UE5 Mannequin skeletal mesh (SK_Mannequin or Manny/Quinn)
   - **WorldRepresentationMesh**: Assign the same skeletal mesh
   - **Verify camera**: Should be attached to the `head` bone (set in C++ constructor)

### BP_SerenePlayerController

1. Content Browser: Right-click in `Content/Blueprints/Game/`
2. **Blueprint Class** > **All Classes** > Search "SerenePlayerController"
3. Select `SerenePlayerController` as parent class
4. Name it `BP_SerenePlayerController`
5. Open it and set these properties in the **Details** panel under **Input**:
   - `DefaultMappingContext` = `IMC_Default` (from Content/Input/Mappings/)
   - `MoveAction` = `IA_Move`
   - `LookAction` = `IA_Look`
   - `SprintAction` = `IA_Sprint`
   - `CrouchAction` = `IA_Crouch`
   - `InteractAction` = `IA_Interact`
   - `LeanLeftAction` = `IA_LeanLeft`
   - `LeanRightAction` = `IA_LeanRight`

### BP_SereneHUD

1. Content Browser: Right-click in `Content/Blueprints/Game/`
2. **Blueprint Class** > **All Classes** > Search "SereneHUD"
3. Select `SereneHUD` as parent class
4. Name it `BP_SereneHUD`
5. Open it and set these properties under **HUD**:
   - `StaminaBarWidgetClass` = `WBP_StaminaBar` (create this widget first, see below)
   - `InteractionPromptWidgetClass` = `WBP_InteractionPrompt` (create this widget first)

### BP_SereneGameMode

1. Content Browser: Right-click in `Content/Blueprints/Game/`
2. **Blueprint Class** > **All Classes** > Search "SereneGameMode"
3. Select `SereneGameMode` as parent class
4. Name it `BP_SereneGameMode`
5. Open it and set these properties under **Classes**:
   - `Default Pawn Class` = `BP_SereneCharacter`
   - `Player Controller Class` = `BP_SerenePlayerController`
   - `HUD Class` = `BP_SereneHUD`

---

## 2. UMG Widget Blueprints

### WBP_StaminaBar

1. Content Browser: Right-click in `Content/Blueprints/UI/`
2. **User Interface** > **Widget Blueprint**
3. In the "Pick Parent Class" dialog, search for `StaminaBarWidget` and select it
4. Name it `WBP_StaminaBar`
5. Open in UMG Designer:
   - Add a **ProgressBar** widget, name it exactly `StaminaBar` (MUST match C++ BindWidget name)
   - Style the progress bar: dark background, red/orange fill color
   - Position it at bottom-center or bottom-left of the screen
   - Set initial visibility to Hidden (the C++ code controls show/hide)
   - **Optional**: Add a widget animation named `FadeAnimation` for smooth show/hide transitions

### WBP_InteractionPrompt

1. Content Browser: Right-click in `Content/Blueprints/UI/`
2. **User Interface** > **Widget Blueprint**
3. In the "Pick Parent Class" dialog, search for `InteractionPromptWidget` and select it
4. Name it `WBP_InteractionPrompt`
5. Open in UMG Designer:
   - Add a **TextBlock** widget, name it exactly `PromptText` (MUST match C++ BindWidget name)
   - Add a **TextBlock** widget, name it exactly `ReticleText` (MUST match C++ BindWidget name)
   - Style: white text with slight drop shadow, centered bottom third of screen
   - `PromptText` displays "E: Open" (or similar) -- set initial visibility to Hidden
   - `ReticleText` displays a small reticle dot "." -- set initial visibility to Hidden

**CRITICAL**: The widget names MUST exactly match the C++ UPROPERTY names. `StaminaBar`, `PromptText`, and `ReticleText` are enforced by BindWidget meta.

---

## 3. TestMap Setup

1. Open `Content/Maps/TestMap.umap`
2. **World Settings** (Window > World Settings):
   - Set `GameMode Override` = `BP_SereneGameMode`
3. Place test geometry:
   - A large floor plane with default material
   - At least one **Directional Light** for shadow testing
4. Place a **PlayerStart** actor
5. Place interactable test actors:
   - 2-3 `DoorActor` instances (assign cube/plane placeholder meshes in Details)
   - 2-3 `PickupActor` instances (assign small cube meshes)
   - 1 `ReadableActor` (assign plane mesh)
   - 1 `DrawerActor` (assign box meshes for frame + drawer components)

---

## Verification (PIE)

Press Play in TestMap and verify:

- [ ] Player spawns in first person, visible body looking down
- [ ] WASD moves, mouse looks, pitch clamped
- [ ] Shift-hold: sprint, stamina bar appears and depletes
- [ ] Ctrl: toggle crouch
- [ ] Look at DoorActor within range: "E: Open" prompt
- [ ] Press E on door: opens/closes
- [ ] Hold Q: lean left. Hold E: lean right
- [ ] Walk: subtle head-bob. Sprint: more bob. Stand still: no bob
- [ ] Player shadow visible on floor
- [ ] Output Log shows footstep surface types
