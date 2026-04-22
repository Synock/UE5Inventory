#include "UI/ItemDescriptionWidget.h"
#include "InventoryUtilities.h"
#include "Definitions.h"
#include "Items/Interfaces/InventoryItemActivatableInterface.h"
#include "Items/Interfaces/InventoryItemBagInterface.h"
#include "Items/InventoryItemWeapon.h"

bool UItemDescriptionWidget::IsLore() const
{
	return ObservedItem->LoreItem;
}

//----------------------------------------------------------------------------------------------------------------------

bool UItemDescriptionWidget::IsMagic() const
{
	return ObservedItem->MagicItem;
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetItemName() const
{
	return ObservedItem->Name;
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetItemDescription() const
{
	return ObservedItem->Description;
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetGeneralString() const
{
	return "Weight: " + FString::SanitizeFloat(ObservedItem->Weight) + "\n" + "Size: " +
		UInventoryUtilities::GetItemSizeString(ObservedItem->ItemSize);
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetSlotString() const
{
	FString Out = "Slot:";
	const UInventoryItemEquipable* Equipable = Cast<UInventoryItemEquipable>(ObservedItem);

	if(!Equipable)
		return {};

	for (size_t i = 1; i < static_cast<size_t>(EEquipmentSlot::Last); ++i)
	{
		const int32 LocalAcceptableBitMask = 1 << static_cast<uint8_t>(i);
		const EEquipmentSlot CurrentSlot = static_cast<EEquipmentSlot>(i);
		if (Equipable->EquipableSlotBitMask & LocalAcceptableBitMask)
		{
			if (CurrentSlot == EEquipmentSlot::WaistBag2 || CurrentSlot == EEquipmentSlot::BackPack2)
				continue;

			Out += " " + UInventoryUtilities::GetSlotName(CurrentSlot);
		}
	}

	return Out;
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetRaceString() const
{
	return {};
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetClassString() const
{
	return {};
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetACString() const
{
	return {};
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetAttributesString() const
{
	return {};
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetEquipableString() const
{
	return GetSlotString() + "\n" + GetRaceString() + "\n" + GetClassString() + "\n" + GetACString() + "\n" +
		GetAttributesString();
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetDamageString() const
{
	return {};
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetWeaponString() const
{
	return {};
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetContainerString() const
{
	const IInventoryItemBagInterface* Bag = Cast<IInventoryItemBagInterface>(ObservedItem);

	if(!Bag)
		return {};

	return "Capacity: " + FString::FormatAsNumber(Bag->GetBagWidth()) + "x" +
		FString::FormatAsNumber(Bag->GetBagHeight()) + " Size capacity:" + UInventoryUtilities::GetItemSizeString(
			Bag->GetBagSize());
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetFoodString() const
{
	return {};
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetDrinkString() const
{
	return {};
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetUsableString() const
{
	FString Out;
	FString FoodString = GetFoodString();
	if (!FoodString.IsEmpty())
	{
		Out += FoodString + "\n";
	}

	FString DrinkString = GetDrinkString();
	if (!DrinkString.IsEmpty())
	{
		Out += DrinkString;
	}

	return Out;
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetSpellString() const
{
	return {};
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetGlobalString() const
{
	FString Out =
		GetItemDescription() + "\n" +
		GetGeneralString() + "\n";

	if (const UInventoryItemEquipable* Equipable = Cast<UInventoryItemEquipable>(ObservedItem))
		Out += GetEquipableString() + "\n";

	if (const UInventoryItemWeapon* Weapon = Cast<UInventoryItemWeapon>(ObservedItem))
		Out += GetWeaponString() + "\n";

	if (const IInventoryItemBagInterface* Bag = Cast<IInventoryItemBagInterface>(ObservedItem))
		Out += GetContainerString() + "\n";

	// Check if item is activatable (food, drink, etc.)
	if (const IInventoryItemActivatableInterface* Activatable = Cast<IInventoryItemActivatableInterface>(ObservedItem))
		Out += GetUsableString() + "\n";

	return Out;
}

//----------------------------------------------------------------------------------------------------------------------

UTexture2D* UItemDescriptionWidget::GetTextureIcon() const
{
	return ObservedItem->Icon;
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetDurabilityConditionString() const
{
	const float DurabilityPercent = GetDurabilityPercentage();

	if (DurabilityPercent >= 100.0f)
		return "Pristine";
	if (DurabilityPercent >= 75.0f)
		return "Pristine";
	if (DurabilityPercent >= 50.0f)
		return "Good";
	if (DurabilityPercent >= 25.0f)
		return "Worn";
	if (DurabilityPercent > 0.0f)
		return "Tattered";
	return "Broken";
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetDurabilityString() const
{
	return "Condition: " + GetDurabilityConditionString();
}

//----------------------------------------------------------------------------------------------------------------------
// IInventoryItemDescriptionWidgetInterface
//----------------------------------------------------------------------------------------------------------------------

void UItemDescriptionWidget::InitDescription_Implementation(const UInventoryItemBase* Item)
{
	ObservedItem = const_cast<UInventoryItemBase*>(Item);
	ItemDurability = ItemMaxDurability;
	OnDescriptionPopulated();
}

//----------------------------------------------------------------------------------------------------------------------

void UItemDescriptionWidget::InitDescriptionWithDurability_Implementation(const UInventoryItemBase* Item,
                                                                           float Durability, float MaxDurability)
{
	ObservedItem = const_cast<UInventoryItemBase*>(Item);
	SetItemDurabilityWithMax(Durability, MaxDurability);
	OnDescriptionPopulated();
}
