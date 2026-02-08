# Codebase Structure

**Analysis Date:** 2026-02-07

## Directory Layout

```
ProjectWalkingSim/
├── .planning/              # GSD planning documents
├── Config/                 # INI configuration files
│   ├── DefaultEditor.ini   # Editor settings (empty)
│   ├── DefaultEngine.ini   # Rendering, platform, map settings
│   ├── DefaultGame.ini     # Project metadata
│   └── DefaultInput.ini    # Input axis and action mappings
├── Content/                # All game assets
│   ├── Blueprints/         # Blueprint assets
│   │   └── Game/           # Gameplay blueprints (empty)
│   ├── Collections/        # Asset collections
│   ├── Developers/         # Per-developer scratch content
│   │   └── gunsy/          # Developer-specific folder
│   └── Maps/               # Level files
│       └── TestMap.umap    # Test level
├── DerivedDataCache/       # Engine-generated cache (not committed)
├── Intermediate/           # Build intermediates (not committed)
├── Saved/                  # Logs, autosaves, configs (not committed)
├── Source/                 # C++ source code
│   ├── ProjectWalkingSim.Target.cs       # Game build target
│   ├── ProjectWalkingSimEditor.Target.cs # Editor build target
│   └── ProjectWalkingSim/                # Primary game module
│       ├── ProjectWalkingSim.Build.cs    # Module build rules
│       ├── ProjectWalkingSim.h           # Module header
│       ├── ProjectWalkingSim.cpp         # Module implementation
│       ├── Public/                       # Public headers (API)
│       │   └── SereneGameMode.h
│       └── Private/                      # Private implementations
│           └── SereneGameMode.cpp
└── ProjectWalkingSim.uproject            # Project definition
```

## Directory Purposes

**Source/ProjectWalkingSim/:**
- Purpose: Primary game module containing all C++ gameplay code
- Contains: Headers, implementations, build configuration
- Key files: `ProjectWalkingSim.Build.cs`, `Public/SereneGameMode.h`

**Source/ProjectWalkingSim/Public/:**
- Purpose: Public API headers exposed to other modules and Blueprints
- Contains: Class declarations with `PROJECTWALKINGSIM_API` macro
- Key files: `SereneGameMode.h`

**Source/ProjectWalkingSim/Private/:**
- Purpose: Private implementation files not exposed outside module
- Contains: .cpp files, internal-only classes
- Key files: `SereneGameMode.cpp`

**Content/Blueprints/:**
- Purpose: Blueprint assets organized by category
- Contains: Blueprint classes, widgets, animation blueprints
- Key files: Currently empty, awaiting content creation

**Content/Blueprints/Game/:**
- Purpose: Gameplay-specific blueprints (GameMode BP, Player BP, etc.)
- Contains: Should contain BP subclasses of C++ gameplay classes
- Key files: Currently empty

**Content/Maps/:**
- Purpose: Level/World assets
- Contains: .umap files and associated level data
- Key files: `TestMap.umap`

**Config/:**
- Purpose: Project and engine configuration
- Contains: INI files controlling rendering, input, platform settings
- Key files: `DefaultEngine.ini` (rendering pipeline), `DefaultInput.ini` (Enhanced Input)

## Key File Locations

**Entry Points:**
- `Source/ProjectWalkingSim/ProjectWalkingSim.cpp`: Module registration
- `Source/ProjectWalkingSim.Target.cs`: Game target definition
- `Source/ProjectWalkingSimEditor.Target.cs`: Editor target definition

**Configuration:**
- `ProjectWalkingSim.uproject`: Project definition, module list, plugin settings
- `Config/DefaultEngine.ini`: Rendering pipeline (Lumen, ray tracing, Substrate)
- `Config/DefaultInput.ini`: Enhanced Input System configuration
- `Config/DefaultGame.ini`: Project name ("Project Serene"), copyright

**Core Logic:**
- `Source/ProjectWalkingSim/Public/SereneGameMode.h`: Game coordinator class
- `Source/ProjectWalkingSim/Private/SereneGameMode.cpp`: GameMode implementation

**Testing:**
- `Content/Maps/TestMap.umap`: Test level for development

## Naming Conventions

**Files:**
- C++ classes: PascalCase matching class name (`SereneGameMode.h`)
- Module files: Match module name (`ProjectWalkingSim.cpp`)
- Config files: `Default{System}.ini` pattern

**Directories:**
- Source folders: PascalCase (`ProjectWalkingSim/`)
- Content folders: PascalCase (`Blueprints/`, `Maps/`)
- Separation: `Public/` for headers, `Private/` for implementations

**Classes:**
- Prefix: Standard UE prefixes (`A` for Actors, `U` for UObjects, `F` for structs)
- GameMode: `ASereneGameMode` (A-prefixed Actor)
- API macro: `PROJECTWALKINGSIM_API` (uppercase module name)

## Where to Add New Code

**New Gameplay Class (Actor-based):**
- Header: `Source/ProjectWalkingSim/Public/{ClassName}.h`
- Implementation: `Source/ProjectWalkingSim/Private/{ClassName}.cpp`
- Include `{ClassName}.generated.h` in header
- Use `PROJECTWALKINGSIM_API` macro for Blueprint exposure

**New Component:**
- Header: `Source/ProjectWalkingSim/Public/Components/{ComponentName}.h`
- Implementation: `Source/ProjectWalkingSim/Private/Components/{ComponentName}.cpp`
- Parent: `UActorComponent` or `USceneComponent`

**New Blueprint:**
- Gameplay: `Content/Blueprints/Game/BP_{Name}.uasset`
- UI/Widgets: `Content/Blueprints/UI/WBP_{Name}.uasset`
- Characters: `Content/Blueprints/Characters/BP_{Name}.uasset`

**New Level:**
- Location: `Content/Maps/{LevelName}.umap`
- Consider World Partition for large open worlds

**Input Actions (Enhanced Input):**
- Input Actions: `Content/Input/Actions/IA_{ActionName}.uasset`
- Input Mapping Contexts: `Content/Input/IMC_{ContextName}.uasset`

**Utilities/Helpers:**
- Function Libraries: `Source/ProjectWalkingSim/Public/Libraries/{LibName}.h`
- Subsystems: `Source/ProjectWalkingSim/Public/Subsystems/{SubsystemName}.h`

## Special Directories

**DerivedDataCache/:**
- Purpose: Engine shader and asset compilation cache
- Generated: Yes (by Unreal Editor)
- Committed: No (add to .gitignore)

**Intermediate/:**
- Purpose: Build system intermediates (object files, PCH)
- Generated: Yes (by Unreal Build Tool)
- Committed: No (add to .gitignore)

**Saved/:**
- Purpose: Logs, autosaves, local config, crash dumps
- Generated: Yes (by Editor and Engine)
- Committed: No (add to .gitignore)

**Content/Developers/:**
- Purpose: Per-developer scratch content (experiments, WIP)
- Generated: No (manually created)
- Committed: Optional (often excluded in team projects)

**.vs/ and .idea/:**
- Purpose: IDE project files (Visual Studio, Rider)
- Generated: Yes (by IDE)
- Committed: Optional (.idea typically committed for Rider, .vs excluded)

---

*Structure analysis: 2026-02-07*
