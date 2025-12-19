#include "UI/TradeSlotWidget.h"

#include "Items/InventoryItemBase.h"

void UTradeSlotWidget::InitializeSlot(int32 InSlotIndex, bool bInIsOurSlot)
{
	SlotIndex = InSlotIndex;
	bIsOurSlot = bInIsOurSlot;

	// Clear the slot initially
	ClearSlot();

	// Make their slots non-interactive
	if (!bIsOurSlot)
	{
		SetIsEnabled(false);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeSlotWidget::SetTradeItem(const UInventoryItemBase* ItemData, AActor* OwnerActor)
{
	if (!ItemData)
	{
		ClearSlot();
		return;
	}

	// Use inherited InitBareData from UItemBaseWidget to set up the item display
	// This automatically handles:
	// - Item icon display
	// - Durability display
	// - Item reference storage
	// - UI updates
	InitBareData(ItemData, OwnerActor, 40.f);
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeSlotWidget::ClearSlot()
{
	// Clear by passing nullptr
	InitBareData(nullptr, nullptr, 40.f);
}

//----------------------------------------------------------------------------------------------------------------------

int32 UTradeSlotWidget::GetCurrentItemID() const
{
	// Get the item from the inherited Item property
	const UInventoryItemBase* CurrentItem = GetReferencedItem();
	return CurrentItem ? CurrentItem->ItemID : 0;
}
