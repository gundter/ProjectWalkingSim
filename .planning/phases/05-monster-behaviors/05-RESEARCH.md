# Phase 5: Monster Behaviors - Research

**Researched:** 2026-02-11
**Domain:** Unreal Engine 5.7 State Tree AI behaviors (chase, search, investigate, spawn)
**Confidence:** HIGH (codebase-grounded, patterns verified against existing architecture)

## Summary

Phase 5 extends the existing Wendigo AI from Phase 4's patrol-and-investigate foundation into a complete predator behavior loop: chase on sight, search when player escapes, investigate stimuli with differentiated reactions, and spawn at designated locations with zone-based patrol selection. The existing State Tree architecture (hierarchical containers with On Tick transitions) provides the scaffolding; Phase 5 adds new states and tasks within this structure.

The primary technical challenges are: (1) a chase task that continuously tracks a moving player using `AAIController::MoveToActor`, (2) a search behavior that generates random navmesh points near the last-known position using `UNavigationSystemV1::GetRandomReachablePointInRadius`, (3) tracking whether the Wendigo witnessed the player entering a hiding spot (perception event + hiding state timing), and (4) a spawn point system with zone-to-patrol-route mapping.

**Primary recommendation:** Build chase, search, and enhanced investigation as new State Tree tasks following the established FSTT_ pattern. Extend AWendigoCharacter and SuspicionComponent with chase/search state tracking. Use a lightweight WendigoSpawnPoint actor with zone-based patrol route arrays. Do NOT use EQS for search point generation -- the simpler GetRandomReachablePointInRadius is sufficient for 2-3 points.

## Standard Stack

### Core (Already in Project)
| Library/Module | Version | Purpose | Status |
|---------|---------|---------|--------|
| StateTreeModule | UE 5.7.2 | State Tree task/condition C++ base classes | Already in Build.cs |
| GameplayStateTreeModule | UE 5.7.2 | StateTreeAIComponent schema for AI controllers | Already in Build.cs |
| AIModule | UE 5.7.2 | AAIController, AI Perception, path following | Already in Build.cs |
| NavigationSystem | UE 5.7.2 | UNavigationSystemV1, NavMesh queries | Already in Build.cs |
| GameplayTags | UE 5.7.2 | Semantic state identification | Already in Build.cs |

### Supporting (No New Dependencies)
| Feature | UE API | Purpose | Notes |
|---------|--------|---------|-------|
| Chase movement | AAIController::MoveToActor | Continuously track moving player | Destination auto-updates |
| Search points | UNavigationSystemV1::GetRandomReachablePointInRadius | Generate reachable random points near last-known location | Use radius 500-800cm |
| LOS check | AAIController::LineOfSightTo | Verify player visibility during chase | Already available via AIController base |
| Head tracking | AAIController::SetFocus / SetFocalPoint | Visual tells for behavior states | Already used in STT_PatrolIdle, STT_OrientToward |
| Actor spawning | UWorld::SpawnActor | Runtime Wendigo spawning at SpawnPoints | Standard UE pattern |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| GetRandomReachablePointInRadius | EQS (Environment Query System) | EQS is more powerful but massive overkill for 2-3 random points. Only justified if search needs scoring/weighting. Keep EQS as future upgrade path. |
| NavLinkProxy for doors | Direct door interaction in chase task | SmartLinks have known reliability issues (events not firing). Simpler to have chase task detect doors in path and call OnInteract directly. |
| Separate chase AI controller | Single controller with state-dependent speed | Keep single controller; speed changes happen in task Enter/Exit (pattern already established in STT_InvestigateLocation). |

**Installation:** No new modules needed. All dependencies already in Build.cs.

## Architecture Patterns

### Recommended State Tree Hierarchy

```
Root
├── Alert (container) ── Enter Condition: AlertLevel >= Alert
│   ├── Chase (leaf) ── STT_ChasePlayer task
│   │   └── On Tick transition: AlertLevel < Alert → Search
│   │   └── On Tick transition: Player caught (within grab range) → GrabAttack
│   ├── GrabAttack (leaf) ── STT_GrabAttack task (cinematic kill)
│   └── Search (leaf) ── STT_SearchArea task
│       └── On Completed: Succeeded → transitions up, falls to Suspicious or Patrol
│
├── Suspicious (container) ── Enter Condition: AlertLevel >= Suspicious
│   ├── InvestigateSight (leaf) ── STT_InvestigateLocation (faster speed, existing task enhanced)
│   │   └── Enter Condition: LastStimulusType == Sight
│   ├── InvestigateSound (leaf) ── STT_InvestigateLocation (slower speed, existing task)
│   │   └── Enter Condition: LastStimulusType == Sound
│   └── OrientToward (leaf) ── STT_OrientToward (existing task, fallback)
│
└── Patrol (container) ── default / Enter Condition: AlertLevel < Suspicious
    ├── MoveToWaypoint (leaf) ── STT_PatrolMoveToWaypoint (existing)
    │   └── On Tick transition: AlertLevel >= Suspicious → parent exits
    └── Idle (leaf) ── STT_PatrolIdle (existing)
        └── On Tick transition: AlertLevel >= Suspicious → parent exits
```

**Key architectural decisions:**
- Alert container at the TOP of the hierarchy (evaluated first, highest priority)
- Chase lives inside Alert; Search is a sibling of Chase within Alert
- Chase-to-Search transition triggers when player breaks LOS for a configurable duration
- Search succeeds (returns to lower states) when timer expires
- Existing Patrol and Suspicious containers move down but keep their structure

### Pattern 1: Chase Task with LOS Timer

**What:** STT_ChasePlayer continuously moves toward the player using MoveToActor. A LOS timer tracks seconds since last sight contact. When the timer exceeds a threshold, the task signals transition to Search.

**When to use:** Active chase when AlertLevel >= Alert and player is in perception range.

**Example:**
```cpp
// In STT_ChasePlayer::Tick
// MoveToActor handles continuous pathfinding updates automatically.
// We only need to monitor LOS for the chase-to-search transition.

bool bCanSeePlayer = Controller.LineOfSightTo(PlayerActor);
if (bCanSeePlayer)
{
    InstanceData.LOSLostTimer = 0.0f;
    // Update last-known position for search fallback
    Wendigo->SetLastKnownPlayerLocation(PlayerActor->GetActorLocation());
}
else
{
    InstanceData.LOSLostTimer += DeltaTime;
    if (InstanceData.LOSLostTimer >= LOSLostTimeout)
    {
        // Player escaped -- transition to search
        return EStateTreeRunStatus::Failed; // Triggers On Failed transition to Search
    }
}

// Check grab range
float Distance = FVector::Dist(Controller.GetPawn()->GetActorLocation(),
                               PlayerActor->GetActorLocation());
if (Distance <= GrabRange && bCanSeePlayer)
{
    return EStateTreeRunStatus::Succeeded; // Triggers On Succeeded transition to GrabAttack
}

return EStateTreeRunStatus::Running;
```

### Pattern 2: Search with Random NavMesh Points

**What:** STT_SearchArea moves to last-known position, then checks 2-3 random nearby points, then succeeds (returning to patrol).

**When to use:** After chase ends (player broke LOS + timer expired).

**Example:**
```cpp
// In STT_SearchArea::EnterState
// Build search point list: last-known position + 2-3 random nearby points
InstanceData.SearchPoints.Empty();
InstanceData.SearchPoints.Add(Wendigo->GetLastKnownPlayerLocation());

UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Controller.GetWorld());
if (NavSys)
{
    FNavLocation RandomResult;
    for (int32 i = 0; i < NumRandomPoints; ++i)
    {
        if (NavSys->GetRandomReachablePointInRadius(
                InstanceData.SearchPoints[0], SearchRadius, RandomResult))
        {
            InstanceData.SearchPoints.Add(RandomResult.Location);
        }
    }
}
InstanceData.CurrentSearchIndex = 0;
InstanceData.SearchStartTime = 0.0f;
```

### Pattern 3: Witnessed Hiding Detection

**What:** Track whether the Wendigo had active sight perception of the player at the moment the player entered a hiding spot. If so, the Wendigo remembers the hiding spot and will check it during search.

**When to use:** Chase or Alert state when player enters hiding.

**Implementation approach:**
```cpp
// On AWendigoAIController -- listen for player hiding state changes
// Bind to HidingComponent::OnHidingStateChanged on the perceived player

void AWendigoAIController::OnPlayerHidingStateChanged(EHidingState NewState)
{
    if (NewState == EHidingState::Entering)
    {
        // Check if we currently see the player
        TArray<AActor*> PerceivedActors;
        AIPerceptionComponent->GetCurrentlyPerceivedActors(
            UAISense_Sight::StaticClass(), PerceivedActors);

        ASereneCharacter* Player = /* find in PerceivedActors */;
        if (Player)
        {
            UHidingComponent* HidingComp = Player->FindComponentByClass<UHidingComponent>();
            if (HidingComp && HidingComp->GetCurrentHidingSpot())
            {
                WitnessedHidingSpot = HidingComp->GetCurrentHidingSpot();
            }
        }
    }
}
```

### Pattern 4: Speed Management via Task Enter/Exit

**What:** Each behavior state sets movement speed in EnterState and restores it in ExitState. This pattern is already established in STT_InvestigateLocation.

**Speed constants to add to AIConstants namespace:**
```cpp
namespace AIConstants
{
    // Existing
    constexpr float WendigoWalkSpeed = 150.0f;          // Patrol

    // New for Phase 5
    constexpr float WendigoInvestigateSpeed = 200.0f;    // Already exists on task
    constexpr float WendigoChaseSpeed = 575.0f;          // ~10-15% faster than player sprint (500)
    constexpr float WendigoSearchSpeed = 180.0f;          // Between patrol and investigate
    constexpr float WendigoInvestigateSightSpeed = 250.0f; // Faster for visual disturbance
}
```

### Pattern 5: SpawnPoint Actor with Zone-Based Patrol Assignment

**What:** A simple AActor placed in the level that marks where a Wendigo can spawn. References an array of compatible patrol routes. On spawn, the Wendigo picks one.

```cpp
UCLASS()
class AWendigoSpawnPoint : public AActor
{
    GENERATED_BODY()
public:
    /** Patrol routes this spawn point can assign to spawned Wendigos. */
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI|Spawn")
    TArray<TObjectPtr<APatrolRouteActor>> AvailablePatrolRoutes;

    /** The Wendigo class to spawn (Blueprint subclass). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Spawn")
    TSubclassOf<AWendigoCharacter> WendigoClass;

    /** Spawn the Wendigo and assign a random patrol route from available routes. */
    AWendigoCharacter* SpawnWendigo();
};
```

### Anti-Patterns to Avoid

- **Storing chase state in State Tree instance data:** Instance data resets on state re-entry. Store LastKnownPlayerLocation, WitnessedHidingSpot, etc. on AWendigoCharacter (documented lesson from Phase 4).
- **Using MoveToLocation in chase task:** Use MoveToActor instead -- it auto-updates destination as the player moves. MoveToLocation requires manual re-pathing every tick.
- **Polling perception in chase task Tick:** The AI controller's Tick already processes perception continuously. Chase task should read state from SuspicionComponent/WendigoCharacter, not re-query perception.
- **EQS for simple random point generation:** EQS requires creating UEnvQuery data assets, FEnvQueryRequest setup, and async callbacks. GetRandomReachablePointInRadius is a single synchronous call.
- **NavLinkProxy for AI door interaction:** SmartLink ReceiveSmartLinkReached has known reliability issues across UE versions. Doors are simpler to handle procedurally in the chase task.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Random navmesh points | Custom point-sampling with raycasts | UNavigationSystemV1::GetRandomReachablePointInRadius | Handles navmesh connectivity, polygon sampling, agent radius |
| Continuous chase pathing | Manual re-path every N frames | AAIController::MoveToActor | Built-in continuous destination update, handles path invalidation |
| LOS checking | Custom raycast from eye socket | AAIController::LineOfSightTo | Checks line to center AND top of target, handles actor bounds |
| Smooth head turning | Manual rotation interpolation | AAIController::SetFocus/SetFocalPoint | Drives UpdateControlRotation automatically with smooth interpolation |
| Nearest waypoint finding | Loop through waypoints manually | FVector::DistSquared comparison loop | Simple enough to write, but ensure it uses DistSquared not Dist (avoid sqrt) |

**Key insight:** UE5's AIModule provides battle-tested solutions for movement, perception, and focus management. The custom work is in the State Tree task orchestration (when to chase, when to search, when to give up) and the game-specific logic (witnessed hiding, grab attack, behavior differentiation).

## Common Pitfalls

### Pitfall 1: MoveToActor Fails When Target Leaves NavMesh
**What goes wrong:** Player jumps or enters a hiding spot at a location not on the navmesh. MoveToActor returns Failed, chase breaks.
**Why it happens:** MoveToActor requires the target actor to be on or near the navmesh.
**How to avoid:** In the chase task, handle MoveToActor failure gracefully -- fall back to MoveToLocation using the last valid player location. Store last-known-good position on every successful tick.
**Warning signs:** Chase task immediately fails when player enters certain geometry.

### Pitfall 2: State Tree Instance Data Reset on State Re-Entry
**What goes wrong:** Chase data (LOS timer, search points) stored in instance data resets when the state is re-entered after a brief interruption.
**Why it happens:** State Tree instance data is re-initialized on EnterState. This is by design.
**How to avoid:** Store persistent data (LastKnownPlayerLocation, WitnessedHidingSpot, current chase target) on AWendigoCharacter. Use instance data only for per-activation state (move request flags, timers that should reset).
**Warning signs:** Wendigo "forgets" where player was after brief perception glitch.

### Pitfall 3: Speed Leak Between States
**What goes wrong:** Wendigo runs at chase speed permanently because ExitState wasn't called (interrupted transition) or forgot to restore.
**Why it happens:** If a higher-priority transition fires mid-task, ExitState may set a different speed than expected, or multiple tasks may fight over MaxWalkSpeed.
**How to avoid:** Always restore to a known baseline speed (WendigoWalkSpeed) in ExitState. Never restore "to previous speed" -- always to the constant. This pattern is already used in STT_InvestigateLocation and must continue.
**Warning signs:** Wendigo patrols at 575 cm/s after a chase.

### Pitfall 4: GetRandomReachablePointInRadius Returns Unreachable Points with Large Radii
**What goes wrong:** With search radius > ~1500cm, the function can return points that are technically on the navmesh but practically unreachable (different floors, across walls).
**Why it happens:** Internal dtNavMeshQuery::findNearestPoly can fail when search covers > 128 navmesh polygons.
**How to avoid:** Keep SearchRadius to 500-800cm (5-8 meters). This matches the "check nearby area" design intent. If a point fails MoveToLocation, skip it and try the next search point.
**Warning signs:** Wendigo runs to a completely different room during search.

### Pitfall 5: Chase MoveToActor Conflicts with SetFocalPoint
**What goes wrong:** During chase, both MoveToActor (which sets movement direction) and SetFocus (for head tracking) fight over the pawn's rotation.
**Why it happens:** bOrientRotationToMovement (set on WendigoCharacter) means movement direction controls body rotation. SetFocus controls where the AI "looks" but doesn't override body rotation when bOrientRotationToMovement is true.
**How to avoid:** During chase, use SetFocus(PlayerActor) so the head tracks the player while the body follows the path. This is correct and desirable -- the Wendigo's body follows the path while its head tracks the player.
**Warning signs:** Wendigo runs sideways or spins during chase.

### Pitfall 6: Door Interaction During Chase
**What goes wrong:** Wendigo cannot open doors during chase because doors require IInteractable::OnInteract which expects a player interactor.
**Why it happens:** DoorActor::OnInteract_Implementation checks for InventoryComponent (for locked doors) on the interactor.
**How to avoid:** Add an AI-compatible door interaction path. Either: (a) Add a public `OpenDoor()` method to ADoorActor that bypasses inventory checks for non-locked doors, or (b) Have DoorActor::CanInteract handle AI actors gracefully (skip inventory check if not a player). Locked doors should remain locked against AI.
**Warning signs:** Wendigo gets stuck at closed doors during chase.

### Pitfall 7: Player Hiding State Change Binding Timing
**What goes wrong:** AI controller tries to bind to player's HidingComponent::OnHidingStateChanged but the player isn't perceived yet.
**Why it happens:** Binding requires a reference to the player character, which the AI only gets when perception first detects them.
**How to avoid:** Bind to the delegate on first sight perception event. Unbind when perception is lost. Check binding validity before accessing.
**Warning signs:** Witnessed-hiding check never triggers.

## Code Examples

### New State Tree Task Template (Chase)
```cpp
// Source: Follows established pattern from STT_InvestigateLocation, STT_PatrolMoveToWaypoint

USTRUCT()
struct FSTT_ChasePlayerInstanceData
{
    GENERATED_BODY()

    /** Timer tracking seconds since LOS was lost. */
    float LOSLostTimer = 0.0f;

    /** True while MoveToActor request is active. */
    bool bMoveRequestActive = false;

    /** Cached reference to the player being chased. */
    TWeakObjectPtr<AActor> ChaseTarget;
};

USTRUCT(meta = (DisplayName = "Chase Player"))
struct FSTT_ChasePlayer : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FSTT_ChasePlayerInstanceData;

    virtual const UStruct* GetInstanceDataType() const override
    {
        return FInstanceDataType::StaticStruct();
    }

    virtual bool Link(FStateTreeLinker& Linker) override;
    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context,
        const float DeltaTime) const override;
    virtual void ExitState(FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override;

    TStateTreeExternalDataHandle<AAIController> ControllerHandle;

    UPROPERTY(EditAnywhere, Category = "Chase", meta = (ClampMin = "1.0"))
    float LOSLostTimeout = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Chase", meta = (ClampMin = "50.0"))
    float ChaseSpeed = 575.0f;

    UPROPERTY(EditAnywhere, Category = "Chase", meta = (ClampMin = "50.0"))
    float GrabRange = 150.0f;

    UPROPERTY(EditAnywhere, Category = "Chase")
    float AcceptanceRadius = 50.0f;
};
```

### NavMesh Random Point Generation for Search
```cpp
// Source: UNavigationSystemV1 API (official UE 5.7 documentation)

#include "NavigationSystem.h"

bool GenerateSearchPoints(UWorld* World, const FVector& Origin, float Radius,
    int32 Count, TArray<FVector>& OutPoints)
{
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
    if (!NavSys)
    {
        return false;
    }

    OutPoints.Empty();
    OutPoints.Add(Origin); // Always include the origin (last-known position)

    for (int32 i = 0; i < Count; ++i)
    {
        FNavLocation RandomResult;
        if (NavSys->GetRandomReachablePointInRadius(Origin, Radius, RandomResult))
        {
            OutPoints.Add(RandomResult.Location);
        }
    }

    return OutPoints.Num() > 1; // At least origin + 1 random point
}
```

### MoveToActor for Continuous Chase
```cpp
// Source: AAIController::MoveToActor (official UE 5.7 API)

// In STT_ChasePlayer::EnterState
const EPathFollowingRequestResult::Type MoveResult = Controller.MoveToActor(
    PlayerActor,
    AcceptanceRadius,
    /*bStopOnOverlap=*/ true,
    /*bUsePathfinding=*/ true,
    /*bCanStrafe=*/ true,   // Enable strafing for chase (head tracks player)
    /*FilterClass=*/ nullptr,
    /*bAllowPartialPath=*/ true
);
// MoveToActor continuously updates destination as PlayerActor moves.
// No need to re-issue move request in Tick.
```

### Finding Nearest Waypoint (Return to Patrol After Search)
```cpp
// On AWendigoCharacter or as utility function

int32 FindNearestWaypointIndex(const APatrolRouteActor* Route, const FVector& Location)
{
    if (!Route || Route->GetNumWaypoints() == 0)
    {
        return 0;
    }

    int32 NearestIndex = 0;
    float NearestDistSq = MAX_FLT;

    for (int32 i = 0; i < Route->GetNumWaypoints(); ++i)
    {
        const float DistSq = FVector::DistSquared(Route->GetWaypoint(i), Location);
        if (DistSq < NearestDistSq)
        {
            NearestDistSq = DistSq;
            NearestIndex = i;
        }
    }

    return NearestIndex;
}
```

### AI Door Interaction (Chase-Compatible)
```cpp
// Add to ADoorActor -- AI-compatible open method

UFUNCTION(BlueprintCallable, Category = "Door")
void OpenForAI(AActor* AIActor);

void ADoorActor::OpenForAI(AActor* AIActor)
{
    // AI cannot unlock locked doors
    if (bIsLocked)
    {
        return;
    }

    // Only open if currently closed
    if (bIsOpen)
    {
        return;
    }

    bIsOpen = true;

    // Always open away from AI
    if (AIActor)
    {
        const FVector ToAI = AIActor->GetActorLocation() - GetActorLocation();
        const float DotProduct = FVector::DotProduct(GetActorForwardVector(), ToAI);
        OpenDirection = (DotProduct >= 0.0f) ? 1.0f : -1.0f;
    }

    TargetAngle = OpenAngle * OpenDirection;
    SetActorTickEnabled(true);
}
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Behavior Tree for all AI | State Tree (hierarchical state machine) | UE 5.1+ | Already using State Tree. Phase 5 extends it, doesn't change approach. |
| EQS for all spatial queries | Simple navmesh queries for basic needs, EQS for complex scoring | UE 5.0+ | GetRandomReachablePointInRadius is sufficient for search. EQS reserved for when scoring/weighting is needed. |
| Manual pathfinding updates | MoveToActor with auto-updating | Long-standing | Chase uses MoveToActor, not repeated MoveToLocation calls. |
| Blackboard key storage | State Tree instance data + character properties | UE 5.1+ | Already using this pattern. Persistent state on character, activation state in instance data. |

**Deprecated/outdated:**
- Behavior Trees: Not deprecated but State Tree is the modern replacement. Project is committed to State Tree.
- EQS as primary spatial query: Still available but overkill for this phase's needs.

## Open Questions

1. **Grab Attack Implementation Depth**
   - What we know: Grab is a cinematic death (not damage numbers). Needs player input disable, camera blend to death cam, brief struggle animation.
   - What's unclear: Should this be a State Tree task that triggers a montage, or a separate kill sequence managed outside the State Tree? How much animation work is needed vs. a simple screen effect + fade to black?
   - Recommendation: Implement as a State Tree task (STT_GrabAttack) that disables player input, sets a camera blend, waits for a duration, then triggers death/restart. Detailed animation can be polished in Phase 8.

2. **Chase Re-Acquisition Window**
   - What we know: If player peeks out before the LOS-lost timer expires, chase re-acquires immediately.
   - What's unclear: Does "peek out" mean the AI perception system detects the player again (automatic via sight config), or do we need an explicit "re-acquisition" check?
   - Recommendation: This happens naturally -- the AI controller's Tick already processes sight perception continuously. When the player becomes visible again, SuspicionComponent stays at Alert level (it hasn't decayed enough in 3 seconds), so the State Tree remains in the Chase state. The LOS timer simply resets in the chase task.

3. **Visual Tells Scope**
   - What we know: Phase 5 adds head movement, speed, stance changes as visual tells. Audio hooks for Phase 6.
   - What's unclear: How sophisticated should stance changes be without animation assets?
   - Recommendation: Speed changes happen automatically via task Enter/Exit. Head tracking via SetFocus/SetFocalPoint. "Stance" can be represented by different AnimBlueprint states keyed off a behavior enum exposed from C++. Actual animation content is Phase 8 polish. Phase 5 exposes the data hooks.

4. **Door Opening During Chase -- Detection Range**
   - What we know: Wendigo should open doors during chase.
   - What's unclear: How does the Wendigo detect a closed door in its path? NavMesh may route through doors if they're always "open" for navigation.
   - Recommendation: Simplest approach -- doors have a trigger volume or the chase task does a short forward raycast to detect ADoorActor. If door is found and closed, call OpenForAI. NavMesh should be configured with doors as NavLink areas, but for demo scope a proximity-based trigger is sufficient.

## WendigoCharacter Extensions Needed

The following properties/methods should be added to AWendigoCharacter to support Phase 5:

```cpp
// --- Phase 5: Chase & Search State ---

/** Last known player world position (updated during sight perception). */
UPROPERTY(BlueprintReadOnly, Category = "AI|Chase")
FVector LastKnownPlayerLocation = FVector::ZeroVector;

/** True if a valid last-known position has been recorded. */
UPROPERTY(BlueprintReadOnly, Category = "AI|Chase")
bool bHasLastKnownPlayerLocation = false;

/** Hiding spot the Wendigo witnessed the player entering (nullptr if not witnessed). */
UPROPERTY(BlueprintReadOnly, Category = "AI|Chase")
TWeakObjectPtr<AHidingSpotActor> WitnessedHidingSpot;

/** Current behavior state for animation/visual tells. */
UPROPERTY(BlueprintReadOnly, Category = "AI|Behavior")
EWendigoBehaviorState BehaviorState = EWendigoBehaviorState::Patrol;

/** Delegate broadcast when behavior state changes (for audio/visual hooks). */
UPROPERTY(BlueprintAssignable, Category = "AI|Behavior")
FOnBehaviorStateChanged OnBehaviorStateChanged;
```

## New Types Needed

```cpp
// In MonsterAITypes.h

/** Behavior state for visual/audio tells. More granular than EAlertLevel. */
UENUM(BlueprintType)
enum class EWendigoBehaviorState : uint8
{
    Patrol          UMETA(DisplayName = "Patrol"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Chasing         UMETA(DisplayName = "Chasing"),
    Searching       UMETA(DisplayName = "Searching"),
    GrabAttack      UMETA(DisplayName = "Grab Attack")
};

/** Broadcast when behavior state changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBehaviorStateChanged, EWendigoBehaviorState, NewState);

/** Stimulus type for differentiated investigation behavior. */
UENUM(BlueprintType)
enum class EStimulusType : uint8
{
    None    UMETA(DisplayName = "None"),
    Sound   UMETA(DisplayName = "Sound"),
    Sight   UMETA(DisplayName = "Sight")
};
```

## New Gameplay Tags Needed

```cpp
// In SereneTags.h -- Phase 5 additions

// AI behavior states (more granular than alert levels)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Behavior_Patrol);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Behavior_Investigating);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Behavior_Chasing);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Behavior_Searching);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Behavior_GrabAttack);

// Spawn system
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Spawn_Zone);
```

## New State Tree Tasks Needed

| Task | Purpose | Key Properties |
|------|---------|---------------|
| STT_ChasePlayer | Pursue player using MoveToActor | ChaseSpeed, LOSLostTimeout, GrabRange |
| STT_SearchArea | Move to last-known + random nearby points | SearchRadius, NumRandomPoints, SearchDuration, LingerDuration |
| STT_GrabAttack | Cinematic player kill sequence | GrabDuration, CameraBlendTime |
| STT_ReturnToNearestWaypoint | Navigate to closest patrol waypoint | AcceptanceRadius |

## New State Tree Conditions Needed

| Condition | Purpose | Key Properties |
|-----------|---------|---------------|
| STC_HasChaseTarget | Check if valid chase target exists | (none -- reads from character) |
| STC_StimulusType | Check last stimulus type (Sight vs Sound) | RequiredType, bInvertCondition |

## Existing Tasks to Enhance

| Task | Enhancement | Why |
|------|-------------|-----|
| STT_InvestigateLocation | Add stimulus-type-aware speed selection and differentiated look-around | Context says sight = faster/direct, sound = cautious/slower |
| SuspicionComponent | Add LastStimulusType tracking, expose via getter | Investigation needs to know if stimulus was sight or sound |

## Sources

### Primary (HIGH confidence)
- Existing codebase: All 8 AI source files read and analyzed in full
- AAIController::MoveToActor API: [Official UE 5.7 Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/AIModule/AAIController/MoveToActor) -- continuously updates destination
- AAIController focus methods: [Official UE 5.7 Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/AIModule/AAIController) -- SetFocus, SetFocalPoint, LineOfSightTo
- State Tree overview: [Official UE 5.7 Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/overview-of-state-tree-in-unreal-engine) -- concurrent tasks, transition types, state selection
- UNavigationSystemV1 random points: [Official UE 5.7 Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/NavigationSystem/UNavigationSystemV1) -- GetRandomReachablePointInRadius
- Spawning Actors: [Official UE 5.7 Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/spawning-actors-in-unreal-engine) -- UWorld::SpawnActor patterns

### Secondary (MEDIUM confidence)
- GetRandomReachablePointInRadius gotchas: [Epic Forums](https://forums.unrealengine.com/t/getrandomreachablepointinradius/380662) -- 128 polygon limit with large radii
- GetRandomReachable vs GetRandomNavigable: [Epic Forums](https://forums.unrealengine.com/t/whats-the-difference-between-getrandomreachablepointinradius-and-getrandompointinnavigableradius-in-which-case-we-should-use-which/426698) -- reachable checks connectivity
- NavLinkProxy reliability issues: [Epic Forums](https://forums.unrealengine.com/t/navlinkproxy-child-actor-smart-link-in-5-4/1946864) -- SmartLink events not firing reliably

### Tertiary (LOW confidence)
- None -- all findings verified against official documentation or existing codebase patterns

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH -- all modules already in project, APIs verified against official docs
- Architecture (State Tree hierarchy): HIGH -- extends proven existing pattern from Phase 4, consistent with State Tree documentation
- New tasks/conditions: HIGH -- follows established FSTT_ / FSTC_ patterns from existing codebase
- Chase mechanics: HIGH -- MoveToActor API verified, LOS pattern standard
- Search mechanics: HIGH -- GetRandomReachablePointInRadius API verified with known caveats documented
- Witnessed hiding: MEDIUM -- implementation approach is sound but requires careful event binding timing
- Door interaction: MEDIUM -- approach is straightforward but NavMesh/door integration needs level-design testing
- Spawn system: HIGH -- standard UWorld::SpawnActor pattern

**Research date:** 2026-02-11
**Valid until:** 2026-03-11 (stable domain -- UE5 AI APIs are mature)
