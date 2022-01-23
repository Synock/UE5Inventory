// Copyright 2021 Maximilien (Synock) Guislain


#pragma once

#include "CoreMinimal.h"
#include "BareItem.generated.h"

UENUM(BlueprintType)
enum class EItemSize : uint8
{
	Tiny UMETA(DisplayName = "Tiny"),
	Small UMETA(DisplayName = "Small"),
	Medium UMETA(DisplayName = "Medium"),
	Large UMETA(DisplayName = "Large"),
	Giant UMETA(DisplayName = "Giant")
};

UENUM(BlueprintType, Meta = (Bitflags))
enum class InventorySlot : uint8
{
	Unknown = 0,
	Primary = 1,
	Secondary = 2,
	Range = 3,
	Ammo = 4,
	Head = 5,
	Face = 6,
	EarL = 7,
	EarR = 8,
	Neck = 9,
	Shoulders = 10,
	Back = 11,
	Torso = 12,
	WristL = 13,
	WristR = 14,
	Hands = 15,
	FingerL = 16,
	FingerR = 17,
	Waist = 18,
	Legs = 19,
	Foot = 20,
	Arms = 21,
	WaistBag1 = 22,
	WaistBag2 = 23,
	BackPack1 = 24,
	BackPack2 = 25
};

UENUM(BlueprintType, Meta = (Bitflags))
enum class BagSlot : uint8
{
	Unknown = 0,
	Pocket1 = 1 UMETA(DisplayName = "Pocket1"),
	Pocket2 = 2 UMETA(DisplayName = "Pocket2"),
	WaistBag1 = 3 UMETA(DisplayName = "WaistBag1"),
	WaistBag2 = 4 UMETA(DisplayName = "WaistBag2"),
	BackPack1 = 5 UMETA(DisplayName = "Backpack1"),
	BackPack2 = 6 UMETA(DisplayName = "Backpack2"),
	LastValidBag = 7,

	LootPool = 20 UMETA(DisplayName = "Lootpool")
};

USTRUCT(BlueprintType)
struct FMerchantDynamicItemStorage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	int64 ItemID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	int32 Quantity = 0;
};

USTRUCT(BlueprintType)
struct FMinimalItemStorage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	int64 ItemID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	int32 TopLeftID = 0;
};

USTRUCT(BlueprintType)
struct FBareItem
{
	GENERATED_BODY()

	FBareItem() = default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nerverquest|ItemData")
	int64 Key = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nerverquest|ItemData")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nerverquest|ItemData")
	int Width = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nerverquest|ItemData")
	int Height = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nerverquest|Visual")
	FString IconName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nerverquest|ItemData")
	EItemSize Size = EItemSize::Tiny;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nerverquest|ItemData")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nerverquest|ItemData")
	float BaseValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nerverquest|ItemData")
	float Weight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nerverquest|Equipable")
	bool Equipable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nerverquest|Equipable")
	bool TwoSlotsItem = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Bitmask, BitmaskEnum = "InventorySlot"), Category = "Nerverquest|Equipable")
	int32 EquipableSlotBitMask = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nerverquest|Bag")
	bool Bag = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nerverquest|Bag")
	EItemSize BagSize = EItemSize::Giant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nerverquest|Bag")
	int BagWidth = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nerverquest|Bag")
	int BagHeight = 1;
};
