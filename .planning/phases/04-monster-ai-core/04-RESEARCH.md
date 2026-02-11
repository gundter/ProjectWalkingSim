# Phase 4: Monster AI Core - Research

**Researched:** 2026-02-11
**Domain:** UE5.7 State Tree AI, AI Perception, NavMesh, Patrol Behaviors
**Confidence:** MEDIUM-HIGH

## Summary

Phase 4 introduces the Wendigo AI character using UE5.7's State Tree system (not Behavior Trees), AI Perception for sight and hearing, and NavMesh pathfinding for patrol behavior. The Wendigo is a tall (8-9ft) monster that patrols fixed routes, perceives the player through a gradual suspicion system influenced by the existing VisibilityScoreComponent, and transitions through three alert levels: Patrol, Suspicious, and Alert.

The standard approach for State Tree AI in UE5 uses `UStateTreeAIComponent` on an `AAIController` subclass with the `StateTreeAIComponentSchema`, which provides context access to both the controller and the controlled pawn. Custom C++ tasks inherit from `FStateTreeTaskCommonBase` and override `EnterState`, `Tick`, and `ExitState`. AI Perception uses `UAIPerceptionComponent` with `UAISenseConfig_Sight` and `UAISenseConfig_Hearing` configured in the controller constructor. The gradual suspicion system is a custom layer on top of perception -- the engine provides binary detection, and we build suspicion accumulation/decay ourselves.

**Primary recommendation:** Build a custom `AWendigoAIController` with `UStateTreeAIComponent` and `UAIPerceptionComponent`, create a `USuspicionComponent` on the Wendigo pawn for suspicion state management, and author 4-5 custom C++ State Tree tasks (patrol waypoint movement, idle/scan, investigate location, orient toward stimulus). Use the existing `UFootstepComponent::OnFootstep` delegate to feed sprint noise into `UAISense_Hearing::ReportNoiseEvent`.

## Standard Stack

The established libraries/tools for this domain:

### Core
| Library/Module | Purpose | Why Standard |
|----------------|---------|--------------|
| `StateTreeModule` | Core State Tree types (FStateTreeTaskCommonBase, FStateTreeExecutionContext) | Engine plugin for hierarchical state machine AI |
| `GameplayStateTreeModule` | UStateTreeAIComponent, StateTreeAIComponentSchema | Provides AI controller context integration for State Tree |
| `AIModule` | AAIController, UAIPerceptionComponent, UAISenseConfig_Sight/Hearing, pathfinding | Core engine AI framework |
| `NavigationSystem` | NavMesh generation, UNavigationSystemV1, pathfinding queries | Required for MoveToLocation navigation |
| `GameplayTags` | Tag-based state identification (already in project) | Standard for AI state communication |

### Supporting
| Library/Module | Purpose | When to Use |
|----------------|---------|-------------|
| `PhysicsCore` | Collision queries for line-of-sight checks | Already in project |
| `RenderCore` / `RHI` | Already in project for VisibilityScoreComponent | No additional need |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| State Tree | Behavior Trees | BTs are legacy; State Tree is Epic's modern replacement with on-demand evaluation. Decision is locked. |
| UAISense_Sight (built-in) | Custom AISense subclass | Custom sense gives full control over detection logic, but built-in sight is sufficient when combined with a custom suspicion layer |
| Spline-based patrol | Waypoint actor array | Splines are overkill for fixed indoor routes; simple waypoint array is more flexible and easier to author |

### Required Build.cs Additions

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    // Existing modules...
    "AIModule",
    "NavigationSystem",
    "StateTreeModule",
    "GameplayStateTreeModule"
});
```

### Required Plugin Enables

In `.uproject` or via Editor Settings > Plugins:
- **StateTree** plugin (should be enabled by default in 5.7)
- **GameplayStateTree** plugin (may need manual enable)

**Confidence: HIGH** - Module names verified via official UE5.7 API docs and Python API reference confirming `GameplayStateTreeModule` as the module for `StateTreeAIComponentSchema`.

## Architecture Patterns

### Recommended Project Structure
```
Source/ProjectWalkingSim/
  Public/
    AI/
      WendigoAIController.h          # AAIController + StateTreeAI + Perception
      WendigoCharacter.h             # ACharacter (the monster pawn)
      SuspicionComponent.h           # Suspicion accumulation/decay logic
      PatrolRouteActor.h             # Waypoint container placed in level
      Tasks/
        STT_PatrolMoveToWaypoint.h   # Custom State Tree task: move to next waypoint
        STT_PatrolIdle.h             # Custom State Tree task: pause, look around
        STT_InvestigateLocation.h    # Custom State Tree task: move to stimulus location
        STT_OrientToward.h           # Custom State Tree task: turn toward target
      Conditions/
        STC_SuspicionLevel.h         # Custom condition: check suspicion threshold
    Tags/
      SereneTags.h                   # Extended with AI tags
  Private/
    AI/
      [matching .cpp files]
```

### Pattern 1: StateTreeAIComponent on AIController

**What:** Place `UStateTreeAIComponent` on the AI Controller (not the pawn). Use `StateTreeAIComponentSchema` to get context access to both controller and pawn.

**When to use:** Always for AI-controlled characters using State Tree.

**Critical detail:** `StartLogic` must be called explicitly -- the `bStartLogicAutomatically` flag reportedly does not work. Call `StartLogic` only after both `BeginPlay` and `OnPossess` have completed.

```cpp
// WendigoAIController.h
#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "WendigoAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UStateTreeAIComponent;

UCLASS()
class PROJECTWALKINGSIM_API AWendigoAIController : public AAIController
{
    GENERATED_BODY()

public:
    AWendigoAIController();

protected:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    TObjectPtr<UStateTreeAIComponent> StateTreeAIComponent;

    UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
    TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

    UPROPERTY()
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UPROPERTY()
    TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

private:
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    bool bBeginPlayCalled = false;
    bool bPossessCalled = false;
    void TryStartStateTree();
};
```

**Confidence: HIGH** - Pattern confirmed across multiple sources including Epic documentation and community guides.

### Pattern 2: Custom C++ State Tree Task

**What:** State Tree tasks are `USTRUCT` types inheriting from `FStateTreeTaskCommonBase`. They define an `FInstanceDataType` for per-instance runtime state, and override `EnterState`, `Tick`, `ExitState`.

**When to use:** For all custom AI behaviors (patrol, investigate, idle).

```cpp
// STT_PatrolMoveToWaypoint.h
#pragma once
#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "STT_PatrolMoveToWaypoint.generated.h"

USTRUCT()
struct PROJECTWALKINGSIM_API FSTT_PatrolMoveToWaypointInstanceData
{
    GENERATED_BODY()

    /** Index of the current target waypoint. */
    UPROPERTY(EditAnywhere, Category = "Patrol")
    int32 CurrentWaypointIndex = 0;

    /** True when MoveToLocation request is active. */
    bool bMoveRequestActive = false;
};

USTRUCT(DisplayName = "Patrol: Move To Waypoint")
struct PROJECTWALKINGSIM_API FSTT_PatrolMoveToWaypoint : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FSTT_PatrolMoveToWaypointInstanceData;

    // Required: return the instance data struct type
    virtual const UStruct* GetInstanceDataType() const override
    {
        return FInstanceDataType::StaticStruct();
    }

    // Called when this task's state becomes active
    virtual EStateTreeRunStatus EnterState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override;

    // Called every tick while state is active (return Running to keep ticking)
    virtual EStateTreeRunStatus Tick(
        FStateTreeExecutionContext& Context,
        const float DeltaTime) const override;

    // Called when leaving this state
    virtual void ExitState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override;

    // Link external data handles
    virtual bool Link(FStateTreeLinker& Linker) override;

    // External data: access to AI Controller and Pawn
    TStateTreeExternalDataHandle<AAIController> ControllerHandle;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    float AcceptanceRadius = 50.0f;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    float WalkSpeed = 150.0f;
};
```

**Key lifecycle:**
- `EnterState` returns `EStateTreeRunStatus::Running` to keep ticking, `Succeeded`/`Failed` to end immediately
- `Tick` returns `Running` to continue, `Succeeded` when goal reached
- `ExitState` returns void -- use for cleanup
- **WARNING:** A global task that returns `Succeeded` immediately will stop the entire State Tree silently

**Confidence: HIGH** - Struct pattern verified across official docs, community guides, and engine API reference.

### Pattern 3: External Data Access via TStateTreeExternalDataHandle

**What:** Tasks access the AIController, Pawn, and other external objects through typed handles linked during initialization.

```cpp
// In task header:
TStateTreeExternalDataHandle<AAIController> ControllerHandle;

// In Link():
virtual bool Link(FStateTreeLinker& Linker) override
{
    Linker.LinkExternalData(ControllerHandle);
    return true;
}

// In EnterState/Tick:
AAIController& Controller = Context.GetExternalData(ControllerHandle);
APawn* Pawn = Controller.GetPawn();
```

**Confidence: HIGH** - Verified in official API docs and multiple community examples.

### Pattern 4: Suspicion Component (Custom)

**What:** A component on the Wendigo pawn that accumulates suspicion from perception events. The engine's AI Perception gives binary "seen/not seen" with stimulus strength. We layer gradual suspicion on top.

**When to use:** For the gradual detection system described in CONTEXT.md.

```cpp
// SuspicionComponent.h - Conceptual structure
UENUM(BlueprintType)
enum class EAlertLevel : uint8
{
    Patrol     UMETA(DisplayName = "Patrol"),
    Suspicious UMETA(DisplayName = "Suspicious"),
    Alert      UMETA(DisplayName = "Alert")
};

UCLASS()
class USuspicionComponent : public UActorComponent
{
    // Current suspicion value (0.0 to 1.0)
    // SuspicionRate scales with player's VisibilityScore
    // DecayRate: 10-20 seconds from full to zero
    // Thresholds: Suspicious at ~0.4, Alert at ~0.8
    // Stores last known stimulus location for investigation
};
```

**Confidence: MEDIUM** - This is a custom design pattern. No engine-provided suspicion system exists. This is the standard approach used in horror/stealth games.

### Pattern 5: Patrol Route via Waypoint Actor

**What:** A simple actor placed in the level containing an array of waypoint locations. The Wendigo pawn holds a reference to its patrol route. The State Tree task reads waypoints sequentially.

```cpp
// PatrolRouteActor.h
UCLASS()
class APatrolRouteActor : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol")
    TArray<FVector> Waypoints;

    // Editor: visualize waypoints with debug spheres and connecting lines
    // Optional: per-waypoint pause duration and detour points
};
```

**Why not splines:** Fixed indoor routes with discrete stop points are better served by waypoint arrays. Splines are better for smooth outdoor paths. Waypoints also make it trivial to add per-point pause durations and detour locations.

**Confidence: MEDIUM** - Standard pattern, but specific implementation is project-specific. Epic's Parrot sample uses splines with a `UParrotEnemyPatrolRigComponent`, but that is for platformer movement, not horror patrol.

### Anti-Patterns to Avoid

- **Global tasks that return Succeeded/Failed immediately:** This silently stops the entire State Tree with no warning. Global tasks should return `Running` and tick forever, or be used only for data-providing evaluators.
- **Binding to Blueprint struct properties:** Structs are passed by value in State Tree bindings, not by reference. Updates to the original struct will not reflect in the tree. Use direct property access or external data handles instead.
- **Calling StartLogic in constructor:** Must wait for both BeginPlay AND OnPossess. Use a two-flag guard pattern.
- **Using StateTreeAIComponent outside an AIController:** Will fail silently. Use `UStateTreeComponent` for non-controller actors.
- **Lock AI Logic on Move To tasks:** The "Lock AI Logic" option on built-in movement tasks prevents all other AI logic including transitions from executing. Leave it disabled.
- **Using bStartLogicAutomatically:** Reportedly does not work. Always call `StartLogic()` explicitly.

## Don't Hand-Roll

Problems that look simple but have existing solutions:

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| AI Pathfinding | Custom A* or movement | `AAIController::MoveToLocation()` + NavMesh | Handles path following, obstacle avoidance, partial paths |
| Sight detection | Custom raycasts from AI | `UAISenseConfig_Sight` + `UAIPerceptionComponent` | Handles FOV cone, range, LOS checks, affiliation filtering |
| Sound detection | Custom distance checks | `UAISense_Hearing` + `ReportNoiseEvent` | Engine-integrated, configurable range, tag filtering |
| State machine | Custom FSM classes | State Tree asset + `UStateTreeAIComponent` | Visual editor, debugger, hierarchical states, built-in transitions |
| Path following | Custom tick-based movement | `UPathFollowingComponent` (auto-created by AIController) | Handles acceleration, path smoothing, goal reached detection |
| AI focus/rotation | Manual rotation toward targets | `AAIController::SetFocus()` / `SetFocalPoint()` | Smooth rotation via `UpdateControlRotation()`, integrates with perception |

**Key insight:** UE5's AI framework provides robust foundational systems. The custom work is in the *decisions layer* (suspicion accumulation, patrol route authoring, visibility score integration) -- not in the movement, pathfinding, or basic detection layers.

## Common Pitfalls

### Pitfall 1: StartLogic Timing
**What goes wrong:** State Tree starts but does nothing -- no tasks execute, no states activate.
**Why it happens:** `UStateTreeAIComponent::StartLogic()` called before `OnPossess` completes, so the component has no pawn context.
**How to avoid:** Use a two-flag guard: set `bBeginPlayCalled` in BeginPlay, `bPossessCalled` in OnPossess. Call `StartLogic()` only when both are true.
**Warning signs:** Visual Logger shows "State Tree Started" with no subsequent state activations.

### Pitfall 2: Context Actor Class Mismatch
**What goes wrong:** State Tree freezes or crashes when accessing external data.
**Why it happens:** The `StateTreeAIComponentSchema` has `AIController Class` and `Context Actor Class` fields. If these don't match the actual runtime classes, external data access fails.
**How to avoid:** Set `AIController Class` to `AWendigoAIController` and `Context Actor Class` to `AWendigoCharacter` in the State Tree asset's schema settings.
**Warning signs:** Crash in `GetExternalData` or tree starts but tasks never enter.

### Pitfall 3: NavMesh Not Generated for Tall Character
**What goes wrong:** Wendigo cannot navigate -- MoveToLocation returns failure.
**Why it happens:** Default NavMesh agent is configured for standard-height characters (~180cm). An 8-9ft (244-274cm) character needs a custom NavMesh agent profile with larger Agent Height.
**How to avoid:** In Project Settings > Navigation Mesh > Agents, add a second agent profile with appropriate height (~275cm) and radius (~60cm). Place a `NavMeshBoundsVolume` in the level. Ensure `ARecastNavMesh` generates for the custom agent.
**Warning signs:** Green NavMesh visualization absent in areas with low ceilings; MoveToLocation returns `Failed`.

### Pitfall 4: AI Perception Delegates Not Firing
**What goes wrong:** `OnTargetPerceptionUpdated` never called despite AI and player in proximity.
**Why it happens:** Multiple possible causes: (1) Delegate bound in constructor instead of BeginPlay, (2) Player pawn not registered as stimulus source, (3) Detection by Affiliation flags not set correctly, (4) `SetDominantSense()` not called.
**How to avoid:** Bind delegates in BeginPlay. Ensure the player has `UAIPerceptionStimuliSourceComponent` or use `bAutoRegisterAsSource = true` in sight config. Set all affiliation flags to true for demo. Set dominant sense to sight.
**Warning signs:** Debug visualization shows sight cone but no detection events in log.

### Pitfall 5: ReportNoiseEvent Not Reaching AI
**What goes wrong:** Sprint footsteps don't trigger hearing detection.
**Why it happens:** `UAISense_Hearing::ReportNoiseEvent` requires: (1) correct World context, (2) Loudness > 0, (3) MaxRange > 0, (4) the AI's `UAISenseConfig_Hearing` hearing range must be >= the noise range, (5) Tag matching if tags are configured.
**How to avoid:** Call `ReportNoiseEvent` with generous loudness (1.0) and range (2000.0 for sprint). Verify HearingConfig range is at least as large. Use `FName()` for no tag filtering.
**Warning signs:** AI perception debug view shows hearing sense active but no stimuli registered.

### Pitfall 6: State Tree Tasks Completing Parent States
**What goes wrong:** A child state transition causes the parent state's tasks to receive unexpected `EnterState`/`ExitState` calls.
**Why it happens:** All tasks in active states receive notifications from all active tasks at or below their level. Parent state tasks get re-entered when child states transition.
**How to avoid:** Set `bShouldStateChangeOnReselect = false` in task class defaults. For C++ tasks, check `Transition.ChangeType` to filter out sustained transitions.
**Warning signs:** Parent state task runs its initialization logic repeatedly.

## Code Examples

### AI Controller Constructor with Perception Setup

```cpp
// Source: UE5.7 API docs + community verified patterns
AWendigoAIController::AWendigoAIController()
{
    // State Tree AI Component
    StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>(
        TEXT("StateTreeAIComponent"));

    // Perception Component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(
        TEXT("AIPerceptionComponent"));
    SetPerceptionComponent(*AIPerceptionComponent);

    // Sight Configuration
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(
        TEXT("SightConfig"));
    SightConfig->SightRadius = 2500.0f;          // ~25m detection range
    SightConfig->LoseSightRadius = 3000.0f;       // Slightly larger to avoid flicker
    SightConfig->PeripheralVisionAngleDegrees = 45.0f; // 90 degree total FOV
    SightConfig->SetMaxAge(5.0f);                 // Forget after 5s without seeing
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    AIPerceptionComponent->ConfigureSense(*SightConfig);

    // Hearing Configuration
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(
        TEXT("HearingConfig"));
    HearingConfig->HearingRange = 2000.0f;        // ~20m hearing range
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    AIPerceptionComponent->ConfigureSense(*HearingConfig);

    // Set sight as dominant sense
    AIPerceptionComponent->SetDominantSense(
        UAISense_Sight::StaticClass());
}
```

**Confidence: HIGH** - Constructor pattern verified in official docs. Delegate binding must be in BeginPlay.

### Delegate Binding in BeginPlay (NOT Constructor)

```cpp
void AWendigoAIController::BeginPlay()
{
    Super::BeginPlay();

    // IMPORTANT: Bind perception delegates in BeginPlay, never constructor
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
            this, &AWendigoAIController::OnTargetPerceptionUpdated);
    }

    bBeginPlayCalled = true;
    TryStartStateTree();
}

void AWendigoAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    bPossessCalled = true;
    TryStartStateTree();
}

void AWendigoAIController::TryStartStateTree()
{
    if (bBeginPlayCalled && bPossessCalled && StateTreeAIComponent)
    {
        StateTreeAIComponent->StartLogic();
    }
}
```

**Confidence: HIGH** - Two-flag guard pattern recommended by multiple sources to avoid timing issues.

### Reporting Sprint Noise from FootstepComponent

```cpp
// In WendigoCharacter or a bridge component listening to player footsteps
// The player's FootstepComponent broadcasts OnFootstep with Volume.
// Sprint Volume is 1.5 (SprintVolumeMultiplier). Walk is 1.0. Crouch is 0.3.
// Per CONTEXT.md: only sprinting generates audible noise.

// On the player character or a listener:
void HandleFootstep(EPhysicalSurface SurfaceType, float Volume)
{
    // Only report noise for sprinting (Volume > 1.0 means sprint)
    if (Volume > 1.0f)
    {
        UAISense_Hearing::ReportNoiseEvent(
            GetWorld(),
            GetActorLocation(),       // Noise location
            Volume,                   // Loudness (1.5 for sprint)
            this,                     // Instigator (player)
            2000.0f,                  // MaxRange in cm (~20m)
            FName()                   // No tag filter
        );
    }
}
```

**Confidence: HIGH** - `ReportNoiseEvent` signature verified in UE5.7 API docs. Integration with existing FootstepComponent is straightforward via its `OnFootstep` delegate.

### Wendigo Character Setup (Tall Pawn)

```cpp
// WendigoCharacter.h
UCLASS()
class AWendigoCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    AWendigoCharacter();

    UPROPERTY(VisibleAnywhere, Category = "AI")
    TObjectPtr<USuspicionComponent> SuspicionComponent;

    /** Reference to the patrol route this Wendigo follows. Set in level. */
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI|Patrol")
    TObjectPtr<APatrolRouteActor> PatrolRoute;

    /** Wendigo walk speed -- slower than player walk (250). */
    static constexpr float WendigoWalkSpeed = 150.0f;
};

// WendigoCharacter.cpp
AWendigoCharacter::AWendigoCharacter()
{
    // Tall capsule for 8-9ft character (~260cm = 130 half-height)
    GetCapsuleComponent()->InitCapsuleSize(45.0f, 130.0f);

    // Movement defaults
    GetCharacterMovement()->MaxWalkSpeed = WendigoWalkSpeed;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 120.0f, 0.0f);

    // AI controller class assignment
    AIControllerClass = AWendigoAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // Suspicion component
    SuspicionComponent = CreateDefaultSubobject<USuspicionComponent>(
        TEXT("SuspicionComponent"));
}
```

**Confidence: MEDIUM** - Capsule sizing is approximate (8.5ft = ~259cm, half-height ~130cm). NavMesh agent must be configured to match.

### Suspicion Accumulation Logic

```cpp
// Conceptual: called each tick or on perception update
void USuspicionComponent::ProcessSightStimulus(float PlayerVisibilityScore, float DeltaTime)
{
    // Below threshold: cannot see player at all
    if (PlayerVisibilityScore < VisibilityThreshold) // ~0.3
    {
        // No accumulation from sight
        return;
    }

    // Above threshold: accumulate suspicion scaled by visibility
    // Higher visibility = faster detection
    float EffectiveVisibility = (PlayerVisibilityScore - VisibilityThreshold)
        / (1.0f - VisibilityThreshold); // Normalize to 0-1 above threshold

    float SuspicionGain = BaseSuspicionRate * EffectiveVisibility * DeltaTime;
    CurrentSuspicion = FMath::Clamp(CurrentSuspicion + SuspicionGain, 0.0f, 1.0f);

    UpdateAlertLevel();
}

void USuspicionComponent::DecaySuspicion(float DeltaTime)
{
    // Slow decay: 10-20 seconds from full
    // At DecayRate of 0.05-0.1 per second
    CurrentSuspicion = FMath::Max(0.0f,
        CurrentSuspicion - SuspicionDecayRate * DeltaTime);

    UpdateAlertLevel();
}

void USuspicionComponent::UpdateAlertLevel()
{
    EAlertLevel NewLevel;
    if (CurrentSuspicion >= AlertThreshold)       // ~0.8
        NewLevel = EAlertLevel::Alert;
    else if (CurrentSuspicion >= SuspiciousThreshold) // ~0.4
        NewLevel = EAlertLevel::Suspicious;
    else
        NewLevel = EAlertLevel::Patrol;

    if (NewLevel != CurrentAlertLevel)
    {
        CurrentAlertLevel = NewLevel;
        OnAlertLevelChanged.Broadcast(NewLevel);
    }
}
```

**Confidence: MEDIUM** - Custom design. Threshold values (0.3 visibility, 0.4/0.8 suspicion) are starting points for tuning.

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Behavior Trees (UBehaviorTree) | State Tree (UStateTree) | UE 5.0+ | State Tree is Epic's recommended modern AI. Hierarchical states + selectors. Better performance, cleaner architecture. |
| Blackboard for AI data | State Tree Instance Data + External Data Handles | UE 5.0+ | No separate blackboard asset needed; data flows through typed handles |
| Blueprint tasks for State Tree | C++ FStateTreeTaskCommonBase | Always | C++ tasks perform significantly better and offer finer control (disable ticking, etc.) |
| Manual FSM code | State Tree visual editor | UE 5.0+ | Visual authoring with built-in debugger/trace |
| PawnSensingComponent | UAIPerceptionComponent | UE 4.x+ | Perception is the modern replacement; PawnSensing is deprecated |

**Deprecated/outdated:**
- `UPawnSensingComponent`: Legacy sensing, replaced by `UAIPerceptionComponent`
- Behavior Trees: Still functional but State Tree is the modern path
- `Blackboard` assets: Not needed with State Tree (data lives in instance data and external handles)

## Open Questions

Things that couldn't be fully resolved:

1. **Built-in State Tree Movement Task**
   - What we know: There appears to be a built-in "Move To" task in the GameplayStateTree plugin, but its exact class name and parameters could not be confirmed from documentation.
   - What's unclear: Whether `FStateTreeMoveTo` exists as a ready-made task, or if movement must be custom-built.
   - Recommendation: Check engine source at `Plugins/Runtime/GameplayStateTree/Source/` for built-in tasks. If a MoveTo task exists, use it instead of writing a custom one. If not, the custom `FSTT_PatrolMoveToWaypoint` pattern above is the fallback.

2. **State Tree Asset Creation via C++/Python**
   - What we know: State Tree assets (`.uasset`) are typically created in the Editor's State Tree editor UI.
   - What's unclear: Whether State Tree assets can be created programmatically or must be authored manually in-editor.
   - Recommendation: Create the State Tree asset manually in the Editor. C++ defines the tasks/conditions; the asset wires them together visually. This follows the project's "C++ for logic, Blueprint/assets for data" pattern.

3. **Wendigo Skeletal Mesh and Animations**
   - What we know: No Wendigo-specific assets exist in the project yet. Only Mannequin assets are present in `Content/Characters/Mannequins/`. The CONTEXT.md mentions "existing Wendigo asset has preloaded animations" but this was not found.
   - What's unclear: Whether a Wendigo asset needs to be imported from an external source (Marketplace, custom) or if the Mannequin will be used as placeholder.
   - Recommendation: Use the Mannequin (SK_Mannequin) as placeholder, scaled up to 8-9ft. The Mannequin has walk, idle, jog, and attack animations that can serve as temporary patrol/idle behavior. Import actual Wendigo mesh later.

4. **NavMesh Multi-Agent Support in UE5.7**
   - What we know: Multiple NavMesh agents with different sizes require separate `ARecastNavMesh` instances per agent profile, configured in Project Settings > Navigation Mesh > Agents.
   - What's unclear: Exact UE5.7 UI path and whether NavMesh generates correctly for very tall agents in indoor environments with low ceilings.
   - Recommendation: Configure a second NavMesh agent. Test early -- if the Wendigo's height causes NavMesh gaps in doorways, reduce agent height to just clear standard doorframes (~240cm) while keeping the visual mesh at full height.

5. **Exact Includes for State Tree C++ Tasks**
   - What we know: Tasks include from `StateTreeTaskBase.h` and use types from `StateTreeExecutionContext.h`, `StateTreeLinker.h`.
   - What's unclear: The exact include paths may vary (e.g., `#include "StateTreeTaskBase.h"` vs `#include "Blueprint/StateTreeTaskBlueprintBase.h"`).
   - Recommendation: When implementing the first task, check the engine's `Plugins/Runtime/StateTree/Source/StateTreeModule/Public/` directory for exact header names. Key headers likely include: `StateTreeTaskBase.h`, `StateTreeExecutionContext.h`, `StateTreeLinker.h`, `StateTreeTypes.h`.

## Recommended Discretion Values

Per CONTEXT.md, these are Claude's discretion. Recommended starting values:

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Wendigo walk speed | 150 cm/s | Player walks at 250; ~60% of player speed feels deliberate |
| Sight range | 2500 cm (25m) | Indoor horror: should detect across 1-2 rooms |
| Lose sight range | 3000 cm (30m) | 20% hysteresis to prevent detection flicker |
| Sight FOV | 90 degrees total (45 half-angle) | Per CONTEXT.md decision |
| Hearing range | 2000 cm (20m) | Sprint noise carries ~2 rooms |
| Visibility threshold | 0.3 | Below this, player is invisible to AI |
| Suspicion rate (base) | 0.15/sec at full visibility | ~7 seconds of sustained exposure to reach Alert |
| Suspicion decay rate | 0.065/sec | ~15 seconds from full to zero (within 10-20s spec) |
| Suspicious threshold | 0.4 suspicion | ~2.5s sustained view triggers Suspicious |
| Alert threshold | 0.8 suspicion | ~5.5s sustained view triggers Alert |
| Patrol pause frequency | Every 2-3 waypoints | Pause for 3-6 seconds to look around |
| Patrol detour chance | 30% at each pause | Minor detour to nearby point of interest |
| Capsule half-height | 130 cm | ~8.5ft total (260cm) |
| Capsule radius | 45 cm | Proportional for a large humanoid |

## Integration Points with Existing Systems

### VisibilityScoreComponent (Phase 3 -> Phase 4)
- `UVisibilityScoreComponent::GetVisibilityScore()` returns 0.0-1.0
- AI controller reads this from the player pawn during perception updates
- Score feeds into `USuspicionComponent::ProcessSightStimulus()`
- Player hiding (score near 0) makes them effectively invisible

### FootstepComponent (Phase 1 -> Phase 4)
- `UFootstepComponent::OnFootstep` broadcasts with `(EPhysicalSurface, float Volume)`
- Sprint volume is 1.5 (SprintVolumeMultiplier), walk is 1.0, crouch is 0.3
- Phase 4 adds a listener that calls `UAISense_Hearing::ReportNoiseEvent` for sprint-level volumes only
- This could be a component on the player or a global listener

### HidingComponent (Phase 3 -> Phase 4)
- `UHidingComponent::IsHiding()` returns true when player is fully hidden
- `UHidingComponent::GetHidingState()` returns the 4-state enum
- During hiding, VisibilityScore is reduced by the hiding spot's reduction value
- AI should not detect a hidden player (visibility score will be well below 0.3 threshold)
- The Player.Hiding gameplay tag is set when hiding -- can be checked as additional insurance

### Gameplay Tags to Add
```cpp
// In SereneTags.h/.cpp
namespace SereneTags
{
    // AI alert levels
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Alert_Patrol);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Alert_Suspicious);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Alert_Alert);

    // AI perception
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Stimulus_Sight);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Stimulus_Hearing);
}
```

## Sources

### Primary (HIGH confidence)
- [UE5.7 StateTree Overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/overview-of-state-tree-in-unreal-engine) - Architecture, tasks, evaluators, conditions
- [UE5.7 StateTree Quick Start](https://dev.epicgames.com/documentation/en-us/unreal-engine/statetree-quick-start-guide) - Setup workflow, component schema
- [UE5.7 AI Perception](https://dev.epicgames.com/documentation/en-us/unreal-engine/ai-perception-in-unreal-engine) - Sight/Hearing sense configs, delegates
- [UE5.7 AAIController API](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/AIModule/AAIController) - MoveToLocation, perception methods, focus system
- [UE5.7 StateTreeModule API](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/StateTreeModule) - Core types, FStateTreeTaskBase, UStateTree
- [UE5.5 StateTreeAIComponentSchema Python API](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/StateTreeAIComponentSchema?application_version=5.5) - Confirmed GameplayStateTreeModule, schema properties
- [UE5.7 StateTree Debugger Guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/statetree-debugger-quick-start-guide) - Debug visualization, trace recording

### Secondary (MEDIUM confidence)
- [Custom StateTree Tasks (zomgmoz)](https://zomgmoz.tv/unreal/State-Tree/Custom-StateTree-tasks) - FStateTreeTaskCommonBase pattern, instance data, run status
- [StateTreeAIComponent (zomgmoz)](https://zomgmoz.tv/unreal/State-Tree/StateTreeAIComponent) - StartLogic issue, schema configuration
- [State Tree Context (zomgmoz)](https://zomgmoz.tv/unreal/State-Tree/State-Tree-Context) - Context actor class, external data access
- [Troubleshooting State Trees (zomgmoz)](https://zomgmoz.tv/unreal/State-Tree/Troubleshooting-State-Trees) - Timing issues, common failures
- [AI Perception C++ (Lost Ferry)](https://lostferry.com/2-50-using-ai-perception-component-unreal-c/) - OnTargetPerceptionUpdated callback pattern
- [Configuring AI Perception in C++ (zomgmoz)](https://zomgmoz.tv/unreal/AI-Perception/Configuring-AIPerception-Senses-in-C++) - Constructor vs BeginPlay binding

### Tertiary (LOW confidence)
- [State Tree Pitfalls (Jean-Paul Software)](https://jeanpaulsoftware.com/2024/08/13/state-tree-hell/) - Gotchas with concurrent tasks, subtrees, global tasks. Written for UE 5.4; some issues may be resolved in 5.7.
- [Parrot Enemy AI (Epic)](https://dev.epicgames.com/documentation/en-us/unreal-engine/enemy-characters-in-parrot-for-unreal-engine) - Patrol rig pattern using splines (uses BT not State Tree; concept transferable)

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - Module names and dependencies verified via official API docs
- Architecture: HIGH - State Tree task pattern well-documented across multiple authoritative sources
- AI Perception: HIGH - Well-established engine system, API stable across UE5 versions
- Suspicion system: MEDIUM - Custom design; no engine equivalent; based on standard stealth game patterns
- NavMesh for tall characters: MEDIUM - Standard system but large agent configuration less documented
- Pitfalls: HIGH - Multiple sources corroborate the same issues (StartLogic timing, global task completion, delegate binding location)
- Exact include paths: LOW - Could not verify exact header file paths for State Tree C++ types

**Research date:** 2026-02-11
**Valid until:** 2026-03-11 (30 days; State Tree API is stable in UE5.7)
