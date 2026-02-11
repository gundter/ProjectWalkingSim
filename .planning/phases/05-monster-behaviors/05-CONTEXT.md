# Phase 5: Monster Behaviors - Context

**Gathered:** 2026-02-11
**Status:** Ready for planning

<domain>
## Phase Boundary

Wendigo exhibits full range of hunt behaviors: chase player when spotted, investigate sounds and visual disturbances with differentiated reactions, search area when player escapes or hides, and spawn at designated locations with zone-based patrol assignment. Phase 4's perception/patrol foundation becomes a complete predator loop.

</domain>

<decisions>
## Implementation Decisions

### Chase dynamics
- Grab attack on catch: Wendigo grabs player, brief struggle animation, then death
- Wendigo can open doors during chase — no room is truly safe unless locked
- Chase speed ~10-15% faster than player sprint (player can't outrun forever but has time to find hiding)
- Chase ends when player breaks line of sight + timer delay (a few seconds of no LOS before transitioning to search)
- Chase can re-acquire if player peeks out before timer expires
- If Wendigo saw the player enter a hiding spot, it should check that spot and grab the player
- If Wendigo didn't witness the hiding, hiding spots are safe havens

### Search patterns
- Wendigo goes to last-known position (sight or sound source), then checks 2-3 random nearby locations
- Random nearby points prevent predictable search patterns
- Search duration: short (15-20 seconds) before returning to patrol
- After search completes, Wendigo returns to its predetermined patrol route (nearest waypoint)
- Hiding spot checking only if Wendigo witnessed player entering — otherwise hiding spots are safe

### Investigation depth
- Different reactions for sight vs sound stimuli:
  - Sound: cautious approach, slower, head-scanning
  - Sight: faster, more direct, higher urgency
- Brief linger at investigation location if nothing found (pause, look around, then return)
- Investigation escalates immediately to chase if Wendigo spots the player — no alert transition delay
- Audio + visual tells for investigation behavior:
  - Visual tells (head movement, speed, stance changes) are Phase 5 scope
  - Audio tells (growls, sniffing) — Phase 5 adds behavior hooks/events, Phase 6 wires actual sounds (AUDO-03)

### Spawn & zone design
- One main Wendigo active at a time for the demo
- Fixed spawn location for demo, but build the spawn point system for future use
- SpawnPoint actors with zone-based patrol route selection (not 1:1 spawn-to-route mapping)
- Wendigo picks from available patrol routes within its spawn zone
- System supports multiple spawn points per level (future Director AI will choose)

### Claude's Discretion
- Exact chase speed value (target ~10-15% faster than sprint)
- Timer duration for LOS-lost chase timeout
- Number and selection of random search points
- SpawnPoint/zone actor implementation details
- How "witnessed hiding" is tracked (perception event vs direct flag)
- Investigation linger duration at empty locations

</decisions>

<specifics>
## Specific Ideas

- Chase-to-search transition should feel like Alien: Isolation — the Wendigo doesn't just give up, it actively hunts the area
- Player should be able to read the Wendigo's behavior state through visual tells (learned gameplay)
- The grab attack creates a cinematic death moment, not just a damage number
- Patrol routes are placeholder for demo — main release will likely use more dynamic AI navigation

</specifics>

<deferred>
## Deferred Ideas

- Minor enemies with simpler patrol patterns between main Wendigo encounters — new enemy type, own phase
- Director AI for dynamic spawn location selection — future system beyond demo scope
- Dynamic patrol routes replacing static waypoints — main release consideration
- Chase end condition using LOS + distance combo — revisit for main release (LOS + timer for demo)

</deferred>

---

*Phase: 05-monster-behaviors*
*Context gathered: 2026-02-11*
