# External Integrations

**Analysis Date:** 2025-02-07

## APIs & External Services

**Online Services:**
- None currently enabled
- OnlineSubsystem module available but commented out in `Source/ProjectWalkingSim/ProjectWalkingSim.Build.cs`
- Steam integration available but not enabled

## Data Storage

**Databases:**
- None detected
- Local save games via Unreal's built-in save system (not yet implemented)

**File Storage:**
- Local filesystem only
- Saved games directory: `Saved/SaveGames/`

**Caching:**
- Derived Data Cache: `DerivedDataCache/`
- Engine-managed asset caching

## Authentication & Identity

**Auth Provider:**
- None configured
- No online subsystem enabled

## Monitoring & Observability

**Error Tracking:**
- None external
- Unreal Engine built-in crash reporter available

**Logs:**
- Unreal Engine logging system
- Output to `Saved/Logs/`

## CI/CD & Deployment

**Hosting:**
- Not configured (standalone game)

**CI Pipeline:**
- None detected

**Version Control:**
- Git repository present (`.git/`)

## Environment Configuration

**Required env vars:**
- None (all configuration in INI files)

**Secrets location:**
- None required currently

## Mobile Development

**Android File Server:**
- Plugin enabled for development (`Config/DefaultEngine.ini`)
- Connection: USB Only
- Security token configured
- Not included in shipping builds

## VR Support

**Input Configuration (DefaultInput.ini):**
- Vive controllers configured
- Oculus Touch configured
- Valve Index configured
- Windows Mixed Reality configured

**VR Plugins:**
- Not explicitly enabled in `.uproject`
- Input bindings suggest VR consideration

## Webhooks & Callbacks

**Incoming:**
- None

**Outgoing:**
- None

## Third-Party Libraries

**Marketplace Plugins:**
- None detected

**Custom Plugins:**
- None (no `Plugins/` directory)

**Engine Plugins Used:**
- ModelingToolsEditorMode (built-in, editor only)

## Potential Future Integrations

Based on commented code in `ProjectWalkingSim.Build.cs`:

**Online Features (not enabled):**
```cpp
// Uncomment if you are using online features
// PrivateDependencyModuleNames.Add("OnlineSubsystem");
```

**Slate UI (not enabled):**
```cpp
// Uncomment if you are using Slate UI
// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
```

**Steam (not enabled):**
```cpp
// To include OnlineSubsystemSteam, add it to the plugins section
```

---

*Integration audit: 2025-02-07*
