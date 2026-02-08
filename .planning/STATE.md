# State: The Juniper Tree

**Purpose:** Session continuity and project memory for Claude.

---

## Project Reference

**Core Value:** The player must feel the dread of being hunted while slowly questioning their own reality and identity.

**Current Focus:** Phase 1 code complete. All C++ classes, components, and data assets created. Awaiting manual editor setup (Blueprint subclasses, UMG widgets, TestMap) and PIE verification before closing Phase 1.

**Key Constraints:**
- Engine: Unreal Engine 5.7.2
- Quality: High-quality C++ with performance reviews after each phase
- AI: State Tree (not Behavior Trees)
- Rendering: Lumen GI + Virtual Shadow Maps
- Player: First-person with visible body

---

## Current Position

**Phase:** 1 of 8 (Foundation)
**Plan:** 6 of 6 (Task 1 complete, awaiting human verification checkpoint)
**Status:** Awaiting editor setup + PIE verification
**Last activity:** 2026-02-08 - Executed 01-06-PLAN.md Task 1 (Input Assets, IMC, editor setup guide)

**Progress:**
```
Phase 1: [######] 6/6 plans executed (pending verification)
Overall: [........] 0/8 phases complete
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

*Task 2 is human-verify checkpoint

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
| PythonScriptPlugin kept as editor-only | Useful for future automation; no runtime cost with TargetAllowList = Editor | 01-06 |

### Technical Discoveries

| Discovery | Context | Phase |
|-----------|---------|-------|
| UE5.7 Python API: InputAction_Factory (not InputActionFactory) | UE naming convention differs from docs | 01-06 |
| FKey.import_text() required for key construction in Python | Key(key_name=...) rejects args in Python wrapper | 01-06 |
| IMC.map_key(action, key) returns mutable EnhancedActionKeyMapping | Correct API for adding bindings with modifiers/triggers | 01-06 |
| Custom UCLASS without Blueprintable not accessible in Python commandlet | Only SereneGameMode was loadable; others returned None | 01-06 |
| IMC.mappings deprecated in UE5.7 in favor of default_key_mappings | Property still works but triggers DeprecationWarning | 01-06 |

### TODOs

- [x] Plan Phase 1: Foundation
- [x] Research UE5.7 first-person rendering before Phase 1 plans
- [x] Execute 01-03-PLAN.md (Stamina, Head-Bob, Lean Components)
- [x] Execute 01-05-PLAN.md (Footsteps, Stamina HUD, Integration)
- [x] Execute 01-06-PLAN.md Task 1 (Input Assets, IMC, editor setup)
- [ ] Complete manual editor setup (see Scripts/EDITOR_SETUP.md)
- [ ] PIE verification of all Phase 1 features (01-06 checkpoint)
- [ ] Plan Phase 2: Inventory

### Blockers

- **Manual editor setup required:** Blueprint subclasses, UMG widgets, and TestMap must be configured in Unreal Editor before PIE verification can proceed. See `Scripts/EDITOR_SETUP.md` for step-by-step instructions.

---

## Session Continuity

### Last Session

**Date:** 2026-02-08
**Completed:**
- Executed 01-06-PLAN.md Task 1 (Input Assets, IMC, Editor Setup Guide)
  - 7 Input Action assets: IA_Move, IA_Look, IA_Sprint, IA_Crouch, IA_Interact, IA_LeanLeft, IA_LeanRight
  - IMC_Default with 10 key mappings (WASD+Mouse+Shift+Ctrl+E+Q+E-hold)
  - Python automation script (Scripts/create_phase1_assets.py)
  - Editor setup guide (Scripts/EDITOR_SETUP.md)
  - EnhancedInput + PythonScriptPlugin enabled in .uproject

**Stopped at:** Checkpoint - human-verify (Task 2 of 01-06-PLAN.md)

**Next:** User completes manual editor setup, then approves or reports issues at checkpoint

### Context for Next Session

The Juniper Tree is a psychological horror game demo. The player is a detective investigating a missing boy, eventually discovering they ARE the murdered boy. A Wendigo (the father transformed by cannibalism) stalks the player.

The roadmap has 8 phases:
1. Foundation - Player controller, movement, interaction
2. Inventory - 8-slot system with items
3. Hiding - Hide spots and visibility
4. Monster AI Core - State Tree, patrol, perception
5. Monster Behaviors - Chase, investigate, search, spawns
6. Light and Audio - Flashlight, Lumen, spatial audio
7. Save System - Checkpoints and manual saves
8. Demo Polish - Environment, story, optimization

Phase 1 has 6 plans. All 6 plans executed. The project now has:
- Build.cs with EnhancedInput, UMG, GameplayTags, PhysicsCore, Slate, SlateCore
- IInteractable, IHideable, ISaveable interfaces
- 11 native gameplay tags (Interaction, Movement, Player categories)
- LogSerene log category
- USereneGameInstance with accessibility settings (head-bob toggle, crouch mode)
- ASereneCharacter with FP rendering, head-bone camera, WorldRepMesh, grounded CMC
- ASerenePlayerController with 7 input bindings (Move, Look, Sprint, Crouch, Interact, LeanLeft, LeanRight)
- ASereneGameMode in Core/ with character+controller+HUD defaults
- UStaminaComponent: drain/regen with 1.5s delay, exhaustion threshold, 3 delegates
- UHeadBobComponent: procedural sine-wave bob, sprint/crouch multipliers, toggleable
- ULeanComponent: 30cm lateral offset, 5-degree roll, smooth transitions
- Camera offset aggregation in character Tick() for HeadBob + Lean coexistence
- UInteractionComponent: per-tick camera line trace, focus management, OnInteractableChanged delegate
- UInteractionPromptWidget: C++ base with BindWidget slots for UMG
- AInteractableBase: abstract base with IInteractable, InteractionText, InteractionTag, MeshComponent
- ADoorActor, APickupActor, AReadableActor, ADrawerActor: four interactable types
- UFootstepComponent: surface detection via downward trace, timer-based trigger, OnFootstep delegate
- UStaminaBarWidget: progress bar with auto-show/hide and 2s delay
- ASereneHUD: widget lifecycle manager for StaminaBar + InteractionPrompt
- 7 Input Action assets (IA_Move, IA_Look, IA_Sprint, IA_Crouch, IA_Interact, IA_LeanLeft, IA_LeanRight)
- IMC_Default with 10 key bindings (WASD, Mouse, Shift, Ctrl, E-press, Q, E-hold)
- EnhancedInput + PythonScriptPlugin plugins enabled

All 5 character components wired: Stamina, HeadBob, Lean, Interaction, Footstep.
All 29 v1 requirements are mapped. No orphans.

**Remaining before Phase 1 closure:**
1. Manual editor setup: Create BP_SereneCharacter, BP_SerenePlayerController, BP_SereneHUD, BP_SereneGameMode, WBP_StaminaBar, WBP_InteractionPrompt (see Scripts/EDITOR_SETUP.md)
2. TestMap configuration: GameMode override, floor, lights, PlayerStart, test interactables
3. PIE verification: 22-point checklist

---

*State initialized: 2026-02-07*
*Last updated: 2026-02-08 (01-06 Task 1 completion, awaiting checkpoint)*
