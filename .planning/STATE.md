# State: The Juniper Tree

**Purpose:** Session continuity and project memory for Claude.

---

## Project Reference

**Core Value:** The player must feel the dread of being hunted while slowly questioning their own reality and identity.

**Current Focus:** Phase 3 complete. Ready for Phase 4 (Monster AI Core) — State Tree AI, patrol, perception.

**Key Constraints:**
- Engine: Unreal Engine 5.7.2
- Quality: High-quality C++ with performance reviews after each phase
- AI: State Tree (not Behavior Trees)
- Rendering: Lumen GI + Virtual Shadow Maps
- Player: First-person with visible body

---

## Current Position

**Phase:** 4 of 8 (Monster AI Core)
**Plan:** 0 of ? (not yet planned)
**Status:** Ready to plan Phase 4
**Last activity:** 2026-02-10 - Phase 3 complete, all features verified

**Progress:**
```
Phase 1: [######] 6/6 plans complete ✓
Phase 2: [######] 6/6 plans complete ✓
Phase 3: [######] 6/6 plans complete ✓
Overall: [███.....] 3/8 phases complete
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

### TODOs

- [x] Plan Phase 1: Foundation
- [x] Research UE5.7 first-person rendering before Phase 1 plans
- [x] Execute Phase 1 (all 6 plans)
- [x] Plan Phase 2: Inventory
- [x] Execute Phase 2 (all 6 plans)
- [x] Plan Phase 3: Hiding System
- [x] Execute Phase 3 (all 6 plans)
- [ ] Plan Phase 4: Monster AI Core
- [ ] Execute Phase 4

### Blockers

None — Phase 3 complete, ready for Phase 4.

---

## Session Continuity

### Last Session

**Date:** 2026-02-10
**Completed:**
- Executed all 6 Phase 3 plans across 4 waves
- Wave 1: Foundation types (03-01) + Visibility score component (03-02) in parallel
- Wave 2: HidingSpotActor (03-03) + HidingComponent (03-04) in parallel
- Wave 3: Character integration (03-05)
- Wave 4: Editor assets + PIE verification (03-06) with checkpoint
- Fixed 4 integration bugs during PIE verification:
  1. Commented-out OnInteract delegation (parallel plan artifact)
  2. Exit sequencing: OnExitHiding called after pointer cleared
  3. Hiding camera bAutoActivate=false preventing CalcCamera from reading it
  4. Controller rotation not mirrored to hiding camera for look-around
- Simplified input: removed IA_ExitHiding, reuse IA_Interact
- Verified full hiding flow in PIE (enter, look-around, exit, re-enter)

**Stopped at:** Phase 3 complete

**Next:** Plan Phase 4 (Monster AI Core)

### Context for Next Session

The Juniper Tree is a psychological horror game demo. The player is a detective investigating a missing boy, eventually discovering they ARE the murdered boy. A Wendigo (the father transformed by cannibalism) stalks the player.

The roadmap has 8 phases:
1. Foundation - Player controller, movement, interaction ✓
2. Inventory - 8-slot system with items ✓
3. Hiding - Hide spots and visibility ✓
4. Monster AI Core - State Tree, patrol, perception
5. Monster Behaviors - Chase, investigate, search, spawns
6. Light and Audio - Flashlight, Lumen, spatial audio
7. Save System - Checkpoints and manual saves
8. Demo Polish - Environment, story, optimization

Phase 3 complete. The project now has:
- All Phase 1 features (character, movement, interaction, HUD)
- All Phase 2 features (inventory, items, doors, combine)
- UHidingComponent: 4-state machine (Free/Entering/Hidden/Exiting) with montage, camera blend, look constraints, IMC switching
- AHidingSpotActor: dual IInteractable + IHideable, SpotMesh, HidingCamera, SpotData reference
- UVisibilityScoreComponent: 8x8 SceneCapture light sampling, 0.0-1.0 score, crouch/hiding modifiers
- UHidingSpotDataAsset: per-type config (camera limits, blend times, interaction text, visibility reduction)
- 5 gameplay tags: Player.Hiding, Interaction.HidingSpot, HidingSpot.Locker/Closet/UnderBed
- IMC_Hiding: F -> IA_Interact (exit via controller routing), Mouse2D -> IA_Look
- 3 data assets: DA_HidingSpot_Locker, DA_HidingSpot_Closet, DA_HidingSpot_UnderBed
- ASereneCharacter: 9 components total, hiding-aware Tick bypass

All 9 character components wired: Stamina, HeadBob, Lean, Interaction, Footstep, Inventory, Camera, Hiding, VisibilityScore.
10/29 v1 requirements complete. No orphans.

**Phase 4 delivers:**
- State Tree-driven AI controller for Wendigo
- Patrol behavior within designated zones
- AI Perception system (sight + hearing)

---

*State initialized: 2026-02-07*
*Last updated: 2026-02-10 (Phase 3 complete)*
