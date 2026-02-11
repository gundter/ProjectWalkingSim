# Phase 3 Editor Setup Guide: Hiding System

After running the Python asset creation script, complete these manual steps in the Unreal Editor.

The C++ code for the hiding system is complete (Plans 01-05). This guide covers creating the data assets, Blueprint configuration, and test map setup that cannot be automated via Python.

---

## Part 1: Run Python Script

### Execute create_phase3_assets.py

1. Open Unreal Editor with the project
2. Go to **Window > Output Log** to monitor progress
3. Run the Python script via one of these methods:

**Method A: Editor Console**
1. Open **Window > Developer Tools > Output Log**
2. At the bottom, enter: `py "Scripts/create_phase3_assets.py"`
3. Press Enter

**Method B: Command Line (outside editor)**
```
"C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "H:\Unreal Projects\ProjectWalkingSim\ProjectWalkingSim.uproject" -ExecutePythonScript="H:\Unreal Projects\ProjectWalkingSim\Scripts\create_phase3_assets.py" -nullrhi -unattended
```

### What the Script Creates

- **IMC_Hiding** - Input Mapping Context with two mappings:
  - F key -> IA_Interact (reuses existing interact action; controller routes to ExitHidingSpot when hiding)
  - Mouse2D -> IA_Look (constrained look while hiding)

### Verify Script Results

1. Navigate to `Content/Input/Mappings/`
2. Open `IMC_Hiding` and verify:
   - F is bound to IA_Interact with Pressed trigger
   - Mouse2D is bound to IA_Look

---

## Part 2: Create HidingSpotDataAsset Instances

These data assets configure per-type behavior for hiding spots. The C++ class `UHidingSpotDataAsset` defines the properties. Each hiding spot type in the level references one of these assets.

### Create Folder

1. Content Browser: Navigate to `Content/`
2. Right-click > **New Folder** > Name it `Hiding` (if not already present)

### DA_HidingSpot_Locker

1. Right-click in `Content/Hiding/` > **Miscellaneous > Data Asset**
2. In the class picker, select **HidingSpotDataAsset**
3. Name: `DA_HidingSpot_Locker`
4. Open and set these properties:

| Property | Value |
|----------|-------|
| **Animation** | |
| EntryMontage | None (no animation assets yet) |
| ExitMontage | None |
| **Camera** | |
| CameraBlendInTime | 0.5 |
| CameraBlendOutTime | 0.3 |
| ViewPitchMin | -20.0 |
| ViewPitchMax | 15.0 |
| ViewYawMin | -35.0 |
| ViewYawMax | 35.0 |
| **Visual** | |
| PeekOverlayMaterial | None |
| **Interaction** | |
| InteractionText | Hide in Locker |
| ExitText | Exit Locker |
| **Classification** | |
| SpotTypeTag | HidingSpot.Locker |
| **Gameplay** | |
| HidingVisibilityReduction | 0.6 |

**Note on SpotTypeTag:** The tag `HidingSpot.Locker` is defined in C++ via `UE_DEFINE_GAMEPLAY_TAG`. It should appear in the gameplay tag picker. If it does not appear, ensure the project has compiled successfully.

### DA_HidingSpot_Closet

1. Right-click in `Content/Hiding/` > **Miscellaneous > Data Asset**
2. Select **HidingSpotDataAsset**
3. Name: `DA_HidingSpot_Closet`
4. Set properties:

| Property | Value |
|----------|-------|
| **Animation** | |
| EntryMontage | None |
| ExitMontage | None |
| **Camera** | |
| CameraBlendInTime | 0.6 |
| CameraBlendOutTime | 0.4 |
| ViewPitchMin | -25.0 |
| ViewPitchMax | 20.0 |
| ViewYawMin | -15.0 |
| ViewYawMax | 15.0 |
| **Visual** | |
| PeekOverlayMaterial | None |
| **Interaction** | |
| InteractionText | Hide in Closet |
| ExitText | Exit Closet |
| **Classification** | |
| SpotTypeTag | HidingSpot.Closet |
| **Gameplay** | |
| HidingVisibilityReduction | 0.55 |

**Design note:** Closet has narrower yaw range (-15 to +15) than locker (-35 to +35) to simulate looking through a door crack.

### DA_HidingSpot_UnderBed

1. Right-click in `Content/Hiding/` > **Miscellaneous > Data Asset**
2. Select **HidingSpotDataAsset**
3. Name: `DA_HidingSpot_UnderBed`
4. Set properties:

| Property | Value |
|----------|-------|
| **Animation** | |
| EntryMontage | None |
| ExitMontage | None |
| **Camera** | |
| CameraBlendInTime | 0.8 |
| CameraBlendOutTime | 0.5 |
| ViewPitchMin | -10.0 |
| ViewPitchMax | 5.0 |
| ViewYawMin | -60.0 |
| ViewYawMax | 60.0 |
| **Visual** | |
| PeekOverlayMaterial | None |
| **Interaction** | |
| InteractionText | Hide Under Bed |
| ExitText | Crawl Out |
| **Classification** | |
| SpotTypeTag | HidingSpot.UnderBed |
| **Gameplay** | |
| HidingVisibilityReduction | 0.5 |

**Design note:** Under-bed has the widest yaw range (-60 to +60) but tightest pitch (-10 to +5) to simulate being flat on the ground with wide horizontal view but minimal vertical movement.

---

## Part 3: Configure BP_SereneCharacter

Wire up the HidingComponent's input references on the Blueprint.

1. Open `Content/Blueprints/Game/BP_SereneCharacter`
2. In the Components panel, select **HidingComponent**
3. In the Details panel, find the **Input** category
4. Set these two properties:

| Property | Value |
|----------|-------|
| DefaultMappingContext | IMC_Default |
| HidingMappingContext | IMC_Hiding |

5. **Compile** and **Save** the Blueprint

### Why these matter

- **DefaultMappingContext (IMC_Default):** The HidingComponent removes this when entering a hiding spot to disable WASD movement, sprint, crouch, and other default inputs. It restores this on exit.
- **HidingMappingContext (IMC_Hiding):** Added while hiding. Contains only Look (mouse) and Interact (F key). The controller's HandleInteract routes F to ExitHidingSpot() when the player is hiding.

---

## Part 4: Test Map Setup

Place at least one hiding spot actor in the test map for PIE verification.

### Place a Locker Hiding Spot

1. Open `Content/Maps/TestMap`
2. In the Place Actors panel, search for `HidingSpotActor`
3. Drag an instance into the level near the player start
4. With the actor selected, configure in Details:

**Components:**
- **SpotMesh:** Assign a static mesh to serve as the visual and trace target
  - Use `Engine Content > BasicShapes > Cube` (or any visible mesh)
  - Scale to approximately locker size (e.g., X=0.5, Y=0.5, Z=2.0 for a tall box)
  - Ensure collision is set to **BlockAll** (default for static mesh)

- **HidingCamera:** Position inside/behind the mesh where the player would view from while hiding
  - Move it slightly inside the mesh volume (e.g., Z=150 relative to actor, X=-20 behind the door side)
  - Rotate to face the "door opening" direction (the way the player would peek out)

**Configuration:**
- **SpotData:** Assign `DA_HidingSpot_Locker`

### Optional: Place Additional Hiding Spots

For testing multiple types, repeat with:
- A second HidingSpotActor using `DA_HidingSpot_Closet` (wider mesh, camera near a gap)
- A third using `DA_HidingSpot_UnderBed` (flat mesh, camera at floor level looking outward)

### Save the Map

Press **Ctrl+S** to save TestMap.

---

## Part 5: PIE Verification

### Basic Flow Test

1. **Play in Editor** (Alt+P or Play button)
2. Walk toward the hiding spot mesh
3. Look at it -- an interaction prompt should appear: **"F: Hide in Locker"**
4. Press **F** to enter hiding:
   - Camera smoothly blends to the HidingCamera position
   - WASD movement is disabled
   - Sprint/crouch should not function
5. While hidden:
   - Mouse look is constrained to the yaw/pitch limits in the data asset
   - The interaction prompt is no longer visible
6. Press **F** to exit hiding:
   - Camera blends back to first-person view
   - Movement is restored
   - All controls work normally

### Output Log Verification

1. Open **Window > Developer Tools > Output Log**
2. Filter by `LogSerene`
3. Expected log messages during the hide/exit cycle:
   - `Entering hiding spot [HidingSpotActor_X]`
   - `Now hidden in [HidingSpotActor_X]`
   - `Exited hiding spot, now Free`
4. VisibilityScoreComponent should log initialization at BeginPlay

### Edge Case Tests

- **Inventory while hiding:** Press Tab while hidden -- inventory should open normally
- **Re-interact blocked:** Cannot interact with the same hiding spot while already inside
- **Walk away and return:** Leaving and returning to the hiding spot -- prompt reappears
- **Camera constraints:** While hidden, try to look beyond the yaw/pitch limits -- camera should stop

---

## Troubleshooting

### "Hide in Locker" Prompt Not Appearing

**Cause:** SpotMesh has no collision, or InteractionComponent trace is not hitting it.

**Fix:**
1. Select the SpotMesh component on the HidingSpotActor
2. Verify collision preset is **BlockAll** (not NoCollision or OverlapAll)
3. Verify the mesh is visible and has a valid static mesh assigned
4. Check that the player is within interaction trace range (default: 200 units)

### Camera Does Not Blend

**Cause:** HidingCamera not properly positioned, or SpotData not assigned.

**Fix:**
1. Verify SpotData is assigned on the HidingSpotActor instance
2. Verify HidingCamera is positioned inside the actor (not at origin)
3. Check Output Log for any error messages

### F Key Does Not Exit

**Cause:** IMC_Hiding not active, or HidingMappingContext not assigned on HidingComponent.

**Fix:**
1. Open BP_SereneCharacter > HidingComponent
2. Verify HidingMappingContext = IMC_Hiding
3. Open IMC_Hiding and verify F is mapped to IA_Interact
4. Compile and save the Blueprint

### Movement Not Restored After Exit

**Cause:** TransitionToFreeState not reached (montage interruption or state issue).

**Fix:**
1. Check Output Log for "Exited hiding spot, now Free" message
2. If missing, check for errors during the exit sequence
3. Verify no entry/exit montages are assigned (they should be None for now)

---

## Summary of All Assets

After completing this guide, the project should have:

| Asset | Path | Type |
|-------|------|------|
| IMC_Hiding | Content/Input/Mappings/ | Input Mapping Context (F -> IA_Interact, Mouse2D -> IA_Look) |
| DA_HidingSpot_Locker | Content/Hiding/ | HidingSpotDataAsset |
| DA_HidingSpot_Closet | Content/Hiding/ | HidingSpotDataAsset |
| DA_HidingSpot_UnderBed | Content/Hiding/ | HidingSpotDataAsset |
| HidingSpotActor | TestMap (placed) | Actor instance with SpotData assigned |
| BP_SereneCharacter | Content/Blueprints/Game/ | Updated with HidingComponent input refs |
