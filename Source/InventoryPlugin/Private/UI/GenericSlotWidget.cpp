// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/GenericSlotWidget.h"

#include "Interfaces/InventoryPlayerInterface.h"
#include "UI/ItemWidget.h"

//----------------------------------------------------------------------------------------------------------------------

void UGenericSlotWidget::UpdateItemImageVisibility()
{
	if (ItemImagePointer)
		ItemImagePointer->SetVisibility(Item ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

//----------------------------------------------------------------------------------------------------------------------

void UGenericSlotWidget::UpdateSlotState()
{
	EnabledSlot = true;//Item.ItemID != 0;
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
	if(EnabledSlot)
	{
		UpdateItemImageVisibility();
		UpdateItemImage();
		UpdateSlotState();
	}

}

//----------------------------------------------------------------------------------------------------------------------

void UGenericSlotWidget::ResetTransaction()
{
	if (IInventoryPlayerInterface* PC = GetInventoryPlayerInterface())
		PC->ResetTransaction();
}

//----------------------------------------------------------------------------------------------------------------------

void UGenericSlotWidget::HideItem()
{
	Item = nullptr;
	UGenericSlotWidget::InnerRefresh();
}

//----------------------------------------------------------------------------------------------------------------------

bool UGenericSlotWidget::CanDropItem(const UInventoryItemBase* InputItem) const
{
	if (Item || !EnabledSlot)
	{
		UE_LOG(LogTemp, Log, TEXT("Slot is currently unvailable"));
		return false;
	}

	return true;
}
