// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/ItemDescriptionWidget.h"
#include "InventoryUtilities.h"
#include "Definitions.h"
#include "Items/InventoryItemActionnable.h"
#include "Items/InventoryItemBag.h"
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
	const UInventoryItemBag* Bag = Cast<UInventoryItemBag>(ObservedItem);

	if(!Bag)
		return {};

	return "Capacity: " + FString::FormatAsNumber(Bag->BagWidth) + "x" +
		FString::FormatAsNumber(Bag->BagHeight) + " Size capacity:" + UInventoryUtilities::GetItemSizeString(
			Bag->BagSize);
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

	if (const UInventoryItemBag* Bag = Cast<UInventoryItemBag>(ObservedItem))
		Out += GetContainerString() + "\n";

	if (const UInventoryItemActionnable* Actionnable = Cast<UInventoryItemActionnable>(ObservedItem))
		Out += GetUsableString() + "\n";

	return Out;
}

//----------------------------------------------------------------------------------------------------------------------

UTexture2D* UItemDescriptionWidget::GetTextureIcon() const
{
	return ObservedItem->Icon;
}
