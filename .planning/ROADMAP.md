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

**Goal:** Core C++ architecture and player character are operational.

**Dependencies:** None (starting phase)

**Requirements:**
- PLYR-01: First-person with visible body
- PLYR-02: Movement with crouch, sprint, stamina
- PLYR-03: Head-bob camera with motion sickness option
- PLYR-04: Interaction system

**Success Criteria:**
1. Player can walk, run, crouch in first-person with visible arms/hands when looking down
2. Stamina depletes during sprint and regenerates when walking/standing
3. Head-bob can be toggled off in options without affecting gameplay
4. Player can approach objects and see interaction prompts appear
5. Core interfaces (IInteractable, IHideable, ISaveable) compile and can be implemented

---

### Phase 2: Inventory System

**Goal:** Player can collect, manage, and use items in an 8-slot inventory.

**Dependencies:** Phase 1 (interaction system required)

**Requirements:**
- INVT-01: 8-slot inventory with UI
- INVT-02: Item actions (use, combine, info, discard)
- INVT-03: Discarded items drop into world
- INVT-04: Item types (key items, healing, flashlight)

**Success Criteria:**
1. Player can pick up items and see them appear in inventory UI
2. Player can use items (healing restores health, keys unlock doors)
3. Player can discard items and see them appear in the world as pickups
4. Inventory displays item info on hover/selection
5. Inventory is limited to 8 slots and rejects items when full

---

### Phase 3: Hiding System

**Goal:** Player can hide from threats in designated hiding spots.

**Dependencies:** Phase 1 (player controller, interaction)

**Requirements:**
- HIDE-01: Context-sensitive hiding spots (lockers, closets, under beds)
- HIDE-02: Line of sight / darkness concealment

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

**Success Criteria:**
1. Wendigo follows patrol route through designated zones
2. Wendigo visually detects player who is visible and not hiding
3. Wendigo reacts to loud player sounds (running, knocking objects)
4. State Tree drives behavior transitions observable in debug view

---

### Phase 5: Monster Behaviors

**Goal:** Wendigo exhibits full range of hunt behaviors.

**Dependencies:** Phase 4 (core AI required)

**Requirements:**
- WNDG-03: Investigate sounds and visual disturbances
- WNDG-04: Chase player when spotted
- WNDG-05: Search behavior when player escapes/hides
- WNDG-06: Multiple spawn locations with per-spawn patrol zones

**Success Criteria:**
1. Wendigo investigates location of sounds before returning to patrol
2. Wendigo chases spotted player at increased speed
3. Wendigo searches area when player breaks line of sight or enters hiding
4. Wendigo can spawn at different locations with zone-appropriate patrol routes
5. Player can escape chase by hiding and waiting for search to end

---

### Phase 6: Light and Audio

**Goal:** Atmosphere systems create dread and inform player of threats.

**Dependencies:** Phase 5 (AI needs audio/light integration for perception)

**Requirements:**
- LGHT-01: Flashlight (always on for demo)
- LGHT-02: Lumen GI atmospheric lighting
- AUDO-01: Spatial audio (3D monster sounds)
- AUDO-02: Ambient horror soundscape
- AUDO-03: Monster audio cues (footsteps, breathing, growls)
- AUDO-04: Dynamic music and tension stingers

**Success Criteria:**
1. Player flashlight illuminates environment with Lumen GI bounce
2. Environment has atmospheric horror lighting (dark shadows, moody)
3. Monster footsteps have 3D positioning (player can locate by sound)
4. Ambient soundscape plays continuously without obvious loops
5. Music intensity increases when monster is near or chasing

---

### Phase 7: Save System

**Goal:** Player can save progress and resume play.

**Dependencies:** Phase 6 (all saveable systems complete)

**Requirements:**
- SAVE-01: Checkpoint auto-saves
- SAVE-02: Manual save anywhere

**Success Criteria:**
1. Game auto-saves when player reaches checkpoint locations
2. Player can manually save from pause menu
3. Loading a save restores player position, inventory, and world state
4. Save/load does not cause crashes or corrupt state

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
| 1 | Foundation | PLYR-01, PLYR-02, PLYR-03, PLYR-04 | Not Started |
| 2 | Inventory System | INVT-01, INVT-02, INVT-03, INVT-04 | Not Started |
| 3 | Hiding System | HIDE-01, HIDE-02 | Not Started |
| 4 | Monster AI Core | WNDG-01, WNDG-02, WNDG-07 | Not Started |
| 5 | Monster Behaviors | WNDG-03, WNDG-04, WNDG-05, WNDG-06 | Not Started |
| 6 | Light and Audio | LGHT-01, LGHT-02, AUDO-01, AUDO-02, AUDO-03, AUDO-04 | Not Started |
| 7 | Save System | SAVE-01, SAVE-02 | Not Started |
| 8 | Demo Polish | DEMO-01, DEMO-02, DEMO-03, DEMO-04 | Not Started |

**Coverage:** 29/29 requirements mapped

---

*Roadmap created: 2026-02-07*
*Last updated: 2026-02-07*
