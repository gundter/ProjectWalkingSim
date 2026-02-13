# Phase 8: Demo Polish - Context

**Gathered:** 2026-02-13
**Status:** Ready for planning

<domain>
## Phase Boundary

Complete, polished demo experience ready for players. Abandoned rural farmstead environment, opening narrative hook via detective monologue, environmental storytelling through notes/photos/interactable objects, and demo ending with discovery cliffhanger. All gameplay systems (Phases 1-7) are built; this phase assembles the experience.

</domain>

<decisions>
## Implementation Decisions

### Environment Layout
- Rural cabin/farmstead setting — isolated property with main cabin, cellar, barn, yard, shed
- Surrounding woods areas for Wendigo introduction — exposed, dangerous feeling contrast to claustrophobic cabin
- Moderate scale (5-6 indoor areas + outdoor woods sections)
- Player starts outside/near the cabin, investigates cabin first, then woods section mid-demo, returns to cabin or another building for climax
- Locked doors with keys for progression gating — uses existing Phase 2 locked door + key item systems

### Narrative Delivery
- Opening: audio monologue from the detective as player begins (establishing the missing boy case)
- Mid-exploration: mix of readable notes + triggered internal monologues when detective sees significant things
- Voice acting: placeholder TTS/AI voice for the demo build, replaceable later with real VO
- Twist pacing: subtle hints toward the end only — detective says things he shouldn't know, objects feel oddly familiar — but no explicit reveal in the demo

### Storytelling Objects
- Three types: readable notes/letters, photographs, interactable objects (child's toy, bloodstain, etc.)
- Full-screen inspection mode — camera zooms to object, player reads/examines, key press to return
- Some objects required for progression (e.g., a note with a door code), most are optional discovery
- 5-8 total storytelling objects — sparse, each one significant and memorable
- Quality over quantity — every object should advance the story or deepen the atmosphere

### Demo Arc and Ending
- Target length: 25-35 minutes for first-time player
- Wendigo becomes active threat at midpoint (~15 min) — player explores cabin safely first, then Wendigo appears during or after woods section
- Ending: discovery cliffhanger — player finds something deeply unsettling (tied to the boy/the truth). Cut to black.
- End screen: fade to black, then title card ("The Juniper Tree"). Clean, no stats or credits.

### Claude's Discretion
- Specific room layouts and furniture placement within the farmstead
- Exact note/letter content and wording
- Hiding spot placement throughout the environment
- Wendigo patrol routes and spawn points for the demo map
- Save point (tape recorder) placement
- Performance optimization approach for 60 FPS target
- Sound design for the woods section and ambient atmosphere
- Specific monologue trigger locations and timing

</decisions>

<specifics>
## Specific Ideas

- The Juniper Tree folktale setting: rural cabin/farmstead matches the source material's domestic horror
- Woods as the Wendigo's domain — being outdoors should feel exposed and dangerous
- Detective's monologue establishes the case but also plants seeds of wrongness (he knows too much, remembers things he shouldn't)
- Discovery at demo end should be something visceral and identity-shaking — tied to the boy's fate
- Progression flow: cabin exterior → cabin interior investigation → woods (Wendigo encounter) → return to cabin/building for climax discovery

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 08-demo-polish*
*Context gathered: 2026-02-13*
