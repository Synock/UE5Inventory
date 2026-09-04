
#include "UI/InventoryEquipmentWidget.h"

#include "InventoryPlugin.h"
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
		UE_LOG(LogInventoryPlugin, Verbose, TEXT("RegisterSlotWidget: slot %d already known — skipping"), static_cast<int32>(NewSlotWidget->GetSlotID()));
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
