// Copyright 2023 Maximilien (Synock) Guislain


#include "Interfaces/InventoryModularCharacterInterface.h"


void IInventoryModularCharacterInterface::SetEquipment(const UInventoryItemEquipable* Item, EEquipmentSlot Slot)
{
	if (!Item)
		return;

	if (!Item->EquipmentMesh)
		return;

	if (auto SkeletaComponent = GetEquipmentComponentFromSlot(Slot))
	{
		SkeletaComponent->SetSkeletalMeshAsset(Item->EquipmentMesh);

		if (Item->EquipmentMeshMaterialOverride.OverrideMaterial)
		{
			SkeletaComponent->SetMaterial(Item->EquipmentMeshMaterialOverride.MaterialID,
			                              Item->EquipmentMeshMaterialOverride.OverrideMaterial);
		}
	}
}

USkeletalMeshComponent* IInventoryModularCharacterInterface::GetEquipmentComponentFromSlot(EEquipmentSlot Slot)
{
	if (Slot == EEquipmentSlot::Torso)
		return GetTorsoComponent();

	if (Slot == EEquipmentSlot::Arms)
		return GetArmsComponent();

	if (Slot == EEquipmentSlot::Foot)
		return GetFootComponent();

	if (Slot == EEquipmentSlot::Hands)
		return GetHandsComponent();

	if (Slot == EEquipmentSlot::Legs)
		return GetLegsComponent();

	if (Slot == EEquipmentSlot::Head)
		return GetHeadComponent();

	return nullptr;
}
