// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Definitions.h"
#include "Items/InventoryItemEquipable.h"
#include "UObject/Interface.h"
#include "InventoryModularCharacterInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInventoryModularCharacterInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class INVENTORYPLUGIN_API IInventoryModularCharacterInterface
{
	GENERATED_BODY()
public:

	virtual USkeletalMeshComponent* GetHeadComponent() = 0;
	virtual USkeletalMeshComponent* GetTorsoComponent() = 0;
	virtual USkeletalMeshComponent* GetArmsComponent() = 0;
	virtual USkeletalMeshComponent* GetHandsComponent() = 0;
	virtual USkeletalMeshComponent* GetLegsComponent() = 0;
	virtual USkeletalMeshComponent* GetFootComponent() = 0;

	virtual void SetEquipment(const UInventoryItemEquipable* Item, EEquipmentSlot Slot);

	virtual USkeletalMeshComponent* GetEquipmentComponentFromSlot(EEquipmentSlot Slot);
};
