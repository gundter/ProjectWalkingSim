# State: The Juniper Tree

**Purpose:** Session continuity and project memory for Claude.

---

## Project Reference

**Core Value:** The player must feel the dread of being hunted while slowly questioning their own reality and identity.

**Current Focus:** Phase 6 in progress (Light and Audio) -- Plans 02, 04 complete. Remaining: 01, 03, 05.

**Key Constraints:**
- Engine: Unreal Engine 5.7.2
- Quality: High-quality C++ with performance reviews after each phase
- AI: State Tree (not Behavior Trees)
- Rendering: Lumen GI + Virtual Shadow Maps
- Player: First-person with visible body
- Process: Performance + code quality audit required before phase is marked complete (Epic coding standards, tick overhead, memory patterns, UE5 pitfalls)

---

## Current Position

**Phase:** 6 of 8 (Light and Audio)
**Plan:** 2 of 5 complete (06-02, 06-04)
**Status:** In progress
**Last activity:** 2026-02-12 - Completed 06-02-PLAN.md (MonsterAudioComponent)

**Progress:**
```
Phase 1: [######] 6/6 plans complete
Phase 2: [######] 6/6 plans complete
Phase 3: [######] 6/6 plans complete
Phase 4: [#######] 7/7 plans complete
Phase 5: [#####] 5/5 plans complete
Phase 6: [##...] 2/5 plans complete (06-02, 06-04)
Overall: [█████░...] 6/8 phases in progress
```

---

## Performance Metrics

| Phase | Plans | Tasks | Time | Issues |
|-------|-------|-------|------|--------|
| 1-01  | 1/6   | 2/2   | ~2m  | 0      |
| 1-02  | 2/6   | 2/2   | ~3m  | 0      |
| 1-03  | 3/6   | 2/2   | ~5m  | 0      |
| 1-04  | 4/6   | 2/2   | ~5m  | 0      |
| 1-05  | 5/6   | 2/2   | ~3m  | 0      |
| 1-06  | 6/6   | 1/2*  | ~13m | 0      |
| 2-01  | 1/6   | 2/2   | ~12m | 0      |
| 2-02  | 2/6   | 2/2   | ~7m  | 0      |
| 2-03  | 3/6   | 2/2   | ~9m  | 0      |
| 2-04  | 4/6   | 2/2   | ~7m  | 0      |
| 2-05  | 5/6   | 2/2   | ~7m  | 0      |
| 2-06  | 6/6   | 2/3*  | ~4m  | 0      |
| 3-01  | 1/6   | 2/2   | ~2m  | 0      |
| 3-02  | 2/6   | 2/2   | ~3m  | 0      |
| 3-03  | 3/6   | 1/1   | ~5m  | 0      |
| 3-04  | 4/6   | 1/1   | ~4m  | 0      |
| 3-05  | 5/6   | 2/2   | ~3m  | 0      |
| 3-06  | 6/6   | 2/2*  | ~15m | 4      |
| 4-01  | 1/?   | 2/2   | ~2m  | 0      |
| 4-02  | 2/?   | 2/2   | ~7m  | 1      |
| 4-03  | 3/?   | 1/1   | ~4m  | 2      |
| 4-04  | 4/7   | 2/2   | ~6m  | 1      |
| 4-05  | 5/7   | 2/2   | ~14m | 0      |
| 4-06  | 6/7   | 2/2   | ~5m  | 0      |
| 5-01  | 1/5   | 2/2   | ~6m  | 0      |
| 5-02  | 2/5   | 2/2   | ~8m  | 0      |
| 5-03  | 3/5   | 2/2   | ~6m  | 2      |
| 5-04  | 4/5   | 2/2   | ~8m  | 0      |
| 5-05  | 5/5   | 2/2*  | ~15m | 1      |
| 6-02  | 2/5   | 2/2   | ~5m  | 2      |
| 6-04  | 1/5   | 2/2   | ~4m  | 0      |

*Checkpoint tasks require human verification

---

## Accumulated Context

### Key Decisions

| Decision | Rationale | Phase |
|----------|-----------|-------|
| State Tree for AI | Epic's modern replacement for BT; on-demand evaluation | Roadmap |
| Component-based architecture | Decoupled, testable, reusable | Roadmap |
| 8 phases | Natural delivery boundaries from requirements | Roadmap |
| Monster AI split into 2 phases | Core + behaviors separates foundation from complexity | Roadmap |
| USaveGame for settings persistence | Simple, engine-standard, separate slot from gameplay saves | 01-01 |
| All interfaces BlueprintNativeEvent + BlueprintCallable | Allows C++ and Blueprint implementation; Execute_ wrappers auto-generated | 01-01 |
| Single LogSerene category | Small project scope; avoids fragmentation; can split later | 01-01 |
| Camera directly on head bone, no spring arm | First-person camera must track head animations; spring arm is third-person pattern | 01-02 |
| Mouse sensitivity via Enhanced Input modifiers | Data-driven; designers can tune without code changes; supports remapping | 01-02 |
| Crouch defaults to toggle mode | Matches plan spec; hold mode ready via GameInstance bCrouchToggleMode | 01-02 |
| Camera offset aggregation pattern | Components compute offsets, character Tick() applies combined result | 01-03 |
| ExhaustionThreshold at 20% before re-sprint | Prevents sprint-tap spam at low stamina; creates genuine chase tension | 01-03 |
| OnStaminaChanged fires only on actual change | KINDA_SMALL_NUMBER threshold avoids unnecessary broadcasts at idle | 01-03 |
| Sprint detection via MaxWalkSpeed > 400 | HeadBob reads CMC state without direct character class coupling | 01-03 |
| Re-broadcast interaction text after TryInteract | Door text changes "Open" to "Close"; HUD needs immediate refresh | 01-04 |
| Screen-space prompt widget, not world-space UWidgetComponent | Cleaner rendering, no lighting artifacts, still feels world-space via positioning | 01-04 |
| Door swing direction via dot product | Natural behavior: door opens away from player's approach side | 01-04 |
| Timer-based footsteps as default, bUseAnimNotify toggle | No animation assets in Phase 1; timer fallback reliable; AnimNotify ready later | 01-05 |
| StaminaBarWidget auto-hides 2s after full stamina | Matches CONTEXT.md "stamina bar only shows while sprinting then fades" | 01-05 |
| SereneHUD defers pawn binding via 0.1s timer | BeginPlay timing: pawn may not be possessed yet when HUD initializes | 01-05 |
| Python editor scripting for IA/IMC creation | Reproducible, version-controllable, avoids manual binary asset creation errors | 01-06 |
| Manual Blueprint creation required | Custom C++ classes not accessible in Python commandlet; only Blueprintable classes get Python bindings | 01-06 |
| TArray<FInventorySlot> over TMap | Preserves slot ordering required for UI display (8 fixed positions) | 02-01 |
| UPrimaryDataAsset for item definitions | Small item count benefits from per-asset editing in Content Browser | 02-01 |
| GetPrimaryAssetPath().TryLoad() for asset loading | LoadPrimaryAsset returns FStreamableHandle not UObject*; TryLoad is synchronous | 02-01 |
| PythonScriptPlugin kept as editor-only | Useful for future automation; no runtime cost with TargetAllowList = Editor | 01-06 |
| ItemTooltip BindWidget member name | Linter enforced; matches UMG child widget naming convention | 02-02 |
| mutable bInventoryFullOnLastCheck | CanInteract is const but must signal state to GetInteractionText | 02-03 |
| Allow locked door interaction attempts | Player gets "Locked" feedback; matches horror game convention | 02-03 |
| Key consumed on unlock, door opens immediately | Single interaction to unlock and open; no separate steps | 02-03 |
| SetIgnoreLookInput for inventory open | Prevents camera rotation from mouse while WASD still works | 02-04 |
| Immediate HandleInventoryChanged on ShowInventory | Ensures slots are current when panel opens | 02-04 |
| Empty recipe TMap for demo | InitCombineRecipes placeholder ready for expansion; UI flow works end-to-end | 02-05 |
| Order-independent recipe lookup | TryCombineItems checks both (A,B) and (B,A) permutations | 02-05 |
| SetKeyboardFocus on ShowInventory | Ensures NativeOnKeyDown receives input when inventory opens | 02-05 |
| Super::NativeOnKeyDown pass-through | Unhandled keys (WASD) fall through to game input for movement | 02-05 |
| Use !IsNull() for TSoftObjectPtr check | IsValid() checks if loaded; IsNull() checks if path set; need path check before LoadSynchronous | 02-06 |
| Two-click discard for key items | Safety against accidental key item loss; show warning on first click | 02-06 |
| SetVisibility for tooltip show/hide | RenderOpacity allows Tab focus on hidden buttons; Visibility blocks input properly | 02-06 |
| Tab key handled in InventoryWidget | Widget has focus for keyboard nav; must explicitly handle Tab to close | 02-06 |
| TObjectPtr for data asset references (not TSoftObjectPtr) | Data assets always loaded when referenced; no need for async load | 03-01 |
| Reuse IA_Interact for hiding exit | Controller already routes interact to ExitHidingSpot when hiding; avoids redundant action | 03-06 |
| Activate hiding camera (no bAutoActivate=false) | CalcCamera only reads active camera components; inactive camera falls back to fixed actor rotation | 03-06 |
| Mirror controller rotation to hiding camera via tick | SetViewTargetWithBlend locks view to camera component; controller rotation needs explicit sync | 03-06 |
| Notify hiding spot before montage block | TransitionToFreeState clears CurrentHidingSpot; OnExitHiding must fire before pointer nulled | 03-06 |
| Visibility normalization: (score - threshold) / (1 - threshold) | Maps above-threshold visibility to clean 0-1 range for suspicion scaling | 04-02 |
| Fixed hearing bump (0.25) not scaled | Single noise gets attention; two push to Suspicious; matches horror convention | 04-02 |
| PatrolRouteActor stub created early | UHT requires UPROPERTY class types to exist at parse time; stub unblocks WendigoCharacter | 04-02 |
| HearingSuspicionBump as component property, not AIConstants | Higher-level gameplay param vs raw perception constant; per-instance tunable | 04-02 |
| Renamed AITypes.h to MonsterAITypes.h | UHT error: project header name conflicted with engine AIModule/Classes/AITypes.h | 04-03 |
| UAISense::GetSenseID<T>() for sense identification | Cleaner and type-safe vs FAIPerceptionSystem::GetSenseClassForStimulus | 04-03 |
| Clamp waypoint index instead of modulo wrap | Safer boundary behavior; GetNextWaypointIndex handles advancement explicitly | 04-04 |
| Mutable PingPongDirection for const GetNextWaypointIndex | Query method needs to track direction state for ping-pong; mutable is appropriate | 04-04 |
| Look-around at 40% idle, clear at 75% | Creates observable scan pattern players can learn to exploit | 04-04 |
| STT_ prefix for State Tree task structs | Consistent naming convention: FSTT_PatrolMoveToWaypoint, FSTT_PatrolIdle | 04-04 |
| Volume threshold > not >= for sprint noise | Walk=1.0 exactly is silent; only sprint (1.5) generates AI noise | 04-05 |
| NoiseReportingComponent on player, not Wendigo | Player is noise source; component reports outward via ReportNoiseEvent | 04-05 |
| Tick for continuous sight, delegate for discrete hearing | Sight needs DeltaTime for accumulation; hearing is instant bump | 04-05 |
| FStateTreeConditionCommonBase for conditions | Schema-safe subclass; allows State Tree schemas to include all common conditions | 04-06 |
| Empty instance data struct required for ST conditions | State Tree conditions need GetInstanceDataType() even if stateless; "missing instance value" error otherwise | 04-06 |
| InvestigateLocation clears stimulus on completion | Allows natural return to patrol without stale stimulus data | 04-06 |
| STC_ prefix for State Tree condition structs | Parallel to STT_ for tasks: FSTC_SuspicionLevel | 04-06 |
| AActor* for WitnessedHidingSpot | Avoids circular header dependency between AI and Hiding modules; cast at use-site | 05-01 |
| BehaviorState separate from AlertLevel | EWendigoBehaviorState tracks actions (Patrol/Chasing/etc), EAlertLevel tracks perception; orthogonal | 05-01 |
| LastStimulusType persists through ClearStimulusLocation | Investigation reads type after location consumed; only full reset clears it | 05-01 |
| RestartLevel console command for demo death | Simple demo-scope approach; Phase 8 replaces with proper death/respawn system | 05-03 |
| bUseStimulusTypeSpeed defaults true | New stimulus-aware speed is desired default; flag preserves backward compatibility | 05-03 |
| BehaviorState lifecycle in tasks | Set in EnterState, restore to Patrol in ExitState; clean state for next task | 05-03 |
| Bind player hiding delegate on first sight, not at init | Player may not exist at controller init; binding on first sight ensures HidingComponent available | 05-04 |
| Forward declare EHidingState in AI header | Full include only in .cpp; avoids circular AI-Hiding header dependency | 05-04 |
| SetPatrolRoute method (Option B) for runtime route assignment | Preserves EditInstanceOnly restriction for level designers while enabling spawn system | 05-04 |
| DoorActor state fields promoted to protected for OpenForAI | bIsOpen/CurrentAngle/TargetAngle/OpenDirection accessible by subclass and same-class methods | 05-04 |
| MoveToActor for chase, not MoveToLocation | Auto-updates destination as player moves; avoids per-frame re-issue | 05-02 |
| Grab range requires LOS | No blind grabs through walls; Succeeded only when within GrabRange AND LineOfSightTo | 05-02 |
| SearchRadius 600cm max | GetRandomReachablePointInRadius degrades >1500cm; 600cm is reliable for tight search pattern | 05-02 |
| SearchArea ExitState wipes all chase/search state | Prevents stale LastKnownPlayerLocation and WitnessedHidingSpot from influencing next alert cycle | 05-02 |
| Option A for ReturnToPatrol in State Tree | Dedicated leaf state inside Alert container after Search for smooth patrol resumption | 05-05 |
| PlaySound2D for player footsteps | Non-spatialized; player hears own steps centered, not world-positioned | 06-04 |
| bIsUISound=true for heartbeat | Non-spatialized player-internal sound; stays centered regardless of camera | 06-04 |
| TWeakObjectPtr<AActor> for CachedWendigo | Safe reference that auto-invalidates on destroy/respawn; re-cached on next timer tick | 06-04 |
| No AudioConstants.h dependency in PlayerAudioComponent | Inline UPROPERTY defaults enable Wave 1 parallel execution | 06-04 |

### Technical Discoveries

| Discovery | Context | Phase |
|-----------|---------|-------|
| UE5.7 Python API: InputAction_Factory (not InputActionFactory) | UE naming convention differs from docs | 01-06 |
| FKey.import_text() required for key construction in Python | Key(key_name=...) rejects args in Python wrapper | 01-06 |
| IMC.map_key(action, key) returns mutable EnhancedActionKeyMapping | Correct API for adding bindings with modifiers/triggers | 01-06 |
| Custom UCLASS without Blueprintable not accessible in Python commandlet | Only SereneGameMode was loadable; others returned None | 01-06 |
| IMC.mappings deprecated in UE5.7 in favor of default_key_mappings | Property still works but triggers DeprecationWarning | 01-06 |
| UAssetManager::LoadPrimaryAsset returns FStreamableHandle | Not UObject* as expected; use GetPrimaryAssetPath().TryLoad() for sync load | 02-01 |
| UWidget::Slot member shadows local variable names | MSVC C4458 warning-as-error; use SlotData instead of Slot in loops | 02-02 |
| Asset Manager config needs Project Settings UI | DefaultEngine.ini config alone may not work; use Editor UI to configure | 02-06 |
| AActor::CalcCamera skips inactive UCameraComponents | bAutoActivate=false means CalcCamera falls back to GetActorEyesViewPoint; camera component rotation changes ignored | 03-06 |
| SetViewTargetWithBlend uses actor CalcCamera, not controller rotation | When view target is non-pawn, controller AddYawInput/AddPitchInput don't affect the view | 03-06 |
| Parallel plan execution can leave cross-references commented out | Plans 03-03 and 03-04 ran simultaneously; OnInteract delegation to HidingComponent was stubbed | 03-06 |
| UHT requires UPROPERTY class types to exist even with forward declaration | TObjectPtr<APatrolRouteActor> in UPROPERTY fails if class not defined in any header | 04-02 |
| Project header names must not collide with engine module headers | AITypes.h conflicted with AIModule/Classes/AITypes.h; UHT rejects duplicate names | 04-03 |
| Two-flag guard required for StateTree StartLogic | bBeginPlayCalled + bPossessCalled; bStartLogicAutomatically reportedly fails | 04-03 |
| Perception delegates must bind in BeginPlay, not constructor | Delegates don't work from constructor; binding in BeginPlay is safe | 04-03 |
| AActor::Instigator member shadows parameter names | ProcessHearingPerception(AActor* Instigator) triggers C4458; renamed to NoiseInstigator | 04-04 |
| State Tree tasks need explicit includes for linker and context | StateTreeLinker.h and StateTreeExecutionContext.h not transitively included from StateTreeTaskBase.h | 04-04 |
| Parallel wave agents may pre-implement cross-plan work | 04-04 implemented 04-05 Task 1 (perception wiring) as part of its own scope | 04-05 |
| StateTreeConditionBase.h includes StateTreeConditionCommonBase | Both base and common base in same header; conditions need StateTreeConditionBase.h not a separate file | 04-06 |
| UE5.7 SpawnActor<T> with UClass requires references not pointers | 4-arg overload: SpawnActor<T>(UClass*, FVector const&, FRotator const&, FActorSpawnParameters); pass values not &GetActorTransform() | 05-03 |
| Wave-parallel execution can pre-implement entire plan tasks | 05-02 committed WendigoSpawnPoint + OpenForAI + SetPatrolRoute which was Task 1 of 05-04 | 05-04 |
| WendigoAIController.cpp needs HidingSpotActor.h for upcast | AActor* WitnessedHidingSpot needs full type include in .cpp for implicit upcast; forward decl insufficient | 05-05 |

### TODOs

- [x] Plan Phase 1: Foundation
- [x] Research UE5.7 first-person rendering before Phase 1 plans
- [x] Execute Phase 1 (all 6 plans)
- [x] Plan Phase 2: Inventory
- [x] Execute Phase 2 (all 6 plans)
- [x] Plan Phase 3: Hiding System
- [x] Execute Phase 3 (all 6 plans)
- [x] Plan Phase 4: Monster AI Core
- [x] Execute Phase 4 (all 7 plans)
- [x] Plan Phase 5: Monster Behaviors
- [x] Execute Phase 5 (all 5 plans)
- [x] Performance audit for Phase 5
- [ ] Plan Phase 6: Light and Audio
- [ ] Execute Phase 6
- [ ] Future: Consider spline-based patrol routes for polish/main release (current MakeEditWidget waypoints work but less designer-friendly; may not need static routes in main release)
- [ ] Future: Replace On Tick State Tree transitions with event-driven triggers (OnAlertLevelChanged delegate, gameplay tags, or reduced tick interval) for performance -- On Tick is fine for demo but won't scale for complex Alien: Isolation-style State Trees

### Blockers

None -- Phase 6 in progress.

---

## Session Continuity

### Last Session

**Date:** 2026-02-12
**Completed:**
- Executed 06-04-PLAN.md: PlayerAudioComponent (footstep playback + heartbeat proximity)
- Surface-type footstep sounds via OnFootstep delegate binding
- Heartbeat proximity system with 4Hz timer, MetaSound Intensity parameter
- Build verified: all Audio files compile and link cleanly

**Stopped at:** Completed 06-04-PLAN.md

**Next:** Continue Phase 6 execution (plans 01, 02, 03, 05 remaining)

### Context for Next Session

The Juniper Tree is a psychological horror game demo. The player is a detective investigating a missing boy, eventually discovering they ARE the murdered boy. A Wendigo (the father transformed by cannibalism) stalks the player.

The roadmap has 8 phases:
1. Foundation - Player controller, movement, interaction COMPLETE
2. Inventory - 8-slot system with items COMPLETE
3. Hiding - Hide spots and visibility COMPLETE
4. Monster AI Core - State Tree, patrol, perception COMPLETE
5. Monster Behaviors - Chase, investigate, search, spawns COMPLETE
6. Light and Audio - Flashlight, Lumen, spatial audio
7. Save System - Checkpoints and manual saves
8. Demo Polish - Environment, story, optimization

Phase 5 complete. The project now has:
- All Phase 1-4 features (character, movement, interaction, HUD, inventory, hiding, AI core)
- Complete predator behavior loop validated in PIE:
  - Patrol -> Suspicious (investigate sound at 200cm/s or sight at 250cm/s) -> Alert (chase at 575cm/s)
  - Chase -> LOS lost -> Search (last-known + 2-3 random NavMesh points, 18s timeout)
  - Chase -> grab range -> GrabAttack (disable input, 2s, restart level)
  - Search complete -> ReturnToNearestWaypoint -> Patrol resume
- State Tree hierarchy: Alert > Suspicious > Patrol (top-to-bottom priority)
- On Tick transitions for escalation during investigation (Suspicious -> Alert)
- AWendigoSpawnPoint with zone-based patrol route selection
- Witnessed-hiding detection (AI sees player enter hiding spot)
- AI door opening (respects lock state)
- 30+ AI source files, ~2500 lines of C++

17/29 v1 requirements complete. No orphans.

**Phase 6 will deliver:**
- LGHT-01: Flashlight (always on for demo)
- LGHT-02: Lumen GI atmospheric lighting
- AUDO-01: Spatial audio (3D monster sounds)
- AUDO-02: Ambient horror soundscape
- AUDO-03: Monster audio cues (footsteps, breathing, growls)
- AUDO-04: Dynamic music and tension stingers

---

*State initialized: 2026-02-07*
*Last updated: 2026-02-12 (Phase 6 plan 04 complete -- PlayerAudioComponent)*
