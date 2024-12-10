// Copyright 2023 Maximilien (Synock) Guislain


#include "Interfaces/InventoryModularCharacterInterface.h"


USkeletalMeshComponent* IInventoryModularCharacterInterface::GetHelmetComponent()
{
	return nullptr;
}

USkeletalMeshComponent* IInventoryModularCharacterInterface::GetShoulderPadComponent()
{
	return nullptr;
}

USkeletalMeshComponent* IInventoryModularCharacterInterface::GetNeckComponent()
{
	return nullptr;
}

USkeletalMeshComponent* IInventoryModularCharacterInterface::GetBackComponent()
{
	return nullptr;
}

USkeletalMeshComponent* IInventoryModularCharacterInterface::GetRightBracerComponent()
{
	return nullptr;
}

USkeletalMeshComponent* IInventoryModularCharacterInterface::GetLeftBracerComponent()
{
	return nullptr;
}

void IInventoryModularCharacterInterface::SetEquipment(const UInventoryItemEquipable* Item, EEquipmentSlot Slot)
{
	if (!Item)
		return;

	if (!Item->EquipmentMesh)
		return;

	if (auto SkeletaComponent = GetEquipmentComponentFromSlot(Slot))
	{
		SkeletaComponent->SetSkeletalMeshAsset(Item->EquipmentMesh);

		for (auto& Material : Item->EquipmentMeshMaterialOverride)
		{
			SkeletaComponent->SetMaterial(Material.MaterialID, Material.OverrideMaterial);
		}
	}
}

USkeletalMeshComponent* IInventoryModularCharacterInterface::GetEquipmentComponentFromSlot(EEquipmentSlot Slot)
{
	switch (Slot)
	{
	case EEquipmentSlot::Head:
		return GetHelmetComponent();
	case EEquipmentSlot::Face:
		break;
	case EEquipmentSlot::Neck:
		return GetNeckComponent();
	case EEquipmentSlot::Shoulders:
		return GetShoulderPadComponent();
	case EEquipmentSlot::Back:
		return GetBackComponent();
	case EEquipmentSlot::Torso:
		return GetTorsoComponent();
	case EEquipmentSlot::WristL:
		return GetLeftBracerComponent();
	case EEquipmentSlot::WristR:
		return GetRightBracerComponent();
	case EEquipmentSlot::Hands:
		return GetHandsComponent();
	case EEquipmentSlot::Waist:
		break;
	case EEquipmentSlot::Legs:
		return GetLegsComponent();
	case EEquipmentSlot::Foot:
		return GetFootComponent();
	case EEquipmentSlot::Arms:
		return GetArmsComponent();
	case EEquipmentSlot::WaistBag1:
		break;
	case EEquipmentSlot::WaistBag2:
		break;
	case EEquipmentSlot::BackPack1:
		break;
	case EEquipmentSlot::BackPack2:
		break;
	default: return nullptr;
	}

	return nullptr;
}

bool IInventoryModularCharacterInterface::IsBodyPart(EEquipmentSlot Slot)
{
	return Slot == EEquipmentSlot::Legs || Slot == EEquipmentSlot::Foot || Slot == EEquipmentSlot::Arms || Slot ==
		EEquipmentSlot::Torso || Slot == EEquipmentSlot::Hands;
}

bool IInventoryModularCharacterInterface::IsEquipmentPart(EEquipmentSlot Slot)
{
	return !IsBodyPart(Slot);
}
