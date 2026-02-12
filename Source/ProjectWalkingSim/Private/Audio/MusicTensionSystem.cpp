// Copyright Null Lantern.

#include "Audio/MusicTensionSystem.h"

#include "AI/WendigoCharacter.h"
#include "AI/SuspicionComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogMusicTension, Log, All);

UMusicTensionSystem::UMusicTensionSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMusicTensionSystem::BeginPlay()
{
	Super::BeginPlay();

	CreateMusicLayers();
	BindToWendigo();

	// Start with the calm layer playing at full volume.
	if (UAudioComponent* CalmComp = GetLayerComponent(EMusicIntensity::Calm))
	{
		if (CalmComp->Sound)
		{
			CalmComp->Play();
		}
	}
}

void UMusicTensionSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Stop all music layers and stinger.
	for (int32 i = 0; i < 3; ++i)
	{
		if (MusicLayers[i])
		{
			MusicLayers[i]->Stop();
		}
	}

	if (StingerComp)
	{
		StingerComp->Stop();
	}

	Super::EndPlay(EndPlayReason);
}

// --- Music Layer Creation ---

void UMusicTensionSystem::CreateMusicLayers()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Music layer asset references indexed to match EMusicIntensity values.
	const TSoftObjectPtr<USoundBase>* LayerAssets[3] = { &CalmMusic, &TenseMusic, &IntenseMusic };

	for (int32 i = 0; i < 3; ++i)
	{
		UAudioComponent* Comp = NewObject<UAudioComponent>(Owner);
		Comp->SetupAttachment(Owner->GetRootComponent());
		Comp->bAutoActivate = false;
		Comp->bAutoDestroy = false;
		Comp->bIsUISound = true;  // Non-spatialized 2D music.

		// Load and assign sound asset if valid.
		if (!LayerAssets[i]->IsNull())
		{
			if (USoundBase* Sound = LayerAssets[i]->LoadSynchronous())
			{
				Comp->SetSound(Sound);
			}
		}

		Comp->RegisterComponent();
		MusicLayers[i] = Comp;
	}

	// Create stinger component (non-spatial, 2D).
	StingerComp = NewObject<UAudioComponent>(Owner);
	StingerComp->SetupAttachment(Owner->GetRootComponent());
	StingerComp->bAutoActivate = false;
	StingerComp->bAutoDestroy = false;
	StingerComp->bIsUISound = true;
	StingerComp->RegisterComponent();
}

// --- Wendigo Binding ---

void UMusicTensionSystem::BindToWendigo()
{
	// Owner-first: plan 05 places MusicTensionSystem as a default subobject on WendigoCharacter.
	AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(GetOwner());

	// Fallback: search the world if owner is not a Wendigo.
	if (!Wendigo)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWendigoCharacter::StaticClass(), FoundActors);
		if (FoundActors.Num() > 0)
		{
			Wendigo = Cast<AWendigoCharacter>(FoundActors[0]);
		}
	}

	if (Wendigo)
	{
		BindToWendigoCharacter(Wendigo);
	}
	else
	{
		UE_LOG(LogMusicTension, Warning, TEXT("MusicTensionSystem: No AWendigoCharacter found. Wendigo may spawn later -- call BindToWendigoCharacter() manually."));
	}
}

void UMusicTensionSystem::BindToWendigoCharacter(AWendigoCharacter* Wendigo)
{
	if (!Wendigo)
	{
		return;
	}

	// Bind to alert level changes for music intensity crossfade.
	if (USuspicionComponent* Suspicion = Wendigo->GetSuspicionComponent())
	{
		Suspicion->OnAlertLevelChanged.AddDynamic(this, &UMusicTensionSystem::OnAlertLevelChanged);
	}

	// Bind to behavior state changes for stinger triggers.
	Wendigo->OnBehaviorStateChanged.AddDynamic(this, &UMusicTensionSystem::OnBehaviorStateChangedForStingers);

	// Sync initial state.
	PreviousBehaviorState = Wendigo->BehaviorState;
}

// --- Delegate Handlers ---

void UMusicTensionSystem::OnAlertLevelChanged(EAlertLevel NewLevel)
{
	SetMusicIntensity(AlertLevelToIntensity(NewLevel));
}

void UMusicTensionSystem::OnBehaviorStateChangedForStingers(EWendigoBehaviorState NewState)
{
	// Chasing entry from any non-Chasing state: monster spotted the player.
	if (NewState == EWendigoBehaviorState::Chasing && PreviousBehaviorState != EWendigoBehaviorState::Chasing)
	{
		if (!SpottedStinger.IsNull())
		{
			PlayStinger(SpottedStinger.LoadSynchronous());
		}
	}
	// Searching entry from Chasing: player escaped the chase.
	else if (NewState == EWendigoBehaviorState::Searching && PreviousBehaviorState == EWendigoBehaviorState::Chasing)
	{
		if (!EscapedStinger.IsNull())
		{
			PlayStinger(EscapedStinger.LoadSynchronous());
		}
	}
	// GrabAttack entry: monster grabbed the player.
	else if (NewState == EWendigoBehaviorState::GrabAttack)
	{
		if (!GrabAttackStinger.IsNull())
		{
			PlayStinger(GrabAttackStinger.LoadSynchronous());
		}
	}

	PreviousBehaviorState = NewState;
}

// --- Public API ---

void UMusicTensionSystem::SetMusicIntensity(EMusicIntensity NewIntensity)
{
	if (NewIntensity == CurrentIntensity)
	{
		return;
	}

	// Silent: fade out all layers.
	if (NewIntensity == EMusicIntensity::Silent)
	{
		for (int32 i = 0; i < 3; ++i)
		{
			if (MusicLayers[i] && MusicLayers[i]->IsPlaying())
			{
				MusicLayers[i]->FadeOut(CrossfadeDuration, 0.0f, EAudioFaderCurve::SCurve);
			}
		}
		CurrentIntensity = NewIntensity;
		return;
	}

	// Fade out current layer (if not Silent -- Silent means all are already faded).
	if (CurrentIntensity != EMusicIntensity::Silent)
	{
		if (UAudioComponent* CurrentComp = GetLayerComponent(CurrentIntensity))
		{
			CurrentComp->FadeOut(CrossfadeDuration, 0.0f, EAudioFaderCurve::SCurve);
		}
	}

	// Fade in new layer.
	if (UAudioComponent* NewComp = GetLayerComponent(NewIntensity))
	{
		if (!NewComp->IsPlaying())
		{
			NewComp->Play();
		}
		NewComp->FadeIn(CrossfadeDuration, 1.0f, 0.0f, EAudioFaderCurve::SCurve);
	}

	CurrentIntensity = NewIntensity;
}

void UMusicTensionSystem::PlayStinger(USoundBase* StingerSound)
{
	if (!StingerSound || !StingerComp)
	{
		return;
	}

	StingerComp->SetSound(StingerSound);
	StingerComp->Play();
}

// --- Helpers ---

EMusicIntensity UMusicTensionSystem::AlertLevelToIntensity(EAlertLevel Level) const
{
	switch (Level)
	{
	case EAlertLevel::Patrol:     return EMusicIntensity::Calm;
	case EAlertLevel::Suspicious: return EMusicIntensity::Tense;
	case EAlertLevel::Alert:      return EMusicIntensity::Intense;
	default:                      return EMusicIntensity::Calm;
	}
}

UAudioComponent* UMusicTensionSystem::GetLayerComponent(EMusicIntensity Intensity) const
{
	const int32 Index = static_cast<int32>(Intensity);
	if (Index >= 0 && Index < 3)
	{
		return MusicLayers[Index];
	}
	return nullptr;
}
