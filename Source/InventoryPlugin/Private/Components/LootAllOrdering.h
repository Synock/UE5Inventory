#pragma once

#include "InventoryItem.h"
#include "Items/InventoryItemEquipable.h"
#include "Items/Interfaces/InventoryItemBagInterface.h"

namespace LootAllOrdering
{
inline TArray<FMinimalItemStorage> Build(TArray<FMinimalItemStorage> Items,
	TFunctionRef<const UInventoryItemBase*(int32)> FindItem)
{
	Items.StableSort([](const FMinimalItemStorage& Left, const FMinimalItemStorage& Right)
	{
		return Left.TopLeftID < Right.TopLeftID;
	});

	TArray<FMinimalItemStorage> Equipment;
	TArray<FMinimalItemStorage> OrdinaryItems;
	int32 FirstDependentIndex = INDEX_NONE;
	const uint32 WaistBit = 1u << static_cast<uint32>(EEquipmentSlot::Waist);

	for (const FMinimalItemStorage& Storage : Items)
	{
		const UInventoryItemEquipable* Item = Cast<UInventoryItemEquipable>(FindItem(Storage.ItemID));
		if (!Item)
		{
			OrdinaryItems.Add(Storage);
			continue;
		}

		const bool bSupportsWaist = (static_cast<uint32>(Item->GetEquipableSlotBitMask()) & WaistBit) != 0;
		if (bSupportsWaist && FirstDependentIndex != INDEX_NONE)
		{
			// Move waist support only far enough to precede bags and weapons.
			Equipment.Insert(Storage, FirstDependentIndex);
			++FirstDependentIndex;
			continue;
		}

		const IInventoryItemBagInterface* Bag = Cast<IInventoryItemBagInterface>(Item);
		const bool bDependsOnWaist = !bSupportsWaist && (Item->IsWeapon() || (Bag && Bag->IsBag()));
		if (bDependsOnWaist && FirstDependentIndex == INDEX_NONE)
			FirstDependentIndex = Equipment.Num();

		Equipment.Add(Storage);
	}

	Equipment.Append(OrdinaryItems);
	return Equipment;
}
}
