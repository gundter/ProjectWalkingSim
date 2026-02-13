// Copyright Null Lantern.

#include "Narrative/NarrativeTriggerActor.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Core/SereneLogChannels.h"

ANarrativeTriggerActor::ANarrativeTriggerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetBoxExtent(FVector(200.0f, 200.0f, 100.0f));
	TriggerBox->SetGenerateOverlapEvents(true);
	SetRootComponent(TriggerBox);

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ANarrativeTriggerActor::OnTriggerOverlap);
}

void ANarrativeTriggerActor::OnTriggerOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// One-shot guard
	if (bOneShot && bHasTriggered)
	{
		return;
	}

	// Only trigger for player-controlled pawns
	const APawn* OverlappingPawn = Cast<APawn>(OtherActor);
	if (!OverlappingPawn || !OverlappingPawn->IsPlayerControlled())
	{
		return;
	}

	bHasTriggered = true;

	UE_LOG(LogSerene, Log, TEXT("NarrativeTrigger [%s] activated by player"), *GetName());

	if (TriggerDelay > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			TriggerTimerHandle,
			this,
			&ANarrativeTriggerActor::ExecuteTrigger,
			TriggerDelay,
			false);
	}
	else
	{
		ExecuteTrigger();
	}
}

void ANarrativeTriggerActor::ExecuteTrigger()
{
	if (MonologueSound)
	{
		UGameplayStatics::PlaySound2D(this, MonologueSound);
		UE_LOG(LogSerene, Log, TEXT("NarrativeTrigger [%s]: Playing monologue sound"), *GetName());
	}

	OnNarrativeTriggered.Broadcast();
	UE_LOG(LogSerene, Log, TEXT("NarrativeTrigger [%s]: OnNarrativeTriggered broadcast"), *GetName());
}
