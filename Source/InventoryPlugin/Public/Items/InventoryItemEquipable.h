// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemBase.h"

#include "InventoryItemEquipable.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UInventoryItemEquipable : public UInventoryItemBase
{
public:
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipable")
	bool Equipable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Bitmask, BitmaskEnum = "/Script/InventoryPlugin.EEquipmentSlot"),
		Category = "Inventory|Equipable")
	int32 EquipableSlotBitMask = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipable")
	bool TwoSlotsItem = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Shield")
	bool Shield = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Weapon")
	bool Weapon = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Visual")
	USkeletalMesh* EquipmentMesh = nullptr;

};
