# Phase 2: Inventory System - Context

**Gathered:** 2026-02-08
**Status:** Ready for planning

<domain>
## Phase Boundary

Player can collect, manage, and use items in an 8-slot inventory. Covers pickup via interaction system, inventory UI with item management (use, combine, info, discard), world-drop on discard, and item types (key items for the demo). Flashlight is a separate equipped tool, not an inventory item. No health/healing system in the demo — caught by the Wendigo is instant death.

</domain>

<decisions>
## Implementation Decisions

### Inventory UI layout
- Single horizontal row (1x8) along bottom center of screen
- Real-time — game does NOT pause while inventory is open
- Player retains full movement while inventory is open
- Toggle open/close with a key press (not hold)
- Each slot shows item icon + quantity badge for stackable items
- Selecting a slot shows a compact tooltip popup above the slot with item name, description, and action buttons
- Both mouse cursor and keyboard navigation supported simultaneously
- Keep UI components modular for easy iteration after playtesting

### Item interaction flow
- Action buttons displayed in the tooltip popup (Use, Combine, Info, Discard)
- Combine flow: select item A, click Combine, then click item B's slot (two-step)
- Discard is immediate for regular items; key items show a confirmation warning
- Failed combinations show feedback (no silent failure)

### Pickup and drop behavior
- Pickup uses the existing interaction system (look at item, press F to pick up)
- Interaction key is F (changed from E — update IMC_Default binding for IA_Interact)
- Item disappears from world + subtle pickup sound on collection
- Discarded items toss forward slightly with small velocity, land ~1m ahead
- When inventory is full, the interaction prompt text changes to "Inventory Full" (no separate message)

### Item types
- Demo focuses on keys and codes (physical keys for locked doors, codes for padlocks)
- Stretch goal: 1-2 environmental puzzle items (fuse, handle, etc.) — still within phase scope if time allows
- Key items CAN be discarded but with a strong warning; item drops into world and can be re-picked up
- Flashlight is a separate equipped tool, not an inventory item (implemented in Phase 6)

### Claude's Discretion
- Info panel content style (flavor text vs functional info, or blend)
- Exact tooltip popup sizing, animation, and visual style
- Keyboard navigation key bindings (Q/E, arrows, number keys, etc.)
- Inventory open/close animation and sound design
- Combine failure feedback presentation
- Stackable item max stack sizes

</decisions>

<specifics>
## Specific Ideas

- Real-time inventory creates tension: fumbling for items while the Wendigo is nearby
- Modular UI components so layout/behavior can be easily changed after playtesting or demo feedback
- Interaction key changed from E to F — needs IMC_Default rebinding
- No health system in demo — healing items deferred to post-demo/v2

</specifics>

<deferred>
## Deferred Ideas

- Healing items and health system — deferred to v2 (post-demo). Demo uses instant death on Wendigo catch.
- Item combination puzzles beyond basic key/lock — stretch goal within phase, or future phase if complex
- Drag-and-drop item reordering — could add later if players want it

</deferred>

---

*Phase: 02-inventory-system*
*Context gathered: 2026-02-08*
