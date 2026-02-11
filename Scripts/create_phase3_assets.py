"""
Phase 3 Asset Creation Script for The Juniper Tree.

Run via: UnrealEditor-Cmd.exe <project> -ExecutePythonScript=<this_script> -nullrhi -unattended

Creates programmatically:
  - 1 Input Mapping Context (IMC_Hiding) with F -> IA_Interact and Mouse2D -> IA_Look

Manual steps required after this script (see EDITOR_SETUP_PHASE3.md):
  - 3 HidingSpotDataAsset instances (DA_HidingSpot_Locker, Closet, UnderBed)
  - Blueprint configuration (HidingComponent input references)
  - Test map setup with HidingSpotActor
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
# 1. Create IMC_Hiding Input Mapping Context
# ===================================================================

def create_imc_hiding():
    """
    Create IMC_Hiding with two mappings:
    1. F key -> IA_Interact (reuses existing interact action; controller routes to ExitHidingSpot when hiding)
    2. Mouse2D -> IA_Look (existing action for look input)
    """

    path = "/Game/Input/Mappings"
    ensure_directory(path)
    asset_path = f"{path}/IMC_Hiding"

    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log_warning("[SKIP] IMC_Hiding already exists")
        return unreal.EditorAssetLibrary.load_asset(asset_path)

    at = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.InputMappingContext_Factory()

    imc = at.create_asset("IMC_Hiding", path, unreal.InputMappingContext, factory)
    if imc is None:
        unreal.log_error("[FAIL] Could not create IMC_Hiding")
        return None

    # --- Mapping 1: F key -> IA_Interact (reuse existing) ---
    ia_interact_path = "/Game/Input/Actions/IA_Interact"
    if unreal.EditorAssetLibrary.does_asset_exist(ia_interact_path):
        ia_interact = unreal.EditorAssetLibrary.load_asset(ia_interact_path)
        if ia_interact:
            m_interact = imc.map_key(ia_interact, make_key("F"))
            pressed = unreal.InputTriggerPressed()
            m_interact.set_editor_property("triggers", [pressed])
            unreal.log_warning("[OK] IMC_Hiding: F -> IA_Interact (Pressed)")
    else:
        unreal.log_warning("[WARN] IA_Interact not found - Interact mapping not added to IMC_Hiding")
        unreal.log_warning("[WARN] Add Interact mapping manually after running Phase 1 script")

    # --- Mapping 2: Mouse2D -> IA_Look (reuse existing) ---
    ia_look_path = "/Game/Input/Actions/IA_Look"
    if unreal.EditorAssetLibrary.does_asset_exist(ia_look_path):
        ia_look = unreal.EditorAssetLibrary.load_asset(ia_look_path)
        if ia_look:
            imc.map_key(ia_look, make_key("Mouse2D"))
            unreal.log_warning("[OK] IMC_Hiding: Mouse2D -> IA_Look")
    else:
        unreal.log_warning("[WARN] IA_Look not found - Look mapping not added to IMC_Hiding")
        unreal.log_warning("[WARN] Add Look mapping manually after running Phase 1 script")

    save(asset_path)

    # Verify mapping count
    dkm = imc.get_editor_property("default_key_mappings")
    count = len(dkm.get_editor_property("mappings"))
    unreal.log_warning(f"[OK] IMC_Hiding created with {count} mappings")

    return imc

# ===================================================================
# Main
# ===================================================================

def main():
    unreal.log_warning("=" * 60)
    unreal.log_warning("Phase 3 Asset Creation - The Juniper Tree")
    unreal.log_warning("=" * 60)

    # Create IMC_Hiding (maps F -> IA_Interact, Mouse2D -> IA_Look)
    imc = create_imc_hiding()

    unreal.log_warning("=" * 60)
    unreal.log_warning("PHASE 3 AUTOMATED ASSET CREATION COMPLETE")
    unreal.log_warning("=" * 60)
    unreal.log_warning("")
    unreal.log_warning("Created: IMC_Hiding (F -> IA_Interact, Mouse2D -> IA_Look)")
    unreal.log_warning("")
    unreal.log_warning("MANUAL STEPS REQUIRED:")
    unreal.log_warning("See Scripts/EDITOR_SETUP_PHASE3.md for:")
    unreal.log_warning("  - 3 HidingSpotDataAsset instances")
    unreal.log_warning("  - BP_SereneCharacter HidingComponent configuration")
    unreal.log_warning("  - TestMap hiding spot placement")

main()
