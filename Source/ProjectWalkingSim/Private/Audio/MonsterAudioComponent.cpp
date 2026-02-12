// Copyright Null Lantern.

#include "Audio/MonsterAudioComponent.h"

#include "AI/WendigoCharacter.h"
#include "Audio/AudioConstants.h"
#include "Core/SereneLogChannels.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundAttenuation.h"
#include "Kismet/GameplayStatics.h"

UMonsterAudioComponent::UMonsterAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMonsterAudioComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Bind to WendigoCharacter's behavior state delegate.
	if (AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Owner))
	{
		Wendigo->OnBehaviorStateChanged.AddDynamic(this, &UMonsterAudioComponent::OnBehaviorStateChanged);
	}
	else
	{
		UE_LOG(LogSerene, Warning, TEXT("MonsterAudioComponent: Owner %s is not AWendigoCharacter. Delegate not bound."), *Owner->GetName());
	}

	// Create breathing audio component (3D spatial, looping).
	BreathingAudioComp = NewObject<UAudioComponent>(Owner, TEXT("MonsterBreathingAudio"));
	if (BreathingAudioComp)
	{
		BreathingAudioComp->SetupAttachment(Owner->GetRootComponent());
		BreathingAudioComp->bAutoActivate = false;
		BreathingAudioComp->bAutoDestroy = false;
		if (MonsterAttenuation)
		{
			BreathingAudioComp->AttenuationSettings = MonsterAttenuation;
		}
		BreathingAudioComp->RegisterComponent();
	}

	// Start with Patrol breathing and footstep timer.
	TransitionBreathingSound(EWendigoBehaviorState::Patrol);
	UpdateFootstepTimer();
}

void UMonsterAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear footstep timer.
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FootstepTimerHandle);
	}

	// Stop breathing audio.
	if (BreathingAudioComp)
	{
		BreathingAudioComp->Stop();
	}

	// Unbind from WendigoCharacter.
	if (AActor* Owner = GetOwner())
	{
		if (AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(Owner))
		{
			Wendigo->OnBehaviorStateChanged.RemoveDynamic(this, &UMonsterAudioComponent::OnBehaviorStateChanged);
		}
	}

	Super::EndPlay(EndPlayReason);
}

// --- Delegate Handler ---

void UMonsterAudioComponent::OnBehaviorStateChanged(EWendigoBehaviorState NewState)
{
	TransitionBreathingSound(NewState);
	PlayVocalization(NewState);
	UpdateFootstepTimer();
}

// --- Internal Methods ---

void UMonsterAudioComponent::TransitionBreathingSound(EWendigoBehaviorState NewState)
{
	if (!BreathingAudioComp)
	{
		return;
	}

	const TSoftObjectPtr<USoundBase>* FoundSound = BreathingSounds.Find(NewState);
	if (!FoundSound || FoundSound->IsNull())
	{
		// No breathing sound for this state -- fade out current.
		if (BreathingAudioComp->IsPlaying())
		{
			BreathingAudioComp->FadeOut(BreathingCrossfadeDuration, 0.0f);
		}
		return;
	}

	USoundBase* NewSound = FoundSound->LoadSynchronous();
	if (!NewSound)
	{
		return;
	}

	// Crossfade: fade out current, switch sound, fade in.
	if (BreathingAudioComp->IsPlaying())
	{
		BreathingAudioComp->FadeOut(BreathingCrossfadeDuration * 0.5f, 0.0f);
	}

	BreathingAudioComp->SetSound(NewSound);
	BreathingAudioComp->FadeIn(BreathingCrossfadeDuration * 0.5f, 1.0f);
}

void UMonsterAudioComponent::PlayMonsterFootstep()
{
	if (FootstepSound.IsNull())
	{
		return;
	}

	USoundBase* Sound = FootstepSound.LoadSynchronous();
	if (!Sound)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Play 3D positional sound at the Wendigo's location.
	// Do NOT report to AI hearing -- this is for the player to hear only.
	UGameplayStatics::PlaySoundAtLocation(
		this,
		Sound,
		Owner->GetActorLocation(),
		FRotator::ZeroRotator,
		1.0f,   // VolumeMultiplier
		1.0f,   // PitchMultiplier
		0.0f,   // StartTime
		MonsterAttenuation
	);
}

void UMonsterAudioComponent::PlayVocalization(EWendigoBehaviorState State)
{
	// Roll against VocalizationChance.
	if (FMath::FRand() > VocalizationChance)
	{
		return;
	}

	const TSoftObjectPtr<USoundBase>* FoundSound = VocalizationSounds.Find(State);
	if (!FoundSound || FoundSound->IsNull())
	{
		return;
	}

	USoundBase* Sound = FoundSound->LoadSynchronous();
	if (!Sound)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// One-shot 3D vocalization at the Wendigo's location.
	UGameplayStatics::PlaySoundAtLocation(
		this,
		Sound,
		Owner->GetActorLocation(),
		FRotator::ZeroRotator,
		1.0f,
		1.0f,
		0.0f,
		MonsterAttenuation
	);
}

float UMonsterAudioComponent::GetFootstepInterval() const
{
	const AWendigoCharacter* Wendigo = Cast<AWendigoCharacter>(GetOwner());
	if (!Wendigo)
	{
		return AudioConstants::MonsterFootstepInterval_Patrol;
	}

	switch (Wendigo->BehaviorState)
	{
	case EWendigoBehaviorState::Patrol:
		return AudioConstants::MonsterFootstepInterval_Patrol;
	case EWendigoBehaviorState::Investigating:
		return AudioConstants::MonsterFootstepInterval_Investigate;
	case EWendigoBehaviorState::Chasing:
		return AudioConstants::MonsterFootstepInterval_Chase;
	case EWendigoBehaviorState::Searching:
		return AudioConstants::MonsterFootstepInterval_Search;
	case EWendigoBehaviorState::GrabAttack:
		return 0.0f; // No footsteps during grab attack
	default:
		return AudioConstants::MonsterFootstepInterval_Patrol;
	}
}

void UMonsterAudioComponent::UpdateFootstepTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(FootstepTimerHandle);

	const float Interval = GetFootstepInterval();
	if (Interval <= 0.0f)
	{
		return; // No footsteps for this state (e.g., GrabAttack)
	}

	World->GetTimerManager().SetTimer(
		FootstepTimerHandle,
		this,
		&UMonsterAudioComponent::PlayMonsterFootstep,
		Interval,
		true // bLoop
	);
}
