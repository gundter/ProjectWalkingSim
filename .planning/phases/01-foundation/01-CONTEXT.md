# Phase 1: Foundation - Context

**Gathered:** 2026-02-08
**Status:** Ready for planning

<domain>
## Phase Boundary

Core C++ architecture and player character operational. First-person controller with visible body, movement system (walk/crouch/sprint/stamina), head-bob camera with accessibility toggle, interaction system with core object types, and lean mechanic. No health system, no damage, no AI — just the player and the world.

</domain>

<decisions>
## Implementation Decisions

### Movement feel
- Heavy and grounded movement (Outlast as reference). Slight acceleration/deceleration, physical feel
- Sprint is ~2x walk speed — significant burst, feels urgent
- Stamina: moderate drain during sprint, moderate regen after a short delay post-sprint
- Stamina regens in any stance (standing, walking, crouched) — only blocked while sprinting
- When stamina depletes: forced walk speed + audible heavy breathing (vulnerable window)
- Crouching is significantly quieter than walking — primary stealth mode
- No crouch-sprint — crouching is always slow and deliberate
- Crouch: player's choice in settings (toggle or hold), default to toggle
- No jump — grounded movement only
- Context-sensitive obstacle interactions (climbing over debris, squeezing through gaps) for navigation and map loading/transitions
- Obstacle transitions use first-person animated sequences (hands on ledge, squeezing through)
- Player is safe during obstacle transitions (no monster interruption)
- Lean left/right (Q/E) — camera-only peek, no collision/visibility exposure
- Lean works both standing and crouched
- Sprinting and heavy breathing generate gameplay-relevant noise for monster AI (implemented in later phases, but system designed for it)
- Surface-dependent footstep sounds (wood, concrete, gravel) — different audio per physical material
- Stamina only in Phase 1, health system added when monster deals damage (Phase 4/5)

### Visible body rendering
- Full body visible when looking down (arms, torso, legs, feet)
- Player casts full shadow from all light sources
- Target character: realistic adult male detective
- Phase 1 uses UE5 mannequin as stand-in — swap to detective model when art is ready

### Interaction design
- Two-stage interaction prompt: reticle dot fades in/out based on screen-center proximity to interactable, world-space prompt ("E: Open") appears once dot is fully visible
- Close interaction range (~1.5m) — must be right next to objects
- Doors: animated open/close (press interact, door plays animation). Not physics-based
- Foundation interactable types: doors, pickups, readables (notes/documents), drawers/cabinets

### Head-bob & camera
- Subtle head-bob by default — gentle sway that adds life without drawing attention
- Head-bob scales with movement speed (walk = minimal, sprint = more pronounced)
- Head-bob can be toggled off in accessibility settings
- Standard FOV (~90 degrees)
- Static FOV — no dynamic changes during sprint or crouch

### HUD
- Minimal HUD style — stamina bar only shows while sprinting then fades, clean screen most of the time
- Stamina feedback: HUD bar (appears during sprint) + audio cues (breathing intensifies as stamina depletes)
- No screen-edge visual effects for stamina

### Claude's Discretion
- Exact acceleration/deceleration curves for movement
- Stamina drain/regen rates and delay timing
- Interaction dot fade radius and timing
- Head-bob animation curves and intensity values
- Lean angle and camera offset amounts
- Obstacle transition animation specifics
- Drawer/cabinet open behavior (slide vs swing)
- Footstep sound variation per surface type

</decisions>

<specifics>
## Specific Ideas

- Movement reference: Outlast — responsive but weighty, good sprint-to-hide loop, stamina feels urgent
- Obstacle interactions like Resident Evil / Hellblade — short first-person animations for climbing/squeezing
- Interaction system: no reticle clutter. Dot appears only when near an interactable, then world-space prompt on lock-on
- Full body visibility for immersion — player should feel physically present in the space
- Surface-dependent footsteps from Phase 1 (not deferred to audio phase) — foundation for AI noise detection later

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 01-foundation*
*Context gathered: 2026-02-08*
