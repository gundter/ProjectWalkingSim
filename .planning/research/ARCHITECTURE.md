# Architecture Patterns

**Domain:** First-Person Psychological Horror Game (UE5)
**Project:** The Juniper Tree
**Researched:** 2026-02-07
**Confidence:** HIGH (verified via official UE5 documentation and established patterns)

## Executive Summary

First-person psychological horror games in UE5 follow a component-based architecture with clear separation between player systems, AI systems, and game state management. The recommended architecture uses UE5's Enhanced Input, AI Perception, Behavior Trees, and Subsystem patterns to create loosely-coupled, maintainable systems.

Key architectural insight: Horror games require bidirectional awareness between Player and Monster systems, mediated through a central Horror State Manager that tracks tension, monster awareness, and player visibility status.

---

## Recommended Architecture

```
+------------------------------------------------------------------+
|                        GAME FRAMEWORK LAYER                       |
|  ASereneGameMode  |  AHorrorGameState  |  AHorrorPlayerState     |
+------------------------------------------------------------------+
           |                    |                    |
           v                    v                    v
+------------------------------------------------------------------+
|                       SUBSYSTEM LAYER                             |
|  UHorrorStateSubsystem  |  USaveSubsystem  |  UTensionSubsystem  |
|  (GameInstanceSubsystem)   (GameInstanceSubsystem)                |
+------------------------------------------------------------------+
           |                    |                    |
           v                    v                    v
+------------------------------------------------------------------+
|                      ACTOR/COMPONENT LAYER                        |
|                                                                   |
|  +----------------+     +------------------+     +---------------+|
|  | AHorrorPlayer  |     | AWendigoMonster  |     | AHidingSpot   ||
|  |----------------|     |------------------|     |---------------||
|  | UInventory     |     | UAIPerception    |     | UHideVolume   ||
|  | UVisibility    |     | UBehaviorTree    |     | UInteract     ||
|  | UInteraction   |     | UBlackboard      |     +---------------+|
|  | UStamina       |     | UMonsterAudio    |                      |
|  | ULightAware    |     +------------------+                      |
|  +----------------+                                               |
+------------------------------------------------------------------+
           |                    |                    |
           v                    v                    v
+------------------------------------------------------------------+
|                       INTERFACE LAYER                             |
|  IInteractable  |  IHideable  |  ILightSource  |  ISaveable      |
+------------------------------------------------------------------+
           |                    |                    |
           v                    v                    v
+------------------------------------------------------------------+
|                        DATA LAYER                                 |
|  UItemDataAsset  |  USaveGameData  |  UMonsterBehaviorData       |
+------------------------------------------------------------------+
```

### Component Boundaries

| Component | Responsibility | Communicates With | Data Owned |
|-----------|----------------|-------------------|------------|
| **AHorrorPlayer** | Player pawn, input handling, camera | All player components, HorrorStateSubsystem | Transform, health |
| **UInventoryComponent** | Item storage, use/combine logic | IInteractable actors, UI | TArray<FInventorySlot> |
| **UVisibilityComponent** | Track player's visibility to AI | LightAwareComponent, HorrorStateSubsystem | float VisibilityLevel |
| **UInteractionComponent** | Detect and trigger interactions | IInteractable actors, UI | Current target actor |
| **UStaminaComponent** | Sprint/stamina management | Player movement, UI | float CurrentStamina |
| **ULightAwareComponent** | Detect ambient light level | VisibilityComponent, InventoryComponent | float LightExposure |
| **AWendigoMonster** | Monster pawn, animation, audio | AI Controller, HorrorStateSubsystem | Transform, state |
| **AWendigoAIController** | AI decision making, behavior tree | Monster pawn, perception, blackboard | AI state |
| **UAIPerceptionComponent** | Sight/hearing detection | AI Controller, behavior tree | Perceived actors |
| **AHidingSpot** | Hiding location actor | Player, Monster AI | bOccupied, HideType |
| **UHorrorStateSubsystem** | Central horror state coordination | All major systems | Tension, awareness |

### Data Flow

**Player Input to Action Flow:**
```
Enhanced Input System
    |
    v
AHorrorPlayerController::SetupInputComponent()
    |
    +---> IA_Move --> UCharacterMovementComponent
    +---> IA_Look --> Camera/Spring Arm
    +---> IA_Interact --> UInteractionComponent::TryInteract()
    +---> IA_UseItem --> UInventoryComponent::UseItem()
    +---> IA_Sprint --> UStaminaComponent::StartSprint()
    +---> IA_Hide --> Check IHideable, enter hiding state
```

**Monster Perception to Behavior Flow:**
```
UAIPerceptionComponent (Sight/Hearing)
    |
    v
OnTargetPerceptionUpdated delegate
    |
    v
AWendigoAIController::HandlePerception()
    |
    +---> Update Blackboard keys (PlayerLocation, LastKnownLocation)
    +---> Notify UHorrorStateSubsystem (player spotted/lost)
    |
    v
Behavior Tree evaluates conditions
    |
    +---> BTT_Patrol (no awareness)
    +---> BTT_Investigate (heard noise)
    +---> BTT_Stalk (sees player far away)
    +---> BTT_Chase (sees player close)
    +---> BTT_Search (lost player)
```

**Visibility Calculation Flow:**
```
ULightAwareComponent::CalculateLightExposure()
    |
    +---> Sample scene lighting at player location
    +---> Check held light sources (flashlight, lantern)
    |
    v
UVisibilityComponent::UpdateVisibility()
    |
    +---> Combine light exposure + movement + stance
    +---> Output: float VisibilityLevel (0.0 = hidden, 1.0 = fully visible)
    |
    v
UHorrorStateSubsystem::OnPlayerVisibilityChanged()
    |
    +---> Modifies AI perception range
    +---> Updates tension based on visibility
```

**Hiding System Flow:**
```
Player approaches IHideable actor
    |
    v
UInteractionComponent detects AHidingSpot
    |
    v
Player presses IA_Interact
    |
    v
AHidingSpot::OnPlayerEnter()
    |
    +---> Set player hidden state
    +---> Disable player collision with AI
    +---> Play enter animation
    +---> Notify HorrorStateSubsystem
    |
    v
AWendigoAIController::CanSeePlayer() returns false
    |
    +---> But monster may search hiding spots
    +---> Tension increases while monster nearby
```

---

## Patterns to Follow

### Pattern 1: Component-Based Actor Composition

**What:** Build complex actors from small, focused components rather than monolithic actor classes.

**When:** Always. Every major system should be a component that can be added to actors.

**Why:** Enables reuse (NPCs could have inventory too), easier testing, cleaner dependencies.

**Example:**
```cpp
// In AHorrorPlayer.h
UCLASS()
class AHorrorPlayer : public ACharacter
{
    GENERATED_BODY()

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UInventoryComponent> InventoryComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UVisibilityComponent> VisibilityComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UInteractionComponent> InteractionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaminaComponent> StaminaComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<ULightAwareComponent> LightAwareComponent;
};
```

### Pattern 2: Interface-Based Communication

**What:** Use UInterfaces for cross-system communication instead of direct actor references.

**When:** Whenever an actor needs to interact with actors of unknown/varied types.

**Why:** Decouples systems, enables polymorphism, avoids casting chains.

**Example:**
```cpp
// IInteractable.h
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
    GENERATED_BODY()
};

class IInteractable
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    bool CanInteract(AActor* Interactor) const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void OnInteract(AActor* Interactor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    FText GetInteractionPrompt() const;
};

// Usage in InteractionComponent
void UInteractionComponent::TryInteract()
{
    if (CurrentTarget && CurrentTarget->Implements<UInteractable>())
    {
        if (IInteractable::Execute_CanInteract(CurrentTarget, GetOwner()))
        {
            IInteractable::Execute_OnInteract(CurrentTarget, GetOwner());
        }
    }
}
```

### Pattern 3: GameInstanceSubsystem for Global State

**What:** Use UGameInstanceSubsystem for state that persists across levels and needs global access.

**When:** Horror state management, save system, audio management.

**Why:** Automatic lifecycle, easy access via GetGameInstance(), survives level transitions.

**Example:**
```cpp
// UHorrorStateSubsystem.h
UCLASS()
class UHorrorStateSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Tension Management
    UFUNCTION(BlueprintCallable)
    float GetCurrentTension() const { return CurrentTension; }

    UFUNCTION(BlueprintCallable)
    void AddTension(float Amount);

    // Monster Awareness
    UFUNCTION(BlueprintCallable)
    EMonsterAwareness GetMonsterAwareness() const { return MonsterAwareness; }

    // Events for UI/Audio to bind
    UPROPERTY(BlueprintAssignable)
    FOnTensionChanged OnTensionChanged;

    UPROPERTY(BlueprintAssignable)
    FOnMonsterAwarenessChanged OnMonsterAwarenessChanged;

private:
    float CurrentTension = 0.0f;
    EMonsterAwareness MonsterAwareness = EMonsterAwareness::Unaware;
};

// Access from anywhere
UHorrorStateSubsystem* HorrorState = GetGameInstance()->GetSubsystem<UHorrorStateSubsystem>();
```

### Pattern 4: Data-Driven Item Definitions

**What:** Define item properties in UDataAsset subclasses, not in code.

**When:** All item definitions, monster behavior parameters, audio cues.

**Why:** Designer-friendly, no recompile for balance changes, easy serialization.

**Example:**
```cpp
// UItemDataAsset.h
UCLASS(BlueprintType)
class UItemDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FName ItemID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText Description;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSoftObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EItemType ItemType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    bool bCanCombine;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bCanCombine"))
    TArray<FItemCombination> Combinations;
};
```

### Pattern 5: Event-Driven UI Updates

**What:** Use multicast delegates for UI updates instead of polling.

**When:** All UI that displays game state (inventory, stamina, tension indicator).

**Why:** No tick overhead, immediate updates, clean separation of concerns.

**Example:**
```cpp
// In UInventoryComponent.h
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, int32, SlotIndex, UItemDataAsset*, Item);

UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
FOnInventoryChanged OnInventoryChanged;

// In InventoryComponent.cpp
void UInventoryComponent::AddItem(UItemDataAsset* Item)
{
    int32 SlotIndex = FindEmptySlot();
    if (SlotIndex != INDEX_NONE)
    {
        Items[SlotIndex] = Item;
        OnInventoryChanged.Broadcast(SlotIndex, Item);
    }
}

// In UI Widget Blueprint
// Bind to OnInventoryChanged -> Refresh slot display
```

### Pattern 6: Behavior Tree with Blackboard for AI

**What:** Use UE5's Behavior Tree system with Blackboard for AI decision making.

**When:** Monster AI states (patrol, stalk, chase, search).

**Why:** Visual debugging, designer-friendly, industry-standard approach.

**Key Blackboard Keys:**
- `PlayerActor` (Object): Direct reference when player is seen
- `LastKnownPlayerLocation` (Vector): Where player was last seen
- `NoiseLocation` (Vector): Where a sound was heard
- `AwarenessLevel` (Float): How alert the monster is
- `CurrentState` (Enum): Patrol/Investigate/Stalk/Chase/Search

---

## Anti-Patterns to Avoid

### Anti-Pattern 1: God Actor

**What:** Putting all player logic in AHorrorPlayer class directly.

**Why bad:** 3000+ line actor classes, impossible to test, tight coupling.

**Instead:** Extract to components. AHorrorPlayer should be mostly component references.

### Anti-Pattern 2: Direct Actor References for Communication

**What:** Storing TObjectPtr<AWendigoMonster> in player class.

**Why bad:** Tight coupling, breaks if monster doesn't exist, hard to test.

**Instead:** Use interfaces or communicate through HorrorStateSubsystem.

### Anti-Pattern 3: Tick-Based State Checking

**What:** Checking `if (bIsHiding)` every tick in monster AI.

**Why bad:** Performance overhead, delayed response, hard to debug.

**Instead:** Event-driven with delegates when hiding state changes.

### Anti-Pattern 4: Blueprint-Only Complex Logic

**What:** Implementing inventory combination logic entirely in Blueprint.

**Why bad:** Hard to version control, performance for complex logic, debugging difficulty.

**Instead:** Core logic in C++, Blueprint for configuration and simple extensions.

### Anti-Pattern 5: Monolithic Save System

**What:** Single giant SaveGame struct with everything.

**Why bad:** Version migration nightmares, all-or-nothing saves.

**Instead:** Interface-based saving where each system handles its own serialization.

---

## Core Systems Architecture

### 1. First-Person Character Architecture

**Dual-Mesh Approach for Visible Body:**

```cpp
UCLASS()
class AHorrorPlayer : public ACharacter
{
    GENERATED_BODY()

protected:
    // First-person arms mesh (owner-only visible)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;

    // Third-person mesh for shadow/reflection (owner-hidden)
    // Uses GetMesh() from ACharacter

    AHorrorPlayer()
    {
        // First-person mesh setup
        FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPMesh"));
        FirstPersonMesh->SetOnlyOwnerSee(true);
        FirstPersonMesh->SetupAttachment(GetFirstPersonCamera());
        FirstPersonMesh->bCastDynamicShadow = false;
        FirstPersonMesh->CastShadow = false;

        // Hide main mesh from owner, visible for shadow
        GetMesh()->SetOwnerNoSee(true);
    }
};
```

**Camera Setup:**
- Camera attached to spring arm or directly to root
- First Person Rendering features for proper FOV on arms
- Head bob component for movement immersion

### 2. Inventory System Architecture

```
+-------------------+
| UInventoryComponent |
+-------------------+
| - TArray<FInventorySlot> Slots |
| - int32 MaxSlots (8) |
+-------------------+
| + AddItem() |
| + RemoveItem() |
| + UseItem() |
| + CombineItems() |
| + DiscardItem() |
+-------------------+
         |
         | References
         v
+-------------------+
| UItemDataAsset |
+-------------------+
| - FName ItemID |
| - EItemType Type |
| - UseAbility |
+-------------------+
```

**FInventorySlot Structure:**
```cpp
USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UItemDataAsset> Item;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity = 0;

    bool IsEmpty() const { return Item == nullptr || Quantity <= 0; }
};
```

### 3. Monster AI Architecture

```
+------------------------+
| AWendigoMonster |
| (ACharacter) |
+------------------------+
         |
         | Possessed by
         v
+------------------------+
| AWendigoAIController |
| (AAIController) |
+------------------------+
| - UBehaviorTreeComponent |
| - UBlackboardComponent |
| - UAIPerceptionComponent |
+------------------------+
         |
         | Runs
         v
+------------------------+
| BT_Wendigo |
| (UBehaviorTree) |
+------------------------+
| Selector |
| +-- Chase (if sees player close) |
| +-- Stalk (if sees player far) |
| +-- Investigate (if heard noise) |
| +-- Search (if recently lost player) |
| +-- Patrol (default) |
+------------------------+
```

**AI Perception Configuration:**
- Sight: Primary sense, 90 degree FOV, 2000 unit range
- Hearing: Secondary, reacts to player footsteps, item use
- Custom "Fear Sense": Detect player based on visibility component

### 4. Hiding System Architecture

```
+-------------------+
| AHidingSpot |
| (AActor) |
+-------------------+
| - UBoxComponent TriggerVolume |
| - USkeletalMeshComponent DoorMesh |
| - FTransform HiddenPlayerTransform |
| - EHidingType Type (Closet/UnderBed/Locker) |
+-------------------+
| + IInteractable interface |
| + CanHide() |
| + EnterHiding() |
| + ExitHiding() |
+-------------------+
         |
         | Notifies
         v
+-------------------+
| UHorrorStateSubsystem |
+-------------------+
| + SetPlayerHidden(bool) |
| + IsPlayerHidden() |
+-------------------+
         |
         | Checked by
         v
+-------------------+
| AWendigoAIController |
+-------------------+
| + CanSeePlayer() returns false if hidden |
| + MaySearchHidingSpots() |
+-------------------+
```

### 5. Save System Architecture

**ISaveable Interface:**
```cpp
UINTERFACE(MinimalAPI)
class USaveable : public UInterface
{
    GENERATED_BODY()
};

class ISaveable
{
    GENERATED_BODY()
public:
    virtual void OnSave(FArchive& Ar) = 0;
    virtual void OnLoad(FArchive& Ar) = 0;
    virtual FString GetSaveID() const = 0;
};
```

**Save Subsystem:**
```cpp
UCLASS()
class USaveSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void SaveGame(const FString& SlotName);

    UFUNCTION(BlueprintCallable)
    void LoadGame(const FString& SlotName);

private:
    void CollectSaveables(TArray<TScriptInterface<ISaveable>>& OutSaveables);
};
```

**What Gets Saved:**
- Player transform and state
- Inventory contents
- Monster position and awareness
- World state (doors opened, items picked up)
- Progression flags

---

## Build Order Implications

Based on component dependencies, the recommended build order is:

### Phase 1: Foundation (No Dependencies)
1. **Core Interfaces** - IInteractable, IHideable, ISaveable, ILightSource
2. **Data Assets** - UItemDataAsset, UMonsterBehaviorData
3. **Base Subsystems** - UHorrorStateSubsystem (empty implementation)

### Phase 2: Player Core (Depends on Phase 1)
1. **AHorrorPlayer** - Basic character with FP/TP mesh setup
2. **UInteractionComponent** - Uses IInteractable
3. **UStaminaComponent** - Standalone, notifies subsystem
4. **Enhanced Input Setup** - Input actions and mappings

### Phase 3: Inventory (Depends on Phase 2)
1. **UInventoryComponent** - Uses UItemDataAsset
2. **APickupItem** - Implements IInteractable
3. **Inventory UI** - Binds to component events

### Phase 4: Light & Visibility (Depends on Phase 2)
1. **ULightAwareComponent** - Calculates light exposure
2. **UVisibilityComponent** - Combines light + movement
3. **AFlashlightItem** - Implements ILightSource
4. **ALanternItem** - Implements ILightSource

### Phase 5: Hiding System (Depends on Phase 2, 4)
1. **AHidingSpot** - Implements IHideable, IInteractable
2. **Hiding State in Subsystem** - Tracks player hiding
3. **Hiding Animations** - Enter/exit/peek

### Phase 6: Monster AI (Depends on Phase 4, 5)
1. **AWendigoMonster** - Basic character setup
2. **AWendigoAIController** - Perception, blackboard
3. **BT_Wendigo** - Behavior tree with states
4. **EQS Queries** - Patrol points, hiding spot checks

### Phase 7: Save System (Depends on All)
1. **USaveSubsystem** - Core save/load
2. **ISaveable Implementations** - All systems implement
3. **Save UI** - Slot selection, confirmation

### Phase 8: Polish (Depends on All)
1. **Tension System Integration** - Audio, visual effects
2. **Performance Optimization** - Profiling, LOD
3. **Bug Fixes and Balancing**

---

## Scalability Considerations

| Concern | Current Scope | If Expanded |
|---------|---------------|-------------|
| Multiple Monsters | Single Wendigo | Use monster manager subsystem, shared blackboard |
| Larger Levels | Demo environment | Streaming levels, AI throttling by distance |
| More Item Types | ~20 items | Category-based DataAsset inheritance |
| Complex Puzzles | Simple key/door | Puzzle subsystem with dependency graph |
| Save Compatibility | Demo saves | Version numbers, migration functions |

---

## Technical Specifications

**Performance Targets:**
- 60 FPS minimum on target hardware
- AI perception checks: 10ms budget max
- Visibility calculation: 2ms budget max
- Save/Load: Async, < 500ms

**Memory Considerations:**
- Soft references for item icons (load on demand)
- Monster audio pooling
- Behavior tree shared across monster instances (if multiple)

---

## Sources

### Official Documentation (HIGH Confidence)
- [First Person Template in UE5](https://dev.epicgames.com/documentation/en-us/unreal-engine/first-person-template-in-unreal-engine)
- [Implementing Your Character](https://dev.epicgames.com/documentation/en-us/unreal-engine/implementing-your-character-in-unreal-engine)
- [First Person Rendering](https://dev.epicgames.com/documentation/en-us/unreal-engine/first-person-rendering)
- [AI Perception](https://dev.epicgames.com/documentation/en-us/unreal-engine/ai-perception-in-unreal-engine)
- [Behavior Tree User Guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/behavior-tree-in-unreal-engine---user-guide)
- [Environment Query System](https://dev.epicgames.com/documentation/en-us/unreal-engine/environment-query-system-in-unreal-engine)
- [Programming Subsystems](https://dev.epicgames.com/documentation/en-us/unreal-engine/programming-subsystems-in-unreal-engine)
- [Enhanced Input](https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine)
- [Saving and Loading Your Game](https://dev.epicgames.com/documentation/en-us/unreal-engine/saving-and-loading-your-game-in-unreal-engine)
- [Game Mode and Game State](https://dev.epicgames.com/documentation/en-us/unreal-engine/game-mode-and-game-state-in-unreal-engine)
- [Interfaces in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/interfaces-in-unreal-engine)
- [Multicast Delegates](https://dev.epicgames.com/documentation/en-us/unreal-engine/multicast-delegates-in-unreal-engine)

### Community Tutorials (MEDIUM Confidence)
- [Horror AI Tutorial Part 1](https://dev.epicgames.com/community/learning/tutorials/2JzM/horror-ai-unreal-engine-5-tutorial-part-1)
- [Enhanced Input with Gameplay Tags](https://dev.epicgames.com/community/learning/tutorials/aqrD/unreal-engine-enhanced-input-binding-with-gameplay-tags-c)
- [How to Make a Horror Game in UE5](https://dev.epicgames.com/community/learning/tutorials/yy39/how-to-make-a-horror-game-in-unreal-engine-5-full-beginner-course)
- [Event Dispatchers Tutorial](https://dev.epicgames.com/community/learning/tutorials/ZdaB/event-dispatchers-delegates)

### Architecture References (MEDIUM Confidence)
- [Tom Looman - UE5 Save System](https://tomlooman.com/unreal-engine-cpp-save-system/)
- [Inventory System Component Guide](https://www.spongehammer.com/unreal-engine-5-inventory-system-cpp-guide/)
- [AI Behavior Trees Guide](https://outscal.com/blog/ai-behavior-trees-and-blackboards-ue5-guide)
- [Interfaces as Component Providers](https://medium.com/@imane.taruf/using-interfaces-as-component-providers-in-unreal-engine-5-c-with-blueprint-support-52a133bd50e1)

---

*Architecture research completed: 2026-02-07*
