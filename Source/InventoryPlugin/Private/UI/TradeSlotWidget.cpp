#include "UI/TradeSlotWidget.h"

#include "Items/InventoryItemBase.h"
#include "UI/ItemWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "Interfaces/InventoryPlayerInterface.h"

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

//----------------------------------------------------------------------------------------------------------------------

bool UTradeSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                    UDragDropOperation* InOperation)
{
	// Only our slots can accept drops
	if (!bIsOurSlot || !EnabledSlot)
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	// Check if an ItemWidget is being dropped
	UItemWidget* DroppedItemWidget = Cast<UItemWidget>(InOperation->Payload);
	if (!DroppedItemWidget)
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	// Check if the item belongs to the player (not from loot, merchant, etc.)
	if (!DroppedItemWidget->IsBelongingToSelf())
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	// Get the item information
	const int32 ItemID = DroppedItemWidget->GetReferencedItem()->ItemID;
	const EBagSlot BagSlot = DroppedItemWidget->GetBagID();
	const int32 TopLeft = DroppedItemWidget->GetTopLeftID();

	// Get the player controller
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	// Get the inventory player interface
	IInventoryPlayerInterface* PlayerInterface = Cast<IInventoryPlayerInterface>(PC);
	if (!PlayerInterface)
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	// Add the item to trade
	PlayerInterface->PlayerAddItemToTrade(ItemID, BagSlot, TopLeft);

	return true;
}

