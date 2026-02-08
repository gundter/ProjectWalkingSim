# Research Summary: The Juniper Tree

**Project:** First-person psychological horror game
**Engine:** Unreal Engine 5.7.2
**Synthesized:** 2026-02-07
**Overall Confidence:** HIGH

---

## Executive Summary

The Juniper Tree is a first-person psychological horror game with a single persistent monster (Wendigo), hide-and-seek survival mechanics, an 8-slot inventory, and visible player body. Research confirms this is a well-documented domain with clear industry patterns. The recommended approach uses UE5.7's native first-person rendering system, State Tree for AI (replacing behavior trees), component-based architecture, and Lumen/VSM for atmospheric lighting. The core technical challenge is the monster AI: avoiding predictable patterns while maintaining fair gameplay requires a two-layer architecture (AI Director + sense-driven Wendigo actor) inspired by Alien: Isolation.

The demo scope is appropriate. Table stakes for the genre include hiding mechanics, spatial audio, light management, and environmental storytelling. The fairy tale aesthetic and unreliable narrator elements are strong differentiators that few horror games execute well. Critical pitfalls to avoid are predictable AI behavior, jump scare dependency, broken save states, and first-person body desync. All of these are solvable with proper architecture established early.

Build order should prioritize: (1) player controller with first-person body, (2) core interfaces and data architecture, (3) inventory and interaction systems, (4) hiding mechanics, (5) monster AI with State Tree, (6) light/visibility integration, (7) save system, (8) demo polish. The C++ foundation must be established from the start to avoid Blueprint performance debt.

---

## Key Findings

### From STACK.md

| Technology | Purpose | Rationale |
|------------|---------|-----------|
| **UE5.7 First Person Rendering** | Visible body with self-shadow | Native system eliminates dual-skeleton complexity |
| **State Tree** | Monster AI behavior | Epic's modern replacement for BT; on-demand evaluation, not per-frame tick |
| **AI Perception + EQS** | Wendigo senses | Built-in sight/hearing with configurable ranges; EQS for patrol/search queries |
| **Enhanced Input** | Player controls | Mandatory in UE5.7; legacy deprecated |
| **Custom UActorComponent** | Inventory (8 slots) | GAS is overkill; component approach enables reuse |
| **Lumen + VSM** | Horror atmosphere | Real-time GI essential for flashlight bounce; VSM for film-quality shadows |
| **USaveGame + Async** | Save/load | Native serialization with async to prevent hitches |

**Critical version requirement:** UE5.7.2 for first-person rendering features and State Tree improvements.

**Do NOT use:** GAS for inventory, Behavior Trees (use State Tree instead), Blueprint-only complex logic.

### From FEATURES.md

**Table Stakes (must have for demo):**
- Hiding mechanics (lockers, closets, under beds)
- Light/darkness system with battery management
- Environmental storytelling (notes, examinable objects)
- Spatial audio for threat awareness
- Resource scarcity (8-slot inventory)
- Monster encounters with survival stakes
- Safe zones for pacing
- Checkpoint/save system

**Differentiators (should have):**
- Single persistent monster with personality (Wendigo)
- Visible player body/hands
- Unreliable narrator touches
- Fairy tale aesthetic (unique in horror space)

**Defer to post-demo:**
- Full sanity system
- Adaptive AI that learns player patterns
- Reality-shifting environments
- Progressive monster behavior unlocking

**Anti-features (explicitly avoid):**
- Combat system (destroys horror tension)
- Overused jump scares (1-2 per hour max)
- Predictable monster patterns
- Invincible always-pursuing monster
- Power fantasy late game

### From ARCHITECTURE.md

**Major Components:**

| Component | Responsibility |
|-----------|----------------|
| AHorrorPlayer | Player pawn, camera, input handling |
| UInventoryComponent | 8-slot item storage, use/combine logic |
| UVisibilityComponent | Track player visibility to AI |
| UInteractionComponent | Detect and trigger interactions |
| ULightAwareComponent | Calculate ambient light exposure |
| AWendigoMonster | Monster pawn, animation, audio |
| AWendigoAIController | State Tree execution, perception handling |
| UHorrorStateSubsystem | Central tension/awareness coordination |
| AHidingSpot | Hiding location actor with IHideable |

**Key Patterns:**
1. **Component-based composition** - Build actors from focused components, not monolithic classes
2. **Interface-based communication** - IInteractable, IHideable, ISaveable for decoupling
3. **GameInstanceSubsystem** - For state that persists across levels (horror state, save manager)
4. **Data-driven definitions** - UDataAsset for items, not hardcoded
5. **Event-driven UI** - Multicast delegates, no tick polling
6. **State Tree with AI Perception** - Director + actor architecture for unpredictable AI

**Data Flow:**
- Player Input -> Enhanced Input -> Components -> Subsystems
- AI Perception -> State Tree -> Blackboard -> Behavior execution
- Visibility = Light exposure + movement + stance
- Hiding state communicated via subsystem, affects AI perception

### From PITFALLS.md

**Top 5 Critical Pitfalls:**

| Pitfall | Risk Level | Prevention |
|---------|------------|------------|
| **Predictable Monster AI** | CRITICAL | Two-layer architecture (Director + Actor); randomized variance; progressive behavior unlocking |
| **Jump Scare Dependency** | CRITICAL | Atmosphere-first design; earned scares only; audio restraint |
| **Broken Save States** | CRITICAL | Never save mid-chase; validate state before write; rolling saves (keep last 3) |
| **First-Person Body Desync** | CRITICAL | Separate first-person mesh for reliability; proper Aim Offset; motion sickness options |
| **Lumen/VSM Performance** | MODERATE | Choose tracing method early; profile continuously; expose quality options |

**Phase-specific warnings:**
- Core Architecture: Establish C++ foundation immediately, avoid Blueprint tick functions
- Player Controller: Prototype animation approach before building content
- Monster AI: Director + actor architecture; behavioral variety; not pure state machine
- Demo: Ruthless scope protection; time-box development

---

## Implications for Roadmap

Based on combined research, the recommended phase structure is:

### Phase 1: Foundation (2-3 weeks)

**Rationale:** All systems depend on core interfaces, data architecture, and player controller. First-person body awareness must be solved before building content.

**Delivers:**
- Core C++ class hierarchy
- IInteractable, IHideable, ISaveable, ILightSource interfaces
- UItemDataAsset structure
- UHorrorStateSubsystem skeleton
- Enhanced Input setup with Input Mapping Contexts

**Pitfalls to avoid:** #10 Blueprint Performance Debt, #4 Body Desync

**Research needed:** LOW - patterns well-documented in UE5 docs

### Phase 2: Player Controller (2-3 weeks)

**Rationale:** Player is the foundation for all gameplay. First-person with visible body is technically challenging and must be proven early.

**Delivers:**
- AHorrorPlayer with first-person rendering
- Movement with crouch, stamina
- Camera setup with head-bob options
- UInteractionComponent
- Basic interaction prompts

**Pitfalls to avoid:** #4 Body Desync (camera attachment, animation blending)

**Research needed:** MEDIUM - study UE5.7 first-person rendering docs closely

### Phase 3: Inventory System (1-2 weeks)

**Rationale:** Inventory enables item-based gameplay (flashlight, keys, documents) required for hiding and light mechanics.

**Delivers:**
- UInventoryComponent (8 slots)
- Item data assets
- APickupItem actor
- Inventory UI (radial or grid)
- Item use/combine/discard

**Pitfalls to avoid:** #6 Inventory Immersion Break (real-time with audio cues, not full pause)

**Research needed:** LOW - standard component pattern

### Phase 4: Hiding System (2 weeks)

**Rationale:** Core survival mechanic. Must work before monster AI can test against it.

**Delivers:**
- AHidingSpot actor with IHideable
- Enter/exit hiding state
- Hidden player collision channel
- Peek camera mode
- Multiple hiding types (locker, under bed, closet)

**Pitfalls to avoid:** #8 Hide Spot Saturation (meaningful variety, not "locker forest")

**Research needed:** LOW - pattern established by Amnesia/Outlast

### Phase 5: Monster AI (3-4 weeks)

**Rationale:** Central to horror experience. Requires hiding system to be complete for testing.

**Delivers:**
- AWendigoMonster character
- AWendigoAIController with State Tree
- AI Perception (sight + hearing)
- States: Patrol, Investigate, Chase, Search
- Director system for "psychopathic serendipity"
- EQS queries for patrol/search

**Pitfalls to avoid:** #1 Predictable AI (CRITICAL - most important phase to get right)

**Research needed:** HIGH - State Tree + Director architecture deserves dedicated research sprint

### Phase 6: Light & Visibility (2 weeks)

**Rationale:** Integrates with AI perception and inventory (flashlight/batteries). Enables meaningful stealth gameplay.

**Delivers:**
- ULightAwareComponent
- UVisibilityComponent
- Flashlight item with battery drain
- Visibility affects AI perception range
- Lumen settings optimized for horror

**Pitfalls to avoid:** #5 Battery Anxiety (meaningful tradeoffs, not arbitrary depletion), #7 Lumen Performance

**Research needed:** MEDIUM - Lumen + flashlight performance needs testing

### Phase 7: Save System (1-2 weeks)

**Rationale:** Depends on all saveable systems being complete. Should be robust before demo playtesting.

**Delivers:**
- USaveSubsystem
- ISaveable implementations for all systems
- Async saving
- Rolling saves (last 3)
- Save state validation (no soft-locks)

**Pitfalls to avoid:** #3 Broken Save States (CRITICAL - validate before write, never save mid-chase)

**Research needed:** LOW - pattern well-established

### Phase 8: Demo Polish (2-3 weeks)

**Rationale:** Final integration, pacing, and quality pass. Ruthless scope protection essential.

**Delivers:**
- One complete demo area
- Horror pacing pass (tension curves)
- Environmental storytelling (notes, details)
- Audio implementation (spatial, ambient)
- Performance optimization
- Accessibility options

**Pitfalls to avoid:** #11 Scope Creep, #2 Jump Scare Dependency, #9 Pacing Whiplash

**Research needed:** LOW - execution phase, not research phase

---

## Research Flags

**Phases needing `/gsd:research-phase`:**
- Phase 2 (Player Controller): UE5.7 first-person rendering implementation details
- Phase 5 (Monster AI): State Tree + Director architecture is critical; deserves deep research
- Phase 6 (Light): Lumen performance with player flashlight as primary GI source

**Phases with standard patterns (skip research):**
- Phase 1 (Foundation): Well-documented UE5 patterns
- Phase 3 (Inventory): Component pattern, many examples
- Phase 4 (Hiding): Genre-standard implementation
- Phase 7 (Save System): Native UE5 pattern
- Phase 8 (Demo Polish): Execution, not research

---

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | Official UE5.7 documentation; State Tree is current Epic direction |
| Features | HIGH | Multiple reference games analyzed; genre is well-documented |
| Architecture | HIGH | Standard UE5 patterns; component-based approach proven |
| Pitfalls | MEDIUM-HIGH | Cross-referenced from GDC talks, post-mortems, community reports |

### Gaps to Address During Planning

1. **First-person rendering specifics:** UE5.7 documentation exists but implementation details need hands-on testing
2. **State Tree + Director integration:** How to implement AI Director pattern with State Tree (vs. Behavior Tree examples from Alien: Isolation)
3. **Lumen + flashlight performance:** Real-world performance of player-held spotlight as primary GI source
4. **Save validation logic:** Specific checks for preventing soft-locks in hide-and-seek scenarios

---

## Aggregated Sources

### Official Documentation (HIGH Confidence)
- [First Person Rendering - UE5.7](https://dev.epicgames.com/documentation/en-us/unreal-engine/first-person-rendering)
- [State Tree in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/state-tree-in-unreal-engine)
- [Enhanced Input](https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine)
- [AI Perception](https://dev.epicgames.com/documentation/en-us/unreal-engine/ai-perception-in-unreal-engine)
- [Lumen Global Illumination](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-global-illumination-and-reflections-in-unreal-engine)
- [Virtual Shadow Maps](https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-shadow-maps-in-unreal-engine)
- [Saving and Loading](https://dev.epicgames.com/documentation/en-us/unreal-engine/saving-and-loading-your-game-in-unreal-engine)

### Game Design Analysis (HIGH Confidence)
- [The Perfect Organism: AI of Alien: Isolation](https://www.gamedeveloper.com/design/the-perfect-organism-the-ai-of-alien-isolation)
- [Amnesia Sanity Meter Deep Dive](https://www.gamedeveloper.com/design/game-design-deep-dive-i-amnesia-i-s-sanity-meter-)
- [SOMA Design Pillars](https://frictionalgames.com/2013-12-the-five-foundational-design-pillars-of-soma/)
- [Jump Scare Design Analysis](https://www.gamedeveloper.com/design/a-lack-of-fright-examining-jump-scare-horror-game-design)

### Community Resources (MEDIUM Confidence)
- [Horror AI Tutorial](https://dev.epicgames.com/community/learning/tutorials/2JzM/horror-ai-unreal-engine-5-tutorial-part-1)
- [Horror Lighting in UE5](https://forums.unrealengine.com/t/lighting-shadows-in-horror-games-how-we-used-unreal-engine-5-to-build-devil-of-plague/2351092)
- [Inventory System Guide](https://www.spongehammer.com/unreal-engine-5-inventory-system-cpp-guide/)
- [Common Horror Game Mistakes](https://drwedge.uk/2024/10/11/common-horror-game-mistakes/)

---

*Synthesis completed: 2026-02-07*
*Ready for roadmap definition*
