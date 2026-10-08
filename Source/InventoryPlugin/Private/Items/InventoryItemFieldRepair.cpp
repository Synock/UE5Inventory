#include "Items/InventoryItemFieldRepair.h"
#include "Items/InventoryItemEquipable.h"

bool UInventoryItemFieldRepair::CanRepairItem(const UInventoryItemBase* TargetItem, float TargetDurability,
                                               float TargetMaxDurability, float KitDurability, FText& OutReason) const
{
	// Null check
	if (!TargetItem)
	{
		OutReason = FText::FromString("No item selected");
		return false;
	}

	// Check if repair kit has charges (durability > 0)
	if (KitDurability <= 0.0f)
	{
		OutReason = FText::FromString("Repair kit has no charges remaining");
		return false;
	}

	// Check if target is equipable (only equipable items can be repaired)
	const UInventoryItemEquipable* EquipableItem = Cast<UInventoryItemEquipable>(TargetItem);
	if (!EquipableItem)
	{
		OutReason = FText::FromString("Item cannot be repaired (not equipable)");
		return false;
	}

	// Calculate durability percentage
	float DurabilityPercent = TargetMaxDurability > 0.0f ? (TargetDurability / TargetMaxDurability) : 0.0f;

	// Check minimum threshold
	if (DurabilityPercent < MinDurabilityThreshold)
	{
		OutReason = FText::FromString(FString::Printf(TEXT("Item is too damaged (below %.0f%%)"),
			MinDurabilityThreshold * 100.0f));
		return false;
	}

	// Check maximum threshold
	if (DurabilityPercent >= MaxDurabilityThreshold)
	{
		OutReason = FText::FromString(FString::Printf(TEXT("Item is above maximum repair threshold (%.0f%%)"),
			MaxDurabilityThreshold * 100.0f));
		return false;
	}

	// Check weapon restriction
	if (WeaponsOnly && !EquipableItem->IsWeapon())
	{
		OutReason = FText::FromString("This repair kit can only repair weapons");
		return false;
	}

	// Check shield restriction
	if (ShieldsOnly && !EquipableItem->IsShield())
	{
		OutReason = FText::FromString("This repair kit can only repair shields");
		return false;
	}

	// Check armor restriction (is equipable but not weapon/shield)
	if (ArmorOnly && (EquipableItem->IsWeapon() || EquipableItem->IsShield()))
	{
		OutReason = FText::FromString("This repair kit can only repair armor");
		return false;
	}

	// Check equipment slot restriction
	if (AllowedEquipmentSlotBitMask != 0)
	{
		// Check if target item's slots overlap with allowed slots
		if ((EquipableItem->GetEquipableSlotBitMask() & AllowedEquipmentSlotBitMask) == 0)
		{
			OutReason = FText::FromString("This repair kit cannot repair items for that equipment slot");
			return false;
		}
	}

	// All checks passed
	OutReason = FText::GetEmpty();
	return true;
}

float UInventoryItemFieldRepair::CalculateRepairAmount(float TargetDurability, float TargetMaxDurability) const
{
	if (TargetMaxDurability <= 0.0f)
		return 0.0f;

	// Random repair amount between min and max percentage
	const float RepairPercent = FMath::RandRange(MinRepairPercentage, MaxRepairPercentage);
	float RepairAmount = TargetMaxDurability * RepairPercent;

	// Calculate what the new durability would be
	const float NewDurability = TargetDurability + RepairAmount;
	const float MaxAllowedDurability = TargetMaxDurability * MaxDurabilityThreshold;

	// Clamp to not exceed the maximum threshold
	if (NewDurability > MaxAllowedDurability)
	{
		RepairAmount = MaxAllowedDurability - TargetDurability;
	}

	return FMath::Max(0.0f, RepairAmount);
}

FText UInventoryItemFieldRepair::GetRepairKitDescription() const
{
return {};
}


