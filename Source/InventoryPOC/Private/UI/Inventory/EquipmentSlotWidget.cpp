// Copyright 2021 Maximilien (Synock) Guislain


#include "UI/Inventory/EquipmentSlotWidget.h"
#include "UI/Inventory/ItemWidget.h"

void UEquipmentSlotWidget::InitData(InventorySlot iSlotID, float iTileSize, AActor* iOwner)
{
	InitBareData(FBareItem(), iOwner, iTileSize);
	PCOwner = Cast<AInventoryPOCCharacter>(iOwner);
	SlotID = iSlotID;

	Refresh();
	PCOwner->GetEquipmentDispatcher().AddDynamic(this, &UEquipmentSlotWidget::Refresh);
}

//----------------------------------------------------------------------------------------------------------------------

void UEquipmentSlotWidget::StopDrag()
{
	Super::StopDrag();
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentSlotWidget::CanEquipItem(const FBareItem& InputItem) const
{
	if(Item.Key >= 0 || !EnabledSlot)
	{
		UE_LOG(LogTemp, Log, TEXT("Slot is currently unvailable"));
		return false;
	}

	if (!CanEquipItemAtSlot(InputItem, SlotID))
		return false;

	//Other check are performed here
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentSlotWidget::CanEquipItemAtSlot(const FBareItem& InputItem, InventorySlot InputSlot)
{
	if (InputItem.Key < 0)
	{
		UE_LOG(LogTemp, Log, TEXT("item is not equippable as it is not a valid item"));
		return false;
	}

	if (!InputItem.Equipable)
	{
		UE_LOG(LogTemp, Log, TEXT("%s cannot be equipped"), *InputItem.Name);
		return false;
	}

	const int32 localAcceptableBitMask = std::pow(2., static_cast<double>(InputSlot));

	if(InputItem.TwoSlotsItem)
	{
		if(InputSlot == InventorySlot::WaistBag2 || InputSlot == InventorySlot::BackPack2)
		{
			return false;
		}
	}

	if (InputItem.EquipableSlotBitMask & localAcceptableBitMask)
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------

bool UEquipmentSlotWidget::TryEquipItem(UItemWidget* InputItem)
{
	if (CanEquipItem(InputItem->GetReferencedItem()))
	{
		return true;
	}
	return false;
}
