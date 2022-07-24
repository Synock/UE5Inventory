// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Definitions.generated.h"

UENUM(BlueprintType)
enum class ECurrencyType : uint8
{
	Copper UMETA(DisplayName = "Copper"),
	Silver UMETA(DisplayName = "Silver"),
	Gold UMETA(DisplayName = "Gold"),
	Platinum UMETA(DisplayName = "Platinum")
};

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	Unknown UMETA(DisplayName = "Unknown"),
	Throwable UMETA(DisplayName = "Throwable"),
	Bolts UMETA(DisplayName = "Bolts"),
	Arrows UMETA(DisplayName = "Arrows")
};

UENUM(BlueprintType)
enum class EEquipmentSocket : uint8
{
	Unknown = 0,
	Primary = 1,
	Secondary = 2,
	AmmoBag = 3,
	WaistBag1,
	WaistBag2,
	ShoulderBag1,
	ShoulderBag2,
	Backpack,
	PrimarySheath,
	SecondarySheath,
	BackSheath
};

UENUM(BlueprintType, Meta = (Bitflags))
enum class EEquipmentSlot : uint8
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
	BackPack2 = 25,
	Last
};

UENUM(BlueprintType, Meta = (Bitflags))
enum class EBagSlot : uint8
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

UENUM(BlueprintType)
enum class EItemSize : uint8
{
	Tiny UMETA(DisplayName = "Tiny"),
	Small UMETA(DisplayName = "Small"),
	Medium UMETA(DisplayName = "Medium"),
	Large UMETA(DisplayName = "Large"),
	Giant UMETA(DisplayName = "Giant")
};

USTRUCT(BlueprintType)
struct INVENTORYPLUGIN_API FAttributeStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	int Str = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	int Sta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	int Agi = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	int Dex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	int Int = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	int Wis = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	int Cha = 0;


	FAttributeStruct& operator+=(const FAttributeStruct& OtherAttribute);

	int Sum() const;
};