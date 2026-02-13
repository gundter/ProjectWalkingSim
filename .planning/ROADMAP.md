# Roadmap: The Juniper Tree

**Created:** 2026-02-07
**Depth:** Standard (5-8 phases)
**Core Value:** The player must feel the dread of being hunted while slowly questioning their own reality and identity.

---

## Overview

This roadmap delivers a playable horror demo through 8 phases that build from core infrastructure to complete experience. The player controller and inventory systems enable survival mechanics. The hiding system and monster AI create the core gameplay loop of tension and evasion. Light, audio, and save systems support atmosphere and persistence. Final polish integrates all systems into a cohesive demo.

---

## Phases

### Phase 1: Foundation

**Goal:** Core C++ architecture and player character are operational. First-person controller with visible body, movement system (walk/crouch/sprint/stamina), head-bob camera with accessibility toggle, lean mechanic, interaction system with four actor types, and footstep surface detection.

**Dependencies:** None (starting phase)

**Requirements:**
- PLYR-01: First-person with visible body
- PLYR-02: Movement with crouch, sprint, stamina
- PLYR-03: Head-bob camera with motion sickness option
- PLYR-04: Interaction system

**Plans:** 6 plans

Plans:
- [x] 01-01-PLAN.md -- Project scaffolding, Build.cs, interfaces, tags, log channels
- [x] 01-02-PLAN.md -- Character with First Person Rendering, PlayerController, Enhanced Input
- [x] 01-03-PLAN.md -- Stamina, head-bob, and lean components
- [x] 01-04-PLAN.md -- Interaction component and four interactable actor types
- [x] 01-05-PLAN.md -- Footstep component, stamina HUD, full system wiring
- [x] 01-06-PLAN.md -- Data assets, Blueprint subclasses, UMG widgets, integration test

**Success Criteria:**
1. Player can walk, run, crouch in first-person with visible arms/hands when looking down
2. Stamina depletes during sprint and regenerates when walking/standing
3. Head-bob can be toggled off in options without affecting gameplay
4. Player can approach objects and see interaction prompts appear
5. Core interfaces (IInteractable, IHideable, ISaveable) compile and can be implemented

---

### Phase 2: Inventory System

**Goal:** Player can collect, manage, and use items in an 8-slot real-time inventory with horizontal UI, key/code item types, locked door integration, and two-step combine flow.

**Dependencies:** Phase 1 (interaction system required)

**Requirements:**
- INVT-01: 8-slot inventory with UI
- INVT-02: Item actions (use, combine, info, discard)
- INVT-03: Discarded items drop into world
- INVT-04: Item types (key items, healing, flashlight)

**Plans:** 6 plans

Plans:
- [x] 02-01-PLAN.md -- Inventory data foundation: types, item data asset, inventory component
- [x] 02-02-PLAN.md -- Inventory UI widgets: slot, tooltip, container (C++ base classes)
- [x] 02-03-PLAN.md -- Actor integration: PickupActor inventory add, DoorActor locked doors
- [x] 02-04-PLAN.md -- HUD + controller wiring: toggle, input mode, delegate binding
- [x] 02-05-PLAN.md -- Combine system + keyboard navigation
- [x] 02-06-PLAN.md -- Editor setup: input assets, Asset Manager config, UMG Blueprints, verification

**Success Criteria:**
1. Player can pick up items and see them appear in inventory UI
2. Player can use items (keys unlock doors)
3. Player can discard items and see them appear in the world as pickups
4. Inventory displays item info on hover/selection
5. Inventory is limited to 8 slots and rejects items when full

---

### Phase 3: Hiding System

**Goal:** Player can hide from threats in designated hiding spots with animated entry/exit, constrained look-around while hidden, and a gradient visibility score based on ambient light that AI perception (Phase 4) will read.

**Dependencies:** Phase 1 (player controller, interaction)

**Requirements:**
- HIDE-01: Context-sensitive hiding spots (lockers, closets, under beds)
- HIDE-02: Line of sight / darkness concealment

**Plans:** 6 plans

Plans:
- [x] 03-01-PLAN.md -- Foundation types: IHideable expansion, HidingSpotDataAsset, tags, EHidingState
- [x] 03-02-PLAN.md -- VisibilityScoreComponent with SceneCapture light sampling
- [x] 03-03-PLAN.md -- HidingSpotActor base class with dual IInteractable + IHideable interface
- [x] 03-04-PLAN.md -- HidingComponent state machine (entry/exit, camera, montage, input)
- [x] 03-05-PLAN.md -- Character integration: Tick bypass, component wiring, controller exit
- [x] 03-06-PLAN.md -- Editor assets: data assets, input actions, Blueprint config, PIE verification

**Success Criteria:**
1. Player can enter a locker/closet and view shifts to inside perspective
2. Player can exit hiding spot and return to normal first-person view
3. Multiple hiding spot types work (locker, closet, under-bed)
4. Player in darkness is harder to detect than player in light (visibility system)

---

### Phase 4: Monster AI Core

**Goal:** Wendigo patrols, perceives, and reacts to the player.

**Dependencies:** Phase 3 (hiding system required for testing chase/escape)

**Requirements:**
- WNDG-01: State Tree-driven AI controller
- WNDG-02: Patrol behavior within zones
- WNDG-07: AI Perception system (sight + hearing)

**Plans:** 7 plans

Plans:
- [x] 04-01-PLAN.md -- AI module dependencies, plugin enables, AI types, gameplay tags
- [x] 04-02-PLAN.md -- WendigoCharacter pawn + SuspicionComponent
- [x] 04-03-PLAN.md -- WendigoAIController with StateTree + AI Perception
- [x] 04-04-PLAN.md -- PatrolRouteActor + patrol State Tree tasks
- [x] 04-05-PLAN.md -- Perception-to-suspicion wiring + sprint noise reporting
- [x] 04-06-PLAN.md -- Investigation tasks + orient task + suspicion condition
- [x] 04-07-PLAN.md -- NavMesh config, Blueprint creation, State Tree asset, PIE verification

**Success Criteria:**
1. Wendigo follows patrol route through designated zones
2. Wendigo visually detects player who is visible and not hiding
3. Wendigo reacts to loud player sounds (running, knocking objects)
4. State Tree drives behavior transitions observable in debug view

---

### Phase 5: Monster Behaviors

**Goal:** Wendigo exhibits full range of hunt behaviors: chase on sight, search when player escapes, investigate stimuli with differentiated sight/sound reactions, spawn at designated locations with zone-based patrol selection.

**Dependencies:** Phase 4 (core AI required)

**Requirements:**
- WNDG-03: Investigate sounds and visual disturbances
- WNDG-04: Chase player when spotted
- WNDG-05: Search behavior when player escapes/hides
- WNDG-06: Multiple spawn locations with per-spawn patrol zones

**Plans:** 5 plans

Plans:
- [x] 05-01-PLAN.md -- AI types extension: enums, constants, WendigoCharacter state, SuspicionComponent stimulus tracking
- [x] 05-02-PLAN.md -- Chase, Search, and ReturnToNearestWaypoint State Tree tasks
- [x] 05-03-PLAN.md -- GrabAttack task, StimulusType condition, investigation enhancement
- [x] 05-04-PLAN.md -- SpawnPoint actor, AI controller witnessed-hiding, AI door interaction
- [x] 05-05-PLAN.md -- State Tree update, SpawnPoint Blueprint, PIE verification

**Success Criteria:**
1. Wendigo investigates location of sounds before returning to patrol
2. Wendigo chases spotted player at increased speed
3. Wendigo searches area when player breaks line of sight or enters hiding
4. Wendigo can spawn at different locations with zone-appropriate patrol routes
5. Player can escape chase by hiding and waiting for search to end

---

### Phase 6: Light and Audio

**Goal:** Atmosphere systems create dread and inform player of threats. Flashlight with Lumen GI bounce, behavior-state-driven monster spatial audio, alert-level-driven dynamic music with crossfade, ambient horror soundscape, player footstep audio, heartbeat proximity system, and flashlight-attracts-Wendigo mechanic.

**Dependencies:** Phase 5 (AI needs audio/light integration for perception)

**Requirements:**
- LGHT-01: Flashlight (always on for demo)
- LGHT-02: Lumen GI atmospheric lighting
- AUDO-01: Spatial audio (3D monster sounds)
- AUDO-02: Ambient horror soundscape
- AUDO-03: Monster audio cues (footsteps, breathing, growls)
- AUDO-04: Dynamic music and tension stingers

**Plans:** 5 plans

Plans:
- [x] 06-01-PLAN.md -- FlashlightComponent + AudioConstants foundation
- [x] 06-02-PLAN.md -- MonsterAudioComponent (behavior-state-driven spatial audio)
- [x] 06-03-PLAN.md -- MusicTensionSystem + AmbientAudioManager
- [x] 06-04-PLAN.md -- PlayerAudioComponent (footstep playback + heartbeat proximity)
- [x] 06-05-PLAN.md -- Character wiring, flashlight-AI detection, PIE verification

**Success Criteria:**
1. Player flashlight illuminates environment with Lumen GI bounce
2. Environment has atmospheric horror lighting (dark shadows, moody)
3. Monster footsteps have 3D positioning (player can locate by sound)
4. Ambient soundscape plays continuously without obvious loops
5. Music intensity increases when monster is near or chasing

---

### Phase 7: Save System

**Goal:** Manual save-point system at world-placed tape recorders with 3 save slots, screenshot thumbnails, Game Over screen on death that loads last save, and pause menu with Continue/Load Game. Wendigo resets on load. No auto-saves -- purely manual saves create survival horror tension.

**Dependencies:** Phase 6 (all saveable systems complete)

**Requirements:**
- SAVE-01: Save points at tape recorder objects (replaces checkpoint auto-saves per CONTEXT.md)
- SAVE-02: Manual save at tape recorders with 3-slot picker (replaces save-anywhere per CONTEXT.md)

**Plans:** 4 plans

Plans:
- [x] 07-01-PLAN.md -- Save data foundation: SaveTypes, SereneSaveGame, SaveSubsystem
- [x] 07-02-PLAN.md -- ISaveable implementation on actors, death system, GameMode load flow
- [x] 07-03-PLAN.md -- UI widgets: GameOverWidget, SaveSlotWidget, SaveLoadMenuWidget
- [x] 07-04-PLAN.md -- TapeRecorderActor, PauseMenuWidget, Esc binding, final wiring

**Success Criteria:**
1. Player can interact with tape recorder to save game to one of 3 slots
2. Save slot shows screenshot thumbnail and timestamp
3. Death shows Game Over screen with Load Last Save / Quit options
4. Pause menu has Continue (latest save) and Load Game (slot picker)
5. Loading a save restores player position, inventory, and world state (doors, pickups)
6. Save/load does not cause crashes or corrupt state

---

### Phase 8: Demo Polish

**Goal:** Complete, polished demo experience ready for players.

**Dependencies:** Phase 7 (all systems operational)

**Requirements:**
- DEMO-01: Abandoned building environment
- DEMO-02: Opening narrative hook
- DEMO-03: Environmental storytelling (notes, photos, objects)
- DEMO-04: Demo ending/cliffhanger

**Success Criteria:**
1. Demo has complete, explorable abandoned building environment
2. Opening sequence establishes detective premise and hooks player
3. Notes and objects tell story of the missing boy throughout environment
4. Demo has clear ending that leaves player wanting more
5. Performance maintains 60 FPS on target hardware

---

## Progress

| Phase | Name | Requirements | Status |
|-------|------|--------------|--------|
| 1 | Foundation | PLYR-01, PLYR-02, PLYR-03, PLYR-04 | ✓ Complete |
| 2 | Inventory System | INVT-01, INVT-02, INVT-03, INVT-04 | ✓ Complete |
| 3 | Hiding System | HIDE-01, HIDE-02 | ✓ Complete |
| 4 | Monster AI Core | WNDG-01, WNDG-02, WNDG-07 | ✓ Complete |
| 5 | Monster Behaviors | WNDG-03, WNDG-04, WNDG-05, WNDG-06 | ✓ Complete |
| 6 | Light and Audio | LGHT-01, LGHT-02, AUDO-01, AUDO-02, AUDO-03, AUDO-04 | ✓ Complete |
| 7 | Save System | SAVE-01, SAVE-02 | ✓ Complete |
| 8 | Demo Polish | DEMO-01, DEMO-02, DEMO-03, DEMO-04 | Not Started |

**Coverage:** 29/29 requirements mapped

---

*Roadmap created: 2026-02-07*
*Last updated: 2026-02-12 (Phase 7 complete)*
