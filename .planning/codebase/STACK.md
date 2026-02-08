# Technology Stack

**Analysis Date:** 2025-02-07

## Languages

**Primary:**
- C++ (C++20) - Core gameplay logic in `Source/ProjectWalkingSim/`

**Secondary:**
- C# - Build tool scripts (`*.Build.cs`, `*.Target.cs`)
- Blueprint Visual Scripting - Game logic in `Content/Blueprints/`

## Runtime

**Engine:**
- Unreal Engine 5.7

**Build Settings:**
- BuildSettingsVersion.V6 (UE5.7 latest)
- PCHUsage: UseExplicitOrSharedPCHs

## Frameworks

**Core Engine Modules:**
- `Core` - Fundamental types and containers
- `CoreUObject` - UObject system, reflection, serialization
- `Engine` - Game framework, actors, components
- `InputCore` - Input handling foundation

**Input System:**
- Enhanced Input (`EnhancedPlayerInput`, `EnhancedInputComponent`)
- Configured in `Config/DefaultInput.ini`

**UI Framework:**
- CommonUI (enabled in `Config/DefaultGame.ini`)
- Slate/SlateCore available but not currently enabled

## Build Configuration

**Target Types:**
- Game: `Source/ProjectWalkingSim.Target.cs`
- Editor: `Source/ProjectWalkingSimEditor.Target.cs`

**Module Definition:**
- `Source/ProjectWalkingSim/ProjectWalkingSim.Build.cs`
- Type: Runtime
- LoadingPhase: Default

## Graphics Configuration

**Rendering Features (DefaultEngine.ini):**
- Ray Tracing: Enabled (`r.RayTracing=True`)
- Substrate: Enabled (`r.Substrate=True`)
- Virtual Shadow Maps: Enabled (`r.Shadow.Virtual.Enable=1`)
- Dynamic Global Illumination: Lumen (`r.DynamicGlobalIlluminationMethod=1`)
- Reflection Method: Lumen (`r.ReflectionMethod=1`)
- Mesh Distance Fields: Enabled
- Static Lighting: Disabled (fully dynamic)

**Shader Models:**
- Windows DX12: SM6 (`PCD3D_SM6`)
- Windows DX11: SM5 (`PCD3D_SM5`)
- Linux Vulkan: SM6 (`SF_VULKAN_SM6`)
- Mac Metal: SM6 (`SF_METAL_SM6`)

**Default RHI:**
- Windows: DirectX 12 (`DefaultGraphicsRHI_DX12`)

## Plugins

**Enabled:**
- ModelingToolsEditorMode (Editor only) - 3D modeling tools

**Available but Not Enabled:**
- OnlineSubsystem (commented in Build.cs)
- OnlineSubsystemSteam (mentioned in Build.cs)

## Platform Configuration

**Primary Target:**
- Desktop (Maximum graphics quality)

**Platform Support Configured:**
- Windows (Primary)
- Linux (Vulkan)
- Mac (Metal)
- Android (File server enabled for development)

**Hardware Targeting:**
- Class: Desktop
- Graphics: Maximum

## Development Environment

**IDE:**
- Visual Studio 2022 (`.vs/`, `.vsconfig`)
- JetBrains Rider (`.idea/`)

**Visual Studio Components Required:**
- Unreal Engine Debugger
- Unreal Engine IDE Support
- MSVC v14.38 and v14.44 toolsets
- LLVM/Clang
- Windows 11 SDK (22621)
- .NET 4.6.2 Targeting Pack

## Audio Configuration

**Sample Rate:** 48000 Hz
**Callback Buffer Size:** 1024 frames
**Max Channels:** Default (0)
**Source Workers:** 4

## Project Metadata

**Project Name:** Project Serene
**Copyright:** Copyright Null Lantern
**Default Map:** `/Engine/Maps/Templates/OpenWorld`

---

*Stack analysis: 2025-02-07*
