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

	/// these are character component that get replaced by stuff
	virtual USkeletalMeshComponent* GetHeadComponent() = 0;
	virtual USkeletalMeshComponent* GetTorsoComponent() = 0;
	virtual USkeletalMeshComponent* GetArmsComponent() = 0;
	virtual USkeletalMeshComponent* GetHandsComponent() = 0;
	virtual USkeletalMeshComponent* GetLegsComponent() = 0;
	virtual USkeletalMeshComponent* GetFootComponent() = 0;

	/// these are stuff that get on top of other stuff
	virtual USkeletalMeshComponent* GetHelmetComponent();
	virtual USkeletalMeshComponent* GetShoulderPadComponent();
	virtual USkeletalMeshComponent* GetNeckComponent();
	virtual USkeletalMeshComponent* GetRightBracerComponent();
	virtual USkeletalMeshComponent* GetLeftBracerComponent();
	virtual USkeletalMeshComponent* GetBackComponent();

	virtual void SetEquipment(const UInventoryItemEquipable* Item, EEquipmentSlot Slot);

	virtual USkeletalMeshComponent* GetEquipmentComponentFromSlot(EEquipmentSlot Slot);

	virtual bool IsBodyPart(EEquipmentSlot Slot);

	virtual bool IsEquipmentPart(EEquipmentSlot Slot);

};
