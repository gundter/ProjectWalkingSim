# State: The Juniper Tree

**Purpose:** Session continuity and project memory for Claude.

---

## Project Reference

**Core Value:** The player must feel the dread of being hunted while slowly questioning their own reality and identity.

**Current Focus:** Phase 1 nearing completion. All 5 character components wired. Stamina HUD, footstep surface detection, and full interaction pipeline operational. One plan remaining (01-06).

**Key Constraints:**
- Engine: Unreal Engine 5.7.2
- Quality: High-quality C++ with performance reviews after each phase
- AI: State Tree (not Behavior Trees)
- Rendering: Lumen GI + Virtual Shadow Maps
- Player: First-person with visible body

---

## Current Position

**Phase:** 1 of 8 (Foundation)
**Plan:** 5 of 6 complete
**Status:** In progress
**Last activity:** 2026-02-08 - Completed 01-05-PLAN.md (Footsteps, Stamina HUD, Integration)

**Progress:**
```
Phase 1: [#####.] 5/6 plans complete
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

### Technical Discoveries

*None yet. Will be populated during implementation.*

### TODOs

- [x] Plan Phase 1: Foundation
- [x] Research UE5.7 first-person rendering before Phase 1 plans
- [x] Execute 01-03-PLAN.md (Stamina, Head-Bob, Lean Components)
- [x] Execute 01-05-PLAN.md (Footsteps, Stamina HUD, Integration)
- [ ] Execute remaining Phase 1 plan (01-06)

### Blockers

*None currently.*

---

## Session Continuity

### Last Session

**Date:** 2026-02-08
**Completed:**
- Executed 01-05-PLAN.md (Footsteps, Stamina HUD, Integration)
  - UFootstepComponent: timer-based trigger, surface detection via bReturnPhysicalMaterial, sprint/crouch multipliers, OnFootstep delegate
  - UStaminaBarWidget: progress bar auto-show/hide with 2s delay, optional FadeAnimation
  - ASereneHUD: creates widgets, binds StaminaComponent + InteractionComponent delegates
  - All 5 components created in ASereneCharacter constructor
  - ASereneGameMode sets HUDClass = ASereneHUD
  - Component status logging in BeginPlay

**Next:** Execute 01-06-PLAN.md (final Foundation plan)

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

Phase 1 has 6 plans. Plans 01-01 through 01-05 complete. The project now has:
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

All 5 character components wired: Stamina, HeadBob, Lean, Interaction, Footstep.
All 29 v1 requirements are mapped. No orphans.

---

*State initialized: 2026-02-07*
*Last updated: 2026-02-08 (01-05 completion)*
