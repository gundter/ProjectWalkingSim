// Copyright Null Lantern.

#include "AI/WendigoAIController.h"
#include "AI/WendigoCharacter.h"
#include "AI/SuspicionComponent.h"
#include "Visibility/VisibilityScoreComponent.h"
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
			SuspicionComp->ProcessSightStimulus(VisibilityScore, DeltaTime);
			bSeeingPlayer = true;
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
