"""
Phase 1 Asset Creation Script for The Juniper Tree.

Run via: UnrealEditor-Cmd.exe <project> -ExecutePythonScript=<this_script> -nullrhi -unattended

Creates programmatically:
  - 7 Input Action assets (IA_Move, IA_Look, IA_Sprint, IA_Crouch, IA_Interact, IA_LeanLeft, IA_LeanRight)
  - 1 Input Mapping Context (IMC_Default) with all key bindings configured

Manual steps required after this script (see EDITOR_SETUP.md):
  - Blueprint subclasses (BP_SereneCharacter, BP_SerenePlayerController, BP_SereneGameMode, BP_SereneHUD)
  - UMG Widget Blueprints (WBP_StaminaBar, WBP_InteractionPrompt)
  - TestMap configuration

The custom C++ classes (ASereneCharacter, ASerenePlayerController, etc.) are not
accessible from the Python scripting environment in commandlet mode, so Blueprint
subclasses must be created manually in the Unreal Editor.
"""

import unreal

# ===================================================================
# Helper
# ===================================================================

def ensure_directory(path):
    """Create content directory if it does not exist."""
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)

def save(asset_path):
    """Save an asset to disk."""
    unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False)

def make_key(name):
    """Create an FKey from a key name string."""
    k = unreal.Key()
    k.import_text(name)
    return k

# ===================================================================
# 1. Input Action Assets
# ===================================================================

def create_input_actions():
    """Create 7 Input Action assets under /Game/Input/Actions/."""

    path = "/Game/Input/Actions"
    ensure_directory(path)

    at = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.InputAction_Factory()

    specs = [
        ("IA_Move",      unreal.InputActionValueType.AXIS2D),
        ("IA_Look",      unreal.InputActionValueType.AXIS2D),
        ("IA_Sprint",    unreal.InputActionValueType.BOOLEAN),
        ("IA_Crouch",    unreal.InputActionValueType.BOOLEAN),
        ("IA_Interact",  unreal.InputActionValueType.BOOLEAN),
        ("IA_LeanLeft",  unreal.InputActionValueType.BOOLEAN),
        ("IA_LeanRight", unreal.InputActionValueType.BOOLEAN),
    ]

    actions = {}
    for name, value_type in specs:
        asset_path = f"{path}/{name}"
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            unreal.log_warning(f"[SKIP] {name} already exists")
            actions[name] = unreal.EditorAssetLibrary.load_asset(asset_path)
            continue

        ia = at.create_asset(name, path, unreal.InputAction, factory)
        if ia is None:
            unreal.log_error(f"[FAIL] Could not create {name}")
            continue

        ia.set_editor_property("value_type", value_type)

        # IA_Interact: add Pressed trigger so it fires on initial press only
        if name == "IA_Interact":
            trigger = unreal.InputTriggerPressed()
            ia.set_editor_property("triggers", [trigger])

        save(asset_path)
        actions[name] = ia
        unreal.log_warning(f"[OK] Created {name} (type={value_type})")

    return actions

# ===================================================================
# 2. Input Mapping Context
# ===================================================================

def create_imc(actions):
    """Create IMC_Default with all key bindings under /Game/Input/Mappings/."""

    path = "/Game/Input/Mappings"
    ensure_directory(path)
    asset_path = f"{path}/IMC_Default"

    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log_warning("[SKIP] IMC_Default already exists")
        return unreal.EditorAssetLibrary.load_asset(asset_path)

    at = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.InputMappingContext_Factory()
    imc = at.create_asset("IMC_Default", path, unreal.InputMappingContext, factory)

    if imc is None:
        unreal.log_error("[FAIL] Could not create IMC_Default")
        return None

    # --- IA_Move: WASD (Axis2D) ---
    ia_move = actions.get("IA_Move")
    if ia_move:
        # W -> +Y (forward): Swizzle YXZ maps 1D input to Y axis of 2D
        m_w = imc.map_key(ia_move, make_key("W"))
        swizzle_w = unreal.InputModifierSwizzleAxis()
        swizzle_w.set_editor_property("order", unreal.InputAxisSwizzle.YXZ)
        m_w.set_editor_property("modifiers", [swizzle_w])

        # S -> -Y (backward): Negate + Swizzle YXZ
        m_s = imc.map_key(ia_move, make_key("S"))
        negate_s = unreal.InputModifierNegate()
        swizzle_s = unreal.InputModifierSwizzleAxis()
        swizzle_s.set_editor_property("order", unreal.InputAxisSwizzle.YXZ)
        m_s.set_editor_property("modifiers", [negate_s, swizzle_s])

        # D -> +X (right): No modifiers needed (default maps to X)
        imc.map_key(ia_move, make_key("D"))

        # A -> -X (left): Negate
        m_a = imc.map_key(ia_move, make_key("A"))
        negate_a = unreal.InputModifierNegate()
        m_a.set_editor_property("modifiers", [negate_a])

        unreal.log_warning("[OK] IA_Move: W(+Y), S(-Y), D(+X), A(-X)")

    # --- IA_Look: Mouse XY ---
    ia_look = actions.get("IA_Look")
    if ia_look:
        imc.map_key(ia_look, make_key("Mouse2D"))
        unreal.log_warning("[OK] IA_Look: Mouse2D")

    # --- IA_Sprint: Left Shift ---
    ia_sprint = actions.get("IA_Sprint")
    if ia_sprint:
        imc.map_key(ia_sprint, make_key("LeftShift"))
        unreal.log_warning("[OK] IA_Sprint: LeftShift")

    # --- IA_Crouch: Left Ctrl ---
    ia_crouch = actions.get("IA_Crouch")
    if ia_crouch:
        imc.map_key(ia_crouch, make_key("LeftControl"))
        unreal.log_warning("[OK] IA_Crouch: LeftControl")

    # --- IA_Interact: E key (Pressed trigger) ---
    ia_interact = actions.get("IA_Interact")
    if ia_interact:
        m_interact = imc.map_key(ia_interact, make_key("E"))
        pressed = unreal.InputTriggerPressed()
        m_interact.set_editor_property("triggers", [pressed])
        unreal.log_warning("[OK] IA_Interact: E (Pressed)")

    # --- IA_LeanLeft: Q key ---
    ia_lean_left = actions.get("IA_LeanLeft")
    if ia_lean_left:
        imc.map_key(ia_lean_left, make_key("Q"))
        unreal.log_warning("[OK] IA_LeanLeft: Q")

    # --- IA_LeanRight: E key (Hold trigger, 0.2s) ---
    ia_lean_right = actions.get("IA_LeanRight")
    if ia_lean_right:
        m_lean_right = imc.map_key(ia_lean_right, make_key("E"))
        hold = unreal.InputTriggerHold()
        hold.set_editor_property("hold_time_threshold", 0.2)
        hold.set_editor_property("is_one_shot", False)
        m_lean_right.set_editor_property("triggers", [hold])
        unreal.log_warning("[OK] IA_LeanRight: E (Hold 0.2s)")

    save(asset_path)

    # Verify mapping count
    dkm = imc.get_editor_property("default_key_mappings")
    count = len(dkm.get_editor_property("mappings"))
    unreal.log_warning(f"[OK] IMC_Default created with {count} mappings")

    return imc

# ===================================================================
# Main
# ===================================================================

def main():
    unreal.log_warning("=" * 60)
    unreal.log_warning("Phase 1 Asset Creation - The Juniper Tree")
    unreal.log_warning("=" * 60)

    actions = create_input_actions()
    imc = create_imc(actions)

    unreal.log_warning("=" * 60)
    unreal.log_warning("AUTOMATED ASSET CREATION COMPLETE")
    unreal.log_warning("=" * 60)
    unreal.log_warning("")
    unreal.log_warning("Created: 7 Input Actions + 1 IMC (fully configured)")
    unreal.log_warning("")
    unreal.log_warning("MANUAL STEPS REQUIRED:")
    unreal.log_warning("See Scripts/EDITOR_SETUP.md for Blueprint, Widget, and TestMap setup")

main()
