// Copyright Null Lantern.

#include "Visibility/VisibilityScoreComponent.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "Core/SereneLogChannels.h"

UVisibilityScoreComponent::UVisibilityScoreComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVisibilityScoreComponent::BeginPlay()
{
	Super::BeginPlay();

	// --- Create HDR render target ---
	RenderTarget = NewObject<UTextureRenderTarget2D>(this);
	RenderTarget->RenderTargetFormat = RTF_RGBA16f;
	RenderTarget->InitAutoFormat(CaptureResolution, CaptureResolution);

	UE_LOG(LogSerene, Log, TEXT("VisibilityScoreComponent: RenderTarget created (%dx%d, RGBA16f)"),
		CaptureResolution, CaptureResolution);

	// --- Create SceneCaptureComponent2D at runtime ---
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogSerene, Error, TEXT("VisibilityScoreComponent: No owner actor. Cannot create SceneCapture."));
		return;
	}

	SceneCapture = NewObject<USceneCaptureComponent2D>(Owner, TEXT("VisibilitySceneCapture"));
	SceneCapture->RegisterComponent();
	SceneCapture->AttachToComponent(
		Owner->GetRootComponent(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	// Configure capture settings
	SceneCapture->TextureTarget = RenderTarget;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorHDR;
	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->bAlwaysPersistRenderingState = true;
	SceneCapture->FOVAngle = 90.0f;

	// ShowFlags optimization: disable expensive features not needed for brightness sampling
	SceneCapture->ShowFlags.SetBloom(false);
	SceneCapture->ShowFlags.SetMotionBlur(false);
	SceneCapture->ShowFlags.SetParticles(false);
	SceneCapture->ShowFlags.SetSkeletalMeshes(false);
	SceneCapture->ShowFlags.SetFog(false);
	SceneCapture->ShowFlags.SetPostProcessing(false);

	// Disable GI for performance (WARN-09): direct lighting alone is sufficient
	// for visibility scoring since we measure "how lit is the player", not
	// how bounce light affects visibility. Reflections kept for specular cues.
	SceneCapture->ShowFlags.SetGlobalIllumination(false);
	SceneCapture->ShowFlags.SetReflectionEnvironment(true);

	UE_LOG(LogSerene, Log, TEXT("VisibilityScoreComponent: SceneCapture initialized (FOV=%.0f, Interval=%.2fs)"),
		SceneCapture->FOVAngle, CaptureInterval);

	// --- Start periodic timer ---
	GetWorld()->GetTimerManager().SetTimer(
		CaptureTimerHandle,
		this,
		&UVisibilityScoreComponent::PerformCapture,
		CaptureInterval,
		true);
}

void UVisibilityScoreComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CaptureTimerHandle);
	}

	if (SceneCapture)
	{
		SceneCapture->DestroyComponent();
		SceneCapture = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UVisibilityScoreComponent::PerformCapture()
{
	if (!SceneCapture || !RenderTarget)
	{
		return;
	}

	SceneCapture->CaptureScene();
	ComputeScore();
}

void UVisibilityScoreComponent::ComputeScore()
{
	FTextureRenderTargetResource* Resource = RenderTarget->GameThread_GetRenderTargetResource();
	if (!Resource)
	{
		return;
	}

	// Reuse cached pixel buffer to avoid per-call heap allocation (CRIT-01).
	// NOTE: ReadFloat16Pixels is a synchronous GPU readback -- known performance concern.
	// Future improvement: use ReadPixelsAsync or FRenderCommandFence for async readback.
	CachedPixels.Reset();
	Resource->ReadFloat16Pixels(CachedPixels);

	// Compute average luminance using Rec.709 coefficients
	float TotalLuminance = 0.0f;
	for (const FFloat16Color& Pixel : CachedPixels)
	{
		const float R = Pixel.R.GetFloat();
		const float G = Pixel.G.GetFloat();
		const float B = Pixel.B.GetFloat();
		TotalLuminance += 0.2126f * R + 0.7152f * G + 0.0722f * B;
	}

	const float AvgLuminance = (CachedPixels.Num() > 0) ? TotalLuminance / CachedPixels.Num() : 0.0f;

	// Normalize raw light level
	RawLightLevel = FMath::Clamp(AvgLuminance / MaxExpectedLuminance, 0.0f, 1.0f);

	// Apply modifiers
	float Score = RawLightLevel;

	// Crouch reduction: use engine's built-in bIsCrouched to avoid circular dependency
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character && Character->bIsCrouched)
	{
		Score -= CrouchReduction;
	}

	// Hiding reduction (set by HidingComponent via SetHidingReduction)
	Score -= CurrentHidingReduction;

	VisibilityScore = FMath::Clamp(Score, 0.0f, 1.0f);

	UE_LOG(LogSerene, Verbose, TEXT("VisibilityScore: Raw=%.3f, Crouch=%s, HidingReduction=%.2f, Final=%.3f"),
		RawLightLevel,
		(Character && Character->bIsCrouched) ? TEXT("Yes") : TEXT("No"),
		CurrentHidingReduction,
		VisibilityScore);
}

void UVisibilityScoreComponent::SetHidingReduction(float Reduction)
{
	CurrentHidingReduction = Reduction;
}
