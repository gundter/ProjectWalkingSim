---
phase: 05-monster-behaviors
verified: 2026-02-12T19:30:00Z
status: passed
score: 5/5 must-haves verified
gaps: []
human_verification:
  - test: "Chase behavior -- walk into Wendigo line of sight, wait for Alert level, verify fast pursuit and grab attack on catch"
    expected: "Wendigo chases at ~575cm/s with head tracking; grab attack disables input and restarts level"
    why_human: "Runtime behavior, speed feel, and visual head tracking cannot be verified structurally"
  - test: "Search behavior -- trigger chase, break LOS for 3+ seconds, observe search pattern"
    expected: "Wendigo moves to last-known position, checks 2-3 nearby spots with look-around pauses, returns to patrol after ~18s"
    why_human: "NavMesh point generation and linger behavior require runtime observation"
  - test: "Hide-to-escape -- trigger chase, hide while visible vs hide while unseen"
    expected: "Witnessed hiding records spot (log message); unseen hiding is safe; search ends and Wendigo returns to patrol"
    why_human: "Delegate timing and perception-to-hiding interaction needs live testing"
  - test: "Investigation differentiation -- trigger sound stimulus vs sight stimulus"
    expected: "Sound investigation at ~200cm/s (cautious); sight investigation at ~250cm/s (aggressive)"
    why_human: "Speed difference requires visual observation in PIE"
  - test: "Spawn point -- place BP_WendigoSpawnPoint with patrol routes, call SpawnWendigo"
    expected: "Wendigo spawns at spawn point location and patrols one of the assigned routes"
    why_human: "Runtime spawn and patrol route assignment needs PIE verification"
---
# Phase 5: Monster Behaviors Verification Report

**Phase Goal:** Wendigo exhibits full range of hunt behaviors: chase on sight, search when player escapes, investigate stimuli with differentiated sight/sound reactions, spawn at designated locations with zone-based patrol selection.
**Verified:** 2026-02-12T19:30:00Z
**Status:** PASSED
**Re-verification:** No -- initial verification

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | Wendigo investigates location of sounds before returning to patrol | VERIFIED | STT_InvestigateLocation.cpp reads StimulusLocation from SuspicionComponent (line 44), navigates there, waits LookAroundDuration (4s), then clears stimulus and returns Succeeded (line 129). SuspicionComponent.cpp sets LastStimulusType=Sound in ProcessHearingStimulus (line 38). STC_StimulusType condition enables State Tree branching. |
| 2 | Wendigo chases spotted player at increased speed | VERIFIED | STT_ChasePlayer.cpp EnterState sets MaxWalkSpeed to ChaseSpeed=575cm/s (line 50), issues MoveToActor for continuous pursuit (lines 57-65), SetFocus for head tracking (line 54). Tick monitors LOS and grab range. |
| 3 | Wendigo searches area when player breaks line of sight or enters hiding | VERIFIED | STT_ChasePlayer.cpp returns Failed after LOSLostTimeout=3s (line 121). STT_SearchArea.cpp builds search points from LastKnownPlayerLocation + GetRandomReachablePointInRadius (lines 47-71), navigates through them with LingerDuration pauses (line 169), returns Succeeded after MaxSearchDuration=18s (line 129). |
| 4 | Wendigo can spawn at different locations with zone-appropriate patrol routes | VERIFIED | WendigoSpawnPoint.cpp SpawnWendigo() calls SpawnActor (lines 35-36), selects random route from AvailablePatrolRoutes (lines 47-48), assigns via SetPatrolRoute (line 52). WendigoCharacter.h has SetPatrolRoute inline method (line 92). |
| 5 | Player can escape chase by hiding and waiting for search to end | VERIFIED | STT_ChasePlayer returns Failed on LOS lost timeout (player hiding breaks LOS). STT_SearchArea ExitState clears LastKnownPlayerLocation and WitnessedHidingSpot (lines 242-243). WendigoAIController.cpp OnPlayerHidingStateChanged records WitnessedHidingSpot only when Wendigo has active sight perception (lines 250-253). |

**Score:** 5/5 truths verified
### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| Source/.../AI/MonsterAITypes.h | EWendigoBehaviorState, EStimulusType, AIConstants | VERIFIED | 116 lines. 5 enum values for behavior state, 3 for stimulus type, 8 new AIConstants. No stubs. |
| Source/.../AI/WendigoCharacter.h/.cpp | Chase/search persistent state, BehaviorState | VERIFIED | 105+70 lines. LastKnownPlayerLocation, WitnessedHidingSpot, BehaviorState, SetBehaviorState, SetPatrolRoute. |
| Source/.../AI/SuspicionComponent.h/.cpp | LastStimulusType tracking | VERIFIED | 138+112 lines. Getter/setter. Sight on ProcessSightStimulus, Sound on ProcessHearingStimulus, None on ResetSuspicion. |
| Source/.../Tags/SereneTags.h/.cpp | 6 new gameplay tags | VERIFIED | TAG_AI_Behavior_Patrol/Investigating/Chasing/Searching/GrabAttack + TAG_AI_Spawn_Zone declared and defined. |
| Source/.../AI/Tasks/STT_ChasePlayer.h/.cpp | Chase task with LOS timer, grab range | VERIFIED | 94+182 lines. MoveToActor pursuit, LOS timer, grab range check requiring LOS, speed restore in ExitState. |
| Source/.../AI/Tasks/STT_SearchArea.h/.cpp | Search with NavMesh random points | VERIFIED | 109+255 lines. GetRandomReachablePointInRadius, linger with look-around, MaxSearchDuration timeout, wipes chase state. |
| Source/.../AI/Tasks/STT_GrabAttack.h/.cpp | Cinematic kill sequence | VERIFIED | 80+128 lines. DisableInput, GrabDuration wait, RestartLevel, safety re-enable in ExitState. |
| Source/.../AI/Tasks/STT_ReturnToNearestWaypoint.h/.cpp | Nearest waypoint navigation | VERIFIED | 66+132 lines. DistSquared comparison, sets CurrentWaypointIndex, patrol speed. |
| Source/.../AI/Conditions/STC_StimulusType.h/.cpp | Stimulus type condition | VERIFIED | 62+38 lines. Reads GetLastStimulusType, compares to RequiredType with invert. Empty instance data struct. |
| Source/.../AI/Tasks/STT_InvestigateLocation.h/.cpp | Enhanced stimulus-aware speed | VERIFIED | 97+163 lines. bUseStimulusTypeSpeed, InvestigationSightSpeed=250, sound=200. BehaviorState lifecycle. |
| Source/.../AI/WendigoSpawnPoint.h/.cpp | Spawn point with zone routes | VERIFIED | 52+73 lines. AvailablePatrolRoutes, WendigoClass, SpawnWendigo with random route selection. |
| Source/.../AI/WendigoAIController.h/.cpp | Witnessed hiding, delegate binding | VERIFIED | 104+281 lines. BindToPlayerDelegates on first sight, OnPlayerHidingStateChanged, continuous location updates. |
| Source/.../Interaction/DoorActor.h/.cpp | OpenForAI method | VERIFIED | Method at line 84/111. Checks bIsLocked, checks bIsOpen, swing-direction logic, enables tick. |
| Content/AI/Behavior/ST_WendigoAI.uasset | Updated State Tree | EXISTS | Binary asset exists. Internal structure requires human verification. |
| Content/AI/Character/BP_WendigoSpawnPoint.uasset | SpawnPoint Blueprint | EXISTS | Binary asset exists. |
| Content/Python/setup_phase5_assets.py | Editor setup script | EXISTS | Script for asset creation. |
### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| STT_ChasePlayer | AAIController::MoveToActor | EnterState + Tick re-issue | WIRED | Lines 57-65 (EnterState), 139-149 (Tick re-issue on idle). |
| STT_ChasePlayer | WendigoCharacter::SetLastKnownPlayerLocation | Tick updates while LOS maintained | WIRED | Line 110: SetLastKnownPlayerLocation on each visible tick. |
| STT_SearchArea | GetRandomReachablePointInRadius | EnterState generates search points | WIRED | Lines 59-71 using FNavigationSystem::GetCurrent. |
| STT_GrabAttack | PlayerController::DisableInput | EnterState disables, ExitState re-enables | WIRED | Line 54 (disable), lines 102-113 (safety re-enable). |
| STT_InvestigateLocation | GetLastStimulusType | EnterState reads for speed | WIRED | Line 51: reads stimulus type, branches on Sight. |
| STC_StimulusType | GetLastStimulusType | TestCondition compares | WIRED | Lines 34-36: reads and compares with RequiredType. |
| WendigoSpawnPoint | UWorld::SpawnActor | SpawnWendigo creates character | WIRED | Lines 35-36: SpawnActor with collision handling. |
| WendigoSpawnPoint | SetPatrolRoute | Assigns route after spawn | WIRED | Line 52: SetPatrolRoute after random selection. |
| WendigoAIController | OnHidingStateChanged | AddDynamic on first sight | WIRED | Lines 226-227: delegate binding with guard. |
| WendigoAIController | SetLastKnownPlayerLocation | Tick updates every frame | WIRED | Line 129: updates while sight perception active. |
| DoorActor::OpenForAI | bIsLocked check | Returns if locked | WIRED | Lines 114-117: locked door bypass prevention. |
| SuspicionComponent | EStimulusType | Process methods set LastStimulusType | WIRED | Line 27: Sight, line 38: Sound, line 69: None on reset. |
| WendigoCharacter | FOnBehaviorStateChanged | SetBehaviorState broadcasts | WIRED | .cpp line 46: Broadcast with change guard at line 36. |

### Requirements Coverage

| Requirement | Status | Blocking Issue |
|-------------|--------|----------------|
| WNDG-03: Investigate sounds and visual disturbances | SATISFIED | None |
| WNDG-04: Chase player when spotted | SATISFIED | None |
| WNDG-05: Search behavior when player escapes/hides | SATISFIED | None |
| WNDG-06: Multiple spawn locations with per-spawn patrol zones | SATISFIED | None |

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| (none found) | - | - | - | Zero TODO, FIXME, placeholder, or stub patterns across all 23 Phase 5 files (2709 total lines). All ExitState methods restore baseline constant speed. GrabAttack safely re-enables input. |
### Human Verification Required

Six items need PIE testing to confirm runtime behavior. All structural verification passes.

### 1. Chase Behavior (WNDG-04)
**Test:** Walk into Wendigo line of sight, wait for Alert level, observe chase and grab attack.
**Expected:** Wendigo chases at noticeably faster speed (~575cm/s vs patrol 150cm/s). Head tracks player. Grab attack disables input and restarts level within ~2s.
**Why human:** Runtime movement speed, visual head tracking, and input disable/restart cannot be verified structurally.

### 2. Search Behavior (WNDG-05)
**Test:** Trigger chase, break LOS by going around a corner, wait 3+ seconds.
**Expected:** Wendigo transitions to Search, moves to last-known position, checks 2-3 random nearby spots with ~3s pauses, returns to patrol after ~18s.
**Why human:** NavMesh random point generation and navigation behavior requires runtime observation.

### 3. Hide-to-Escape (WNDG-05)
**Test:** (a) Chase + hide while visible. (b) Chase + break LOS + hide.
**Expected:** (a) Log "witnessed player entering hiding spot". (b) No witnessed hiding log. Both: search ends and patrol resumes.
**Why human:** Delegate timing between perception and hiding state change needs live testing.

### 4. Investigation Differentiation (WNDG-03)
**Test:** Sprint near Wendigo for sound, then partially enter view for sight.
**Expected:** Sound investigation: slower (~200cm/s). Sight investigation: faster (~250cm/s).
**Why human:** Speed difference requires visual comparison in PIE.

### 5. Spawn Point (WNDG-06)
**Test:** Place BP_WendigoSpawnPoint, assign 2+ patrol routes, call SpawnWendigo.
**Expected:** Wendigo spawns at spawn point location and patrols one of the assigned routes.
**Why human:** Runtime spawn and patrol route assignment needs PIE verification.

### 6. State Tree Hierarchy (Integration)
**Test:** Open ST_WendigoAI. Verify: Alert (Chase/GrabAttack/Search/ReturnToPatrol) > Suspicious (InvestigateSight/InvestigateSound/OrientToward) > Patrol (MoveToWaypoint/Idle).
**Expected:** Full hierarchy with correct enter conditions, task assignments, and transitions.
**Why human:** Binary State Tree asset cannot be read programmatically.

### Gaps Summary

No gaps found. All Phase 5 observable truths are verified through structural code analysis:

1. **Data layer complete:** EWendigoBehaviorState, EStimulusType, FOnBehaviorStateChanged, 8 AIConstants, WendigoCharacter persistent state, SuspicionComponent LastStimulusType tracking, 6 gameplay tags.

2. **Hunt cycle tasks complete:** STT_ChasePlayer (MoveToActor pursuit, LOS timer, grab range), STT_SearchArea (NavMesh random points, linger, timeout), STT_GrabAttack (input disable, RestartLevel), STT_ReturnToNearestWaypoint (DistSquared nearest waypoint).

3. **Supporting systems complete:** STC_StimulusType condition, enhanced STT_InvestigateLocation with stimulus-aware speed, AWendigoSpawnPoint with zone-based route assignment, WendigoAIController witnessed-hiding detection, DoorActor::OpenForAI.

4. **Quality patterns maintained:** All tasks restore baseline speed in ExitState (no speed leaks), GrabAttack safely re-enables input on interrupted transitions, delegate binding on first sight (not constructor), no circular header dependencies, no TODO/FIXME/placeholder patterns.

5. **Editor assets exist:** ST_WendigoAI.uasset, BP_WendigoSpawnPoint.uasset, setup_phase5_assets.py. Binary asset internal wiring flagged for human verification.

---

_Verified: 2026-02-12T19:30:00Z_
_Verifier: Claude (gsd-verifier)_