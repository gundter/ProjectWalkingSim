# The Juniper Tree

## What This Is

A psychological horror game adapting the Brothers Grimm fairytale "The Juniper Tree" into a modern, first-person experience. The player is a detective investigating a missing boy case, gradually discovering through unreliable narrator elements that they ARE the murdered boy. A Wendigo monster (the father, transformed by cannibalism) stalks and torments the player throughout.

## Core Value

The player must feel the dread of being hunted while slowly questioning their own reality and identity.

## Requirements

### Validated

- Project structure with custom GameMode (`ASereneGameMode`) — existing
- Enhanced Input System configured — existing
- High-fidelity rendering pipeline (Lumen GI, ray tracing capable) — existing

### Active

- [ ] First-person character with visible body (FP + TP animation support)
- [ ] Inventory system: 8 slots, use/combine/info/discard actions
- [ ] Items drop into world when discarded, can be picked back up
- [ ] Item types: key items, healing items, light sources (batteries/matches)
- [ ] Light management: darkness/light affects visibility and hiding
- [ ] Hiding system: context-sensitive hide spots (closets, under beds, lockers)
- [ ] Hiding system: line-of-sight/darkness-based concealment
- [ ] Wendigo monster AI: patrol, search, chase behaviors
- [ ] Wendigo monster: uses existing 3D model from Fab marketplace
- [ ] Story hook: opening sequence establishing detective narrative
- [ ] Demo environment: one detailed atmospheric location
- [ ] Save/load system for demo progression

### Out of Scope

- Combat system — this is hide/avoid only, no fighting
- Multiple monsters — single Wendigo creates focused dread
- Multiplayer — single-player experience only
- VR support — traditional first-person only for v1
- Full 3-5 hour game — demo first, full game later

## Context

**Source Material:** The Juniper Tree (Brothers Grimm) - a dark tale of a boy murdered by his stepmother, fed to his father as stew, and reborn as a vengeful bird. The cannibalism element connects to the Wendigo mythology.

**Narrative Structure:** Detective investigating missing boy → strange occurrences → reality breaks down → revelation that player IS the boy → confrontation with the truth.

**Monster Asset:** Wendigo model from Fab marketplace (https://www.fab.com/listings/cd6f16df-2b85-4776-9b20-440642f10d7b) represents the father transformed by unknowing cannibalism.

**Technical Environment:** Unreal Engine 5.7.2 with Enhanced Input, Lumen GI, ray tracing support. Single-module C++ project with Public/Private separation.

**Codebase State:** Early-stage project with minimal custom code. `ASereneGameMode` exists as empty GameMode subclass. All major systems need to be built.

## Constraints

- **Engine**: Unreal Engine 5.7.2 — project is already configured for this version
- **Quality**: High-quality C++ architecture with performance consideration — code must be production-grade
- **Performance**: Performance reviews required after each phase — non-negotiable checkpoint
- **Animation**: First-person with visible body requires FP + TP animation support — affects character architecture
- **Asset**: Wendigo model from Fab marketplace — monster visuals are fixed, AI/behavior is custom

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Detective as player identity | Provides narrative frame for investigation, enables unreliable narrator twist | — Pending |
| Father as Wendigo | Connects cannibalism from fairy tale to Wendigo mythology, tragic horror | — Pending |
| Single monster design | Focused dread, player learns patterns, more memorable than multiple threats | — Pending |
| Demo before full game | Validate core loop, gather feedback, potential funding before full production | — Pending |
| Hiding spots + line of sight | Two complementary systems: guaranteed safety vs. skillful evasion | — Pending |

---
*Last updated: 2026-02-07 after initialization*
