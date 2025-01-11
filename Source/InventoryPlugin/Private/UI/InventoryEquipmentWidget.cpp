// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/InventoryEquipmentWidget.h"

#include "Interfaces/InventoryPlayerInterface.h"
#include "Items/InventoryItemBase.h"
#include "UI/EquipmentSlotWidget.h"
#include "UI/ItemWidget.h"

bool UInventoryEquipmentWidget::HandleItemDrop(UItemWidget* InputItem)
{
	if (!InputItem)
		return false;

	IInventoryPlayerInterface* PC = Cast<IInventoryPlayerInterface>(GetOwningPlayer());
	if (!PC)
		return false;

	const int32 ItemID = InputItem->GetReferencedItem()->ItemID;

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

void UInventoryEquipmentWidget::RegisterSlotWidget(UEquipmentSlotWidget* NewSlotWidget)
{
	if (!KnownEquipmentSlot.Contains(NewSlotWidget->GetSlotID()))
	{
		KnownEquipmentSlot.Add(NewSlotWidget->GetSlotID(), NewSlotWidget);
		NewSlotWidget->SetParentComponent(this);

	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Trying to register a slot that is already known : %d"), NewSlotWidget->GetSlotID());
	}
}

void UInventoryEquipmentWidget::ForceRefresh()
{
	for(auto& CurrentSlot : KnownEquipmentSlot)
	{
		CurrentSlot.Value->Refresh();
	}
}

UEquipmentSlotWidget* UInventoryEquipmentWidget::GetSlotWidget(EEquipmentSlot WantedSlot) const
{
	return KnownEquipmentSlot.FindRef(WantedSlot);
}
