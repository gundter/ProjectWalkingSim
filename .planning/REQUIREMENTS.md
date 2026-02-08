# Requirements: The Juniper Tree

**Defined:** 2026-02-07
**Core Value:** The player must feel the dread of being hunted while slowly questioning their own reality and identity.

## v1 Requirements (Demo)

Requirements for initial demo release. Each maps to roadmap phases.

### Player Character

- [ ] **PLYR-01**: First-person with visible body (hands, arms, legs visible when looking down)
- [ ] **PLYR-02**: Movement with crouch, sprint, and stamina system
- [ ] **PLYR-03**: Head-bob camera with motion sickness disable option
- [ ] **PLYR-04**: Interaction system (pick up items, open doors, examine objects)

### Inventory

- [ ] **INVT-01**: 8-slot inventory with UI (grid or radial)
- [ ] **INVT-02**: Item actions: use, combine, info, discard
- [ ] **INVT-03**: Discarded items drop into world and can be picked up again
- [ ] **INVT-04**: Item types: key items, healing items, flashlight

### Hiding

- [ ] **HIDE-01**: Context-sensitive hiding spots (lockers, closets, under beds)
- [ ] **HIDE-02**: Line of sight / darkness concealment from monster

### Wendigo Monster

- [ ] **WNDG-01**: State Tree-driven AI controller
- [ ] **WNDG-02**: Patrol behavior within designated patrol zones
- [ ] **WNDG-03**: Investigate sounds and visual disturbances
- [ ] **WNDG-04**: Chase player when spotted
- [ ] **WNDG-05**: Search behavior when player escapes/hides
- [ ] **WNDG-06**: Multiple spawn locations with per-spawn patrol zones
- [ ] **WNDG-07**: AI Perception system (sight + hearing)

### Light System

- [ ] **LGHT-01**: Flashlight (always on, no battery drain for demo)
- [ ] **LGHT-02**: Lumen GI atmospheric lighting

### Audio

- [ ] **AUDO-01**: Spatial audio (3D positioning for monster sounds)
- [ ] **AUDO-02**: Ambient horror soundscape
- [ ] **AUDO-03**: Monster audio cues (footsteps, breathing, growls)
- [ ] **AUDO-04**: Dynamic music and tension stingers

### Save System

- [ ] **SAVE-01**: Checkpoint auto-saves at specific locations
- [ ] **SAVE-02**: Manual save anywhere

### Demo Content

- [ ] **DEMO-01**: Abandoned building environment (one detailed location)
- [ ] **DEMO-02**: Opening narrative hook (establish detective premise)
- [ ] **DEMO-03**: Environmental storytelling (notes, photos, objects)
- [ ] **DEMO-04**: Demo ending/cliffhanger

## v2 Requirements (Full Game)

Deferred to full game release. Tracked but not in current roadmap.

### Light Management

- **LGHT-03**: Battery drain for flashlight
- **LGHT-04**: Matches as temporary light source
- **LGHT-05**: Visibility calculation affects AI detection range

### Advanced Hiding

- **HIDE-03**: Peek from hiding to check safety
- **HIDE-04**: Breath holding mechanic when monster is close

### Advanced AI

- **WNDG-08**: AI Director system (Alien: Isolation style spawn timing)
- **WNDG-09**: Adaptive AI that learns player hiding patterns
- **WNDG-10**: Progressive behavior unlocking (monster gets smarter)

### Psychological Horror

- **PSYC-01**: Unreliable narrator moments (reality questioning)
- **PSYC-02**: Visual/audio distortion effects
- **PSYC-03**: Environmental changes (things move when not looking)

### Save System

- **SAVE-03**: Rolling saves (keep last 3 for soft-lock prevention)
- **SAVE-04**: Save state validation

## Out of Scope

Explicitly excluded. Documented to prevent scope creep.

| Feature | Reason |
|---------|--------|
| Combat system | Destroys horror tension; hide/avoid only |
| Multiple monsters | Single Wendigo creates focused dread |
| VR support | Scope; traditional first-person for v1 |
| Multiplayer | Single-player experience |
| Mobile/console ports | PC first, ports later |

## Traceability

Which phases cover which requirements. Updated during roadmap creation.

| Requirement | Phase | Status |
|-------------|-------|--------|
| PLYR-01 | Phase 1: Foundation | Pending |
| PLYR-02 | Phase 1: Foundation | Pending |
| PLYR-03 | Phase 1: Foundation | Pending |
| PLYR-04 | Phase 1: Foundation | Pending |
| INVT-01 | Phase 2: Inventory System | Pending |
| INVT-02 | Phase 2: Inventory System | Pending |
| INVT-03 | Phase 2: Inventory System | Pending |
| INVT-04 | Phase 2: Inventory System | Pending |
| HIDE-01 | Phase 3: Hiding System | Pending |
| HIDE-02 | Phase 3: Hiding System | Pending |
| WNDG-01 | Phase 4: Monster AI Core | Pending |
| WNDG-02 | Phase 4: Monster AI Core | Pending |
| WNDG-03 | Phase 5: Monster Behaviors | Pending |
| WNDG-04 | Phase 5: Monster Behaviors | Pending |
| WNDG-05 | Phase 5: Monster Behaviors | Pending |
| WNDG-06 | Phase 5: Monster Behaviors | Pending |
| WNDG-07 | Phase 4: Monster AI Core | Pending |
| LGHT-01 | Phase 6: Light and Audio | Pending |
| LGHT-02 | Phase 6: Light and Audio | Pending |
| AUDO-01 | Phase 6: Light and Audio | Pending |
| AUDO-02 | Phase 6: Light and Audio | Pending |
| AUDO-03 | Phase 6: Light and Audio | Pending |
| AUDO-04 | Phase 6: Light and Audio | Pending |
| SAVE-01 | Phase 7: Save System | Pending |
| SAVE-02 | Phase 7: Save System | Pending |
| DEMO-01 | Phase 8: Demo Polish | Pending |
| DEMO-02 | Phase 8: Demo Polish | Pending |
| DEMO-03 | Phase 8: Demo Polish | Pending |
| DEMO-04 | Phase 8: Demo Polish | Pending |

**Coverage:**
- v1 requirements: 29 total
- Mapped to phases: 29
- Unmapped: 0

---
*Requirements defined: 2026-02-07*
*Last updated: 2026-02-07 after roadmap creation*
