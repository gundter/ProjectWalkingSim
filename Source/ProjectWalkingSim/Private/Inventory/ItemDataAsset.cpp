// Copyright Null Lantern.

#include "Inventory/ItemDataAsset.h"

FPrimaryAssetId UItemDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("Item", GetFName());
}
