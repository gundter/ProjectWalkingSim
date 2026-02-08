# Codebase Concerns

**Analysis Date:** 2026-02-07

## Tech Debt

**Empty GameMode Implementation:**
- Issue: `ASereneGameMode` is an empty shell class with no custom logic
- Files: `Source/ProjectWalkingSim/Public/SereneGameMode.h`, `Source/ProjectWalkingSim/Private/SereneGameMode.cpp`
- Impact: No game-specific spawning, rules, or initialization implemented; relies entirely on engine defaults
- Fix approach: Implement virtual overrides for `BeginPlay()`, `StartPlay()`, player spawning, and game state management

**No Default Game Map:**
- Issue: Project uses engine template map `/Engine/Maps/Templates/OpenWorld` instead of custom map
- Files: `Config/DefaultEngine.ini` line 4
- Impact: Starting the game loads an engine template; `Content/Maps/TestMap.umap` exists but is not set as default
- Fix approach: Set `GameDefaultMap=/Game/Maps/TestMap` in DefaultEngine.ini

**Boilerplate Copyright Notices:**
- Issue: All source files contain placeholder "Fill out your copyright notice in the Description page of Project Settings"
- Files: All `.cpp` and `.h` files in `Source/ProjectWalkingSim/`
- Impact: Minor; unprofessional appearance in source files
- Fix approach: Replace with "Copyright Null Lantern" as specified in DefaultGame.ini

**Empty Blueprints Directory:**
- Issue: `Content/Blueprints/Game/` directory exists but contains no Blueprint assets
- Files: `Content/Blueprints/Game/` (empty)
- Impact: Suggests incomplete setup; no player character, controller, or game Blueprints exist
- Fix approach: Create required Blueprint classes (PlayerCharacter, PlayerController, GameMode Blueprint) or implement in C++

## Known Bugs

No runtime bugs identified - project is too early-stage for bug accumulation.

## Security Considerations

**Android Security Token Exposed:**
- Risk: `SecurityToken=8AB725E24559C42975422087C145612B` hardcoded in config
- Files: `Config/DefaultEngine.ini` line 92
- Current mitigation: Token only used for development file server (USB/local connection)
- Recommendations: Ensure this token is regenerated before any public builds; add DefaultEngine.ini to .gitignore or use environment-specific configs for sensitive values

## Performance Bottlenecks

**High-Fidelity Rendering Pipeline:**
- Problem: Project configured for maximum graphical fidelity without scalability settings
- Files: `Config/DefaultEngine.ini` lines 6-31
- Cause: Ray tracing, Substrate materials, Virtual Shadow Maps, Lumen GI all enabled simultaneously
- Improvement path: Implement scalability settings (Engine Scalability Groups); add quality presets for different hardware tiers

**No Static Lighting:**
- Problem: `r.AllowStaticLighting=False` means all lighting is fully dynamic
- Files: `Config/DefaultEngine.ini` line 7
- Cause: Design decision for dynamic time-of-day, but impacts performance on lower-end hardware
- Improvement path: Consider mixed lighting for indoor/static areas; implement LOD systems for distance-based GI quality

## Fragile Areas

**Minimal C++ Foundation:**
- Files: Only 4 source files total in `Source/ProjectWalkingSim/`
- Why fragile: No custom systems implemented; any future development requires building from scratch
- Safe modification: Project is clean slate - no existing patterns to break
- Test coverage: No tests exist

**No Input Action Mappings:**
- Files: `Config/DefaultInput.ini` - Enhanced Input configured but no Input Actions/Mapping Contexts
- Why fragile: Player cannot interact with the game; walking simulator has no walk input
- Safe modification: Create InputAction assets and InputMappingContext in Content; bind in PlayerController
- Test coverage: No input handling to test

## Scaling Limits

**Single Module Architecture:**
- Current capacity: Suitable for small to medium project scope
- Limit: Single `ProjectWalkingSim` module will become unwieldy beyond ~50-100 classes
- Scaling path: Plan module structure early; consider separate modules for Systems, UI, Environment, etc.

**No World Partition Setup:**
- Current capacity: TestMap is a single level (non-partitioned)
- Limit: Large open world walking simulator will hit streaming and memory limits
- Scaling path: Enable World Partition in level settings for large maps; implement HLOD for distant objects

## Dependencies at Risk

**UE 5.7 Preview/Early Adoption:**
- Risk: UE 5.7 is cutting-edge; API changes and stability issues possible
- Impact: Build breaks on engine updates; undocumented behavior changes
- Migration plan: Pin to specific engine version; maintain 5.6 compatibility path if needed

**Minimal Module Dependencies:**
- Risk: `Core`, `CoreUObject`, `Engine`, `InputCore` only - no gameplay modules enabled
- Impact: Need to add dependencies as features are built (AI, Navigation, Niagara, etc.)
- Migration plan: Add modules incrementally in `ProjectWalkingSim.Build.cs` as needed

## Missing Critical Features

**No Player Character:**
- Problem: Walking simulator has no player pawn or character class
- Blocks: Cannot spawn player, no camera, no movement, no interaction

**No PlayerController:**
- Problem: No custom PlayerController to handle input and camera
- Blocks: Cannot process input, cannot possess pawns, no HUD integration

**No Enhanced Input Assets:**
- Problem: Enhanced Input System configured but no InputAction or InputMappingContext assets
- Blocks: No input binding possible; player cannot move or interact

**No Custom Logging Category:**
- Problem: No `DECLARE_LOG_CATEGORY_EXTERN` for project-specific logging
- Blocks: All logging goes to LogTemp; difficult to filter project-specific logs

**No Game Instance:**
- Problem: No custom UGameInstance class for persistent data across levels
- Blocks: Cannot save settings, progress, or state across map transitions

## Test Coverage Gaps

**No Automated Tests:**
- What's not tested: Entire codebase (4 source files)
- Files: `Source/ProjectWalkingSim/**/*`
- Risk: No regression detection; manual testing only
- Priority: Low (project too early for meaningful tests)

**No Automation Project:**
- What's not tested: Functional/integration tests via Gauntlet or Session Frontend
- Files: No `Tests/` directory or test module
- Risk: Cannot validate gameplay flows automatically
- Priority: Medium - establish testing infrastructure early

---

*Concerns audit: 2026-02-07*
