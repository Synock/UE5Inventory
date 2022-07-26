// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/GenericSlotWidget.h"

#include "Interfaces/InventoryPlayerInterface.h"

//----------------------------------------------------------------------------------------------------------------------

void UGenericSlotWidget::UpdateItemImageVisibility()
{
	if (ItemImagePointer)
		ItemImagePointer->SetVisibility(Item.ItemID >= 0 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

//----------------------------------------------------------------------------------------------------------------------

void UGenericSlotWidget::UpdateSlotState()
{
	EnabledSlot = Item.ItemID != 0;
}

//----------------------------------------------------------------------------------------------------------------------

bool UGenericSlotWidget::HandleItemDrop(UItemWidget* InputItem)
{
	return true;
}

IInventoryPlayerInterface* UGenericSlotWidget::GetInventoryPlayerInterface() const
{
	return Cast<IInventoryPlayerInterface>(GetOwningPlayer());
}

//----------------------------------------------------------------------------------------------------------------------

void UGenericSlotWidget::InnerRefresh()
{
	UpdateItemImageVisibility();
	UpdateItemImage();
	UpdateSlotState();
}

//----------------------------------------------------------------------------------------------------------------------

void UGenericSlotWidget::ResetTransaction()
{
	if(IInventoryPlayerInterface* PC = GetInventoryPlayerInterface())
		PC->ResetTransaction();

}

//----------------------------------------------------------------------------------------------------------------------

bool UGenericSlotWidget::CanDropItem(const FInventoryItem& InputItem) const
{
	if (Item.ItemID >= 0 || !EnabledSlot)
	{
		UE_LOG(LogTemp, Log, TEXT("Slot is currently unvailable"));
		return false;
	}

	return true;
}
