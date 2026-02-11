# Phase 4: Monster AI Core - Context

**Gathered:** 2026-02-11
**Status:** Ready for planning

<domain>
## Phase Boundary

Wendigo patrols designated zones using State Tree AI, perceives the player through sight and hearing, and reacts with a three-level alert system. Full chase, investigation, and search behaviors are Phase 5. Audio polish is Phase 6.

</domain>

<decisions>
## Implementation Decisions

### Patrol feel
- Fixed patrol routes that players can learn and predict (not random)
- Slow and deliberate pace — slower than player walk speed
- Frequent pauses during patrol to look around, sniff, scan
- Minor detours off-route during pauses (check a corner, approach a doorway) before returning to route

### Detection rules
- Gradual suspicion buildup — brief glimpses don't trigger full alert, sustained exposure does
- Visibility score integration: threshold + scaling combined
  - Below threshold (~0.3): Wendigo cannot see the player at all
  - Above threshold: visibility score scales detection speed (higher visibility = faster detection)
- Hearing is forgiving: only sprinting generates audible noise. Walking and crouching are silent.
- Sight cone: 90° standard field of view for demo
- Sight range: Claude's discretion on exact distance values

### Alert response
- Three alert levels: Patrol → Suspicious → Alert
- On detection threshold: stop and orient toward stimulus (menacing pause)
- Suspicious state: abandons patrol route, moves toward stimulus location
- Slow suspicion decay (10-20 seconds) — near-misses create extended tension
- Alert state is the handoff point for Phase 5 behaviors (chase, investigate, search)

### Monster presence
- Tall and looming: 8-9ft, significantly taller than player, towers over doorways
- Movement feel: eerily human but heavy — was the father, now transformed
  - Almost human movement patterns but with weight and something slightly off
  - Uncanny valley quality adds psychological horror layer
- Ambient sounds while patrolling: heavy breathing, distinct footsteps, occasional growls
  - Placeholder audio for Phase 4; Phase 6 replaces with polished spatial audio
  - Player can hear it approaching before seeing it

### Claude's Discretion
- Exact patrol waypoint placement and route design
- Suspicion threshold values and decay curves
- Sight range distance
- Specific pause frequency and detour behavior timing
- NavMesh and State Tree architecture
- Placeholder audio selection

</decisions>

<specifics>
## Specific Ideas

- Existing Wendigo asset has preloaded animations — investigate what's available before creating new ones
- The Wendigo is the MC's transformed father (cannibalism → Wendigo curse) — eerily human quality reflects this narrative
- Alien: Isolation reference for dual sight cones (main eyes + rear proximity) — deferred to full build, not demo
- Audio is a weak spot in user's knowledge but critical to the game — Phase 6 should deeply explore UE5.7 audio capabilities and treat audio as first-class

</specifics>

<deferred>
## Deferred Ideas

- Dual sight cone system (Alien: Isolation style) with secondary rear proximity detection — full build enhancement, not demo
- Full spatial audio system with proper attenuation — Phase 6
- Advanced animation blending for state transitions — Phase 5 or later
- Monster audio cues (detailed breathing patterns, growl variations) — Phase 6

</deferred>

---

*Phase: 04-monster-ai-core*
*Context gathered: 2026-02-11*
