#include "Items/Interfaces/InventoryItemInterface.h"
#include "Items/InventoryItemBase.h"


// Add default functionality here for any IInventoryItemInferface functions that are not pure virtual.
EItemType IInventoryItemInterface::GetItemType() const
{
	return EItemType::Unknown;
}
