# Coding Conventions

**Analysis Date:** 2026-02-07

## Project Overview

This is an early-stage Unreal Engine 5.7 project ("Project Serene") with minimal C++ code. Conventions are based on observed patterns and standard Unreal Engine coding guidelines.

## Naming Patterns

**Files:**
- Module files: `{ModuleName}.cpp`, `{ModuleName}.h` (e.g., `ProjectWalkingSim.cpp`)
- Class files: PascalCase matching class name without prefix (e.g., `SereneGameMode.h` for `ASereneGameMode`)
- Build configuration: `{ModuleName}.Build.cs`

**C++ Classes (UE Prefixes):**
- `A` prefix: Actor-derived classes (e.g., `ASereneGameMode`)
- `U` prefix: UObject-derived classes (not yet used)
- `F` prefix: Structs and non-UObject types (not yet used)
- `E` prefix: Enums (not yet used)
- `I` prefix: Interfaces (not yet used)
- `T` prefix: Templates (not yet used)

**Functions:**
- PascalCase for all functions (Unreal standard)

**Variables:**
- PascalCase for member variables (Unreal standard)
- `b` prefix for boolean variables (e.g., `bIsEnabled`)

**API Macro:**
- Use `PROJECTWALKINGSIM_API` for classes/functions exported from the module

## Code Style

**Header Guards:**
- Use `#pragma once` (not traditional include guards)

**Include Order:**
1. Module header (matching .cpp file name)
2. Engine includes (`CoreMinimal.h`, framework headers)
3. Generated header last (`{ClassName}.generated.h`)

**Example from `SereneGameMode.h`:**
```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SereneGameMode.generated.h"
```

**Formatting:**
- Tab indentation (Unreal standard)
- Opening braces on new line for class/function definitions
- Opening braces on same line for control structures

**Copyright Notice:**
- Standard UE template: `// Fill out your copyright notice in the Description page of Project Settings.`
- Project copyright: "Copyright Null Lantern." (defined in `Config/DefaultGame.ini`)

## Directory Structure

**Source Layout:**
```
Source/{ModuleName}/
├── Private/          # Implementation files (.cpp)
├── Public/           # Header files (.h)
├── {ModuleName}.cpp  # Module implementation
├── {ModuleName}.h    # Module header
└── {ModuleName}.Build.cs  # Build configuration
```

**Content Layout:**
```
Content/
├── Blueprints/Game/  # Game-specific blueprints
├── Collections/      # Asset collections
├── Developers/       # Per-developer test content
└── Maps/             # Level files (.umap)
```

## UCLASS/USTRUCT/UPROPERTY Usage

**UCLASS Declaration:**
```cpp
UCLASS()
class PROJECTWALKINGSIM_API ASereneGameMode : public AGameModeBase
{
    GENERATED_BODY()
};
```

**Key Patterns:**
- Always include `GENERATED_BODY()` macro immediately after opening brace
- Export classes with `PROJECTWALKINGSIM_API` for cross-module access
- Empty `UCLASS()` specifier for basic classes (no BlueprintType, etc.)

**UPROPERTY Guidelines (for future use):**
```cpp
// Visible in editor, not editable
UPROPERTY(VisibleAnywhere, Category = "Category")

// Editable in editor
UPROPERTY(EditAnywhere, Category = "Category")

// Editable and Blueprint accessible
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Category")

// Blueprint read-only
UPROPERTY(BlueprintReadOnly, Category = "Category")
```

**UFUNCTION Guidelines (for future use):**
```cpp
// Blueprint callable
UFUNCTION(BlueprintCallable, Category = "Category")

// Blueprint implementable event
UFUNCTION(BlueprintImplementableEvent, Category = "Category")

// Native implementation with Blueprint override
UFUNCTION(BlueprintNativeEvent, Category = "Category")
```

## Import Organization

**Build.cs Dependencies:**
```csharp
// Core engine modules (always needed)
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "InputCore"
});

// UI modules (commented template)
// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

// Online features (commented template)
// PrivateDependencyModuleNames.Add("OnlineSubsystem");
```

**PCH Usage:**
- `PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;`

## Input System

**Enhanced Input:**
- Project uses Enhanced Input System (UE5 standard)
- `DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput`
- `DefaultInputComponentClass=/Script/EnhancedInput.EnhancedInputComponent`

**Input Mapping Assets:**
- Create Input Action assets in `Content/Input/Actions/`
- Create Input Mapping Context assets in `Content/Input/Mappings/`

## Error Handling

**Patterns (for future use):**
- Use `check()` for fatal assertions (development only)
- Use `ensure()` for non-fatal assertions with logging
- Use `UE_LOG()` macro for logging
- Return `nullptr` or use `TOptional<>` for fallible operations

## Logging

**Framework:** UE_LOG macro

**Pattern:**
```cpp
// Define log category in header
DECLARE_LOG_CATEGORY_EXTERN(LogSerene, Log, All);

// Define in cpp
DEFINE_LOG_CATEGORY(LogSerene);

// Usage
UE_LOG(LogSerene, Log, TEXT("Message"));
UE_LOG(LogSerene, Warning, TEXT("Warning message"));
UE_LOG(LogSerene, Error, TEXT("Error message"));
```

## Blueprint vs C++ Guidelines

**Use C++ for:**
- Game modes and core systems
- Performance-critical code
- Complex algorithms
- Base classes for Blueprint extension
- Networking/replication logic

**Use Blueprints for:**
- UI/UMG widgets
- Quick prototyping
- Designer-tweakable parameters
- Visual effects and animations
- Level-specific logic

**Hybrid Approach:**
- Create C++ base classes with `BlueprintNativeEvent` or `BlueprintImplementableEvent`
- Expose `UPROPERTY` variables with `EditAnywhere, BlueprintReadWrite`
- Use `BlueprintCallable` for C++ functions needed in Blueprints

## Comments

**Documentation Style:**
```cpp
/**
 * Brief description of class/function.
 *
 * Detailed description if needed.
 *
 * @param ParamName - Description of parameter
 * @return Description of return value
 */
```

**Inline Comments:**
- Use `//` for single-line comments
- Comment non-obvious code, not obvious code
- TODO format: `// TODO: Description`

## Module Design

**Single Runtime Module:**
- Module Name: `ProjectWalkingSim`
- Type: Runtime
- Loading Phase: Default
- Dependencies: Engine

**For New Modules:**
1. Create `Source/{ModuleName}/` directory
2. Add `{ModuleName}.Build.cs`
3. Add module entry in `.uproject` file
4. Create `Public/` and `Private/` subdirectories

---

*Convention analysis: 2026-02-07*
