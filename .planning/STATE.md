# State: The Juniper Tree

**Purpose:** Session continuity and project memory for Claude.

---

## Project Reference

**Core Value:** The player must feel the dread of being hunted while slowly questioning their own reality and identity.

**Current Focus:** Phase 3 in progress (Hiding System) — foundation types and visibility scoring complete, building hiding spots and integration.

**Key Constraints:**
- Engine: Unreal Engine 5.7.2
- Quality: High-quality C++ with performance reviews after each phase
- AI: State Tree (not Behavior Trees)
- Rendering: Lumen GI + Virtual Shadow Maps
- Player: First-person with visible body

---

## Current Position

**Phase:** 3 of 8 (Hiding System)
**Plan:** 2 of 6 complete
**Status:** In progress
**Last activity:** 2026-02-10 - Completed 03-02-PLAN.md (Visibility Score Component)

**Progress:**
```
Phase 1: [######] 6/6 plans complete
Phase 2: [######] 6/6 plans complete
Phase 3: [##....] 2/6 plans complete
Overall: [█████████░░░░░░░░░░░░░░░] 14/18 plans (78%) | 2.3/8 phases
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
| NSLOCTEXT for default hiding interaction text | Supports future localization; "Hide" and "Exit" as defaults | 03-01 |
| Forward declarations in interface headers | Avoids heavy includes (UAnimMontage, UCameraComponent) in widely-included headers | 03-01 |
| UActorComponent for VisibilityScore (not USceneComponent) | SceneCapture is a child of the owning actor, not of this component | 03-02 |
| bIsCrouched from ACharacter base for crouch detection | Avoids circular dependency between Visibility/ and Player/ directories | 03-02 |
| 8x8 HDR render target for light sampling | 64 pixels sufficient for luminance average; trivially cheap to render and read | 03-02 |
| ShowFlags optimization on SceneCapture | Disable Bloom/MotionBlur/Particles/Fog/PostProcessing; keep GI and Reflections | 03-02 |

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

### TODOs

- [x] Plan Phase 1: Foundation
- [x] Research UE5.7 first-person rendering before Phase 1 plans
- [x] Execute Phase 1 (all 6 plans)
- [x] Plan Phase 2: Inventory
- [x] Execute Phase 2 (all 6 plans)
- [x] Plan Phase 3: Hiding System
- [ ] Execute Phase 3 (2/6 plans complete)

### Blockers

None — Phase 3 execution in progress.

---

## Session Continuity

### Last Session

**Date:** 2026-02-10
**Completed:**
- Executed Phase 3 Plan 02 (Visibility Score Component)
- Created UVisibilityScoreComponent with SceneCaptureComponent2D light sampling
- 8x8 RGBA16f render target with 0.25s timer-based capture
- ReadFloat16Pixels with Rec.709 luminance averaging
- Crouch and hiding modifiers reduce score (0.0-1.0 output)
- Added RenderCore and RHI module dependencies to Build.cs

**Stopped at:** Completed 03-02-PLAN.md

**Next:** Execute 03-03-PLAN.md (HidingSpotActor)

### Context for Next Session

Phase 3 Plans 01-02 complete. The hiding system now has:
- EHidingState enum (Free/Entering/Hidden/Exiting) with FOnHidingStateChanged delegate
- UHidingSpotDataAsset with 13 properties (montages, camera, visibility reduction)
- IHideable expanded to 8 methods
- 5 gameplay tags: Player.Hiding, Interaction.HidingSpot, HidingSpot.Locker/Closet/UnderBed
- UVisibilityScoreComponent: SceneCapture light sampling, GetVisibilityScore() 0.0-1.0
- SetHidingReduction() API ready for HidingComponent integration

**Phase 3 remaining plans:**
- 03-03: HidingSpotActor (IHideable implementation, camera, data asset)
- 03-04: HidingComponent (player-side hiding state machine)
- 03-05: Hiding spot integration (interaction, input, HUD)
- 03-06: Verification and polish

---

*State initialized: 2026-02-07*
*Last updated: 2026-02-10 (Phase 3, Plan 02 complete)*
