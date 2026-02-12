// Copyright Null Lantern.

#include "Audio/AmbientAudioManager.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"

DEFINE_LOG_CATEGORY_STATIC(LogAmbientAudio, Log, All);

AAmbientAudioManager::AAmbientAudioManager()
{
	PrimaryActorTick.bCanEverTick = false;

	// Default root component for level placement.
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void AAmbientAudioManager::BeginPlay()
{
	Super::BeginPlay();

	// Create ambient bed audio component (non-spatial, 2D).
	AmbientBedComp = NewObject<UAudioComponent>(this);
	AmbientBedComp->SetupAttachment(GetRootComponent());
	AmbientBedComp->bAutoActivate = false;
	AmbientBedComp->bAutoDestroy = false;
	AmbientBedComp->bIsUISound = true;  // Non-spatialized ambient bed.
	AmbientBedComp->RegisterComponent();

	// Load and play the ambient bed if assigned.
	if (!AmbientBedSound.IsNull())
	{
		if (USoundBase* Sound = AmbientBedSound.LoadSynchronous())
		{
			AmbientBedComp->SetSound(Sound);
			AmbientBedComp->SetVolumeMultiplier(AmbientBedVolume);
			AmbientBedComp->Play();
		}
	}

	// Start the random one-shot timer.
	ScheduleNextOneShot();
}

void AAmbientAudioManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear one-shot timer.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(OneShotTimerHandle);
	}

	// Stop ambient bed.
	if (AmbientBedComp)
	{
		AmbientBedComp->Stop();
	}

	Super::EndPlay(EndPlayReason);
}

// --- Predator Silence ---

void AAmbientAudioManager::EnterPredatorSilence(float FadeDuration)
{
	bIsSilenced = true;

	// Fade out the ambient bed.
	if (AmbientBedComp && AmbientBedComp->IsPlaying())
	{
		AmbientBedComp->FadeOut(FadeDuration, 0.0f);
	}

	// Stop random one-shots during silence.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(OneShotTimerHandle);
	}

	UE_LOG(LogAmbientAudio, Log, TEXT("AmbientAudioManager: Entered predator silence (fade %.1fs)."), FadeDuration);
}

void AAmbientAudioManager::ExitPredatorSilence(float FadeDuration)
{
	bIsSilenced = false;

	// Fade in the ambient bed.
	if (AmbientBedComp)
	{
		AmbientBedComp->FadeIn(FadeDuration, AmbientBedVolume, 0.0f);
	}

	// Resume random one-shots.
	ScheduleNextOneShot();

	UE_LOG(LogAmbientAudio, Log, TEXT("AmbientAudioManager: Exited predator silence (fade %.1fs)."), FadeDuration);
}

// --- Random One-Shots ---

void AAmbientAudioManager::PlayRandomOneShot()
{
	// Don't play during predator silence.
	if (bIsSilenced)
	{
		return;
	}

	if (EnvironmentalOneShotSounds.Num() == 0)
	{
		ScheduleNextOneShot();
		return;
	}

	// Pick a random sound from the pool.
	const int32 Index = FMath::RandRange(0, EnvironmentalOneShotSounds.Num() - 1);
	const TSoftObjectPtr<USoundBase>& SoundRef = EnvironmentalOneShotSounds[Index];

	if (SoundRef.IsNull())
	{
		ScheduleNextOneShot();
		return;
	}

	USoundBase* Sound = SoundRef.LoadSynchronous();
	if (!Sound)
	{
		ScheduleNextOneShot();
		return;
	}

	// Randomize volume within range.
	const float Volume = FMath::FRandRange(0.5f - OneShotVolumeRange, 0.5f + OneShotVolumeRange);

	if (OneShotAttenuation)
	{
		// Play as 3D sound at a random offset from this actor's location.
		const FVector RandomOffset = FMath::VRand() * FMath::FRandRange(0.0f, 500.0f);
		const FVector PlayLocation = GetActorLocation() + RandomOffset;

		UGameplayStatics::PlaySoundAtLocation(
			this,
			Sound,
			PlayLocation,
			FRotator::ZeroRotator,
			Volume,
			1.0f,   // PitchMultiplier
			0.0f,   // StartTime
			OneShotAttenuation
		);
	}
	else
	{
		// Play as 2D sound (no attenuation set).
		UGameplayStatics::PlaySound2D(this, Sound, Volume);
	}

	// Schedule the next one-shot.
	ScheduleNextOneShot();
}

void AAmbientAudioManager::ScheduleNextOneShot()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Interval = FMath::FRandRange(OneShotIntervalRange.X, OneShotIntervalRange.Y);
	World->GetTimerManager().SetTimer(
		OneShotTimerHandle,
		this,
		&AAmbientAudioManager::PlayRandomOneShot,
		Interval,
		false  // Non-looping; re-scheduled after each play.
	);
}
