// Copyright 2025 Maximilien (Synock) Guislain

#pragma once

#include "UObject/Interface.h"
#include "InventoryItemEquipableInterface.generated.h"

UINTERFACE(MinimalAPI)
class UInventoryItemEquipableInterface : public UInterface
{
	GENERATED_BODY()
};

class INVENTORYPLUGIN_API IInventoryItemEquipableInterface
{
	GENERATED_BODY()

public:
	virtual bool IsEquipable() const = 0;
	virtual int32 GetEquipableSlotBitMask() const = 0;
	virtual bool IsMultiSlotItem() const = 0;
	virtual bool IsShield() const = 0;
	virtual bool IsWeapon() const = 0;
	virtual class USkeletalMesh* GetEquipmentMesh() const = 0;
	virtual const TArray<struct FMaterialOverride>& GetEquipmentMeshMaterialOverride() const = 0;
	virtual bool IsUnsheathable() const = 0;
	virtual float GetTotalDurability() const = 0;
	virtual float GetDurabilityModifier() const = 0;
};

