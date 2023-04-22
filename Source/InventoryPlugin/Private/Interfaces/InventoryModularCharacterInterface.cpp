// Copyright 2023 Maximilien (Synock) Guislain


#include "Interfaces/InventoryModularCharacterInterface.h"


// Add default functionality here for any IInventoryModularCharacterInterface functions that are not pure virtual.
void IInventoryModularCharacterInterface::SetEquipment(const UInventoryItemEquipable* Item, EEquipmentSlot Slot)
{
	if(!Item)
		return;

	if(!Item->EquipmentMesh)
		return;

	if(Slot == EEquipmentSlot::Torso)
		GetTorsoComponent()->SetSkeletalMeshAsset(Item->EquipmentMesh);

	if(Slot == EEquipmentSlot::Arms)
		GetArmsComponent()->SetSkeletalMeshAsset(Item->EquipmentMesh);

	if(Slot == EEquipmentSlot::Foot)
		GetFootComponent()->SetSkeletalMeshAsset(Item->EquipmentMesh);

	if(Slot == EEquipmentSlot::Hands)
		GetHandsComponent()->SetSkeletalMeshAsset(Item->EquipmentMesh);

	if(Slot == EEquipmentSlot::Legs)
		GetLegsComponent()->SetSkeletalMeshAsset(Item->EquipmentMesh);

	if(Slot == EEquipmentSlot::Head)
		GetHeadComponent()->SetSkeletalMeshAsset(Item->EquipmentMesh);
}
