# Phase 3: Hiding System - Context

**Gathered:** 2026-02-10
**Status:** Ready for planning

<domain>
## Phase Boundary

Player can hide from threats in designated hiding spots (lockers, closets, under beds). A visibility system based on ambient light and player state determines how detectable the player is. Monster AI perception integration happens in Phase 4/5 — this phase builds the hiding infrastructure and visibility scoring.

</domain>

<decisions>
## Implementation Decisions

### Entry & exit flow
- Uses existing interaction system — walk up, see "Hide in locker" prompt, press F to enter
- Animated entry — player character plays a get-in animation while camera follows (Outlast-style)
- Animated exit — matching get-out animation (open door, crawl out) before returning to first-person
- Player is always free to exit — no restrictions, even if monster is nearby
- Entry/exit animations should be per-hiding-spot-type (locker door swing vs crawling under bed)

### Inside-hiding experience
- Limited look-around — camera can rotate within a restricted range while hidden
- Peek visibility depends on hiding spot type:
  - Locker: see through slats (horizontal bars of light/view)
  - Closet: dark interior, limited crack of light from door edge
  - Under-bed: floor-level view, can see feet/legs passing by
- Passive hiding — no active input required (no hold-breath mini-game)
- Tension comes from watching the monster through limited visibility
- Full inventory access while hidden — player can open and use inventory normally

### Visibility & darkness system
- Gradient visibility score (0.0 to 1.0) based on ambient light hitting the player
- Darker areas = lower visibility score = harder for AI to detect
- No explicit HUD indicator — player infers visibility from environment darkness
- Crouching reduces visibility (smaller profile + implied quieter movement)
- Flashlight (Phase 6) will increase player's own visibility score — creates risk/reward tension
- Visibility score is a component that AI perception (Phase 4) will read from

### Hiding spot discovery & reuse
- Monster can discover player in a hiding spot — opens locker/reaches under bed (leads to game over or chase)
- Monster remembers spots where it found the player — checks those first in future searches (escalating tension)
- Three hiding spot types for the demo: locker, closet, under-bed
- Each type has unique visuals, camera angles, entry/exit animations, and peek behavior

### Claude's Discretion
- Exact animation montage structure and blend times
- Camera lerp/constraint implementation details
- Visibility score calculation algorithm (how to sample light)
- Hiding spot collision/trigger volume design
- How "monster remembers" data is stored (blackboard, component, etc.)

</decisions>

<specifics>
## Specific Ideas

- Entry/exit should feel like Outlast — animated, not instant teleportation
- Each hiding spot type should have a genuinely different feel (locker slats vs floor-level under-bed view)
- The visibility system should make darkness feel like a real tool, not just aesthetic — player should want to stay in shadows
- Monster discovery is important for tension — hiding spots are not invincible safe zones

</specifics>

<deferred>
## Deferred Ideas

- Monster AI perception reading visibility score — Phase 4
- Monster chase/search behavior when discovering player — Phase 5
- Flashlight affecting player visibility — Phase 6 (visibility component interface ready, integration later)

</deferred>

---

*Phase: 03-hiding-system*
*Context gathered: 2026-02-10*
