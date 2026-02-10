// Copyright Null Lantern.

#include "Hiding/HidingSpotDataAsset.h"

UHidingSpotDataAsset::UHidingSpotDataAsset()
	: EntryMontage(nullptr)
	, ExitMontage(nullptr)
	, CameraBlendInTime(0.5f)
	, CameraBlendOutTime(0.3f)
	, ViewPitchMin(-30.0f)
	, ViewPitchMax(30.0f)
	, ViewYawMin(-45.0f)
	, ViewYawMax(45.0f)
	, PeekOverlayMaterial(nullptr)
	, InteractionText(NSLOCTEXT("HidingSpot", "DefaultInteraction", "Hide"))
	, ExitText(NSLOCTEXT("HidingSpot", "DefaultExit", "Exit"))
	, HidingVisibilityReduction(0.5f)
{
}
