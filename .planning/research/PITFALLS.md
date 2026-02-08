# Domain Pitfalls: Psychological Horror Games

**Project:** The Juniper Tree
**Domain:** First-person psychological horror with persistent monster, hide/avoid mechanics
**Researched:** 2026-02-07
**Confidence:** MEDIUM-HIGH (multiple verified sources, cross-referenced with industry post-mortems)

---

## Critical Pitfalls

Mistakes that cause rewrites, abandonment, or fundamentally broken player experience.

---

### Pitfall 1: Predictable Monster AI ("The Chore Monster")

**What goes wrong:** Once players learn the monster's patrol patterns and behavior rules, the Wendigo transforms from a terrifying unknown threat into a predictable obstacle to be managed. Fear evaporates; frustration replaces it.

**Why it happens:** Developers create deterministic AI for easier debugging and balancing. Simple state machines with fixed patrol routes are easier to implement than sophisticated behavioral systems.

**Consequences:**
- Players exploit patterns ("wait here for 30 seconds, then sprint")
- Monster becomes annoying rather than scary
- Hide-and-seek gameplay becomes tedious repetition
- YouTube videos expose all patterns within days of release

**Warning signs:**
- Playtesters describing monster behavior in precise terms ("it always checks the kitchen first")
- Players developing "safe spots" or timing strategies
- Fear diminishing rapidly over play sessions
- Monster encounters feeling like puzzles with solutions

**Prevention strategy:**
Implement a two-layer AI system inspired by [Alien: Isolation's AI architecture](https://www.gamedeveloper.com/design/the-perfect-organism-the-ai-of-alien-isolation):
1. **AI Director:** Omniscient controller that knows player position, manages "menace level," and guides the Wendigo toward tension-creating encounters
2. **Wendigo Actor:** Sense-driven agent that must legitimately find the player through sight, sound, and environmental cues - never allowed to "cheat"

Key implementation details:
- Use a behavior tree that unlocks new behaviors over time (searching lockers, checking previous hiding spots), creating illusion of learning
- Director creates "psychopathic serendipity" - the Wendigo appears at narratively appropriate moments without breaking immersion
- Add randomized variance to patrol routes (never identical twice)
- Monster should sometimes almost find the player, then leave - uncertainty is scarier than consistency

**Phase to address:** Core Monster AI (behavior tree foundation + director system must be architected early)

**Sources:**
- [The Perfect Organism: The AI of Alien: Isolation](https://www.gamedeveloper.com/design/the-perfect-organism-the-ai-of-alien-isolation)
- [Revisiting the AI of Alien: Isolation](https://www.aiandgames.com/p/revisiting-alien-isolation)
- [Should the Monster Play Fair?](https://gamestudies.org/2002/articles/jaroslav_svelch)

---

### Pitfall 2: Jump Scare Dependency ("Horror Junk Food")

**What goes wrong:** Over-reliance on jump scares creates desensitization. Players learn the pattern (quiet buildup, loud BANG), and scares become predictable punchlines rather than genuine fear responses. The game feels like a haunted house ride rather than psychological horror.

**Why it happens:** Jump scares are easy to implement and provide immediate, measurable player reactions. Psychological dread is harder to design and test.

**Consequences:**
- "Scary" and "scary" sections become completely divorced from gameplay
- Players mute game or reduce volume
- Streamers and YouTubers mock predictable scares
- Psychological horror elements get overshadowed
- Game feels dated quickly (jump scares are increasingly seen as a trope to avoid)

**Warning signs:**
- Playtesters describing exact moment before scare ("I knew something was coming")
- Players lowering volume during gameplay
- Tension spikes only occurring during scripted sequences
- No fear during normal exploration

**Prevention strategy:**
Design around atmosphere and uncertainty rather than stimulus-response:
1. **Environmental dread:** Use environmental storytelling, sound design, and lighting to create persistent unease
2. **Earned scares:** Jump scares only after significant tension buildup, and only occasionally - subvert expectation more often than fulfilling it
3. **Player-driven discovery:** Let players find disturbing things on their own rather than forcing reveals
4. **Audio restraint:** Never use max-volume audio stingers - that's "bad design, not tension"
5. **Wendigo presence indicators:** Subtle signs the Wendigo is nearby (cold breath, distant sounds, environmental changes) create sustained dread

**Phase to address:** Level Design / Horror Pacing (atmosphere system, audio design, encounter scripting)

**Sources:**
- [A Lack of Fright: Examining Jump Scare Horror Game Design](https://www.gamedeveloper.com/design/a-lack-of-fright-examining-jump-scare-horror-game-design)
- [Common Horror Game Mistakes](https://drwedge.uk/2024/10/11/common-horror-game-mistakes/)
- [5 Horror Gaming Tropes That Need to Go Extinct](https://www.xda-developers.com/worst-horror-gaming-tropes-i-hope-go-extinct-in-2026/)

---

### Pitfall 3: Broken Save States ("The Soft Lock")

**What goes wrong:** Save system creates unwinnable states - saving a fraction of a second before death, saving with no resources and monster blocking progress, or checkpoints that push players too far back after death.

**Why it happens:** Save systems are complex to test comprehensively. Edge cases are hard to anticipate. Horror games intentionally limit saving to create tension, but this amplifies the impact of bugs.

**Consequences:**
- Players lose 10-30 minutes of progress (The Callisto Protocol notoriously did this)
- Soft locks force complete restarts
- Players quit in frustration
- Negative reviews specifically cite save system

**Warning signs:**
- Any save that captures "in-progress" state (mid-combat, mid-chase)
- Checkpoint saves near hazards or enemies
- Limited save resources with no fallback
- Players reporting "I had to restart" in testing

**Prevention strategy:**
1. **Never save mid-combat or mid-chase:** Only save when player is in a "safe" state
2. **Validate save state:** Before writing save, check: Can player survive? Are resources sufficient? Is path forward possible?
3. **Rolling saves:** Keep last 3 autosaves, not just 1 - player can always go back
4. **Checkpoint spacing:** Horror can limit saves, but 10+ minutes between checkpoints is frustration, not tension
5. **Demo-specific:** For demo, be more generous with saves - first impressions matter more than difficulty

Consider hybrid approach: Manual saves at specific locations (like Resident Evil safe rooms) for atmosphere, plus invisible autosaves at key progression points as safety net.

**Phase to address:** Core Systems (save/load architecture must be designed correctly from start)

**Sources:**
- [The Callisto Protocol Checkpoint Issues](https://gamerant.com/the-callisto-protocol-bad-checkpoints-lesson-other-games/)
- [Saving the Day: Save Systems in Games](https://www.gamedeveloper.com/design/saving-the-day-save-systems-in-games)
- [Checkpoint Saving vs. Limited Saving In Horror Games](https://pekoeblaze.wordpress.com/2022/04/28/checkpoint-saving-vs-limited-saving-in-horror-games/)

---

### Pitfall 4: First-Person Body Desync ("The Floating Head")

**What goes wrong:** Camera and body animations become disconnected - camera turns while body stays in place, arms clip through geometry, looking down shows wrong body position, or animations feel delayed compared to input.

**Why it happens:** True first-person body awareness is technically challenging. Camera attached to head bone can create jarring movement. Animation blending with look direction requires careful Aim Offset configuration.

**Consequences:**
- Immersion breaks constantly
- Players notice "something feels wrong" even if they can't articulate it
- Horror atmosphere undermined by technical jankiness
- Motion sickness in sensitive players

**Warning signs:**
- Camera spinning around body during fast turns
- Arms/hands not matching look direction
- Visible body clipping through world geometry
- Head-bob or camera movement feeling "floaty" or delayed
- Playtesters complaining of nausea or disorientation

**Prevention strategy:**
1. **Separate animation rigs:** Consider using separate first-person arm rig (higher detail, no body) for reliability, with full body only for shadow/reflection
2. **Aim Offset implementation:** Use properly configured Aim Offset assets to blend look direction with body animations
3. **Camera attachment:** Test camera attachment to different bones (head, neck, chest) - each has tradeoffs
4. **Motion sickness mitigation:** Add toggle options for head-bob, camera sway, motion blur - these are major triggers
5. **Reference UE5.6 Horror Template:** Epic's template includes first-person with visible hands/feet - study their implementation

**Phase to address:** Player Controller / Animation System (must be solved before building content on top of it)

**Sources:**
- [True First Person Camera in Unreal Engine](https://www.froyok.fr/blog/2018-06-true-first-person-camera-in-unreal-engine-4/)
- [UE5.6 Template Guide](https://www.unreal-university.blog/unreal-engine-5-6-complete-guide-to-the-new-templates/)
- [Alleviating Motion Sickness in First-Person Games](https://nicolas.busseneau.fr/en/blog/2020/09/alleviating-motion-sickness-in-first-person-video-games)

---

## Moderate Pitfalls

Mistakes that cause delays, technical debt, or degraded player experience.

---

### Pitfall 5: Frustrating Light Management ("Battery Anxiety")

**What goes wrong:** Flashlight battery mechanics feel arbitrary - fresh batteries lasting only minutes is unrealistic, but realistic battery life makes the mechanic irrelevant. Players either never feel tension or constantly feel annoyed.

**Why it happens:** Light management seems like an obvious horror mechanic (darkness = scary), but the design rarely matches player expectations or adds meaningful gameplay.

**Consequences:**
- Players hoard resources and never use flashlight
- Or players constantly frustrated by artificial darkness
- Lighting puzzles feel like busywork
- Missed opportunity for meaningful player choices

**Warning signs:**
- Playtesters either always using flashlight or never using it
- Battery pickups feeling arbitrary
- No meaningful decision-making around light usage
- "I just ran through in the dark" strategies emerging

**Prevention strategy:**
Create meaningful tradeoffs rather than resource depletion:
1. **Alan Wake approach:** Light isn't just visibility - it has gameplay impact (reveals hidden things, affects Wendigo behavior)
2. **Tormented Souls approach:** Force choice between light source and other tools (lighter OR weapon, not both)
3. **If using batteries:** Make depletion predictable, generous, and signaled well in advance. No sudden blackouts.
4. **Wendigo-light interaction:** Perhaps Wendigo is attracted to light, or repelled by it, or reacts to sudden changes - make light a tactical choice, not just visibility
5. **Multiple light sources:** Flashlight, lighter, glow sticks, environmental - each with different properties

**Phase to address:** Light Mechanics / Inventory Integration

**Sources:**
- [Horror Games That Utilize Unique Lighting Mechanics](https://gamerant.com/horror-games-utilize-unique-lighting-mechanics/)
- [Great Horror Games Featuring Immersive Flashlight Mechanics](https://gamerant.com/best-horror-games-immersive-flashlight-mechanics/)

---

### Pitfall 6: Inventory Immersion Break ("The Pause Menu Reflex")

**What goes wrong:** Inventory system either breaks tension by pausing the game (player opens inventory to feel safe) or frustrates with real-time management during high-stress moments (fumbling for items while being chased).

**Why it happens:** It's hard to balance "inventory management should have weight" with "inventory management shouldn't be the game."

**Consequences:**
- If paused: Players use inventory as "time out" button, breaking tension
- If real-time: Clunky controls during stress lead to frustration
- Complex inventory systems distract from horror
- Item management becomes tedious rather than tense

**Warning signs:**
- Playtesters opening inventory to "take a breather"
- Death caused by inventory fumbling feeling unfair
- Players forgetting they have items
- Inventory management taking significant game time

**Prevention strategy:**
1. **Limited inventory, simple interface:** 4-6 slots maximum. Radial menu or quick-select, not grid management.
2. **Real-time with restrictions:** World doesn't fully pause, but perhaps Wendigo movement slows or player gets audio cues of approach
3. **Deliberate clunkiness (maybe):** Some horror games intentionally make inventory slow - but this must feel like character limitation, not UI limitation
4. **Physicality:** Items visible on player body. Accessing inventory means looking away from world. Create real risk without broken controls.
5. **Context-sensitive access:** Quick-use for flashlight/lighter without opening full inventory

**Phase to address:** Inventory System (early - before designing items that use it)

**Sources:**
- [Best Inventory Systems In Horror Games](https://www.thegamer.com/best-inventory-systems-horror-games/)
- [Cool Interface Ideas and Problems in The Forest](https://medium.com/@tetiana.tk.kaliuzhna/cool-interface-ideas-and-some-problems-in-the-forest-e52c754b8277)
- [Tracing the Tendrils of Item Management](https://horror.dreamdawn.com/?p=14800056)

---

### Pitfall 7: Lumen/VSM Performance Issues ("The Pretty But Broken Curse")

**What goes wrong:** UE5's Lumen and Virtual Shadow Maps provide beautiful dynamic lighting perfect for horror, but cause performance issues: shadow flickering, noise, frame drops in complex scenes, and inconsistent behavior.

**Why it happens:** Lumen and VSM are designed for high-end hardware. Horror games rely heavily on dynamic lighting. Default settings aren't optimized for horror use cases (many small light sources, frequent darkness-to-light transitions).

**Consequences:**
- Shadow flickering breaks immersion
- Frame rate drops during tense moments (worst timing)
- Visual quality inconsistency (noisy shadows in some areas)
- Players with mid-range hardware can't enjoy the game

**Warning signs:**
- Flickering shadows when camera moves
- Noisy/grainy shadows especially at distance
- Frame drops when multiple lights active
- Shadows disappearing at certain angles
- Performance variance between scenes

**Prevention strategy:**
1. **Choose tracing method early:** Software ray tracing for broader compatibility, hardware for high-end target
2. **Mesh Distance Field optimization:** Ensure meshes are clean with appropriate Distance Field Resolution Scale
3. **Shadow quality settings:** Expose shadow quality options for players. Test thoroughly at MEDIUM preset (common source of artifacts)
4. **Flashlight as primary GI source:** Works well with Lumen (light bounces dynamically), but test performance with many shadow-casting objects
5. **Hybrid approach consideration:** Pre-baked lighting for static areas, Lumen for flashlight/dynamic sources only
6. **VSM page caching:** Understand that camera cuts, fast movement, and light rotation invalidate shadow caches - design encounters accordingly

**Phase to address:** Lighting/Rendering Setup (establish early, test continuously)

**Sources:**
- [Lumen Performance Guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-performance-guide-for-unreal-engine)
- [Fixing Shadow Noise in UE5](https://prographers.com/blog/fixing-shadow-noise-in-unreal-engine-5-a-guideline-with-practical-tips-for-lumen-and-ray-tracing)
- [Virtual Shadow Maps Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-shadow-maps-in-unreal-engine)
- [UE 5.4.x Common Rendering Issues](https://dev.epicgames.com/community/learning/knowledge-base/yjp8/unreal-engine-ue-5-4-x-most-common-rendering-issues)

---

### Pitfall 8: Hide Spot Saturation ("The Locker Forest")

**What goes wrong:** Too many hiding spots removes tension (player always feels safe) or identical hiding spots make choices feel meaningless. Alternatively, too few spots leads to frustrating trial-and-error deaths.

**Why it happens:** Designers add hiding spots liberally to ensure players can always escape. Or they create beautiful environments with obvious hiding spots that become too predictable.

**Consequences:**
- Monster encounters become formulaic: "see monster, find locker, wait, continue"
- No meaningful decision about where to hide
- Or: players die repeatedly unable to find hiding spot, quit in frustration
- Hiding feels like a mechanic rather than desperate survival

**Warning signs:**
- Players always using the same type of hiding spot
- Playtesters never feeling genuine danger
- Or playtesters dying repeatedly to same encounter
- "I just hid in every encounter" feedback

**Prevention strategy:**
1. **Varied hiding with tradeoffs:** Under bed (faster entry, more exposed), locker (safer but noisy to close), behind furniture (risky but quick)
2. **Hiding spot "heat":** If Wendigo has seen player use a hiding type, it learns to check those first
3. **Environmental hiding:** Not just furniture - shadows, water, behind doors, staying still. More organic than "press X to hide"
4. **Limited safe time:** Hiding isn't permanent safety. Wendigo might hear breathing, or player might need to move.
5. **Spatial design:** Fewer but more meaningful hiding spots. Player should sometimes need to CREATE opportunities (knock something over, throw distraction)

**Phase to address:** Level Design / Stealth Mechanics

**Sources:**
- [Hide and Seek Gameplay Analysis](https://exploringthegames.substack.com/p/horror-games-hide-and-seek-gameplay)
- [Lockers In Horror Games](https://pekoeblaze.wordpress.com/2022/07/15/lockers-in-horror-games/)

---

## Minor Pitfalls

Mistakes that cause annoyance but are recoverable.

---

### Pitfall 9: Pacing Whiplash ("The Horror Rollercoaster")

**What goes wrong:** Game oscillates between "nothing happening" sections and "everything happening" sections with no gradual tension curve. Players feel bored then overwhelmed, never genuinely scared.

**Why it happens:** Set-piece mentality from AAA games. Designers focus on big scare moments rather than sustained atmosphere.

**Consequences:**
- "Nonscary" and "scary" sections feel disconnected
- Players mentally check out during quiet sections
- Big moments lose impact without proper buildup
- Game feels fragmented

**Prevention strategy:**
1. **Tension peaks and valleys:** Design explicit tension curve for each area. Never fully "safe," never constantly chased.
2. **Wendigo presence gradient:** Sometimes visible, sometimes audible, sometimes just evidence. Constant awareness without constant threat.
3. **Environmental tension:** Even "quiet" areas have unsettling elements - the atmosphere never lets up
4. **Interruptible calm:** Safe-feeling moments should be subject to interruption - player never fully relaxes

**Phase to address:** Level Design / Encounter Design

**Sources:**
- [Creating Horror through Level Design](https://www.gamedeveloper.com/design/creating-horror-through-level-design-tension-jump-scares-and-chase-sequences)
- [The Balancing Act of Tension](https://www.gamedeveloper.com/design/the-balancing-act-of-tension-in-horror-game-design)
- [How to Use Pacing and Build Tension](https://www.gamedev.net/blogs/entry/2274395-how-to-use-pacing-and-build-tension-in-a-horror-game/)

---

### Pitfall 10: Blueprint Performance Debt ("The Tick Tax")

**What goes wrong:** Critical systems (AI, player controller, save system) implemented in Blueprint for rapid iteration, but performance degrades as complexity increases. Tick functions in Blueprint are significantly slower than C++.

**Why it happens:** Blueprint is faster to prototype. "We'll optimize later" becomes "we can't afford to rewrite now."

**Consequences:**
- Frame rate issues in complex scenes
- AI responsiveness feels sluggish
- Performance problems compound over development
- Late-project C++ rewrites are expensive

**Warning signs:**
- Blueprint tick functions with many nodes
- Visible frame drops during AI updates
- Performance degradation as features are added
- "We'll optimize later" appearing in planning docs

**Prevention strategy:**
1. **C++ foundation, Blueprint extension:** Core systems (AI behavior tree, player controller base, save system) in C++. Expose to Blueprint for tuning/iteration.
2. **Avoid Blueprint Tick:** Use Timers, Delegates, or event-driven patterns instead of per-frame Blueprint ticks
3. **Performance budget:** Set frame time budgets early. Profile regularly.
4. **Nativize isn't available in UE5:** Don't plan on automatic Blueprint compilation to C++ - it doesn't exist. Design for C++ from start.

**Phase to address:** Architecture / All phases (establish C++ patterns immediately, enforce throughout)

**Sources:**
- [UE5 Blueprint vs C++ Performance](https://www.spongehammer.com/unreal-engine-5-blueprint-vs-cpp-performance/)
- [C++ vs Blueprints 2026 Guide](https://www.wholetomato.com/blog/c-versus-blueprints-which-should-i-use-for-unreal-engine-game-development/)
- [Blueprint Performance Optimization](https://uhiyama-lab.com/en/notes/ue/blueprint-performance-optimization-nativize/)

---

### Pitfall 11: Demo Scope Creep ("Just One More Feature")

**What goes wrong:** Demo scope expands to include features meant for full game. "The demo needs X to be impressive" leads to delayed demo, burned-out developers, and features that don't represent the core experience.

**Why it happens:** Demos feel like the "one chance" to impress. Perfectionism. Unclear boundaries between demo and full game scope.

**Consequences:**
- Demo never ships or ships late
- Developer burnout before main development
- Demo includes half-baked features that hurt impressions
- Core experience not properly showcased

**Warning signs:**
- "We should add X to the demo" discussions
- Demo feature list growing over time
- Demo development taking longer than planned
- Features in demo that won't be in full game

**Prevention strategy:**
1. **Demo MVP definition:** Define demo scope in writing. One location, one monster encounter type, core mechanics only.
2. **Demo = vertical slice:** Not horizontal (many features shallow) but vertical (few features deep)
3. **Time-box ruthlessly:** Demo should take X weeks. If feature doesn't fit, it's not in demo.
4. **Demo feedback loop:** Ship demo, gather feedback, THEN expand for full game. Demo is learning, not proving.
5. **Steam Next Fest target:** Real deadline creates real constraint. Successful indie horror demos (HELLMART, Storebound) were focused experiences, not feature showcases.

**Phase to address:** Demo Planning (define scope clearly, protect it aggressively)

**Sources:**
- [Scope Creep in Indie Games](https://www.wayline.io/blog/scope-creep-indie-games-avoiding-development-hell)
- [How to Avoid Scope Creep in Game Development](https://www.codecks.io/blog/2025/how-to-avoid-scope-creep-in-game-development/)

---

## Wendigo-Specific Considerations

Given the Wendigo as the central antagonist, some domain-specific notes:

### Folklore Authenticity vs. Creative License

**Consideration:** The Wendigo originates from Algonquin (Anishinaabe, Cree) Indigenous folklore and carries significant cultural weight. Until Dawn's use of Wendigos was noted as "set primarily in former Native American territory."

**Recommendation:**
- Research the folklore respectfully but recognize you're making a horror game, not a cultural documentary
- Consider whether to lean into folklore accuracy (winter, cannibalism, isolation themes) or create your own interpretation
- Be aware of cultural sensitivity discussions - some Indigenous groups have expressed concerns about Wendigo depictions in media
- The "malevolent spirit possessing humans" aspect offers rich psychological horror potential

### Behavior Patterns from Folklore

Folklore-accurate Wendigo traits that could inform AI design:
- **Nocturnal:** Inactive during day, hunting at night - potential day/night cycle mechanic
- **Voice mimicry:** Can imitate human voices to lure victims - terrifying audio design potential
- **Former human intelligence:** Retains cunning, can set traps and anticipate behavior
- **Hunger that never ends:** Even after feeding, immediately hungry again - creates persistent threat
- **Winter association:** Cold environmental indicators of presence

**Sources:**
- [Wendigo Folklore](https://monster.fandom.com/wiki/Wendigo_(folklore))
- [Until Dawn Wendigo Analysis](https://until-dawn.fandom.com/wiki/The_Wendigos)
- [The Wendigo Cryptid Legend](https://www.mythfolks.com/wendigo)

---

## Phase-Specific Warnings Summary

| Phase Topic | Primary Pitfalls | Recommended Mitigation |
|-------------|-----------------|------------------------|
| Core Architecture | #10 Blueprint Performance, #3 Save States | Establish C++ foundation, design save system carefully |
| Player Controller | #4 Body Desync, Motion Sickness | Prototype animation approach early, add accessibility options |
| Monster AI | #1 Predictable AI, #8 Hide Spots | Director + actor architecture, behavioral variety |
| Lighting/Rendering | #7 Lumen/VSM Performance | Performance profiling pipeline, hardware target decision |
| Horror Design | #2 Jump Scare Dependency, #9 Pacing | Atmosphere-first design, explicit tension curves |
| Inventory/Items | #5 Light Management, #6 Inventory UI | Meaningful tradeoffs, context-sensitive access |
| Demo | #11 Scope Creep | Ruthless scope definition, time-boxing |

---

## Confidence Assessment

| Area | Confidence | Reasoning |
|------|------------|-----------|
| Monster AI patterns | HIGH | Multiple verified sources (Alien: Isolation GDC talks, academic analysis) |
| Pacing/scare design | HIGH | Extensive game design literature, multiple sources agreeing |
| UE5 technical issues | MEDIUM-HIGH | Official Epic documentation + community reports |
| Save system edge cases | MEDIUM | Post-mortem analyses, but specific to each game |
| First-person body awareness | MEDIUM | UE5.6 template exists, but implementation details require further research |
| Wendigo-specific design | MEDIUM | Folklore well-documented, game design applications extrapolated |

---

## Open Questions for Phase-Specific Research

1. **UE5.6 Horror Template:** How exactly does Epic's template handle first-person body awareness? Worth detailed study.
2. **AI Director implementation in UE5:** Best practices for behavior tree + director pattern in current UE5?
3. **Lumen + flashlight specifically:** Performance characteristics of player-held spotlight as primary GI source?
4. **Save state validation:** What specific checks prevent soft-locks in hide-and-seek horror?

---

*Research compiled from web sources, game design literature, and UE5 documentation. Confidence levels reflect source quality and cross-verification status.*
