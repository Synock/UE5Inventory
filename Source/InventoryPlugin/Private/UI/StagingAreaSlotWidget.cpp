
#include "UI/StagingAreaSlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"
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

void UStagingAreaSlotWidget::InitializeStagedItem(const FInventoryEscrowItem& InStagedItem,
	const UInventoryItemBase* InItem, AActor* InOwner, float InTileSize)
{
	StagedItem = InStagedItem;
	InitBareData(InItem, InOwner, InTileSize, InStagedItem.Durability);
	SetToolTipText(InItem ? FText::FromString(InItem->Name) : FText::GetEmpty());
	Refresh();
}

void UStagingAreaSlotWidget::ClearStagedItem(float InTileSize)
{
	StagedItem = FInventoryEscrowItem();
	InitBareData(nullptr, nullptr, InTileSize);
	SetToolTipText(FText::GetEmpty());
	Refresh();
}

FReply UStagingAreaSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && Item && StagedItem.IsValid())
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UStagingAreaSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	if (!Item || !StagedItem.IsValid())
		return;

	APlayerController* PC = GetOwningPlayer();
	IInventoryPlayerInterface* Player = GetInventoryPlayerInterface();
	if (!PC || !Player)
		return;

	const bool bFromEquipment =
		StagedItem.Source.Kind == EInventoryDeliveryDestinationKind::Equipment;
	UItemWidget* DraggedItem = CreateWidget<UItemWidget>(PC, UItemWidget::StaticClass());
	if (!DraggedItem)
		return;

	DraggedItem->InitData(Item, PC, TileSize,
		bFromEquipment ? 0 : StagedItem.Source.TopLeft,
		bFromEquipment ? EBagSlot::Unknown : StagedItem.Source.Bag,
		bFromEquipment ? StagedItem.Source.EquipmentSlot : EEquipmentSlot::Unknown,
		StagedItem.Durability);

	UDragDropOperation* Operation =
		UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass());
	if (!Operation)
		return;

	Operation->Payload = DraggedItem;
	Operation->DefaultDragVisual = DraggedItem;
	Operation->Pivot = EDragPivot::MouseDown;
	OutOperation = Operation;

	// Reliable RPC ordering restores the item first; a subsequent drop can then
	// move it from its original source using the regular inventory operations.
	Player->ReturnStagingItem(StagedItem.ReservationId);
}

