// Copyright Null Lantern.

#include "Audio/PlayerAudioComponent.h"
#include "Player/Components/FootstepComponent.h"
#include "AI/WendigoCharacter.h"
#include "Components/AudioComponent.h"
#include "Core/SereneLogChannels.h"
#include "Kismet/GameplayStatics.h"

UPlayerAudioComponent::UPlayerAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerAudioComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// --- Bind to FootstepComponent's OnFootstep delegate ---
	if (UFootstepComponent* FootstepComp = Owner->FindComponentByClass<UFootstepComponent>())
	{
		FootstepComp->OnFootstep.AddDynamic(this, &UPlayerAudioComponent::HandleFootstep);
	}
	else
	{
		UE_LOG(LogSerene, Warning, TEXT("PlayerAudioComponent: No FootstepComponent found on %s"), *Owner->GetName());
	}

	// --- Create heartbeat audio component (2D, non-spatialized) ---
	HeartbeatAudioComp = NewObject<UAudioComponent>(Owner, TEXT("HeartbeatAudio"));
	if (HeartbeatAudioComp)
	{
		HeartbeatAudioComp->SetupAttachment(Owner->GetRootComponent());
		HeartbeatAudioComp->bAutoActivate = false;
		HeartbeatAudioComp->bAutoDestroy = false;
		HeartbeatAudioComp->bIsUISound = true; // Non-spatialized; heartbeat is a player-internal sound
		HeartbeatAudioComp->RegisterComponent();

		// Load and assign heartbeat sound
		if (!HeartbeatSound.IsNull())
		{
			if (USoundBase* LoadedSound = HeartbeatSound.LoadSynchronous())
			{
				HeartbeatAudioComp->SetSound(LoadedSound);
				HeartbeatAudioComp->Play();
				// Starts at zero intensity -- MetaSound Intensity param controls output
				HeartbeatAudioComp->SetFloatParameter(HeartbeatIntensityParam, 0.0f);
			}
		}
	}

	// --- Start heartbeat proximity timer ---
	const float Interval = (HeartbeatUpdateRate > 0.0f) ? (1.0f / HeartbeatUpdateRate) : 0.25f;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			HeartbeatTimerHandle,
			this,
			&UPlayerAudioComponent::UpdateHeartbeatProximity,
			Interval,
			true // bLoop
		);
	}

	// Cache initial Wendigo reference
	FindWendigo();
}

void UPlayerAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear heartbeat timer
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
	}

	// Stop heartbeat audio
	if (HeartbeatAudioComp)
	{
		HeartbeatAudioComp->Stop();
	}

	// Unbind from FootstepComponent
	if (AActor* Owner = GetOwner())
	{
		if (UFootstepComponent* FootstepComp = Owner->FindComponentByClass<UFootstepComponent>())
		{
			FootstepComp->OnFootstep.RemoveDynamic(this, &UPlayerAudioComponent::HandleFootstep);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UPlayerAudioComponent::HandleFootstep(EPhysicalSurface SurfaceType, float Volume)
{
	// Look up surface-specific sound
	TSoftObjectPtr<USoundBase>* FoundSound = FootstepSounds.Find(SurfaceType);

	// Fall back to default if surface type not in map
	const TSoftObjectPtr<USoundBase>& SoundRef = (FoundSound && !FoundSound->IsNull()) ? *FoundSound : DefaultFootstepSound;

	if (SoundRef.IsNull())
	{
		return;
	}

	USoundBase* LoadedSound = SoundRef.LoadSynchronous();
	if (!LoadedSound)
	{
		return;
	}

	// Play as 2D sound -- the player hears their own footsteps non-spatially
	UGameplayStatics::PlaySound2D(this, LoadedSound, FootstepBaseVolume * Volume);
}

void UPlayerAudioComponent::UpdateHeartbeatProximity()
{
	// Re-find Wendigo if cached reference is stale
	if (!CachedWendigo.IsValid())
	{
		FindWendigo();
		if (!CachedWendigo.IsValid())
		{
			// No Wendigo in the world -- set intensity to zero
			if (HeartbeatAudioComp)
			{
				HeartbeatAudioComp->SetFloatParameter(HeartbeatIntensityParam, 0.0f);
			}
			return;
		}
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const float Distance = FVector::Dist(
		Owner->GetActorLocation(),
		CachedWendigo->GetActorLocation()
	);

	// Normalize distance to intensity: 0 = far, 1 = very close
	float Intensity = 0.0f;
	if (Distance <= HeartbeatMinDistance)
	{
		Intensity = 1.0f;
	}
	else if (Distance < HeartbeatMaxDistance)
	{
		Intensity = 1.0f - ((Distance - HeartbeatMinDistance) / (HeartbeatMaxDistance - HeartbeatMinDistance));
	}
	// else: Distance >= HeartbeatMaxDistance => Intensity stays 0.0f

	if (HeartbeatAudioComp)
	{
		HeartbeatAudioComp->SetFloatParameter(HeartbeatIntensityParam, Intensity);
	}
}

void UPlayerAudioComponent::FindWendigo()
{
	TArray<AActor*> Wendigos;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWendigoCharacter::StaticClass(), Wendigos);

	if (Wendigos.Num() > 0)
	{
		CachedWendigo = Wendigos[0];
	}
}
