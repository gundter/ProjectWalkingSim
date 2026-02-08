# Feature Landscape: Psychological Horror Games

**Domain:** Psychological horror / hide-and-seek horror (no combat)
**Researched:** 2026-02-07
**Confidence:** HIGH (multiple verified sources from reference games)

## Table Stakes

Features users expect. Missing = product feels incomplete or amateurish.

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| **Hiding mechanics** | Core survival loop for no-combat horror; established by Amnesia/Outlast | Medium | Lockers, closets, under beds, behind objects. Context-sensitive button for instant entry. |
| **Light/darkness system** | Fundamental tension mechanic; darkness = danger but also safety | Medium | Players expect meaningful light sources and consequences for staying in dark/light. |
| **Environmental storytelling** | How narrative is delivered in exploration horror | Medium | Notes, documents, environmental details. Players piece together story themselves. |
| **Spatial audio** | Essential for threat awareness without combat | Medium | Footsteps, ambient sounds, directional audio for monster tracking. Players expect to "hear" danger coming. |
| **Resource scarcity** | Creates meaningful decisions and tension | Low-Medium | Limited batteries, matches, healing items. Your 8-slot inventory supports this. |
| **Physics-based interaction** | Established by Amnesia series; makes world feel tangible | Medium | Opening doors by dragging, picking up/examining objects, moving furniture. |
| **Monster encounters with survival stakes** | Core tension driver | High | Clear win/loss states when monster finds player. Death must feel consequential. |
| **Safe zones/respite areas** | Pacing requirement; prevents desensitization | Low | Areas where player can recover, explore safely, and tension can reset. |
| **First-person perspective** | Genre standard for psychological immersion | Low | Already planned. Visible body/hands enhance this significantly. |
| **Checkpoint/save system** | Player expectation, though implementation varies | Medium | Manual save points (Alien Isolation) or auto-checkpoints (Amnesia). Choice affects tension. |

## Differentiators

Features that set product apart. Not expected, but create competitive advantage.

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| **Unreliable narrator** | Creates unique psychological horror; reality questioning | High | Already planned. Few games execute this well. Layers of Fear and SOMA are notable examples. |
| **Single persistent monster** | Creates relationship/familiarity with threat (Alien Isolation model) | High | Already planned. More memorable than random encounters. Requires sophisticated AI behavior tree. |
| **Visible player body** | Enhanced immersion; hands show damage/fear state | Medium | Already planned. Research shows hands create "intense connection" via rubber hand illusion psychology. |
| **Adaptive AI behavior** | Monster learns from player patterns | Very High | Alien Isolation's system with 100+ behavior nodes unlocked progressively. Premium differentiator. |
| **Sanity/mental state system** | Amnesia's signature mechanic; visual/audio distortion | High | Darkness damages sanity, light restores but attracts monster. Creates push-pull tension. |
| **Reality-shifting environments** | Layers of Fear approach; rooms change when not looking | High | Non-Euclidean geometry, looping hallways (P.T.), architecture that defies logic. |
| **Fairy tale aesthetic** | Brothers Grimm adaptation is unique in horror space | Medium | Differentiates from hospital/abandoned facility settings. Visual identity opportunity. |
| **Light as resource trade-off** | Matches/batteries create meaningful decisions | Medium | Already planned. Combined with sanity system creates deep tension loop. |
| **Progressive monster behavior** | Threat escalates as game progresses | High | New behaviors unlock over time, keeping players on edge. |
| **Meaningful inventory decisions** | 8 slots force trade-offs between safety and progress | Low | Already planned. Classic survival horror tension driver. |

## Anti-Features

Features to explicitly NOT build. Common mistakes in this domain.

| Anti-Feature | Why Avoid | What to Do Instead |
|--------------|-----------|-------------------|
| **Combat system** | Destroys core horror tension; empowerment kills fear | Lean fully into hide/evade. "If you can fight back, the threat is not as big." |
| **Overused jump scares** | Cheap, causes desensitization, lacks narrative substance | Use sparingly (1-2 per hour max). Build dread through atmosphere, not startle reflex. |
| **Predictable monster patterns** | Players adapt, tension dies | Use Alien Isolation-style systemic AI with "psychopathic serendipity" - monster always at wrong time. |
| **Unlimited resources** | Eliminates meaningful decisions | Keep batteries, matches, health items scarce. Force trade-offs. |
| **Combat-lite (throwable distractions only)** | Half-measures feel weak | If including distractions, make them risky and limited. Better to commit fully to powerlessness. |
| **Excessive backtracking** | Amnesia solved this - "no need to return" design | One-way progression through areas. Reduce frustration. |
| **Constant threat presence** | Causes desensitization or player quits | Pacing is "rollercoaster" - high tension moments need calm recovery periods. |
| **Invincible monster always pursuing** | Exhausting, not scary | Monster should have search states, giving up, patrolling. Breathing room is essential. |
| **Distorted max-volume audio** | "Punishes" players, breaks immersion | Keep audio consistent. Jump scare audio should be startling, not painful. |
| **Too many puzzles blocking progress** | Breaks tension pacing | Puzzles should emerge from narrative, not feel like arbitrary gates. |
| **Power fantasy late game** | "Back half problems" - many horror games fail here | Maintain vulnerability throughout. Don't give player weapons or power-ups that trivialize threat. |
| **Empty/padding content** | Horror needs tight pacing | Demo scope is correct instinct. Better to be short and impactful than long and boring. |

## Feature Dependencies

```
Core Loop Dependencies:
  Monster AI ────> Hiding Mechanics (hiding must work against AI behavior)
      │
      └──────────> Audio System (monster needs audio presence)
      │
      └──────────> Light System (monster may react to light)

Atmosphere Dependencies:
  Environmental Storytelling ──> Documents/Notes System
                              ──> Examine Object System
                              ──> Audio Logs (optional)

Tension Systems:
  Light Management ──> Inventory (batteries/matches take slots)
                   ──> Sanity System (if implemented)
                   ──> Monster Detection (light may attract)

Narrative Dependencies:
  Unreliable Narrator ──> Reality-shifting Environments
                      ──> Visual Distortion Effects
                      ──> Sanity/Mental State System
```

## MVP Recommendation

For demo scope, prioritize:

### Must Have (Table Stakes)
1. **Monster AI with hiding counterplay** - Core gameplay loop
2. **Light/darkness system with battery management** - Already planned
3. **Spatial audio** - Essential for no-combat horror
4. **Environmental storytelling basics** - Notes, examinable objects
5. **Inventory system (8 slots)** - Already planned
6. **Safe zone(s)** - At least one area for pacing
7. **Visible hands/body** - Already planned, relatively low cost

### Should Have (Key Differentiators)
1. **Unreliable narrator touches** - A few reality-questioning moments
2. **Single persistent monster with personality** - Already planned
3. **Fairy tale aesthetic** - Visual differentiation

### Could Defer to Post-Demo
- **Full sanity system** - Complex, can add later
- **Adaptive AI that learns** - Very high complexity
- **Reality-shifting environments** - High complexity
- **Progressive monster behavior** - Can ship with fixed behavior first

## Complexity Estimates by Category

| Category | Low | Medium | High |
|----------|-----|--------|------|
| **Core Mechanics** | Inventory UI, Safe zones | Hiding system, Physics interaction, Light management | Monster AI, Adaptive behavior |
| **Atmosphere** | Document reading | Environmental details, Spatial audio | Reality-shifting, Sanity effects |
| **Narrative** | Collectible notes | Environmental storytelling | Unreliable narrator execution |

## Reference Game Feature Breakdown

### Amnesia: The Dark Descent (Primary Reference)
- Sanity meter (darkness damages, light restores)
- Physics-based interaction (doors, objects)
- No combat - hide and hope
- Resource management (oil for lantern, tinderboxes)
- Monster encounters in transitional areas, not puzzle rooms
- Gradual learning curve

### Outlast (Primary Reference)
- Camcorder with night vision (battery-dependent)
- Lockers/beds/closets for hiding
- Sprint + stamina system
- Absolutely no combat
- Found footage aesthetic

### SOMA (Narrative Reference)
- De-emphasized inventory for narrative focus
- Puzzles integrated with story
- Philosophical horror over jump scares
- Player choice moments (optional - may not fit demo)

### Layers of Fear (Environment Reference)
- Reality-shifting architecture
- Environment as the threat (not entity)
- Paintings and art as horror medium
- Minimal puzzle complexity

### P.T. (Atmosphere Reference)
- Looping hallway / non-Euclidean space
- Minimal mechanics, maximum atmosphere
- Sound design over visual horror
- "Don't look back" psychological triggers

### Alien: Isolation (AI Reference)
- Systemic AI with 100+ behavior nodes
- Progressive behavior unlocking
- Manual save points (high stakes)
- Single persistent threat
- "Psychopathic serendipity" - always at wrong time

## Sources

### Amnesia Design Analysis
- [Game Design Deep Dive: Amnesia's Sanity Meter](https://www.gamedeveloper.com/design/game-design-deep-dive-i-amnesia-i-s-sanity-meter-)
- [Immersion, Friction, and Fear in Amnesia](https://mechanicsofmagic.com/2021/04/14/immersion-friction-and-fear-in-amnesia-the-dark-descent/)

### Outlast Mechanics
- [Fear by Design: How Outlast Manipulates Space](https://medium.com/@ocranfield_5608/fear-by-design-how-outlast-manipulates-space-senses-and-the-player-0c6d44bae9fe)
- [Outlast: Flight or Hide](https://game-wisdom.com/analysis/outlast)

### SOMA Design Philosophy
- [The Five Foundational Design Pillars of SOMA](https://frictionalgames.com/2013-12-the-five-foundational-design-pillars-of-soma/)
- [How SOMA Creates Existential Dread](https://www.gamedeveloper.com/design/how-i-soma-i-creates-existential-dread)

### P.T. Analysis
- [P.T. Silent Hills Teaser Game Analysis](https://www.gamedeveloper.com/design/p-t-silent-hills-teaser-game-analysis)
- [Silent Halls: P.T., Freud, and Psychological Horror](https://press-start.gla.ac.uk/press-start/article/download/121/78/700)

### Alien: Isolation AI
- [The Perfect Organism: The AI of Alien: Isolation](https://www.gamedeveloper.com/design/the-perfect-organism-the-ai-of-alien-isolation)
- [Revisiting the AI of Alien: Isolation](https://www.gamedeveloper.com/design/revisiting-the-ai-of-alien-isolation)
- [Game Design Deep Dive: The Save System of Alien: Isolation](https://www.gamedeveloper.com/design/game-design-deep-dive-the-save-system-of-i-alien-isolation-i-)

### Horror Design Principles
- [Common Horror Game Mistakes](https://drwedge.uk/2024/10/11/common-horror-game-mistakes/)
- [10 Design Lessons from 30 Years of Horror Games](https://www.gamedeveloper.com/design/10-horror-game-design-tips)
- [Creating Successful Horror Games](https://medium.com/super-jump/creating-successful-horror-games-c552a2ac779e)
- [A Lack of Fright: Examining Jump Scare Horror Game Design](https://www.gamedeveloper.com/design/a-lack-of-fright-examining-jump-scare-horror-game-design)

### Sound Design
- [Silence is Scary: The Power of Sound Design in Horror Games](https://www.wayline.io/blog/silence-is-scary-sound-design-horror-games)
- [Drones and Ambient Music in Horror Games](https://gamemusic.net/drones-and-ambient-music-in-horror-games/)

### Immersion and Visible Body
- [Resident Evil Village and First-Person Video Game Immersion: Why Hands Create Intense Connection](https://theconversation.com/resident-evil-village-and-first-person-video-game-immersion-why-hands-create-intense-connection-161566)

### Layers of Fear
- [Layers of Fear: The Anatomy of a Remake](https://residentanna.substack.com/p/layers-of-fear-the-anatomy-of-a-remake)
- [Layers of Fear Reimagines Horror with Unreal Engine 5](https://www.unrealengine.com/en-US/developer-interviews/layers-of-fear-reimagines-horror-with-unreal-engine-5)

### Environmental Storytelling
- [When Buildings Dream: Environmental Storytelling in Horror Game Design](https://drwedge.uk/2025/05/04/when-buildings-dream-horror-game-design/)

### Unreliable Narrators
- [Unreliable Narrators in Horror Games: Stories That Keep You Guessing](https://horrorchronicles.com/horror-games-with-unreliable-narrators/)
