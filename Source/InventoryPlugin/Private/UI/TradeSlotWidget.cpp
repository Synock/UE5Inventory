#include "UI/TradeSlotWidget.h"

#include "Items/InventoryItemBase.h"
#include "UI/ItemWidget.h"
#include "UI/TradeWidget.h"
#include "Definitions.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Interfaces/InventoryPlayerInterface.h"

void UTradeSlotWidget::InitializeSlot(int32 InSlotIndex, bool bInIsOurSlot)
{
	SlotIndex = InSlotIndex;
	bIsOurSlot = bInIsOurSlot;

	// Clear the slot initially
	ClearSlot();

	// Note: We don't disable the widget for their slots anymore
	// This allows right-click inspection to work
	// Drop prevention is handled in NativeOnDrop
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeSlotWidget::SetTradeItem(const UInventoryItemBase* ItemData, AActor* OwnerActor,
                                    EBagSlot InSourceBagSlot, int32 InSourceTopLeft, float InDurability)
{
	if (!ItemData)
	{
		ClearSlot();
		return;
	}

	// Store source information for potential drag operations
	SourceBagSlot = InSourceBagSlot;
	SourceTopLeft = InSourceTopLeft;
	ItemDurability = InDurability;

	// Use inherited InitBareData from UItemBaseWidget to set up the item display
	// This automatically handles:
	// - Item icon display
	// - Durability display
	// - Item reference storage
	// - UI updates
	InitBareData(ItemData, OwnerActor, 40.f, InDurability);
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeSlotWidget::ClearSlot()
{
	// Clear source information
	SourceBagSlot = EBagSlot::Unknown;
	SourceTopLeft = -1;
	ItemDurability = 100.0f;

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

	// CLIENT-SIDE VALIDATION: Check if item is from equipment slot
	if (BagSlot == EBagSlot::Unknown)
	{
		// Item is equipped - cannot trade
		// Broadcast error to parent TradeWidget so game side can display message
		UTradeWidget* ParentTradeWidget = Cast<UTradeWidget>(GetOuter());
		if (!ParentTradeWidget)
		{
			// Try to find it in the widget tree
			UUserWidget* Parent = GetTypedOuter<UUserWidget>();
			while (Parent && !ParentTradeWidget)
			{
				ParentTradeWidget = Cast<UTradeWidget>(Parent);
				if (!ParentTradeWidget)
				{
					Parent = Parent->GetTypedOuter<UUserWidget>();
				}
			}
		}

		if (ParentTradeWidget)
		{
			ParentTradeWidget->OnTradeValidationError.Broadcast(
				TEXT("You cannot trade equipped items. Unequip the item first."));
		}

		return false; // Reject the drop
	}

	// Get the player controller
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	// Get the inventory player interface
	IInventoryPlayerInterface* PlayerInterface = Cast<IInventoryPlayerInterface>(PC);
	if (!PlayerInterface)
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	// Add the item to trade (server will validate again)
	PlayerInterface->PlayerAddItemToTrade(ItemID, BagSlot, TopLeft);

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

FReply UTradeSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Allow right-click inspection on both our items and their items
	// Right-click should work even if the slot is disabled for drops
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		// Call parent class to handle right-click display
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	// Handle left-click to initiate drag for items from our trade slots
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// Only allow drag from our own slots, and only if there's an item in the slot
		if (bIsOurSlot && GetReferencedItem() != nullptr)
		{
			// Initiate drag detection - this will call NativeOnDragDetected when drag threshold is met
			return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
		}
	}

	// For other mouse buttons, use default behavior (which respects IsEnabled)
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

//----------------------------------------------------------------------------------------------------------------------

void UTradeSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                            UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	// Only create drag operation for our own slots with items
	if (!bIsOurSlot || !GetReferencedItem())
		return;

	// Validate source information
	if (SourceBagSlot == EBagSlot::Unknown || SourceTopLeft < 0)
		return;

	// Get the player controller
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
		return;

	IInventoryPlayerInterface* PlayerInterface = Cast<IInventoryPlayerInterface>(PC);
	if (!PlayerInterface)
		return;

	// Create an ItemWidget for the drag operation
	UItemWidget* DraggedItemWidget = CreateWidget<UItemWidget>(PC, UItemWidget::StaticClass());
	if (!DraggedItemWidget)
		return;

	// Initialize the ItemWidget with data from the trade slot
	DraggedItemWidget->InitData(
		GetReferencedItem(),           // Item data (already stored in this widget)
		GetOwningPlayer(),          // Owner actor
		40.f,                          // Tile size
		SourceTopLeft,                 // Original inventory position
		SourceBagSlot,                 // Original bag
		EEquipmentSlot::Unknown,       // Not from equipment (already validated when added to trade)
		ItemDurability                 // Item durability
	);

	// Create the drag-drop operation
	UDragDropOperation* DragDropOp = UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass());
	if (DragDropOp)
	{
		DragDropOp->Payload = DraggedItemWidget;
		DragDropOp->DefaultDragVisual = DraggedItemWidget;
		DragDropOp->Pivot = EDragPivot::MouseDown;

		OutOperation = DragDropOp;

		// Remove the item from trade now that drag has started
		// This will update both players and clear the slot
		PlayerInterface->PlayerRemoveItemFromTrade(SlotIndex);
	}
}

