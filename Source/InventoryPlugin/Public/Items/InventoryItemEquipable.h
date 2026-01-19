// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemBase.h"
#include "Items/Interfaces/InventoryItemEquipableInterface.h"
#include "Items/Interfaces/InventoryItemWeaponInterface.h"

#include "InventoryItemEquipable.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UInventoryItemEquipable : public UInventoryItemBase, public IInventoryItemEquipableInterface, public IInventoryItemWeaponInterface
{
public:
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipable")
	bool Equipable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Bitmask, BitmaskEnum = "/Script/InventoryPlugin.EEquipmentSlot"),
		Category = "Inventory|Equipable")
	int32 EquipableSlotBitMask = 0;

	/// If true, this item will take all the slots defined in the EquipableSlotBitMask bitmask.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipable")
	bool MultiSlotItem = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Shield")
	bool Shield = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Weapon")
	bool Weapon = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Equipable|Visual")
	USkeletalMesh* EquipmentMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Equipable|Visual")
	TArray<FMaterialOverride> EquipmentMeshMaterialOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Weapon")
	bool Unsheathable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipable|Durability")
	float TotalDurability = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipable|Durability")
	float DurabilityModifier = 1.0f;

	virtual bool IsEquipable() const override { return Equipable; }
	virtual int32 GetEquipableSlotBitMask() const override { return EquipableSlotBitMask; }
	virtual bool IsMultiSlotItem() const override { return MultiSlotItem; }
	virtual bool IsShield() const override { return Shield; }
	virtual bool IsWeapon() const override { return Weapon; }
	virtual USkeletalMesh* GetEquipmentMesh() const override { return EquipmentMesh; }
	virtual const TArray<FMaterialOverride>& GetEquipmentMeshMaterialOverride() const override { return EquipmentMeshMaterialOverride; }
	virtual bool IsUnsheathable() const override { return Unsheathable; }
	virtual float GetTotalDurability() const override { return TotalDurability; }
	virtual float GetDurabilityModifier() const override { return DurabilityModifier; }

};
