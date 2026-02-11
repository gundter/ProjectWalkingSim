# Phase 4 PIE Verification Guide

## Step 1: Create Blueprint Subclasses

1. Content Browser > right-click > Blueprint Class > search **WendigoCharacter** > Create **BP_WendigoCharacter**
   - Save to `Content/AI/`
   - Open BP_WendigoCharacter > Components > Mesh: assign SK_Mannequin (or any skeletal mesh placeholder)
   - Set Mesh scale to approximately **(1.4, 1.4, 1.4)** to make it visually tall (capsule is already 260cm from C++)

2. Content Browser > right-click > Blueprint Class > search **PatrolRouteActor** > Create **BP_PatrolRoute**
   - Save to `Content/AI/`

## Step 2: Create State Tree Asset

1. Content Browser > right-click > Artificial Intelligence > State Tree > Create **ST_WendigoAI**
   - Save to `Content/AI/`

2. Open ST_WendigoAI:
   - Set Schema to **StateTreeAIComponentSchema**
   - Set AIController Class to **WendigoAIController**
   - Set Context Actor Class to **WendigoCharacter** (or BP_WendigoCharacter)

3. Create the State Tree structure:

   | State | Task | On Succeeded | Enter Condition |
   |-------|------|-------------|-----------------|
   | **Patrol** (default) | STT_PatrolMoveToWaypoint | -> Patrol Idle | (none) |
   | **Patrol Idle** | STT_PatrolIdle | -> Patrol | (none) |
   | **Suspicious** | STT_OrientToward | -> Investigate | STC_SuspicionLevel (RequiredLevel = Suspicious) |
   | **Investigate** | STT_InvestigateLocation | -> Patrol | (none) |

   - Exit condition from Suspicious/Investigate back to Patrol: STC_SuspicionLevel with RequiredLevel = Suspicious, **bInvertCondition = true**

   The key behavior chain:
   - **Normal:** Patrol <-> Patrol Idle (cycling between waypoints)
   - **On detection:** Patrol -> Suspicious -> Investigate -> Patrol

## Step 3: Assign State Tree to Controller

1. Open BP_WendigoCharacter > Details > AI Controller Class should already be **WendigoAIController** (set in C++)
2. If the StateTreeAIComponent is visible in the controller, assign **ST_WendigoAI** to it
   - This may need to be set in a BP subclass of the controller, or directly on the StateTreeAIComponent default

## Step 4: Configure NavMesh

1. Check Project Settings > Navigation Mesh > Agents: should show **Default** + **Wendigo**
   - If Wendigo agent is missing: add manually (Radius=60, Height=275, Step Height=35)
2. Place a **NavMeshBoundsVolume** in the level covering the test area
3. Build navigation: Build > Build Paths (or it auto-builds)
4. Press **P** to visualize NavMesh in viewport -- green overlay should appear

## Step 5: Set Up Test Level

1. Place **BP_PatrolRoute** in level
   - In Details panel, add 3-4 waypoints to the Waypoints array (space 500-1000cm apart)
2. Place **BP_WendigoCharacter** in level
   - In Details panel, set PatrolRoute reference to the BP_PatrolRoute actor
3. Ensure player start is within sight range (~25m / 2500cm) of the patrol route

## Step 6: PIE Verification Checklist

Press **Play** (PIE) and verify:

- [ ] **Patrol works:** Wendigo moves between waypoints at 150 cm/s
- [ ] **Idle pauses:** Wendigo pauses 3-6 seconds at waypoints with look-around
- [ ] **Sight detection:** Walk in front of Wendigo (within 25m, in 90-degree FOV) -- suspicion accumulates
- [ ] **Suspicious reaction:** Wendigo stops and orients toward player (menacing pause)
- [ ] **Hearing detection:** Sprint near Wendigo (within 20m) -- noise triggers reaction
- [ ] **State transitions:** Patrol -> Suspicious -> Investigate -> back to Patrol
- [ ] **Output Log:** "Wendigo State Tree started", suspicion level changes logged

### Known Fix Applied During Verification

STC_SuspicionLevel was missing the required instance data struct for State Tree conditions. Added empty `FSTC_SuspicionLevelInstanceData` and `GetInstanceDataType()` override. Hot reload or editor restart required after fix.

### Debug Commands

- `ShowDebug AI` -- State Tree debug view
- `ai.debug.enablecategory perception` -- perception debug overlay

### Expected Behavior

The Wendigo should cycle between waypoints, pausing periodically. When the player walks into its field of view in a well-lit area, suspicion builds. At the Suspicious threshold, the Wendigo stops and turns toward the player. It then investigates the stimulus location. After investigating, it returns to patrol. Sprint footsteps within 20m should also trigger hearing-based suspicion.
