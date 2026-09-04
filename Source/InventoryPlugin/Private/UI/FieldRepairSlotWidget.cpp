#include "UI/FieldRepairSlotWidget.h"
#include "UI/FieldRepairWidget.h"
#include "UI/ItemWidget.h"
#include "Items/InventoryItemEquipable.h"

bool UFieldRepairSlotWidget::HandleItemDrop(UItemWidget* InputItem)
{
	if (!InputItem)
		return false;

	const UInventoryItemBase* DroppedItem = InputItem->GetReferencedItem();

	// Validate item is equipable
	if (!CanAcceptRepairItem(DroppedItem))
		return false;

	const UInventoryItemEquipable* EquipableItem = Cast<UInventoryItemEquipable>(DroppedItem);
	if (!EquipableItem)
		return false;

	// Get parent repair widget
	UFieldRepairWidget* ParentWidget = GetParentRepairWidget();
	if (!ParentWidget)
		return false;

	// Set the target item in the repair widget
	float ItemDurability = InputItem->GetDurability();
	ParentWidget->SetTargetItem(EquipableItem, ItemDurability);

	// Update this slot's display using the parent's initialization method
	InitBareData(EquipableItem, Owner, TileSize, ItemDurability);

	return true;
}

bool UFieldRepairSlotWidget::CanAcceptRepairItem(const UInventoryItemBase* InputItem) const
{
	if (!InputItem)
		return false;

	// Must be equipable to be repairable
	const UInventoryItemEquipable* EquipableItem = Cast<UInventoryItemEquipable>(InputItem);
	return EquipableItem != nullptr;
}

UFieldRepairWidget* UFieldRepairSlotWidget::GetParentRepairWidget() const
{
	// Search up the widget hierarchy for the repair widget
	UWidget* Parent = GetParent();
	while (Parent)
	{
		if (UFieldRepairWidget* RepairWidget = Cast<UFieldRepairWidget>(Parent))
		{
			return RepairWidget;
		}
		Parent = Parent->GetParent();
	}

	return nullptr;
}

