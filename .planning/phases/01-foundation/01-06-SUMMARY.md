---
phase: 01-foundation
plan: 06
subsystem: input-assets-integration-testing
tags: [enhanced-input, input-action, input-mapping-context, blueprint, umg-widget, testmap, python-automation]

dependency-graph:
  requires:
    - "01-01: Interfaces, GameInstance, build config"
    - "01-02: ASereneCharacter (camera, mesh, CMC), ASerenePlayerController (input action properties)"
    - "01-03: StaminaComponent, HeadBobComponent, LeanComponent"
    - "01-04: InteractionComponent, InteractionPromptWidget, interactable actors"
    - "01-05: FootstepComponent, StaminaBarWidget, SereneHUD"
  provides:
    - "7 Input Action data assets with correct value types and triggers"
    - "IMC_Default with 10 key mappings (WASD+Mouse+Shift+Ctrl+E+Q+E-hold)"
    - "EnhancedInput and PythonScriptPlugin plugins enabled in .uproject"
    - "Python automation script for reproducible asset creation"
    - "Editor setup guide for Blueprint subclasses, UMG widgets, and TestMap"
  affects:
    - "Phase 2 (inventory may add new Input Actions to IMC_Default)"
    - "Phase 3 (hiding may add crouch-to-hide input)"
    - "Phase 6 (flashlight toggle will add new Input Action)"

tech-stack:
  added:
    - "EnhancedInput plugin (explicit in .uproject)"
    - "PythonScriptPlugin (editor-only, for asset automation)"
  patterns:
    - "Python editor scripting via UnrealEditor-Cmd -ExecutePythonScript for data asset creation"
    - "FKey.import_text() for constructing key references in Python"
    - "IMC.map_key(action, key) returns EnhancedActionKeyMapping for modifier/trigger configuration"
    - "E key dual-use: Pressed trigger (interact) vs Hold trigger 0.2s (lean right)"

key-files:
  created:
    - "Content/Input/Actions/IA_Move.uasset"
    - "Content/Input/Actions/IA_Look.uasset"
    - "Content/Input/Actions/IA_Sprint.uasset"
    - "Content/Input/Actions/IA_Crouch.uasset"
    - "Content/Input/Actions/IA_Interact.uasset"
    - "Content/Input/Actions/IA_LeanLeft.uasset"
    - "Content/Input/Actions/IA_LeanRight.uasset"
    - "Content/Input/Mappings/IMC_Default.uasset"
    - "Scripts/create_phase1_assets.py"
    - "Scripts/EDITOR_SETUP.md"
  modified:
    - "ProjectWalkingSim.uproject (added EnhancedInput + PythonScriptPlugin plugins)"

key-decisions:
  - "01-06-d1: Python editor scripting for Input Action and IMC creation instead of manual editor work"
  - "01-06-d2: Custom C++ classes not accessible in commandlet Python; Blueprints require manual editor creation"
  - "01-06-d3: PythonScriptPlugin kept as editor-only for future automation scripts"

patterns-established:
  - "UE5 Python asset automation: InputAction_Factory, InputMappingContext_Factory, map_key API"
  - "Key construction via FKey.import_text() since Key constructor rejects arguments in Python wrapper"
  - "EDITOR_SETUP.md pattern for documenting manual editor steps alongside automation"

metrics:
  duration: "~13min"
  completed: "2026-02-08"
---

# Phase 1 Plan 06: Input Assets, IMC, and Editor Integration Setup Summary

**7 Input Action assets and IMC_Default with 10 key bindings created via Python automation; editor setup guide for Blueprint subclasses, UMG widgets, and TestMap**

## Performance

- **Duration:** ~13 min
- **Started:** 2026-02-08T17:22:54Z
- **Completed:** 2026-02-08T17:36:00Z
- **Tasks:** 1/2 (Task 2 is human-verify checkpoint)
- **Files created:** 11

## Accomplishments

- Created 7 Input Action data assets via Python editor scripting: IA_Move (Axis2D), IA_Look (Axis2D), IA_Sprint (Bool), IA_Crouch (Bool), IA_Interact (Bool, Pressed trigger), IA_LeanLeft (Bool), IA_LeanRight (Bool)
- Created IMC_Default with 10 key mappings: W/S/A/D with Swizzle+Negate modifiers for Axis2D movement, Mouse2D for look, LeftShift for sprint, LeftControl for crouch, E with Pressed trigger for interact, Q for lean left, E with Hold 0.2s trigger for lean right
- Developed reusable Python automation script (create_phase1_assets.py) that discovered and used the UE5.7 Python API: InputAction_Factory, InputMappingContext_Factory, Key.import_text(), IMC.map_key(), and modifier/trigger configuration
- Created comprehensive EDITOR_SETUP.md guide covering all manual editor steps: 4 Blueprint subclasses, 2 UMG widget Blueprints, and TestMap configuration
- Enabled EnhancedInput and PythonScriptPlugin plugins in .uproject

## Task Commits

Each task was committed atomically:

1. **Task 1: Create Input Action assets, IMC, and editor setup guide** - `c495519` (feat)

**Note:** Task 2 is a checkpoint:human-verify requiring PIE testing after manual editor setup.

## Files Created/Modified

- `Content/Input/Actions/IA_Move.uasset` - Axis2D Input Action for WASD movement
- `Content/Input/Actions/IA_Look.uasset` - Axis2D Input Action for mouse look
- `Content/Input/Actions/IA_Sprint.uasset` - Boolean Input Action for sprint
- `Content/Input/Actions/IA_Crouch.uasset` - Boolean Input Action for crouch toggle
- `Content/Input/Actions/IA_Interact.uasset` - Boolean Input Action with Pressed trigger
- `Content/Input/Actions/IA_LeanLeft.uasset` - Boolean Input Action for lean left
- `Content/Input/Actions/IA_LeanRight.uasset` - Boolean Input Action for lean right
- `Content/Input/Mappings/IMC_Default.uasset` - Input Mapping Context with all 10 key bindings
- `Scripts/create_phase1_assets.py` - Python automation script for reproducible asset creation
- `Scripts/EDITOR_SETUP.md` - Step-by-step manual editor setup guide
- `ProjectWalkingSim.uproject` - Added EnhancedInput and PythonScriptPlugin plugins

## Decisions Made

| ID | Decision | Rationale |
|----|----------|-----------|
| 01-06-d1 | Python editor scripting for IA/IMC creation | Reproducible, version-controllable, avoids manual binary asset creation errors |
| 01-06-d2 | Manual Blueprint creation required | Custom C++ classes (ASereneCharacter, etc.) not accessible in Python commandlet; only UCLASS with Blueprintable get Python bindings |
| 01-06-d3 | Keep PythonScriptPlugin as editor-only | Useful for future automation; no runtime cost with TargetAllowList = Editor |

## Deviations from Plan

### Technical Discovery

**1. Custom C++ class inaccessibility in Python commandlet environment**
- **Found during:** Task 1 (asset creation)
- **Issue:** The plan specified creating Blueprint subclasses via scripting, but custom C++ classes (ASereneCharacter, ASerenePlayerController, ASereneHUD, UStaminaBarWidget, UInteractionPromptWidget) are not accessible in the UE5 Python environment. Only ASereneGameMode was loadable. The Python wrapper only generates bindings for classes with explicit Blueprint exposure specifiers.
- **Impact:** Blueprint subclasses, UMG widgets, and TestMap setup must be done manually in the editor
- **Mitigation:** Created EDITOR_SETUP.md with step-by-step instructions for all manual steps
- **No code changes needed:** The C++ UCLASS specifiers are correct for their intended use

**2. UE5.7 Python API differences from documentation**
- `InputActionFactory` does not exist; correct name is `InputAction_Factory`
- `Key(key_name="W")` constructor rejects arguments; must use `Key.import_text("W")`
- `IMC.mappings` property deprecated in favor of `default_key_mappings`
- `map_key(action, to_key)` is the correct API for adding key bindings

---

**Total deviations:** 0 auto-fixed (2 technical discoveries documented)
**Impact on plan:** Blueprint/Widget/TestMap creation shifted to manual editor steps. Input assets fully automated.

## Issues Encountered

- UE5.7 Python API has undocumented differences from public docs (factory names, Key construction, property deprecations). Resolved through iterative API discovery.
- The `-nullrhi` flag may contribute to reduced class availability in Python. Running in full editor mode might expose more classes, but the commandlet approach was chosen for CI-compatibility.

## User Setup Required

**Manual editor configuration required before PIE testing.** See `Scripts/EDITOR_SETUP.md` for:

1. **Blueprint subclasses** (4 total):
   - BP_SereneCharacter (parent: SereneCharacter) - assign Mannequin mesh
   - BP_SerenePlayerController (parent: SerenePlayerController) - assign all 7 IA_ assets + IMC_Default
   - BP_SereneHUD (parent: SereneHUD) - assign WBP_StaminaBar + WBP_InteractionPrompt classes
   - BP_SereneGameMode (parent: SereneGameMode) - set DefaultPawnClass, PlayerControllerClass, HUDClass

2. **UMG Widget Blueprints** (2 total):
   - WBP_StaminaBar (parent: StaminaBarWidget) - add ProgressBar named "StaminaBar"
   - WBP_InteractionPrompt (parent: InteractionPromptWidget) - add TextBlocks "PromptText" and "ReticleText"

3. **TestMap configuration:**
   - Set GameMode Override to BP_SereneGameMode
   - Place floor, lights, PlayerStart, and test interactable actors

## Next Phase Readiness

Phase 1 C++ code is complete. After manual editor setup:
- All 5 ROADMAP success criteria for Phase 1 will be testable
- 22-point PIE verification checklist can be executed
- Phase 2 (Inventory) can begin once Phase 1 is verified

Blockers: Manual editor setup must be completed before PIE verification.

---
*Phase: 01-foundation*
*Completed: 2026-02-08 (pending human verification)*
