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
		const int32 LocalAcceptableBitMask = std::pow(2., static_cast<double>(i));
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
	/*
	return "Blunt AC: " + FString::FormatAsNumber(ObservedItem->BluntAC) + " Slash AC: " +
		FString::FormatAsNumber(ObservedItem->SlashAC) + " Pierce AC: " + FString::FormatAsNumber(ObservedItem->
			PierceAC);
	*/
	return {};
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetAttributesString() const
{
	FString Out;

	/*
	if (ObservedItem->ModifiedAttributes.Str)
		Out += "STR: " + FString::FormatAsNumber(ObservedItem->ModifiedAttributes.Str) + " ";
	if (ObservedItem->ModifiedAttributes.Sta)
		Out += "STA: " + FString::FormatAsNumber(ObservedItem->ModifiedAttributes.Sta) + " ";
	if (ObservedItem->ModifiedAttributes.Dex)
		Out += "DEX: " + FString::FormatAsNumber(ObservedItem->ModifiedAttributes.Dex) + " ";
	if (ObservedItem->ModifiedAttributes.Agi)
		Out += "AGI: " + FString::FormatAsNumber(ObservedItem->ModifiedAttributes.Agi) + " ";
	if (ObservedItem->ModifiedAttributes.Str)
		Out += "WIS: " + FString::FormatAsNumber(ObservedItem->ModifiedAttributes.Wis) + " ";
	if (ObservedItem->ModifiedAttributes.Int)
		Out += "INT: " + FString::FormatAsNumber(ObservedItem->ModifiedAttributes.Int) + " ";
	if (ObservedItem->ModifiedAttributes.Cha)
		Out += "CHA: " + FString::FormatAsNumber(ObservedItem->ModifiedAttributes.Cha) + " ";
	*/
	return Out;
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
	/*return "Blunt Dmg: " + FString::FormatAsNumber(ObservedItem->BluntDamage) + " Slash Dmg: " +
		FString::FormatAsNumber(ObservedItem->SlashDamage) + " Pierce Dmg: " + FString::FormatAsNumber(ObservedItem->
			PierceDamage) + "\n";*/
	return {};
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetWeaponString() const
{
	/*return GetDamageString() + "\n" +
		"Delay: " + FString::SanitizeFloat(ObservedItem->Delay);*/
	return {};
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetContainerString() const
{
	const UInventoryItemBag* Bag = Cast<UInventoryItemBag>(ObservedItem);

	return "Capacity: " + FString::FormatAsNumber(Bag->BagWidth) + "x" +
		FString::FormatAsNumber(Bag->BagHeight) + " Size capacity:" + UInventoryUtilities::GetItemSizeString(
			Bag->BagSize);
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetFoodString() const
{
	const UInventoryItemActionnable* Actionnable = Cast<UInventoryItemActionnable>(ObservedItem);

	if(!Actionnable)
		return {};
/*
	if (ObservedItem->HungerValue <= 0)
		return {};

	if (ObservedItem->HungerValue <= 120)
		return "This is a snack";

	if (ObservedItem->HungerValue <= 240)
		return "This is a light meal";

	if (ObservedItem->HungerValue <= 480)
		return "This is a meal";

	if (ObservedItem->HungerValue <= 960)
		return "This is a heavy meal";
	*/
	return "This is a feast";
}

//----------------------------------------------------------------------------------------------------------------------

FString UItemDescriptionWidget::GetDrinkString() const
{

	const UInventoryItemActionnable* Actionnable = Cast<UInventoryItemActionnable>(ObservedItem);

	if(!Actionnable)
		return {};
/*
	if (ObservedItem->HungerValue <= 0)
		return {};

	if (ObservedItem->HungerValue <= 120)
		return "This is a tiny beverage";

	if (ObservedItem->HungerValue <= 240)
		return "This is a small beverage";

	if (ObservedItem->HungerValue <= 480)
		return "This is a beverage";

	if (ObservedItem->HungerValue <= 960)
		return "This is a refreshing beverage";
	*/
	return "This is a hydrating beverage";
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
	if (!FoodString.IsEmpty())
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

	/*if (ObservedItem->SpellID != 0)
		Out += GetSpellString();*/

	return Out;
}

UTexture2D* UItemDescriptionWidget::GetTextureIcon() const
{
	return ObservedItem->Icon;
}
