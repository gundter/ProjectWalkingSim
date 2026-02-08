# Technology Stack: The Juniper Tree

**Project:** First-person psychological horror game
**Engine:** Unreal Engine 5.7.2
**Researched:** 2026-02-07
**Overall Confidence:** HIGH

---

## Recommended Stack

### Core Character System

| Technology | Version | Purpose | Confidence | Why |
|------------|---------|---------|------------|-----|
| **First Person Rendering System** | UE5.7 Native | True first-person with visible body | HIGH | UE5.7 introduced dedicated first-person rendering with proper world-space representation, self-shadowing via `r.FirstPerson.SelfShadow`, and material nodes for first-person space transforms. Eliminates need for separate first-person arms mesh. |
| **Character Movement Component** | UE5.7 Native | Movement with built-in crouch | HIGH | Native `CanCrouch` capability, `Crouch()`/`UnCrouch()` functions. No custom implementation needed for basic crouch. Extend only for stamina/speed modifiers. |
| **Enhanced Input System** | UE5.7 Native | Input handling | HIGH | **Mandatory** - legacy input deprecated. Provides Input Actions, Input Mapping Contexts, runtime remapping, and chorded actions. C++ binds via `UEnhancedInputComponent::BindAction()`. |

**C++ Setup Pattern for Enhanced Input:**
```cpp
void AJuniperCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    UEnhancedInputComponent* EnhancedInput =
        Cast<UEnhancedInputComponent>(PlayerInputComponent);

    EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered,
        this, &AJuniperCharacter::OnMove);
    EnhancedInput->BindAction(IA_Crouch, ETriggerEvent::Started,
        this, &AJuniperCharacter::OnCrouchPressed);
}
```

**First-Person Setup Pattern:**
```cpp
// In character constructor or BeginPlay
FirstPersonMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);

// For shadow casting (world representation)
WorldSpaceBodyMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::WorldSpaceRepresentation);
WorldSpaceBodyMesh->SetOwnerNoSee(true);
```

---

### Inventory System

| Technology | Version | Purpose | Confidence | Why |
|------------|---------|---------|------------|-----|
| **Custom UActorComponent** | C++ Native | 8-slot inventory | HIGH | GAS is overkill for simple 8-slot inventory. Component-based approach allows attaching to player and any storage containers. Event-driven updates for UI. |
| **FStruct-based Items** | C++ Native | Item data | HIGH | Use `USTRUCT` with `SaveGame` specifier for items. Data Tables for item definitions. Avoid inheritance hierarchies. |
| **DataTable** | UE5.7 Native | Item definitions | HIGH | Designer-friendly item editing without recompile. CSV/JSON import support. |

**DO NOT USE:**
- **Gameplay Ability System (GAS)** for inventory - Adds unnecessary complexity. GAS AttributeSets can only have one instance per class, making multi-item inventories awkward. Save GAS for stamina/health attributes if needed later.
- **Blueprint-only inventory** - Performance overhead for frequent operations, harder to unit test.

**Recommended Architecture:**
```cpp
USTRUCT(BlueprintType)
struct FJuniperItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, SaveGame)
    FName ItemID;

    UPROPERTY(EditAnywhere, SaveGame)
    int32 StackCount = 1;

    UPROPERTY(EditAnywhere)
    FText DisplayName;

    UPROPERTY(EditAnywhere)
    TSoftObjectPtr<UTexture2D> Icon;
};

UCLASS()
class UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

    UPROPERTY(SaveGame)
    TArray<FJuniperItem> Items;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryChanged,
        const TArray<FJuniperItem>&, NewItems);

    UPROPERTY(BlueprintAssignable)
    FOnInventoryChanged OnInventoryChanged;
};
```

---

### AI System (Monster)

| Technology | Version | Purpose | Confidence | Why |
|------------|---------|---------|------------|-----|
| **State Tree** | UE5.7 Native | Monster behavior logic | HIGH | Epic's modern replacement for Behavior Trees. Combines state machine clarity with behavior tree flexibility. Better organization for horror AI states (Patrol, Search, Chase, Lose Interest). |
| **AI Perception Component** | UE5.7 Native | Sight/hearing detection | HIGH | Built-in sight cone with `Sight Radius`, `Lose Sight Radius`, `Peripheral Vision Half Angle`. Hearing via `ReportNoiseEvent()`. Touch sense for hiding detection. |
| **Smart Objects** | UE5.7 Native | Environmental interaction points | MEDIUM | Defines points where AI interacts with world (search locations, patrol points). Pairs well with State Tree. |
| **EQS (Environment Query System)** | UE5.7 Native | Pathfinding queries | HIGH | Query for patrol points, search locations, cover evaluation. Integrates with State Tree tasks. |

**DO NOT USE:**
- **Behavior Trees** - Still functional but State Tree is Epic's modern direction. Behavior Trees tick every frame; State Tree runs on-demand.
- **Custom FSM in C++** - Reinventing the wheel. State Tree provides visual debugging, easier iteration.
- **HTN (Hierarchical Task Network)** - Third-party plugin, overkill for single-monster horror game. Better for complex multi-agent planning.

**Monster AI Architecture:**
```
State Tree Structure:
Root
├── Patrol (default)
│   └── Tasks: Move to EQS point, Wait, Check environment
│   └── Transitions: On sight/sound → Investigate
├── Investigate
│   └── Tasks: Move to stimulus, Search area, Play audio cue
│   └── Transitions: Player spotted → Chase, Timeout → Patrol
├── Chase
│   └── Tasks: Pursue target, Attack if in range
│   └── Transitions: Lost sight → Search, Player hidden → Search
└── Search
    └── Tasks: Check hiding spots, Expand search radius
    └── Transitions: Found → Chase, Timeout → Patrol
```

**AI Perception Setup:**
```cpp
// In AIController constructor
PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));

// Sight configuration
UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
SightConfig->SightRadius = 2500.f;  // 25 meters
SightConfig->LoseSightRadius = 3000.f;
SightConfig->PeripheralVisionAngleDegrees = 60.f;
SightConfig->DetectionByAffiliation.bDetectEnemies = true;
SightConfig->SetMaxAge(5.f);  // Forget after 5 seconds out of sight

// Hearing configuration
UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
HearingConfig->HearingRange = 5000.f;  // 50 meters for loud sounds
HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

PerceptionComponent->ConfigureSense(*SightConfig);
PerceptionComponent->ConfigureSense(*HearingConfig);
PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
```

---

### Lighting and Rendering

| Technology | Version | Purpose | Confidence | Why |
|------------|---------|---------|------------|-----|
| **Lumen GI + Reflections** | UE5.7 Native | Dynamic global illumination | HIGH | Real-time GI essential for horror atmosphere. Flashlight bounce lighting, dynamic shadows. Enabled by default in UE5. |
| **Virtual Shadow Maps (VSM)** | UE5.7 Native | High-resolution shadows | HIGH | Film-quality shadows, works with Nanite. Page-based caching reduces cost. Critical for horror shadow detail. |
| **Post Process Volume** | UE5.7 Native | Darkness/atmosphere control | HIGH | Control Lumen settings per-area. `Skylight Leaking` setting prevents pure black while maintaining horror atmosphere. |

**Performance Notes:**
- **Limit dynamic lights:** Lumen works best with few high-quality lights rather than many low-quality ones. 3-5 major dynamic lights per area.
- **VSM page budget:** Monitor with `stat virtualshadowmaps`. Reduce shadow distance for indoor scenes.
- **Disable static lighting:** Set `Allow Static Lighting = false` in project settings for full Lumen benefits and first-person self-shadow support.

**Horror Lighting Configuration:**
```cpp
// Post Process Volume settings for horror atmosphere
Lumen:
  - Lumen Scene View Distance: 20000 (20 meters indoor, increase for outdoor)
  - Final Gather Quality: 1.0 (balance noise vs performance)
  - Max Trace Distance: 10000 (10 meters minimum for indoor to prevent leaks)

Atmosphere:
  - Skylight Leaking: 0.05 (5% leak prevents pure black, maintains tension)
  - Diffuse Color Boost: 1.0 (keep physical accuracy)

Shadows:
  - Shadow Bias: 0.5 (reduce self-shadowing artifacts)
  - Screen Percentage: 100 (full resolution for detail)
```

**Darkness System Approach:**
- Use **Emissive materials** on key objects for player guidance
- **Point lights with low intensity** for ambient fear (flickering candles)
- Player flashlight as **primary interaction** with environment
- Lumen's real-time bounce from flashlight creates dynamic horror atmosphere

---

### Save/Load System

| Technology | Version | Purpose | Confidence | Why |
|------------|---------|---------|------------|-----|
| **USaveGame** | UE5.7 Native | Save data container | HIGH | Engine-native serialization. Use `UPROPERTY(SaveGame)` specifier for automatic serialization. |
| **AsyncSaveGameToSlot** | UE5.7 Native | Async saving | HIGH | Prevents frame hitches during autosave. Required for certification on consoles. |
| **GameInstance Subsystem** | UE5.7 Native | Save/load management | HIGH | Persists across level loads. Central save manager. |

**Save Architecture:**
```cpp
UCLASS()
class UJuniperSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    // Player state
    UPROPERTY(SaveGame)
    FTransform PlayerTransform;

    UPROPERTY(SaveGame)
    TArray<FJuniperItem> InventoryItems;

    // World state
    UPROPERTY(SaveGame)
    TMap<FName, bool> PickedUpItems;  // Items removed from world

    UPROPERTY(SaveGame)
    TMap<FName, bool> UnlockedDoors;

    UPROPERTY(SaveGame)
    FName CurrentLevelName;

    // Meta
    UPROPERTY(SaveGame)
    FDateTime SaveTimestamp;
};

// Async save pattern
void UJuniperSaveSubsystem::SaveGame(const FString& SlotName)
{
    UJuniperSaveGame* SaveData = Cast<UJuniperSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UJuniperSaveGame::StaticClass()));

    PopulateSaveData(SaveData);

    UGameplayStatics::AsyncSaveGameToSlot(SaveData, SlotName, 0,
        FAsyncSaveGameToSlotDelegate::CreateUObject(this, &UJuniperSaveSubsystem::OnSaveComplete));
}
```

---

### Hiding Mechanics

| Technology | Version | Purpose | Confidence | Why |
|------------|---------|---------|------------|-----|
| **Custom UActorComponent** | C++ Native | Hiding spot detection | HIGH | `UHidingSpotComponent` added to furniture actors. Triggers available for enter/exit. |
| **AI Perception Touch Sense** | UE5.7 Native | Monster finding hidden player | HIGH | Touch sense detects physical contact when monster inspects hiding spots. |
| **Collision Channels** | UE5.7 Native | Visibility toggling | HIGH | Custom collision channel for "hidden" state. Monster sight traces ignore hidden collision. |

**Hiding Implementation Pattern:**
```cpp
UCLASS()
class UHidingSpotComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    FTransform HiddenPlayerTransform;  // Where player goes when hidden

    UPROPERTY(EditAnywhere)
    bool bCanMonsterSearch = true;  // Can monster check this spot?

    void EnterHiding(AJuniperCharacter* Player);
    void ExitHiding(AJuniperCharacter* Player);

private:
    UPROPERTY()
    TWeakObjectPtr<AJuniperCharacter> HiddenPlayer;
};

// When player enters hiding:
// 1. Move player to HiddenPlayerTransform
// 2. Set player collision to "Hidden" channel (invisible to AI sight)
// 3. Disable player movement input
// 4. Enable "peek" camera mode
// 5. Register hiding spot with monster's search list
```

---

### Audio

| Technology | Version | Purpose | Confidence | Why |
|------------|---------|---------|------------|-----|
| **MetaSounds** | UE5.7 Native | Procedural audio | HIGH | Modern replacement for Sound Cues. Procedural audio for dynamic horror soundscapes. |
| **Audio Volumes** | UE5.7 Native | Spatial audio zones | HIGH | Reverb, attenuation per room. Critical for horror atmosphere. |
| **ReportNoiseEvent** | UE5.7 Native | AI sound awareness | HIGH | Player footsteps, door opening, item pickup all generate noise for AI perception. |

---

## Alternatives Considered

| Category | Recommended | Alternative | Why Not Recommended |
|----------|-------------|-------------|---------------------|
| AI Logic | State Tree | Behavior Trees | BT is older approach, ticks every frame. State Tree is Epic's modern direction with on-demand evaluation. |
| AI Logic | State Tree | Custom FSM | Reinventing tested systems. No visual debugging, harder iteration. |
| Inventory | Component | GAS Integration | GAS AttributeSet limitation (one per class) makes multi-slot inventory awkward. Overkill for 8-slot system. |
| Inventory | Component | Blueprint-only | Performance overhead, harder to unit test, compile times don't benefit C++ architecture goal. |
| Shadows | VSM | Cascaded Shadow Maps | CSM is legacy. VSM provides higher resolution, better Nanite integration, required for first-person self-shadow. |
| Input | Enhanced Input | Legacy Input | Legacy input deprecated. Enhanced Input provides runtime remapping, better architecture. |
| First Person | Native FP Rendering | Separate arms mesh | UE5.7 native system eliminates dual-skeleton complexity, provides proper self-shadowing. |

---

## Project Configuration

### Build.cs Dependencies
```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "Core",
    "CoreUObject",
    "Engine",
    "InputCore",
    "EnhancedInput",      // Required for input system
    "AIModule",           // AI perception, behavior
    "GameplayTasks",      // State Tree tasks
    "NavigationSystem"    // AI navigation
});

PrivateDependencyModuleNames.AddRange(new string[]
{
    "Slate",
    "SlateCore",
    "UMG"                 // UI for inventory
});
```

### Project Settings
```
[/Script/Engine.RendererSettings]
r.DynamicGlobalIlluminationMethod=1        ; Lumen
r.ReflectionMethod=1                        ; Lumen
r.Shadow.Virtual.Enable=True                ; VSM
r.AllowStaticLighting=False                 ; Disable for first-person features

[/Script/Engine.Engine]
bUseFixedFrameRate=False                    ; Allow variable framerate
FixedFrameRate=60.0                         ; Target (if fixed)

[/Script/Engine.InputSettings]
DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput
DefaultInputComponentClass=/Script/EnhancedInput.EnhancedInputComponent
```

### Plugins to Enable
```
- EnhancedInput (default in 5.7)
- GameplayStateTreeModule
- SmartObjectsModule (optional, for AI interaction points)
```

---

## Performance Targets

| Metric | Target | Notes |
|--------|--------|-------|
| Frame Rate | 60 FPS (target), 30 FPS (minimum) | Horror games can tolerate 30 FPS, but 60 preferred |
| Draw Calls | < 3000 | Nanite helps, but monitor non-Nanite objects |
| GPU Memory | < 6 GB | Target mid-range GPUs (RTX 3060 / RX 6600) |
| VSM Pages | < 8000 | Monitor with `stat virtualshadowmaps` |
| AI Updates | < 0.5ms | State Tree is more efficient than BT |

---

## Sources

### Official Documentation (HIGH confidence)
- [First Person Rendering - UE5.7](https://dev.epicgames.com/documentation/en-us/unreal-engine/first-person-rendering)
- [State Tree in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/state-tree-in-unreal-engine)
- [Enhanced Input in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine)
- [AI Perception in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/ai-perception-in-unreal-engine)
- [Saving and Loading Your Game](https://dev.epicgames.com/documentation/en-us/unreal-engine/saving-and-loading-your-game-in-unreal-engine)
- [Lumen Global Illumination](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-global-illumination-and-reflections-in-unreal-engine)
- [Virtual Shadow Maps](https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-shadow-maps-in-unreal-engine)
- [Unreal Engine 5.7 Release Notes](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-5-7-release-notes)

### Community Resources (MEDIUM confidence)
- [GAS Documentation - tranek](https://github.com/tranek/GASDocumentation)
- [State Tree Tutorial - Your First 60 Minutes](https://forums.unrealengine.com/t/tutorial-your-first-60-minutes-with-statetree/1901811)
- [UE5 Horror Lighting - Devil of Plague](https://forums.unrealengine.com/t/lighting-shadows-in-horror-games-how-we-used-unreal-engine-5-to-build-devil-of-plague/2351092)
- [Modular Inventory System](https://github.com/MylesColeman/UE5-Modular-Inventory-System)
