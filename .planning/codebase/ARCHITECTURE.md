# Architecture

**Analysis Date:** 2026-02-07

## Pattern Overview

**Overall:** Unreal Engine Gameplay Framework (Single-Module Game Project)

**Key Characteristics:**
- Standard UE5 GameMode-based architecture with `ASereneGameMode` as the primary game coordinator
- Single primary runtime module (`ProjectWalkingSim`) with Public/Private header separation
- Early-stage project with minimal custom classes - relies heavily on engine defaults
- Enhanced Input System configured as the default input handling mechanism
- High-fidelity rendering pipeline with ray tracing, Lumen GI, and Substrate materials

## Layers

**Gameplay Framework Layer:**
- Purpose: Coordinates game rules, spawning, and match flow
- Location: `Source/ProjectWalkingSim/`
- Contains: GameMode, future PlayerController, Pawn, GameState classes
- Depends on: Unreal Engine `GameFramework` module
- Used by: Level Blueprints, spawned actors

**Module Layer:**
- Purpose: Primary game module registration and initialization
- Location: `Source/ProjectWalkingSim/ProjectWalkingSim.cpp`
- Contains: Module implementation via `IMPLEMENT_PRIMARY_GAME_MODULE`
- Depends on: `Core`, `CoreUObject`, `Engine`, `InputCore`
- Used by: Unreal Build Tool, Engine module loader

**Configuration Layer:**
- Purpose: Project-wide settings for rendering, input, and platform targets
- Location: `Config/`
- Contains: INI files for engine, game, input, and editor settings
- Depends on: Engine subsystems
- Used by: All runtime systems

**Content Layer:**
- Purpose: Asset storage for levels, blueprints, and game content
- Location: `Content/`
- Contains: Maps, Blueprint assets, materials, textures (currently minimal)
- Depends on: C++ classes for Blueprint parent types
- Used by: Editor, packaged game

## Data Flow

**Game Initialization Flow:**

1. Engine loads `ProjectWalkingSim` module via `IMPLEMENT_PRIMARY_GAME_MODULE`
2. `DefaultEngine.ini` specifies default map: `/Engine/Maps/Templates/OpenWorld`
3. GameMode class (`ASereneGameMode`) instantiated based on World Settings or project defaults
4. GameMode controls player spawning and game rules

**Input Flow:**

1. Enhanced Input System enabled via `DefaultInput.ini` settings
2. `DefaultPlayerInputClass` set to `EnhancedPlayerInput`
3. `DefaultInputComponentClass` set to `EnhancedInputComponent`
4. Input Actions/Mappings defined in Content (not yet created)
5. Player Controller receives mapped input events

**State Management:**
- Game state managed through UE5 Gameplay Framework hierarchy: GameMode -> GameState -> PlayerState
- No custom state management implemented yet - uses engine defaults

## Key Abstractions

**ASereneGameMode:**
- Purpose: Central game coordinator for "Project Serene" walking simulator
- Examples: `Source/ProjectWalkingSim/Public/SereneGameMode.h`, `Source/ProjectWalkingSim/Private/SereneGameMode.cpp`
- Pattern: Empty subclass of `AGameModeBase`, ready for customization
- API Macro: `PROJECTWALKINGSIM_API` for DLL export

**Module Definition:**
- Purpose: Defines module build rules and dependencies
- Examples: `Source/ProjectWalkingSim/ProjectWalkingSim.Build.cs`
- Pattern: Standard UE5 ModuleRules with explicit PCH usage

## Entry Points

**Game Target:**
- Location: `Source/ProjectWalkingSim.Target.cs`
- Triggers: Building for standalone game
- Responsibilities: Configures `TargetType.Game`, `BuildSettingsVersion.V6`

**Editor Target:**
- Location: `Source/ProjectWalkingSimEditor.Target.cs`
- Triggers: Building for editor
- Responsibilities: Configures `TargetType.Editor`, includes editor-only code

**Module Entry:**
- Location: `Source/ProjectWalkingSim/ProjectWalkingSim.cpp`
- Triggers: Engine module initialization
- Responsibilities: Registers module with `FDefaultGameModuleImpl`

**Default Map:**
- Location: Engine template `/Engine/Maps/Templates/OpenWorld`
- Triggers: Game launch, PIE (Play In Editor)
- Responsibilities: Initial world loading (no custom default map set)

## Error Handling

**Strategy:** Engine-default error handling (no custom implementation)

**Patterns:**
- Relies on UE5 assertion macros (`check`, `ensure`, `verify`)
- Logging via UE_LOG macro (not yet utilized in custom code)

## Cross-Cutting Concerns

**Logging:** Not implemented in custom code; use `UE_LOG(LogTemp, ...)` or define custom log category

**Validation:** No custom validation; use UE5 property specifiers and assertions

**Authentication:** Not applicable (single-player walking simulator)

---

*Architecture analysis: 2026-02-07*
