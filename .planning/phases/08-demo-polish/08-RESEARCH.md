# Phase 8: Demo Polish - Research

**Researched:** 2026-02-13
**Domain:** UE5.7 level assembly, narrative delivery (audio monologue, readable UI), trigger volumes, Level Sequencer, object inspection, performance optimization (Lumen/MegaLights)
**Confidence:** HIGH

## Summary

Phase 8 assembles all Phase 1-7 gameplay systems into a complete playable demo. The codebase already has everything needed for the core loop: player controller, inventory, locked doors, hiding spots, Wendigo AI (patrol/chase/search/grab), spatial audio, music tension, save system, and death handling. This phase primarily creates **new C++ actors and widgets** for narrative delivery (monologue triggers, readable document UI, object inspection, demo ending sequence), then populates a new demo level with all actors placed and configured.

The research identifies four C++ feature areas that need building: (1) a NarrativeTriggerActor for monologue/event triggers, (2) a DocumentReaderWidget for full-screen readable note display, (3) an InspectableActor for 3D object examination, and (4) a DemoEndingManager for the fade-to-black title card ending. All of these follow established UE5 patterns that integrate cleanly with the existing IInteractable framework and HUD widget hierarchy. The final deliverable is a single demo level (DemoMap) containing the farmstead environment with all actors placed, NavMesh configured for indoor/outdoor traversal, and performance validated at 60 FPS.

**Primary recommendation:** Build 4 new C++ classes (NarrativeTriggerActor, DocumentReaderWidget, InspectableActor, DemoEndingManager), expand ReadableActor to open the document widget, create the demo level with farmstead layout, and validate performance with Lumen/MegaLights. Use PlayerCameraManager::StartCameraFade for the ending fade -- simpler and more code-controllable than Level Sequencer for a single fade-to-black.

## Standard Stack

### Core (Already In Project)
| Library/API | Version | Purpose | Why Standard |
|-------------|---------|---------|--------------|
| AInteractableBase | Project | Base for all interactable actors | Already used by Door, Pickup, Readable, Drawer, TapeRecorder |
| AReadableActor | Project | Notes and documents | Already exists; needs OnInteract to open DocumentReaderWidget |
| IInteractable | Project | Interaction interface | Standard contract for all interactable objects |
| ISaveable | Project | Save/load interface | Needed for new saveable actors |
| ASereneHUD / USereneHUDWidget | Project | HUD hierarchy | Widget display root; new widgets added here or standalone |
| AWendigoSpawnPoint | Project | AI spawning | Demo level places these for timed Wendigo activation |
| APatrolRouteActor | Project | Wendigo patrol paths | Demo level needs indoor + outdoor patrol routes |
| AAmbientAudioManager | Project | Ambient soundscape | Demo level places one with cabin/woods ambience |
| ATapeRecorderActor | Project | Save points | Demo level places 1-2 for save opportunities |
| ADoorActor | Project | Locked/unlocked doors | Progression gating with RequiredItemId |
| AHidingSpotActor | Project | Hiding spots | Scattered throughout cabin and barn |
| APickupActor | Project | Collectible items | Key items for locked doors |
| UItemDataAsset | Project | Item definitions | New key items and story objects |

### New C++ Classes Required
| Class | Base | Purpose | Why New |
|-------|------|---------|---------|
| ANarrativeTriggerActor | AActor | Trigger volume that plays audio monologue and/or fires events on player overlap | No existing trigger actor; needed for detective monologues |
| UDocumentReaderWidget | UUserWidget | Full-screen overlay for reading notes/letters | ReadableActor currently only logs; needs actual UI |
| AInspectableActor | AInteractableBase | 3D object examination with full-screen view | Photographs and story objects need examination mode |
| UDemoEndingManager | UActorComponent | Orchestrates fade-to-black and title card display | Demo ending needs coordinated sequence |

### Supporting UE5 APIs
| API | Module | Purpose | When to Use |
|-----|--------|---------|-------------|
| APlayerCameraManager::StartCameraFade | Engine | Fade screen to black | Demo ending sequence |
| UGameplayStatics::PlaySound2D | Engine | Non-positional audio | Detective monologue (2D, centered) |
| UBoxComponent (overlap) | Engine | Trigger volumes | NarrativeTriggerActor overlap detection |
| UGameplayStatics::SetGamePaused | Engine | Pause during document read | Optional: freeze world while reading |
| FTimerManager | Engine | Delayed actions | Monologue delay, ending sequence timing |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| StartCameraFade (chosen) | Level Sequencer Fade Track | Sequencer is powerful but overkill for a single fade-to-black. StartCameraFade is 3 lines of C++ and fully code-controllable |
| SceneCaptureComponent2D for inspection | Camera zoom to object in world | SceneCapture adds GPU cost and complexity; simpler to show a full-screen UMG overlay with static image or description text for the demo scope |
| Custom dialogue system | UDialogueWave/UDialogueVoice | Engine dialogue system is designed for multi-speaker conversations; single detective monologue is simpler with PlaySound2D + USoundBase |
| Level Streaming | Single persistent level | Demo is one contiguous location; no need for streaming. Single level is simpler to build and debug |

### New Module Dependencies
| Module | Why | Add To |
|--------|-----|--------|
| None required | All new classes use Engine, UMG, and existing project modules | N/A |

Note: If Level Sequencer integration is desired later, `LevelSequence` and `MovieScene` modules would need adding to Build.cs. For this demo scope, StartCameraFade avoids that dependency.

## Architecture Patterns

### Recommended Project Structure
```
Source/ProjectWalkingSim/
  Public/
    Narrative/                      # NEW folder for Phase 8
      NarrativeTriggerActor.h       # Trigger volume for monologues/events
      DemoEndingManager.h           # Ending sequence orchestrator (component)
    Player/
      HUD/
        DocumentReaderWidget.h      # Full-screen note/letter reader
    Interaction/
      InspectableActor.h            # 3D object inspection actor
  Private/
    Narrative/
      NarrativeTriggerActor.cpp
      DemoEndingManager.cpp
    Player/
      HUD/
        DocumentReaderWidget.cpp
    Interaction/
      InspectableActor.cpp
      ReadableActor.cpp             # MODIFY: open DocumentReaderWidget on interact
```

### Pattern 1: NarrativeTriggerActor (Overlap-Based Monologue Trigger)

**What:** An actor with a UBoxComponent trigger volume that plays a detective monologue (USoundBase via PlaySound2D) when the player overlaps. Fires once per playthrough (bHasTriggered flag). Optionally fires a delegate that other systems can listen to (e.g., spawn the Wendigo, unlock a door).

**When to use:** Every detective internal monologue trigger point in the demo (opening monologue, mid-investigation observations, Wendigo encounter, ending revelation).

**Example:**
```cpp
// Source: Standard UE5 overlap pattern + project conventions
UCLASS()
class PROJECTWALKINGSIM_API ANarrativeTriggerActor : public AActor
{
    GENERATED_BODY()

public:
    ANarrativeTriggerActor();

    // Delegate fired when trigger activates (for Blueprint wiring)
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNarrativeTriggered);

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnNarrativeTriggered OnNarrativeTriggered;

protected:
    UPROPERTY(VisibleAnywhere, Category = "Trigger")
    TObjectPtr<UBoxComponent> TriggerBox;

    /** Audio to play when triggered (detective monologue). */
    UPROPERTY(EditAnywhere, Category = "Narrative")
    TObjectPtr<USoundBase> MonologueSound;

    /** Optional subtitle text displayed while audio plays. */
    UPROPERTY(EditAnywhere, Category = "Narrative", meta = (MultiLine = true))
    FText SubtitleText;

    /** Whether this trigger should only fire once. */
    UPROPERTY(EditAnywhere, Category = "Narrative")
    bool bOneShot = true;

    /** Delay before playing audio after overlap (seconds). */
    UPROPERTY(EditAnywhere, Category = "Narrative")
    float TriggerDelay = 0.0f;

private:
    bool bHasTriggered = false;

    UFUNCTION()
    void OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);
};
```

### Pattern 2: DocumentReaderWidget (Full-Screen Readable UI)

**What:** A UUserWidget displayed when the player interacts with a ReadableActor. Shows the document title and multi-line content text on a stylized background. Blocks gameplay input while open. Player presses a key (E or Esc) to close and return to gameplay.

**When to use:** Every readable note, letter, and document in the demo.

**Key design:**
```cpp
// Source: Project UMG patterns (InventoryWidget, PauseMenuWidget)
UCLASS()
class PROJECTWALKINGSIM_API UDocumentReaderWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Populate and display the document. */
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowDocument(const FText& Title, const FText& Content);

    /** Close the reader and restore gameplay input. */
    UFUNCTION(BlueprintCallable, Category = "UI")
    void CloseDocument();

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDocumentClosed);

    UPROPERTY(BlueprintAssignable, Category = "UI")
    FOnDocumentClosed OnDocumentClosed;

protected:
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry,
        const FKeyEvent& InKeyEvent) override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TitleText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ContentText;
};
```

**Integration with ReadableActor:** ReadableActor::OnInteract creates or shows the DocumentReaderWidget through SereneHUD or the PlayerController (following the same pattern as inventory/pause menu). The widget is created once and reused, populated with new content each time.

### Pattern 3: InspectableActor (Story Object Examination)

**What:** An interactable actor for photographs, child's toys, bloodstains, etc. On interaction, shows a full-screen overlay (UMG widget) with an image/description. Simpler than a full 3D SceneCapture approach -- uses a static UTexture2D rendered in a UMG Image widget alongside descriptive text.

**When to use:** Photographs, the discovery object at the demo ending, any visual story object.

**Example:**
```cpp
UCLASS()
class PROJECTWALKINGSIM_API AInspectableActor : public AInteractableBase
{
    GENERATED_BODY()

public:
    AInspectableActor();

protected:
    virtual void OnInteract_Implementation(AActor* Interactor) override;

    /** Full-screen image shown during inspection. */
    UPROPERTY(EditAnywhere, Category = "Inspectable")
    TSoftObjectPtr<UTexture2D> InspectionImage;

    /** Description text shown alongside the image. */
    UPROPERTY(EditAnywhere, Category = "Inspectable", meta = (MultiLine = true))
    FText InspectionText;

    /** Optional monologue triggered on first inspection. */
    UPROPERTY(EditAnywhere, Category = "Inspectable")
    TObjectPtr<USoundBase> InspectionMonologue;

    /** Whether inspection has been viewed (for one-time monologue). */
    bool bHasBeenInspected = false;
};
```

### Pattern 4: DemoEndingManager (Fade-to-Black Title Card)

**What:** A UActorComponent placed on a trigger actor or the GameMode that orchestrates the demo ending: (1) disable player input, (2) play final monologue, (3) fade to black via StartCameraFade, (4) show "The Juniper Tree" title card widget, (5) hold indefinitely or return to main menu.

**When to use:** Attached to the final trigger actor in the demo. Activated when the player discovers the ending cliffhanger object.

**Example:**
```cpp
// Source: APlayerCameraManager::StartCameraFade (UE5 built-in)
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTWALKINGSIM_API UDemoEndingManager : public UActorComponent
{
    GENERATED_BODY()

public:
    /** Begin the ending sequence. Disables input, fades, shows title. */
    UFUNCTION(BlueprintCallable, Category = "Demo")
    void TriggerEnding();

protected:
    /** Duration of the fade to black. */
    UPROPERTY(EditAnywhere, Category = "Demo|Ending")
    float FadeDuration = 3.0f;

    /** Delay after fade before title card appears. */
    UPROPERTY(EditAnywhere, Category = "Demo|Ending")
    float TitleCardDelay = 2.0f;

    /** Title card widget class. */
    UPROPERTY(EditDefaultsOnly, Category = "Demo|Ending")
    TSubclassOf<UUserWidget> TitleCardWidgetClass;

    /** Optional final monologue before fade. */
    UPROPERTY(EditAnywhere, Category = "Demo|Ending")
    TObjectPtr<USoundBase> FinalMonologue;
};
```

**Fade implementation:**
```cpp
void UDemoEndingManager::TriggerEnding()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    // Disable all gameplay input
    PC->DisableInput(PC);

    // Play final monologue if set
    if (FinalMonologue)
    {
        UGameplayStatics::PlaySound2D(this, FinalMonologue);
    }

    // Fade to black
    APlayerCameraManager* CamMgr = PC->PlayerCameraManager;
    if (CamMgr)
    {
        CamMgr->StartCameraFade(0.f, 1.f, FadeDuration,
            FLinearColor::Black, /*bShouldFadeAudio=*/ true,
            /*bHoldWhenFinished=*/ true);
    }

    // Show title card after fade + delay
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle,
        [this, PC]()
        {
            if (TitleCardWidgetClass)
            {
                UUserWidget* TitleCard = CreateWidget<UUserWidget>(PC, TitleCardWidgetClass);
                TitleCard->AddToViewport(200);
            }
        },
        FadeDuration + TitleCardDelay, false);
}
```

### Pattern 5: Wendigo Activation via NarrativeTrigger

**What:** The Wendigo does not exist or is dormant at game start. At the midpoint (~15 min), a NarrativeTriggerActor in the woods section fires its OnNarrativeTriggered delegate, which is wired (in the level Blueprint or via a spawning actor) to call AWendigoSpawnPoint::SpawnWendigo().

**When to use:** Activating the Wendigo at the demo midpoint.

**Implementation options:**
1. **Level-placed Wendigo, initially dormant:** Place BP_WendigoCharacter with a custom "dormant" flag that suppresses AI until a narrative trigger sets it active. Simpler but less clean.
2. **Spawn on trigger (recommended):** NarrativeTriggerActor delegate wired to SpawnPoint::SpawnWendigo(). Cleaner separation. Player never encounters Wendigo before trigger.

### Anti-Patterns to Avoid
- **Level Blueprint logic for gameplay:** Keep narrative logic in C++ actors, not Level Blueprint. Level Blueprint should only be used for one-off per-level visual scripting that cannot be generalized.
- **Tick-based triggers:** Use overlap delegates, not per-tick distance checks, for narrative trigger volumes.
- **Hard-coded content in C++:** All text content (note text, monologue subtitles) should be UPROPERTY EditAnywhere so level designers populate them per-instance in the editor, not compiled into C++.
- **Overloading the HUD widget:** New Phase 8 widgets (DocumentReader, Inspection, TitleCard) should be standalone viewport widgets managed by the PlayerController or their owning actors, not crammed into SereneHUDWidget as BindWidget children. This follows the PauseMenuWidget and GameOverWidget precedent.

## Don't Hand-Roll

Problems that look simple but have existing solutions:

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Screen fade to black | Custom material/post-process fade | APlayerCameraManager::StartCameraFade | Built-in, handles audio fade, one line of code |
| Playing 2D audio | Custom audio manager for monologues | UGameplayStatics::PlaySound2D | Already used for footsteps (bIsUISound pattern); no spatialization needed |
| Trigger volume | Custom collision detection | UBoxComponent with OnComponentBeginOverlap | Engine-standard pattern; handles all edge cases |
| Locked door progression | Custom gate system | ADoorActor with RequiredItemId + bIsLocked | Already built and tested in Phase 2 |
| Key items for progression | New item system | UItemDataAsset + APickupActor + InventoryComponent | Already built and tested in Phase 2 |
| Save at specific points | New checkpoint system | ATapeRecorderActor | Already built and tested in Phase 7 |
| Wendigo spawning | Custom spawn logic | AWendigoSpawnPoint::SpawnWendigo() | Already built and tested in Phase 5 |
| Ambient soundscape | Custom audio loop manager | AAmbientAudioManager | Already built in Phase 6 |
| Music tension | Custom music system | UMusicTensionSystem on WendigoCharacter | Already built in Phase 6 |
| AI patrol routes | Custom patrol logic | APatrolRouteActor + State Tree tasks | Already built in Phase 4 |

**Key insight:** Phase 8 should build almost nothing from scratch. The project has 7 phases of systems. This phase creates a thin narrative layer (4 new classes) and then assembles everything into the demo level. Resist the urge to rebuild or refactor existing systems.

## Common Pitfalls

### Pitfall 1: NavMesh Not Covering Outdoor Areas
**What goes wrong:** AI pathfinding fails in the woods/yard because NavMesh bounds volume does not extend to outdoor areas or terrain is too steep.
**Why it happens:** Default NavMeshBoundsVolume only covers the initial test area. Outdoor terrain with slopes > AgentMaxSlope (44 deg default) creates holes.
**How to avoid:** Place one large NavMeshBoundsVolume covering the entire playable area (cabin + yard + woods + barn). Use NavModifierVolumes to mark impassable zones (dense forest edges, cliffs). Validate with `show Navigation` in PIE.
**Warning signs:** Wendigo stops at doorways, gets stuck outdoors, or MoveToLocation returns Failed.

### Pitfall 2: Lumen Dark Interiors / Light Leaking
**What goes wrong:** Cabin interior is too dark to see anything, or outdoor light leaks through walls.
**Why it happens:** Lumen traces can penetrate thin walls (< 10cm). Interior scenes need at least one light source for Lumen to have something to bounce. Emissive materials alone may not be enough.
**How to avoid:** Use wall thickness >= 10cm (ideally 20cm) for all structures. Place at least one dim point/spot light in every enclosed room. Use LumenSceneDetail CVars if needed. Enable MegaLights for better multi-light performance.
**Warning signs:** Pitch-black rooms despite nearby light sources. Bright patches on interior walls facing the sun.

### Pitfall 3: Audio Overlap / Monologue Interruption
**What goes wrong:** Multiple narrative triggers fire simultaneously or player re-enters a trigger zone, causing overlapping monologues.
**Why it happens:** Overlap events fire every time the player crosses the trigger boundary. Walking back and forth through a trigger zone fires it repeatedly.
**How to avoid:** NarrativeTriggerActor uses bOneShot=true and bHasTriggered guard. For the opening monologue, disable the trigger box collision after first fire. Consider a simple queue if multiple monologues can trigger in quick succession (unlikely given sparse placement, but worth noting).
**Warning signs:** Detective talking over himself. Audio clipping or cutting off mid-sentence.

### Pitfall 4: Input Mode Confusion During Document Reading
**What goes wrong:** Player opens a note, then presses Tab (inventory) or Esc (pause) and gets stuck in a broken input state.
**Why it happens:** Multiple UI widgets competing for input mode (Game Only vs UI Only vs Game and UI).
**How to avoid:** Follow the existing pattern: DocumentReaderWidget sets FInputModeUIOnly while open. On close, restore FInputModeGameOnly. Add guards in the controller to prevent inventory/pause while document is open (same pattern as pause-blocked-during-hiding from Phase 7).
**Warning signs:** Mouse cursor visible during gameplay. Movement doesn't work after closing a note. Esc not working.

### Pitfall 5: Performance Drop in Dense Scenes
**What goes wrong:** Frame rate drops below 60 FPS in the cabin with multiple lights, the Wendigo, particle effects, and complex meshes.
**Why it happens:** Lumen + VSM + MegaLights have baseline GPU cost. Adding many shadow-casting lights in a small cabin compounds the cost. Flashlight spotlight adds 2-5ms alone.
**How to avoid:** Budget lights carefully. Use MegaLights (already enabled) for efficient multi-light rendering. Profile early with `stat gpu`, `stat unit`, `stat SceneRendering`. Keep instance count under 100K for HWRT. Use `r.RayTracing.Culling=3` with appropriate radius.
**Warning signs:** GPU time > 16.6ms in stat unit. Lumen passes dominating in stat gpu.

### Pitfall 6: Save/Load Breaking with New Actors
**What goes wrong:** After adding new actor types (InspectableActor, NarrativeTriggerActor), saving and loading corrupts state or crashes.
**Why it happens:** New actors that have runtime state (bHasTriggered, bHasBeenInspected) need ISaveable implementation or the state resets on load.
**How to avoid:** For demo scope, bHasTriggered/bHasBeenInspected can reset on load (player re-experiences triggers after loading a save, which is acceptable). If persistence is needed, implement ISaveable on NarrativeTriggerActor. Document which actors need save support vs which reset.
**Warning signs:** Notes re-triggering monologues after load. Previously-seen objects acting as unseen.

## Code Examples

### Opening Monologue Trigger (NarrativeTriggerActor)
```cpp
// Source: Standard UE5 overlap pattern
ANarrativeTriggerActor::ANarrativeTriggerActor()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    SetRootComponent(TriggerBox);
    TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
    TriggerBox->SetBoxExtent(FVector(200.f, 200.f, 100.f));

    OnActorBeginOverlap.AddDynamic(this, &ANarrativeTriggerActor::OnOverlapBegin);
}

void ANarrativeTriggerActor::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
    if (bOneShot && bHasTriggered) return;

    // Only trigger for the player
    APawn* PlayerPawn = Cast<APawn>(OtherActor);
    if (!PlayerPawn || !PlayerPawn->IsPlayerControlled()) return;

    bHasTriggered = true;

    auto PlayNarrative = [this]()
    {
        if (MonologueSound)
        {
            UGameplayStatics::PlaySound2D(this, MonologueSound);
        }
        OnNarrativeTriggered.Broadcast();
    };

    if (TriggerDelay > 0.f)
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle,
            FTimerDelegate::CreateLambda(PlayNarrative), TriggerDelay, false);
    }
    else
    {
        PlayNarrative();
    }
}
```

### ReadableActor Opening DocumentReaderWidget
```cpp
// Source: Follows PauseMenuWidget/GameOverWidget creation pattern
void AReadableActor::OnInteract_Implementation(AActor* Interactor)
{
    APlayerController* PC = Cast<APlayerController>(
        Cast<APawn>(Interactor)->GetController());
    if (!PC) return;

    // Create or get DocumentReaderWidget (managed by controller or HUD)
    // Following the same pattern as PauseMenuWidget: controller creates and manages
    UDocumentReaderWidget* ReaderWidget = /* get/create widget */;
    if (ReaderWidget)
    {
        ReaderWidget->ShowDocument(ReadableTitle, ReadableContent);
        ReaderWidget->AddToViewport(50);

        // Switch to UI input
        PC->SetShowMouseCursor(false); // Keyboard-driven reading
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(ReaderWidget->TakeWidget());
        PC->SetInputMode(InputMode);
    }
}
```

### Fade to Black Ending
```cpp
// Source: APlayerCameraManager::StartCameraFade (UE5 built-in API)
void UDemoEndingManager::TriggerEnding()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    // 1. Disable gameplay input
    PC->DisableInput(PC);

    // 2. Optional final monologue
    if (FinalMonologue)
    {
        UGameplayStatics::PlaySound2D(this, FinalMonologue);
    }

    // 3. Fade to black with audio fade
    if (APlayerCameraManager* CamMgr = PC->PlayerCameraManager)
    {
        CamMgr->StartCameraFade(
            0.f,                    // FromAlpha (transparent)
            1.f,                    // ToAlpha (fully black)
            FadeDuration,           // Duration
            FLinearColor::Black,    // Color
            true,                   // bShouldFadeAudio
            true                    // bHoldWhenFinished
        );
    }

    // 4. Show title card after fade completes + delay
    FTimerHandle TitleTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TitleTimerHandle,
        [this, PC]()
        {
            if (TitleCardWidgetClass && PC)
            {
                UUserWidget* TitleCard = CreateWidget<UUserWidget>(PC, TitleCardWidgetClass);
                if (TitleCard)
                {
                    TitleCard->AddToViewport(200);
                }
            }
        },
        FadeDuration + TitleCardDelay, false);
}
```

### Demo Level Actor Placement Checklist
```
DemoMap Layout (conceptual):
  Cabin Exterior:
    - PlayerStart (near cabin approach)
    - NarrativeTrigger_Opening (at player start, TriggerDelay=1.0)
    - AmbientAudioManager (cabin exterior ambience)
    - Directional Light (moonlight) + SkyLight

  Cabin Interior (Main Room):
    - DoorActor_FrontDoor (unlocked, first entry)
    - ReadableActor_CaseFile (note #1 on desk)
    - DrawerActor_DeskDrawer (contains key)
    - PickupActor_CellarKey (in drawer)
    - HidingSpotActor_Closet (main room closet)
    - TapeRecorderActor_1 (first save opportunity)
    - Multiple point/spot lights (dim, atmospheric)

  Cellar:
    - DoorActor_CellarDoor (locked, RequiredItemId=CellarKey)
    - ReadableActor_Journal (note #2)
    - InspectableActor_OldPhoto (photograph)
    - HidingSpotActor_UnderStairs

  Woods Section:
    - NarrativeTrigger_WoodsEntry (detective monologue about feeling watched)
    - NarrativeTrigger_WendigoSpawn (OnNarrativeTriggered -> SpawnWendigo)
    - WendigoSpawnPoint_Woods (with outdoor patrol route)
    - PatrolRouteActor_Woods (waypoints through trees)
    - AmbientAudioManager_Woods (forest ambience, owl hoots, wind)

  Barn:
    - DoorActor_BarnDoor (key found in woods or cabin)
    - ReadableActor_FinalNote (note #3, disturbing content)
    - InspectableActor_ChildToy (story object)
    - HidingSpotActor_BarnLocker
    - TapeRecorderActor_2 (second save before climax)

  Climax Area (Cabin Basement or Secret Room):
    - InspectableActor_Discovery (the ending cliffhanger object)
    - NarrativeTrigger_Ending (triggers DemoEndingManager)
    - DemoEndingManager component on trigger actor

NavMesh:
    - NavMeshBoundsVolume covering entire playable area
    - NavModifierVolume on impassable forest edges
    - Verify indoor-outdoor transitions work (doorways)
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Baked/Static lighting | Lumen dynamic GI | UE5.0 (2022) | All lighting is dynamic; no lightmap baking needed |
| Traditional shadow maps per light | MegaLights GPU-driven shadows | UE5.5 (2024) | 50% better performance with many lights; beta in 5.7 |
| Software ray tracing (SWRT) | Hardware ray tracing (HWRT) | UE5.4-5.6 | SWRT being deprecated; HWRT preferred for Lumen |
| Behavior Trees | State Trees | UE5.1+ | Already using State Trees in this project |
| Blueprint trigger volumes | C++ trigger actors | Always available | C++ is more maintainable at project scale |

**Deprecated/outdated:**
- SWRT detail traces: Being deprecated in favor of HWRT. Project already uses HWRT via Lumen.
- CompressImageArray: Deprecated in UE5.7 in favor of FImageUtils::CompressImage with FImageView (already handled in Phase 7).

## Performance Budget

Target: 60 FPS = 16.6ms per frame

| System | Estimated Cost | Notes |
|--------|---------------|-------|
| Lumen GI (HWRT, High) | 4-6ms | Indoor cabin with bounced light |
| MegaLights shadows | 1-2ms | Mostly constant regardless of light count |
| Virtual Shadow Maps | 1-2ms | Per-light shadow detail |
| Flashlight spotlight | 2-3ms | Single movable shadow-casting spot |
| Wendigo AI (State Tree + Perception) | 0.5-1ms | Single AI actor, reasonable |
| NavMesh pathfinding | <0.5ms | Single agent, moderate area |
| Audio (ambient + music + spatial) | <0.5ms | CPU-based, minimal impact |
| Player systems (stamina, bob, footstep) | <0.5ms | All tick-based, lightweight |
| UMG rendering | <0.5ms | Few active widgets at once |
| **Total estimate** | **10-14ms** | **Within 16.6ms budget** |

**Key CVars for optimization:**
```
r.MegaLights 1                          # Already enabled
r.RayTracing.Culling 3                  # Aggressive HWRT culling
r.RayTracing.Culling.Radius 15000       # 150m culling radius
r.Lumen.DiffuseIndirect.AsyncCompute 1  # Overlap Lumen with other passes
r.Lumen.Reflections.AsyncCompute 1      # Async reflections
sg.GlobalIlluminationQuality 2          # High (60fps target)
sg.ReflectionQuality 2                  # High (60fps target)
```

## Open Questions

Things that could not be fully resolved:

1. **Subtitle System**
   - What we know: PlaySound2D plays audio with no built-in subtitle support. UE5 has a subtitle framework (FSubtitleManager) but it integrates with dialogue waves, not raw USoundBase.
   - What's unclear: Whether to build a simple subtitle widget that reads from NarrativeTriggerActor's SubtitleText + duration, or use the engine subtitle system.
   - Recommendation: Build a simple SubtitleWidget (UTextBlock with fade in/out) managed by NarrativeTriggerActor. Simpler than wiring into engine subtitles for a demo. LOW priority -- can be deferred if scope is tight.

2. **Wendigo Dormancy Before Midpoint**
   - What we know: WendigoSpawnPoint::SpawnWendigo() creates the Wendigo at runtime. Alternatively, a pre-placed Wendigo could be set to a "dormant" state.
   - What's unclear: Whether spawn-on-trigger causes a visible frame hitch from character creation + AI initialization.
   - Recommendation: Use spawn-on-trigger. The cost of spawning one character should be negligible. If a hitch is observed, pre-place the Wendigo off-map and teleport it in.

3. **Exact Content Scope**
   - What we know: CONTEXT.md specifies 5-8 storytelling objects, 25-35 minutes playtime.
   - What's unclear: Whether 5-8 objects provides enough content for 25-35 minutes or if additional environmental detail (non-interactive decorations, more locked doors, more rooms) is needed.
   - Recommendation: Start with 6 storytelling objects (2 readables, 2 inspectables, 2 key items) plus 3-4 locked doors for gating. Playtest timing in PIE to validate pacing.

4. **Opening Sequence Timing**
   - What we know: Player starts outside cabin, detective monologue plays.
   - What's unclear: Whether player should have full movement control during opening monologue or be briefly locked in place.
   - Recommendation: Allow full movement. Horror games benefit from player agency. The monologue plays as ambient voiceover while the player approaches the cabin. No movement restriction.

## Sources

### Primary (HIGH confidence)
- Existing project codebase (Phases 1-7) -- all existing actors, interfaces, and patterns directly verified via source code
- [UE5.7 Trigger Volume Actors](https://dev.epicgames.com/documentation/en-us/unreal-engine/trigger-volume-actors-in-unreal-engine) -- overlap event patterns
- [UE5.7 Cinematic Color Fade Track](https://dev.epicgames.com/documentation/en-us/unreal-engine/cinematic-color-fade-track-in-unreal-engine) -- Sequencer fade approach
- [UE5.7 Lumen Performance Guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-performance-guide-for-unreal-engine) -- Lumen CVars, scalability, 60fps budget
- [UE5.7 MegaLights](https://dev.epicgames.com/documentation/en-us/unreal-engine/megalights-in-unreal-engine) -- MegaLights performance characteristics
- [UE5.7 PlaySound2D](https://dev.epicgames.com/documentation/en-us/unreal-engine/BlueprintAPI/Audio/PlaySound2D) -- 2D audio playback
- [UE5.7 UAudioComponent](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/UAudioComponent) -- Audio component API
- [UE5.7 ULevelSequencePlayer](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/LevelSequence/ULevelSequencePlayer) -- Sequencer C++ API (documented but not recommended for this use case)
- [UE5.7 Performance Profiling](https://dev.epicgames.com/documentation/en-us/unreal-engine/introduction-to-performance-profiling-and-configuration-in-unreal-engine) -- stat commands, profiling workflow

### Secondary (MEDIUM confidence)
- [UE5.7 Performance Highlights - Tom Looman](https://tomlooman.com/unreal-engine-5-7-performance-highlights/) -- MegaLights beta status, performance improvements
- [StartCameraFade community usage](https://forums.unrealengine.com/t/how-to-use-aplayercameramanager-startcamerafade/393107) -- Parameter usage patterns confirmed with API docs
- [Unreal C++ Trigger Box patterns](https://unrealcpp.com/trigger-box/) -- Overlap registration patterns

### Tertiary (LOW confidence)
- Subtitle system recommendation -- based on training data knowledge of FSubtitleManager; not verified against UE5.7 docs
- Performance budget estimates -- based on general Lumen profiling data, not measured on this specific project's content

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH -- all based on existing project code and verified UE5.7 APIs
- Architecture: HIGH -- patterns follow established project conventions from Phases 1-7
- New C++ classes: HIGH -- straightforward actors/widgets using proven patterns
- Performance: MEDIUM -- budget estimates are informed but not measured; need PIE validation
- Pitfalls: HIGH -- based on real UE5 Lumen and NavMesh issues documented in official guides

**Research date:** 2026-02-13
**Valid until:** 2026-03-13 (stable -- no fast-moving dependencies)
