#include "Interfaces/RepairInterface.h"
#include "Components/RepairComponent.h"
#include "InventoryUtilities.h"
#include "Items/InventoryItemEquipable.h"

FCoinValue IRepairInterface::CalculateRepairCost(int32 ItemID, float CurrentDurability, float MaxDurability) const
{
	const URepairComponent* RepairComp = GetRepairComponentConst();
	if (!RepairComp)
	{
		return FCoinValue{0, 0, 0, 0};
	}

	// Get the item base value
	const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemID, GetRepairerWorldContext());
	if (!Item)
	{
		return FCoinValue{0, 0, 0, 0};
	}

	return RepairComp->CalculateSingleItemRepairCost(Item->BaseValue, CurrentDurability, MaxDurability);
}

FCoinValue IRepairInterface::CalculateRepairAllCost(TArray<UInventoryItemEquipable*>& Equipment,
                                                     const TArray<float>& EquipmentDurability) const
{
	const URepairComponent* RepairComp = GetRepairComponentConst();
	if (!RepairComp || Equipment.Num() != EquipmentDurability.Num())
	{
		return FCoinValue{0, 0, 0, 0};
	}

	FCoinValue TotalCost{0, 0, 0, 0};

	for (int32 i = 0; i < Equipment.Num(); ++i)
	{
		if (const UInventoryItemEquipable* Item = Equipment[i])
		{
			const float CurrentDurability = EquipmentDurability[i];
			const float MaxDurability = Item->TotalDurability;

			// Only calculate cost for damaged items
			if (CurrentDurability < MaxDurability)
			{
				const FCoinValue ItemRepairCost = RepairComp->CalculateSingleItemRepairCost(
					Item->BaseValue, CurrentDurability, MaxDurability);

				TotalCost += ItemRepairCost;
			}
		}
	}

	return TotalCost;
}


