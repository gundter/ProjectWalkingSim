# Phase 7: Save System - Context

**Gathered:** 2026-02-12
**Status:** Ready for planning

<domain>
## Phase Boundary

Save-point-based game persistence with manual saves at tape recorder objects and load-from-pause-menu. No auto-saves or checkpoints — death loads last manual save. Persists player position, inventory, and world state (doors, pickups). Settings persistence already exists from Phase 1 (USaveGame) and stays separate.

</domain>

<decisions>
## Implementation Decisions

### Save/Load UI flow
- Manual save at world-placed tape recorder objects (IInteractable)
- Short animation on save: player presses record, tape-rolling sound, small UI confirmation (~1-2 seconds)
- Pause menu has "Continue" (loads latest save instantly) and "Load Game" (opens slot picker)
- No save option in pause menu — must use tape recorders in the world

### Death and reload behavior
- No auto-saves, no checkpoints — purely manual saves at tape recorders
- Death (Wendigo grab) shows Game Over screen with "Load Last Save" / "Quit" options
- If player has never saved and dies: restart demo from beginning (no safety net)
- Full consequence design — losing progress is intentional tension

### World state scope
- **Saved:** Player inventory contents, door open/closed/locked states, picked-up item removal, player position (tape recorder location)
- **Not saved:** Wendigo position/alert/patrol state — resets to spawn point on load
- Player always loads standing at the tape recorder they last saved at
- World state is snapshot at save time — loading an older save restores items/doors to that save's state

### Slot management
- 3 save slots (classic survival horror convention)
- Each slot displays: screenshot thumbnail from save moment + timestamp
- Overwriting an existing slot requires confirmation prompt ("Overwrite this save?")
- Settings (sensitivity, head-bob, audio) persist separately from save slots (existing USaveGame)

### Claude's Discretion
- USaveGame vs custom serialization approach
- Save file format and internal structure
- Screenshot capture implementation details
- Tape recorder actor visual/audio polish level
- Game Over screen layout and styling
- Exact fade timing on death sequence

</decisions>

<specifics>
## Specific Ideas

- Tape recorders as save points — detective recording case notes, fits the narrative premise
- Resident Evil / Silent Hill save-point philosophy: limited saves create tension and resource management
- 3 slots forces save management decisions — player must consider which saves to keep
- "Continue" in pause menu for quick resume without browsing slots

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 07-save-system*
*Context gathered: 2026-02-12*
