# State: The Juniper Tree

**Purpose:** Session continuity and project memory for Claude.

---

## Project Reference

**Core Value:** The player must feel the dread of being hunted while slowly questioning their own reality and identity.

**Current Focus:** Roadmap complete. Ready to begin Phase 1: Foundation.

**Key Constraints:**
- Engine: Unreal Engine 5.7.2
- Quality: High-quality C++ with performance reviews after each phase
- AI: State Tree (not Behavior Trees)
- Rendering: Lumen GI + Virtual Shadow Maps
- Player: First-person with visible body

---

## Current Position

**Phase:** 1 - Foundation (Not Started)
**Plan:** None (phase not yet planned)
**Status:** Awaiting `/gsd:plan-phase 1`

**Progress:**
```
[........] 0/8 phases complete
```

---

## Performance Metrics

| Phase | Plans | Tasks | Time | Issues |
|-------|-------|-------|------|--------|
| - | - | - | - | - |

*Metrics populated as phases complete.*

---

## Accumulated Context

### Key Decisions

| Decision | Rationale | Phase |
|----------|-----------|-------|
| State Tree for AI | Epic's modern replacement for BT; on-demand evaluation | Roadmap |
| Component-based architecture | Decoupled, testable, reusable | Roadmap |
| 8 phases | Natural delivery boundaries from requirements | Roadmap |
| Monster AI split into 2 phases | Core + behaviors separates foundation from complexity | Roadmap |

### Technical Discoveries

*None yet. Will be populated during implementation.*

### TODOs

- [ ] Plan Phase 1: Foundation
- [ ] Research UE5.7 first-person rendering before Phase 1 plans

### Blockers

*None currently.*

---

## Session Continuity

### Last Session

**Date:** 2026-02-07
**Completed:**
- Project initialization
- Requirements defined (29 v1 requirements)
- Research completed (STACK, FEATURES, ARCHITECTURE, PITFALLS)
- Roadmap created (8 phases)

**Next:** Plan Phase 1 with `/gsd:plan-phase 1`

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

All 29 v1 requirements are mapped. No orphans.

---

*State initialized: 2026-02-07*
*Last updated: 2026-02-07*
