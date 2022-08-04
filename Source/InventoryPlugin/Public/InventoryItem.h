// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include "Engine/DataTable.h"
#include "Definitions.h"
#include "InventoryItem.generated.h"

USTRUCT(BlueprintType)
struct FMerchantDynamicItemStorage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	int32 ItemID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	int32 Quantity = 0;
};

USTRUCT(BlueprintType)
struct FMinimalItemStorage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	int32 ItemID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	int32 TopLeftID = 0;
};

USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

	FInventoryItem() = default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|ItemData")
	int32 ItemID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|ItemData")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|ItemData")
	uint8 Width = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|ItemData")
	uint8 Height = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Visual")
	FString IconName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Visual")
	FString MeshName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|ItemData")
	EItemSize Size = EItemSize::Tiny;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|ItemData")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|ItemData")
	bool LoreItem = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|ItemData")
	bool MagicItem = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|ItemData")
	float BaseValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|ItemData")
	float Weight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Actionnable")
	bool Activable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Actionnable")
	float HungerValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Actionnable")
	float ThirstValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Actionnable")
	FString BookText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipable")
	bool Equipable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipable")
	bool TwoSlotsItem = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Bitmask, BitmaskEnum = "InventorySlot"),
		Category = "Inventory|Equipable")
	int32 EquipableSlotBitMask = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Bitmask, BitmaskEnum = "RaceId"),
		Category = "Inventory|Equipable")
	int32 RacialRestriction = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Bitmask, BitmaskEnum = "ClassId"),
		Category = "Inventory|Equipable")
	int32 ClassRestriction = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipable")
	FAttributeStruct ModifiedAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipable")
	float BluntAC = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipable")
	float SlashAC = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipable")
	float PierceAC = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipable")
	FString Effect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Shield")
	bool Shield = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Weapon")
	bool Weapon = false;

	//to avoid defining everything we avoid using an include here but rather just a number
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Weapon")
	uint8 MainSkill = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Weapon")
	float BluntDamage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Weapon")
	float SlashDamage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Weapon")
	float PierceDamage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Weapon")
	float Delay = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Weapon")
	float Reach = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Ranged")
	bool Ranged = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Ranged")
	float MaxRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Ranged")
	float InitialVelocity = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Ranged")
	EAmmoType AmmoType = EAmmoType::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Ranged")
    bool Ammo = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Ranged")
	float VelocityMultiplier = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Key")
	bool KeyItem = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Key")
	int32 KeyID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	bool Bag = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	EItemSize BagSize = EItemSize::Giant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	uint8 BagWidth = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	uint8 BagHeight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Spell")
	int32 SpellID = 0;

};
