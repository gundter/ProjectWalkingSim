// Copyright Null Lantern.

#include "Save/SaveSubsystem.h"
#include "Save/SereneSaveGame.h"
#include "Core/SereneLogChannels.h"
#include "Interaction/DoorActor.h"
#include "Interaction/DrawerActor.h"
#include "Interaction/PickupActor.h"
#include "Interaction/SaveableInterface.h"
#include "Player/SereneCharacter.h"
#include "Inventory/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameViewportClient.h"
#include "UnrealClient.h"
#include "ImageUtils.h"
#include "EngineUtils.h"

void USaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogSerene, Log, TEXT("SaveSubsystem initialized with %d slots"), MaxSlots);
}

void USaveSubsystem::Deinitialize()
{
	// Unbind screenshot delegate if still bound
	if (ScreenshotDelegateHandle.IsValid())
	{
		UGameViewportClient::OnScreenshotCaptured().Remove(ScreenshotDelegateHandle);
		ScreenshotDelegateHandle.Reset();
	}

	Super::Deinitialize();
}

// ---------------------------------------------------------------------------
// Save API
// ---------------------------------------------------------------------------

void USaveSubsystem::SaveToSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots)
	{
		UE_LOG(LogSerene, Warning, TEXT("SaveToSlot: invalid slot index %d"), SlotIndex);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogSerene, Error, TEXT("SaveToSlot: no world available"));
		return;
	}

	// Create save game object and begin populating it
	USereneSaveGame* SaveGame = NewObject<USereneSaveGame>(this);
	SaveGame->SlotInfo.Timestamp = FDateTime::Now();

	// Gather world + player state synchronously
	GatherWorldState(SaveGame, World);
	GatherPlayerState(SaveGame, World);

	// Store destroyed pickup tracking data
	SaveGame->DestroyedPickupIds.Reserve(DestroyedPickupTracker.Num());
	for (const FName& Id : DestroyedPickupTracker)
	{
		SaveGame->DestroyedPickupIds.Add(Id);
	}

	// Store pending save state for screenshot callback
	PendingSaveObject = SaveGame;
	PendingSaveSlotIndex = SlotIndex;

	// Unbind any existing screenshot delegate
	if (ScreenshotDelegateHandle.IsValid())
	{
		UGameViewportClient::OnScreenshotCaptured().Remove(ScreenshotDelegateHandle);
		ScreenshotDelegateHandle.Reset();
	}

	// Bind screenshot callback (one-shot)
	ScreenshotDelegateHandle = UGameViewportClient::OnScreenshotCaptured().AddUObject(
		this, &USaveSubsystem::OnScreenshotCaptured);

	// Request screenshot (async -- captured at end of frame)
	FScreenshotRequest::RequestScreenshot(false);

	UE_LOG(LogSerene, Log, TEXT("SaveToSlot: screenshot requested for slot %d"), SlotIndex);
}

void USaveSubsystem::LoadFromSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots)
	{
		UE_LOG(LogSerene, Warning, TEXT("LoadFromSlot: invalid slot index %d"), SlotIndex);
		return;
	}

	const FString SlotName = GetSlotName(SlotIndex);

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		UE_LOG(LogSerene, Warning, TEXT("LoadFromSlot: no save in slot %d (%s)"), SlotIndex, *SlotName);
		return;
	}

	PendingSaveData = Cast<USereneSaveGame>(
		UGameplayStatics::LoadGameFromSlot(SlotName, 0));

	if (!PendingSaveData)
	{
		UE_LOG(LogSerene, Error, TEXT("LoadFromSlot: failed to load save from slot %d (%s)"), SlotIndex, *SlotName);
		return;
	}

	UE_LOG(LogSerene, Log, TEXT("LoadFromSlot: loaded save from slot %d, restarting level"), SlotIndex);

	// Restart the level -- world resets to map defaults
	UWorld* World = GetWorld();
	if (World)
	{
		FString MapName = World->GetMapName();
		MapName.RemoveFromStart(World->StreamingLevelsPrefix);
		UGameplayStatics::OpenLevel(World, FName(*MapName));
	}
}

void USaveSubsystem::LoadLatestSave()
{
	const int32 LatestSlot = GetLatestSlotIndex();
	if (LatestSlot < 0)
	{
		UE_LOG(LogSerene, Warning, TEXT("LoadLatestSave: no saves found"));
		return;
	}

	LoadFromSlot(LatestSlot);
}

bool USaveSubsystem::HasAnySave() const
{
	for (int32 i = 0; i < MaxSlots; ++i)
	{
		if (UGameplayStatics::DoesSaveGameExist(GetSlotName(i), 0))
		{
			return true;
		}
	}
	return false;
}

FSaveSlotInfo USaveSubsystem::GetSlotInfo(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots)
	{
		return FSaveSlotInfo();
	}

	const FString SlotName = GetSlotName(SlotIndex);
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		return FSaveSlotInfo();
	}

	const USereneSaveGame* SaveGame = Cast<USereneSaveGame>(
		UGameplayStatics::LoadGameFromSlot(SlotName, 0));

	if (!SaveGame)
	{
		return FSaveSlotInfo();
	}

	return SaveGame->SlotInfo;
}

void USaveSubsystem::DeleteSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots)
	{
		UE_LOG(LogSerene, Warning, TEXT("DeleteSlot: invalid slot index %d"), SlotIndex);
		return;
	}

	const FString SlotName = GetSlotName(SlotIndex);
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		UGameplayStatics::DeleteGameInSlot(SlotName, 0);
		UE_LOG(LogSerene, Log, TEXT("DeleteSlot: deleted slot %d (%s)"), SlotIndex, *SlotName);
	}
}

bool USaveSubsystem::DoesSaveExist(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= MaxSlots)
	{
		return false;
	}
	return UGameplayStatics::DoesSaveGameExist(GetSlotName(SlotIndex), 0);
}

int32 USaveSubsystem::GetLatestSlotIndex() const
{
	int32 LatestSlot = -1;
	FDateTime LatestTimestamp = FDateTime::MinValue();

	for (int32 i = 0; i < MaxSlots; ++i)
	{
		const FString SlotName = GetSlotName(i);
		if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
		{
			continue;
		}

		const USereneSaveGame* SaveGame = Cast<USereneSaveGame>(
			UGameplayStatics::LoadGameFromSlot(SlotName, 0));

		if (SaveGame && SaveGame->SlotInfo.Timestamp > LatestTimestamp)
		{
			LatestTimestamp = SaveGame->SlotInfo.Timestamp;
			LatestSlot = i;
		}
	}

	return LatestSlot;
}

// ---------------------------------------------------------------------------
// State Management
// ---------------------------------------------------------------------------

void USaveSubsystem::ApplyPendingSaveData(UWorld* World)
{
	if (!PendingSaveData || !World)
	{
		return;
	}

	UE_LOG(LogSerene, Log, TEXT("ApplyPendingSaveData: restoring saved state"));

	// 1. Restore door states via ISaveable
	for (TActorIterator<ADoorActor> It(World); It; ++It)
	{
		ADoorActor* Door = *It;
		if (Door)
		{
			ISaveable::Execute_ReadSaveData(Door, PendingSaveData);
		}
	}

	// 2. Restore drawer states via ISaveable
	for (TActorIterator<ADrawerActor> It(World); It; ++It)
	{
		ADrawerActor* Drawer = *It;
		if (Drawer)
		{
			ISaveable::Execute_ReadSaveData(Drawer, PendingSaveData);
		}
	}

	// 3. Destroy picked-up items
	TArray<APickupActor*> ToDestroy;
	for (TActorIterator<APickupActor> It(World); It; ++It)
	{
		APickupActor* Pickup = *It;
		if (Pickup && PendingSaveData->DestroyedPickupIds.Contains(Pickup->GetFName()))
		{
			ToDestroy.Add(Pickup);
		}
	}
	for (APickupActor* Pickup : ToDestroy)
	{
		UE_LOG(LogSerene, Verbose, TEXT("  Destroying pickup: %s"), *Pickup->GetFName().ToString());
		Pickup->Destroy();
	}

	// 4. Set player position
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
	if (PlayerPawn)
	{
		PlayerPawn->SetActorLocation(PendingSaveData->PlayerLocation);

		AController* Controller = PlayerPawn->GetController();
		if (Controller)
		{
			Controller->SetControlRotation(PendingSaveData->PlayerRotation);
		}

		UE_LOG(LogSerene, Log, TEXT("  Player position restored to %s"),
			*PendingSaveData->PlayerLocation.ToString());
	}

	// 5. Restore inventory
	ASereneCharacter* Character = Cast<ASereneCharacter>(PlayerPawn);
	if (Character)
	{
		UInventoryComponent* Inventory = Character->FindComponentByClass<UInventoryComponent>();
		if (Inventory)
		{
			Inventory->RestoreSavedInventory(PendingSaveData->InventorySlots);
			UE_LOG(LogSerene, Log, TEXT("  Inventory restored (%d slots)"),
				PendingSaveData->InventorySlots.Num());
		}
	}

	// 6. Repopulate destroyed pickup tracker
	DestroyedPickupTracker.Empty();
	for (const FName& Id : PendingSaveData->DestroyedPickupIds)
	{
		DestroyedPickupTracker.Add(Id);
	}

	UE_LOG(LogSerene, Log, TEXT("ApplyPendingSaveData: complete (doors=%d, drawers=%d, destroyed=%d)"),
		PendingSaveData->DoorStates.Num(),
		PendingSaveData->DrawerStates.Num(),
		PendingSaveData->DestroyedPickupIds.Num());

	// Clear pending data -- load is complete
	PendingSaveData = nullptr;
}

void USaveSubsystem::TrackDestroyedPickup(FName PickupId)
{
	DestroyedPickupTracker.Add(PickupId);
	UE_LOG(LogSerene, Verbose, TEXT("TrackDestroyedPickup: %s (total tracked: %d)"),
		*PickupId.ToString(), DestroyedPickupTracker.Num());
}

void USaveSubsystem::ClearDestroyedPickupTracker()
{
	DestroyedPickupTracker.Empty();
}

// ---------------------------------------------------------------------------
// Private Helpers
// ---------------------------------------------------------------------------

FString USaveSubsystem::GetSlotName(int32 SlotIndex) const
{
	return FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);
}

void USaveSubsystem::GatherWorldState(USereneSaveGame* SaveGame, UWorld* World)
{
	SaveGame->DoorStates.Empty();
	SaveGame->DrawerStates.Empty();

	// Gather door states via ISaveable
	for (TActorIterator<ADoorActor> It(World); It; ++It)
	{
		ADoorActor* Door = *It;
		if (!Door || Door->IsActorBeingDestroyed())
		{
			continue;
		}

		ISaveable::Execute_WriteSaveData(Door, SaveGame);
	}

	// Gather drawer states via ISaveable
	for (TActorIterator<ADrawerActor> It(World); It; ++It)
	{
		ADrawerActor* Drawer = *It;
		if (!Drawer || Drawer->IsActorBeingDestroyed())
		{
			continue;
		}

		ISaveable::Execute_WriteSaveData(Drawer, SaveGame);
	}

	UE_LOG(LogSerene, Verbose, TEXT("GatherWorldState: %d doors, %d drawers"),
		SaveGame->DoorStates.Num(), SaveGame->DrawerStates.Num());
}

void USaveSubsystem::GatherPlayerState(USereneSaveGame* SaveGame, UWorld* World)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
	if (!PlayerPawn)
	{
		UE_LOG(LogSerene, Warning, TEXT("GatherPlayerState: no player pawn found"));
		return;
	}

	SaveGame->PlayerLocation = PlayerPawn->GetActorLocation();

	AController* Controller = PlayerPawn->GetController();
	if (Controller)
	{
		SaveGame->PlayerRotation = Controller->GetControlRotation();
	}

	// Gather inventory
	ASereneCharacter* Character = Cast<ASereneCharacter>(PlayerPawn);
	if (Character)
	{
		UInventoryComponent* Inventory = Character->FindComponentByClass<UInventoryComponent>();
		if (Inventory)
		{
			SaveGame->InventorySlots = Inventory->GetSlots();
		}
	}

	UE_LOG(LogSerene, Verbose, TEXT("GatherPlayerState: location=%s, inventory=%d slots"),
		*SaveGame->PlayerLocation.ToString(), SaveGame->InventorySlots.Num());
}

// ---------------------------------------------------------------------------
// Screenshot Capture
// ---------------------------------------------------------------------------

void USaveSubsystem::OnScreenshotCaptured(int32 Width, int32 Height, const TArray<FColor>& Bitmap)
{
	// Unbind delegate (one-shot pattern)
	UGameViewportClient::OnScreenshotCaptured().Remove(ScreenshotDelegateHandle);
	ScreenshotDelegateHandle.Reset();

	if (!PendingSaveObject)
	{
		UE_LOG(LogSerene, Warning, TEXT("OnScreenshotCaptured: no pending save object"));
		return;
	}

	// Compress screenshot to JPEG via FImageUtils::CompressImage
	FImageView ImageView(Bitmap.GetData(), Width, Height);
	TArray64<uint8> CompressedData;
	const bool bCompressed = FImageUtils::CompressImage(CompressedData, TEXT("jpg"), ImageView, 85);

	if (bCompressed && CompressedData.Num() > 0)
	{
		// Copy from TArray64 to TArray for UPROPERTY serialization
		PendingSaveObject->SlotInfo.ScreenshotData.SetNumUninitialized(CompressedData.Num());
		FMemory::Memcpy(
			PendingSaveObject->SlotInfo.ScreenshotData.GetData(),
			CompressedData.GetData(),
			CompressedData.Num());
		PendingSaveObject->SlotInfo.ScreenshotWidth = Width;
		PendingSaveObject->SlotInfo.ScreenshotHeight = Height;

		UE_LOG(LogSerene, Log, TEXT("Screenshot compressed: %dx%d -> %lld bytes JPEG"),
			Width, Height, CompressedData.Num());
	}
	else
	{
		UE_LOG(LogSerene, Warning, TEXT("Screenshot compression failed (%dx%d, %d pixels)"),
			Width, Height, Bitmap.Num());
	}

	// Write save to disk asynchronously
	const FString SlotName = GetSlotName(PendingSaveSlotIndex);

	FAsyncSaveGameToSlotDelegate OnSaveComplete;
	OnSaveComplete.BindLambda([SlotName](const FString& SavedSlotName, const int32 UserIndex, bool bSuccess)
	{
		if (bSuccess)
		{
			UE_LOG(LogSerene, Log, TEXT("Save complete: %s"), *SavedSlotName);
		}
		else
		{
			UE_LOG(LogSerene, Error, TEXT("Save FAILED: %s"), *SavedSlotName);
		}
	});

	UGameplayStatics::AsyncSaveGameToSlot(PendingSaveObject, SlotName, 0, OnSaveComplete);

	// Clear pending save state
	PendingSaveObject = nullptr;
	PendingSaveSlotIndex = -1;
}
