# Phase 6: Light and Audio - Context

**Gathered:** 2026-02-12
**Status:** Ready for planning

<domain>
## Phase Boundary

Atmosphere systems that create dread and inform the player of threats. Delivers: flashlight with Lumen GI bounce, atmospheric horror lighting, spatial 3D monster audio, ambient horror soundscape, per-state monster audio cues, and dynamic music with tension stingers. The demo's abandoned building + surrounding forest setting is the target environment.

</domain>

<decisions>
## Implementation Decisions

### Flashlight feel
- Narrow spotlight beam — tight cone, deep shadows outside the beam, forces active scanning
- Cool (bluish-white) color temperature — modern LED feel, harsh clinical look
- Attracts monster — shining at or near the Wendigo draws its attention (risk/reward tension)
- Does NOT affect visibility score — VisibilityScoreComponent stays ambient-light-only
- Always on for demo (no toggle, no battery)

### Horror sound identity
- Layered blend of industrial decay + supernatural elements — creaking metal, drips, electrical hum as base layer with occasional supernatural sounds (whispers, tonal drones, unidentifiable) bleeding through
- Constant ambient bed — always some ambient playing, tension comes from monster audio and music system
- Indoor + outdoor ambient sets needed — building interior sounds and surrounding forest/grounds sounds
- One scripted silence moment: all ambient drops out before the Wendigo's first introduction — "predator silence" where nature goes completely quiet before the creature appears

### Monster audio presence
- Moderately audible during patrol — footsteps + occasional breathing audible from medium range, player can track and avoid
- Blend voice identity — animal sounds (growls, sniffing, heavy breathing) with occasional human-like vocalizations bleeding through, creating uncanny unease (the father is still in there)
- Distinct sounds per behavior state:
  - Patrol: quiet breathing, soft footsteps
  - Suspicious: sniffing, alert growl
  - Chase: roaring, heavy sprint footsteps
  - Search: frustrated snarls
- Dual proximity cue system: player heartbeat (intensity indicator) + spatial monster breathing (directional indicator) — both layered, but flagged for potential simplification during playtesting if it becomes overwhelming

### Music and tension system
- Alert level driven — music follows AI alert state (calm patrol, tense suspicious, intense chase)
- Silent Hill / Resident Evil inspired score — industrial-ambient-emotional blend with orchestral tension elements, not a direct copy but capturing that feel
- Key moments only for stingers — monster spots player, player escapes chase, grab attack (3-4 defined events, not frequent)
- Smooth crossfade transitions between intensity tiers — gradual tension build over several seconds

### Claude's Discretion
- Lumen GI settings and light placement for atmospheric horror
- Exact attenuation curves and distance falloffs for spatial audio
- MetaSound vs SoundCue implementation choice
- Number and composition of ambient sound layers
- Specific stinger sound design
- Heartbeat/breathing proximity curve tuning

</decisions>

<specifics>
## Specific Ideas

- "Being in a forest and hearing NO animals or bugs or anything is very chilling" — the Wendigo's first introduction should use total silence as a tool, all ambient cuts out before the creature crashes into view
- Silent Hill and Resident Evil franchise scores as inspiration — Akira Yamaoka's industrial-ambient-emotional style blended with RE's orchestral tension. Capture the feel without copying
- Monster voice should hint that the father is still in there — human-like sounds mixed with animal, creating uncanny blend
- Proximity cue system (heartbeat + breathing) may need to be simplified to one layer based on playtesting feedback

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 06-light-and-audio*
*Context gathered: 2026-02-12*
