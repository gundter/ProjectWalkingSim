// Copyright Null Lantern.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI/MonsterAITypes.h"
#include "MusicTensionSystem.generated.h"

class AWendigoCharacter;
class UAudioComponent;

/**
 * Music intensity tiers driven by AI alert state.
 * Each tier maps to a looping music layer that crossfades when the tier changes.
 */
UENUM(BlueprintType)
enum class EMusicIntensity : uint8
{
	Calm     UMETA(DisplayName = "Calm"),
	Tense    UMETA(DisplayName = "Tense"),
	Intense  UMETA(DisplayName = "Intense"),
	Silent   UMETA(DisplayName = "Silent")
};

/**
 * Dynamic music system driven by AI alert level.
 * Manages three looping music layers (Calm, Tense, Intense) with smooth crossfade
 * transitions and plays one-shot stingers on specific gameplay events.
 *
 * Binding: Listens to SuspicionComponent::OnAlertLevelChanged for intensity changes
 * and WendigoCharacter::OnBehaviorStateChanged for stinger triggers.
 * Owner-first binding -- expects to be placed on AWendigoCharacter as a default subobject.
 * Falls back to world search if owner is not a Wendigo.
 *
 * No Tick -- entirely delegate/event driven.
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class PROJECTWALKINGSIM_API UMusicTensionSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UMusicTensionSystem();

	// --- Public API ---

	/** Transition to a new music intensity tier with crossfade. */
	UFUNCTION(BlueprintCallable, Category = "Audio|Music")
	void SetMusicIntensity(EMusicIntensity NewIntensity);

	/** Play a one-shot non-positional stinger sound. */
	UFUNCTION(BlueprintCallable, Category = "Audio|Music")
	void PlayStinger(USoundBase* StingerSound);

	/** Bind to a specific Wendigo character for alert/behavior state tracking. */
	UFUNCTION(BlueprintCallable, Category = "Audio|Music")
	void BindToWendigoCharacter(AWendigoCharacter* Wendigo);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- Music Layer Assets (assigned in editor/Blueprint) ---

	/** Looping calm layer -- ambient drone, sparse. Plays during Patrol. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Music|Tracks")
	TSoftObjectPtr<USoundBase> CalmMusic;

	/** Looping tense layer -- building tension. Plays during Suspicious. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Music|Tracks")
	TSoftObjectPtr<USoundBase> TenseMusic;

	/** Looping intense layer -- full intensity. Plays during Alert/Chase. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Music|Tracks")
	TSoftObjectPtr<USoundBase> IntenseMusic;

	// --- Stinger Assets ---

	/** Stinger when the monster first spots the player (enters chase). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Music|Stingers")
	TSoftObjectPtr<USoundBase> SpottedStinger;

	/** Stinger when the player escapes chase (monster enters search). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Music|Stingers")
	TSoftObjectPtr<USoundBase> EscapedStinger;

	/** Stinger when the monster initiates a grab attack. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Music|Stingers")
	TSoftObjectPtr<USoundBase> GrabAttackStinger;

	// --- Tuning ---

	/** Duration in seconds for crossfade between music layers. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Music|Transition", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float CrossfadeDuration = 3.0f;

private:
	/** Music layer audio components indexed by EMusicIntensity (Calm=0, Tense=1, Intense=2). */
	UPROPERTY()
	TObjectPtr<UAudioComponent> MusicLayers[3];

	/** Non-positional audio component for one-shot stingers. */
	UPROPERTY()
	TObjectPtr<UAudioComponent> StingerComp;

	/** Currently active music intensity tier. */
	EMusicIntensity CurrentIntensity = EMusicIntensity::Calm;

	/** Previous behavior state for detecting transitions (stinger triggers). */
	EWendigoBehaviorState PreviousBehaviorState = EWendigoBehaviorState::Patrol;

	// --- Delegate handlers ---

	/** Called when the Wendigo's alert level changes. Maps alert to music intensity. */
	UFUNCTION()
	void OnAlertLevelChanged(EAlertLevel NewLevel);

	/** Called when the Wendigo's behavior state changes. Triggers stingers on specific transitions. */
	UFUNCTION()
	void OnBehaviorStateChangedForStingers(EWendigoBehaviorState NewState);

	// --- Internal helpers ---

	/** Map an alert level to a music intensity tier. */
	EMusicIntensity AlertLevelToIntensity(EAlertLevel Level) const;

	/** Get the audio component for a given music intensity tier. Returns null for Silent. */
	UAudioComponent* GetLayerComponent(EMusicIntensity Intensity) const;

	/** Create and configure the 3 music layer components and the stinger component. */
	void CreateMusicLayers();

	/** Find the Wendigo (owner-first, then world search) and bind delegates. */
	void BindToWendigo();
};
