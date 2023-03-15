// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/StagingAreaSlotWidget.h"

#include "Interfaces/InventoryPlayerInterface.h"
#include "Items/InventoryItemBase.h"
#include "UI/ItemWidget.h"

bool UStagingAreaSlotWidget::HandleItemDrop(UItemWidget* InputItem)
{
	if (!InputItem)
		return false;

	if (!EnabledSlot)
		return false;

	const int32 ItemID = InputItem->GetReferencedItem()->ItemID;

	IInventoryPlayerInterface* PC = GetInventoryPlayerInterface();
	if (!PC)
		return false;

	if (!InputItem->IsBelongingToSelf()) // Can't Stage from loot
		return false;

	if (InputItem->IsFromEquipment())
		PC->MoveEquipmentToStagingArea(ItemID, InputItem->GetOriginalSlot());

	else
		PC->MoveInventoryItemToStagingArea(ItemID, InputItem->GetTopLeftID(), InputItem->GetBagID());

	return true;
}

