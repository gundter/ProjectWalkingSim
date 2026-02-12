// Copyright Null Lantern.

#include "AI/WendigoAIController.h"
#include "AI/WendigoCharacter.h"
#include "AI/SuspicionComponent.h"
#include "Visibility/VisibilityScoreComponent.h"
#include "Hiding/HidingComponent.h"
#include "Hiding/HidingSpotActor.h"
#include "Hiding/HidingTypes.h"
#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Core/SereneLogChannels.h"

AWendigoAIController::AWendigoAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	// ---- State Tree AI Component ----
	StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>(
		TEXT("StateTreeAIComponent"));

	// ---- AI Perception Component ----
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(
		TEXT("AIPerceptionComponent"));
	SetPerceptionComponent(*AIPerceptionComponent);

	// ---- Sight Configuration ----
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(
		TEXT("SightConfig"));
	SightConfig->SightRadius = 2500.0f;                    // ~25m detection range
	SightConfig->LoseSightRadius = 3000.0f;                // ~30m, prevents flicker at edge
	SightConfig->PeripheralVisionAngleDegrees = 45.0f;     // 90 degree total FOV
	SightConfig->SetMaxAge(5.0f);                          // Forget after 5s without seeing
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	AIPerceptionComponent->ConfigureSense(*SightConfig);

	// ---- Hearing Configuration ----
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(
		TEXT("HearingConfig"));
	HearingConfig->HearingRange = 2000.0f;                 // ~20m hearing range
	HearingConfig->SetMaxAge(3.0f);                        // Forget after 3s
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	AIPerceptionComponent->ConfigureSense(*HearingConfig);

	// ---- Dominant Sense ----
	AIPerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass());
}

void AWendigoAIController::BeginPlay()
{
	Super::BeginPlay();

	// Bind perception delegate in BeginPlay -- delegates do not work from constructor.
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
		UE_LOG(LogSerene, Log, TEXT("Wendigo State Tree started"));
	}
}

void AWendigoAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Get the possessed Wendigo and its SuspicionComponent
	AWendigoCharacter* WendigoChar = Cast<AWendigoCharacter>(GetPawn());
	if (!WendigoChar)
	{
		return;
	}

	USuspicionComponent* SuspicionComp = WendigoChar->GetSuspicionComponent();
	if (!SuspicionComp || !AIPerceptionComponent)
	{
		return;
	}

	// Check if any player is currently perceived by sight
	TArray<AActor*> PerceivedActors;
	AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

	bool bSeeingPlayer = false;

	for (AActor* Actor : PerceivedActors)
	{
		if (!Actor)
		{
			continue;
		}

		// Get the player's visibility score and feed into suspicion accumulation
		UVisibilityScoreComponent* VisComp = Actor->FindComponentByClass<UVisibilityScoreComponent>();
		if (VisComp)
		{
			const float VisibilityScore = VisComp->GetVisibilityScore();

			// Record player location as stimulus for investigation
			SuspicionComp->SetStimulusLocation(Actor->GetActorLocation());

			// Keep last-known player location current on the character every tick while visible
			WendigoChar->SetLastKnownPlayerLocation(Actor->GetActorLocation());

			SuspicionComp->ProcessSightStimulus(VisibilityScore, DeltaTime);
			bSeeingPlayer = true;

			// Debug: log visibility score periodically (every ~1s)
			SightDebugTimer += DeltaTime;
			if (SightDebugTimer >= 1.0f)
			{
				UE_LOG(LogSerene, Log, TEXT("Wendigo sight: Visibility=%.2f, Suspicion=%.3f, Alert=%d"),
					VisibilityScore, SuspicionComp->GetCurrentSuspicion(),
					static_cast<uint8>(SuspicionComp->GetAlertLevel()));
				SightDebugTimer = 0.0f;
			}

			break; // Only one player in this game
		}
	}

	// If no player is currently in sight, decay suspicion
	if (!bSeeingPlayer)
	{
		SuspicionComp->DecaySuspicion(DeltaTime);
	}
}

void AWendigoAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor)
	{
		return;
	}

	// Route to the appropriate handler based on sense type
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		ProcessSightPerception(Actor, Stimulus.WasSuccessfullySensed());
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		ProcessHearingPerception(Actor, Stimulus.StimulusLocation);
	}
}

void AWendigoAIController::ProcessSightPerception(AActor* Player, bool bCurrentlySensed)
{
	if (bCurrentlySensed)
	{
		const UVisibilityScoreComponent* VisComp = Player->FindComponentByClass<UVisibilityScoreComponent>();
		const float Visibility = VisComp ? VisComp->GetVisibilityScore() : 1.0f;
		UE_LOG(LogSerene, Log, TEXT("Wendigo sees %s (Visibility: %.2f)"),
			*Player->GetName(), Visibility);

		// Bind to player's HidingComponent delegate on first sight detection.
		// No-op on subsequent calls (bPlayerDelegateBound guard).
		BindToPlayerDelegates(Player);
	}
	else
	{
		UE_LOG(LogSerene, Log, TEXT("Wendigo lost sight of %s"), *Player->GetName());
	}
}

void AWendigoAIController::ProcessHearingPerception(AActor* NoiseInstigator, FVector StimulusLocation)
{
	AWendigoCharacter* WendigoChar = Cast<AWendigoCharacter>(GetPawn());
	if (!WendigoChar)
	{
		return;
	}

	USuspicionComponent* SuspicionComp = WendigoChar->GetSuspicionComponent();
	if (!SuspicionComp)
	{
		return;
	}

	SuspicionComp->ProcessHearingStimulus(StimulusLocation);
	UE_LOG(LogSerene, Log, TEXT("Wendigo heard noise at %s"), *StimulusLocation.ToString());
}

void AWendigoAIController::BindToPlayerDelegates(AActor* PlayerActor)
{
	// Guard: only bind once
	if (bPlayerDelegateBound && TrackedPlayer.IsValid())
	{
		return;
	}

	if (!PlayerActor)
	{
		return;
	}

	UHidingComponent* HidingComp = PlayerActor->FindComponentByClass<UHidingComponent>();
	if (HidingComp)
	{
		HidingComp->OnHidingStateChanged.AddDynamic(
			this, &AWendigoAIController::OnPlayerHidingStateChanged);

		TrackedPlayer = PlayerActor;
		bPlayerDelegateBound = true;

		UE_LOG(LogSerene, Log, TEXT("Wendigo AI: Bound to player hiding delegate"));
	}
}

void AWendigoAIController::OnPlayerHidingStateChanged(EHidingState NewState)
{
	// Only care about the moment the player starts entering a hiding spot
	if (NewState != EHidingState::Entering)
	{
		return;
	}

	// Check if we currently see the player
	if (!AIPerceptionComponent || !TrackedPlayer.IsValid())
	{
		return;
	}

	TArray<AActor*> PerceivedActors;
	AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

	const bool bCanSeePlayer = PerceivedActors.Contains(TrackedPlayer.Get());
	if (!bCanSeePlayer)
	{
		// Player hid without being seen -- hiding is safe
		return;
	}

	// Wendigo saw the player enter hiding -- record the spot
	AActor* PlayerActor = TrackedPlayer.Get();
	UHidingComponent* HidingComp = PlayerActor->FindComponentByClass<UHidingComponent>();
	if (!HidingComp)
	{
		return;
	}

	AActor* HidingSpot = HidingComp->GetCurrentHidingSpot();
	if (!HidingSpot)
	{
		return;
	}

	AWendigoCharacter* WendigoChar = Cast<AWendigoCharacter>(GetPawn());
	if (WendigoChar)
	{
		WendigoChar->SetWitnessedHidingSpot(HidingSpot);
		UE_LOG(LogSerene, Log, TEXT("Wendigo witnessed player entering hiding spot %s"),
			*HidingSpot->GetName());
	}
}
