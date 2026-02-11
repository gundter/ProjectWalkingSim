// Copyright Null Lantern.

#include "AI/WendigoAIController.h"
#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Core/SereneLogChannels.h"

AWendigoAIController::AWendigoAIController()
{
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

void AWendigoAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor)
	{
		return;
	}

	// Stub: log perception events. Plan 05 will expand this to feed SuspicionComponent.
	const FString SenseName = (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
		? TEXT("Sight")
		: (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
			? TEXT("Hearing")
			: TEXT("Unknown");
	const FString ActiveStr = Stimulus.WasSuccessfullySensed() ? TEXT("Detected") : TEXT("Lost");

	UE_LOG(LogSerene, Log, TEXT("Wendigo perceived: %s (Sense: %s, Status: %s)"),
		*Actor->GetName(), *SenseName, *ActiveStr);
}
