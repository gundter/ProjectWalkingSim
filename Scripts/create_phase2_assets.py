"""
Phase 2 Asset Creation Script for The Juniper Tree.

Run via: UnrealEditor-Cmd.exe <project> -ExecutePythonScript=<this_script> -nullrhi -unattended

Creates/modifies programmatically:
  - 1 Input Action asset (IA_ToggleInventory)
  - Updates IMC_Default with Tab -> IA_ToggleInventory binding
  - Rebinds IA_Interact from E to F key (keeps IA_LeanRight E-hold unchanged)

Manual steps required after this script (see EDITOR_SETUP_PHASE2.md):
  - UMG Widget Blueprints (WBP_InventoryWidget, WBP_InventorySlot, WBP_ItemTooltip)
  - Update WBP_SereneHUD with inventory widget
  - Item Data Assets (DA_Key_FrontDoor, etc.)
  - Blueprint configuration (ToggleInventoryAction assignment)
  - Test map setup with pickups and locked door
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
# 1. Create IA_ToggleInventory Input Action
# ===================================================================

def create_toggle_inventory_action():
    """Create IA_ToggleInventory input action under /Game/Input/Actions/."""

    path = "/Game/Input/Actions"
    ensure_directory(path)
    asset_path = f"{path}/IA_ToggleInventory"

    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log_warning("[SKIP] IA_ToggleInventory already exists")
        return unreal.EditorAssetLibrary.load_asset(asset_path)

    at = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.InputAction_Factory()

    ia = at.create_asset("IA_ToggleInventory", path, unreal.InputAction, factory)
    if ia is None:
        unreal.log_error("[FAIL] Could not create IA_ToggleInventory")
        return None

    # Boolean value type for toggle action
    ia.set_editor_property("value_type", unreal.InputActionValueType.BOOLEAN)

    # Add Pressed trigger so it fires once on key press
    trigger = unreal.InputTriggerPressed()
    ia.set_editor_property("triggers", [trigger])

    save(asset_path)
    unreal.log_warning("[OK] Created IA_ToggleInventory (type=BOOLEAN, trigger=Pressed)")

    return ia

# ===================================================================
# 2. Update IMC_Default with ToggleInventory and rebind Interact
# ===================================================================

def update_imc_default(toggle_inventory_action):
    """
    Update IMC_Default:
    1. Add Tab -> IA_ToggleInventory binding
    2. Rebind IA_Interact from E to F (keep IA_LeanRight E-hold unchanged)
    """

    imc_path = "/Game/Input/Mappings/IMC_Default"

    if not unreal.EditorAssetLibrary.does_asset_exist(imc_path):
        unreal.log_error("[FAIL] IMC_Default does not exist - run Phase 1 script first")
        return None

    imc = unreal.EditorAssetLibrary.load_asset(imc_path)
    if imc is None:
        unreal.log_error("[FAIL] Could not load IMC_Default")
        return None

    # Load IA_Interact for rebinding
    ia_interact_path = "/Game/Input/Actions/IA_Interact"
    ia_interact = None
    if unreal.EditorAssetLibrary.does_asset_exist(ia_interact_path):
        ia_interact = unreal.EditorAssetLibrary.load_asset(ia_interact_path)

    # --- Step 1: Add IA_ToggleInventory -> Tab binding ---
    if toggle_inventory_action:
        m_toggle = imc.map_key(toggle_inventory_action, make_key("Tab"))
        # Add Pressed trigger for single-fire on press
        pressed = unreal.InputTriggerPressed()
        m_toggle.set_editor_property("triggers", [pressed])
        unreal.log_warning("[OK] Added IA_ToggleInventory: Tab (Pressed)")

    # --- Step 2: Rebind IA_Interact from E to F ---
    # The IMC has mappings as a collection. We need to find the E-press mapping
    # for IA_Interact and change its key to F.
    # IA_LeanRight also uses E but with Hold trigger - that one stays unchanged.

    if ia_interact:
        # Get the mappings collection
        dkm = imc.get_editor_property("default_key_mappings")
        mappings = dkm.get_editor_property("mappings")

        e_key = make_key("E")
        f_key = make_key("F")

        # Find and modify the IA_Interact E-key mapping
        found_interact_e = False
        for i, mapping in enumerate(mappings):
            action = mapping.get_editor_property("action")
            key = mapping.get_editor_property("key")

            # Check if this is IA_Interact with E key
            if action and action.get_name() == "IA_Interact":
                # Check the key name
                key_name = str(key.key_name) if hasattr(key, 'key_name') else str(key)
                if "E" in key_name and "E" == key_name.strip():
                    # This is the IA_Interact E binding - change to F
                    mapping.set_editor_property("key", f_key)
                    found_interact_e = True
                    unreal.log_warning(f"[OK] Rebound IA_Interact from E to F (mapping index {i})")
                    break

        if not found_interact_e:
            # Fallback: unmap E and map F fresh
            # First try to unmap the E binding
            unreal.log_warning("[INFO] Could not find IA_Interact E mapping directly, attempting unmap/remap")
            try:
                imc.unmap_key(ia_interact, e_key)
                unreal.log_warning("[OK] Unmapped IA_Interact from E")
            except Exception as ex:
                unreal.log_warning(f"[WARN] unmap_key failed: {ex}")

            # Add F binding with Pressed trigger
            m_interact_f = imc.map_key(ia_interact, f_key)
            pressed = unreal.InputTriggerPressed()
            m_interact_f.set_editor_property("triggers", [pressed])
            unreal.log_warning("[OK] Added IA_Interact: F (Pressed)")

    save(imc_path)

    # Verify final mapping count
    dkm = imc.get_editor_property("default_key_mappings")
    count = len(dkm.get_editor_property("mappings"))
    unreal.log_warning(f"[OK] IMC_Default updated, now has {count} mappings")

    return imc

# ===================================================================
# Main
# ===================================================================

def main():
    unreal.log_warning("=" * 60)
    unreal.log_warning("Phase 2 Asset Creation - The Juniper Tree")
    unreal.log_warning("=" * 60)

    # Create IA_ToggleInventory
    toggle_action = create_toggle_inventory_action()

    # Update IMC_Default
    update_imc_default(toggle_action)

    unreal.log_warning("=" * 60)
    unreal.log_warning("PHASE 2 AUTOMATED ASSET CREATION COMPLETE")
    unreal.log_warning("=" * 60)
    unreal.log_warning("")
    unreal.log_warning("Created: IA_ToggleInventory")
    unreal.log_warning("Updated: IMC_Default (Tab binding, E->F rebind)")
    unreal.log_warning("")
    unreal.log_warning("MANUAL STEPS REQUIRED:")
    unreal.log_warning("See Scripts/EDITOR_SETUP_PHASE2.md for UMG, Data Assets, and TestMap setup")

main()
