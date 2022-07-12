// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/InventoryEquipmentWidget.h"

#include "Interfaces/InventoryPlayerInterface.h"
#include "UI/ItemWidget.h"

bool UInventoryEquipmentWidget::HandleItemDrop(UItemWidget* InputItem)
{
	if (!InputItem)
		return false;

	IInventoryPlayerInterface* PC = Cast<IInventoryPlayerInterface>(GetOwningPlayer());
	if (!PC)
		return false;

	const int32 ItemID = InputItem->GetReferencedItem().ItemID;

	if (!InputItem->IsBelongingToSelf())
	{
		PC->PlayerAutoLootItem(ItemID, InputItem->GetTopLeftID());
		return true;
	}

	if (!InputItem->IsFromEquipment())
	{
		EEquipmentSlot TargetSlot;
		if (PC->PlayerTryAutoEquip(ItemID, TargetSlot))
		{
			PC->PlayerAutoEquipItem(InputItem->GetTopLeftID(), InputItem->GetBagID(), ItemID);
			return true;
		}
	}

	return false;
}
