#include "Interfaces/EquipmentInterface.h"
#include "InventoryUtilities.h"
#include "Components/EquipmentComponent.h"
#include "Components/InventoryComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Items/InventoryItemAmmoBag.h"
#include "Items/Interfaces/InventoryItemBagInterface.h"
#include "Items/InventoryItemBase.h"
#include "Items/InventoryItemEquipable.h"
#include "Items/Interfaces/InventoryItemAmmoInterface.h"


bool IEquipmentInterface::EquipmentHasAuthority()
{
	return GetEquipmentComponent()->GetOwner()->HasAuthority();
}

//----------------------------------------------------------------------------------------------------------------------

UWorld* IEquipmentInterface::EquipmentGetWorldContext() const
{
	return GetEquipmentComponentConst()->GetOwner()->GetWorld();
}

//----------------------------------------------------------------------------------------------------------------------

// Add default functionality here for any IEquipmentInterface functions that are not pure virtual.
const TArray<const UInventoryItemEquipable*>& IEquipmentInterface::GetAllEquipment() const
{
	return GetEquipmentComponentConst()->GetAllEquipment();
}

//----------------------------------------------------------------------------------------------------------------------

const UInventoryItemEquipable* IEquipmentInterface::GetEquippedItem(EEquipmentSlot Slot) const
{
	check(GetEquipmentComponentConst());
	return GetEquipmentComponentConst()->GetItemAtSlot(Slot);
}

//----------------------------------------------------------------------------------------------------------------------

bool IEquipmentInterface::GetEquipmentDurability(EEquipmentSlot Slot, float& OutDurability) const
{
	check(GetEquipmentComponentConst());
	return GetEquipmentComponentConst()->GetEquipmentDurability(Slot, OutDurability);
}

//----------------------------------------------------------------------------------------------------------------------

void IEquipmentInterface::EquipItem(EEquipmentSlot InSlot, int32 InItemId)
{
	if (!EquipmentHasAuthority())
		return;

	const UInventoryItemEquipable* LocalItem = Cast<UInventoryItemEquipable>(
		UInventoryUtilities::GetItemFromID(InItemId, EquipmentGetWorldContext()));

	if (!LocalItem)
		return;

	GetEquipmentComponent()->EquipItem(LocalItem, InSlot);
	//HandleEquipmentEffect(InSlot, LocalItem);
}

//----------------------------------------------------------------------------------------------------------------------

void IEquipmentInterface::EquipItemWithDurability(EEquipmentSlot InSlot, int32 InItemId, float Durability)
{
	if (!EquipmentHasAuthority())
		return;

	const UInventoryItemEquipable* LocalItem = Cast<UInventoryItemEquipable>(
		UInventoryUtilities::GetItemFromID(InItemId, EquipmentGetWorldContext()));

	if (!LocalItem)
		return;

	GetEquipmentComponent()->EquipItemWithDurability(LocalItem, InSlot, Durability);
}

//----------------------------------------------------------------------------------------------------------------------

bool IEquipmentInterface::TryAutoEquip(int32 InItemId, EEquipmentSlot& PossibleEquipment) const
{
	const UInventoryItemEquipable* LocalItem = Cast<UInventoryItemEquipable>(
		UInventoryUtilities::GetItemFromID(InItemId, EquipmentGetWorldContext()));
	PossibleEquipment = EEquipmentSlot::Unknown;

	if (LocalItem)
		PossibleEquipment = GetEquipmentComponentConst()->FindSuitableSlot(LocalItem);

	if (PossibleEquipment != EEquipmentSlot::Unknown)
		return true;

	return false;
}

//----------------------------------------------------------------------------------------------------------------------

void IEquipmentInterface::UnequipItem(EEquipmentSlot OutSlot)
{
	if (!EquipmentHasAuthority())
		return;

	GetEquipmentComponent()->RemoveItem(OutSlot);
}

//----------------------------------------------------------------------------------------------------------------------

void IEquipmentInterface::SwapEquipment(EEquipmentSlot DroppedInSlot, EEquipmentSlot DraggedOutSlot)
{
	if (!EquipmentHasAuthority())
		return;

	const UInventoryItemEquipable* ItemToMove = GetEquippedItem(DroppedInSlot);
	const UInventoryItemEquipable* DroppedItem = GetEquippedItem(DraggedOutSlot);

	GetEquipmentComponent()->RemoveItem(DroppedInSlot);
	HandleUnEquipmentEffect(DroppedInSlot, ItemToMove);
	GetEquipmentComponent()->EquipItem(DroppedItem, DroppedInSlot);
	HandleEquipmentEffect(DroppedInSlot, DroppedItem);

	GetEquipmentComponent()->RemoveItem(DraggedOutSlot);
	HandleUnEquipmentEffect(DraggedOutSlot, DroppedItem);
	GetEquipmentComponent()->EquipItem(ItemToMove, DraggedOutSlot);
	HandleEquipmentEffect(DraggedOutSlot, ItemToMove);
}

EEquipmentSlot IEquipmentInterface::FindSuitableSlot(const UInventoryItemEquipable* Item) const
{
	return GetEquipmentComponentConst()->FindSuitableSlot(Item);
}

//----------------------------------------------------------------------------------------------------------------------

void IEquipmentInterface::HandleEquipmentEffect(EEquipmentSlot InSlot, const UInventoryItemEquipable* LocalItem)
{
	if (!EquipmentHasAuthority())
		return;

	HandleTwoSlotItemEquip(LocalItem, InSlot);

	if (const IInventoryItemBagInterface* LocalBag = Cast<IInventoryItemBagInterface>(LocalItem); LocalBag)
	{
		const ACharacter* Chara = Cast<ACharacter>(GetEquipmentComponent()->GetOwner());
		if (!Chara)
			return;

		const EBagSlot AffectedSlot = UInventoryComponent::GetBagSlotFromInventory(InSlot);
		if (IInventoryPlayerInterface* Inventory = Cast<IInventoryPlayerInterface>(Chara->GetController()))
		{
			Inventory->GetInventoryComponent()->BagSet(AffectedSlot, true, LocalBag->GetBagWidth(), LocalBag->GetBagHeight(),
			                                           LocalBag->GetBagSize(),
			                                           FMath::Clamp(0.f, 1.f, 1.f - LocalBag->GetWeightReduction()));

			if (const IInventoryItemAmmoBagInterface* LocalQuiver = Cast<IInventoryItemAmmoBagInterface>(LocalBag);
				LocalQuiver)
			{
				Inventory->GetInventoryComponent()->QuiverSpecificSetup(AffectedSlot, LocalQuiver->GetAmmoType());
			}
		}
	}

	// Override this function do equipment specific stuff here
	// such as computing new damage output, edit armor, stuff like that
	// But call this one if you want internal stuff to work properly
}

//----------------------------------------------------------------------------------------------------------------------

void IEquipmentInterface::HandleUnEquipmentEffect(EEquipmentSlot InSlot, const UInventoryItemEquipable* LocalItem)
{
	if (!EquipmentHasAuthority())
		return;

	HandleTwoSlotItemUnequip(LocalItem, InSlot);
	//do equipment specific stuff here

	if (const IInventoryItemBagInterface* LocalBag = Cast<IInventoryItemBagInterface>(LocalItem); LocalBag)
	{
		const ACharacter* Chara = Cast<ACharacter>(GetEquipmentComponent()->GetOwner());
		if (!Chara)
			return;

		const EBagSlot AffectedSlot = UInventoryComponent::GetBagSlotFromInventory(InSlot);
		if (IInventoryPlayerInterface* Inventory = Cast<IInventoryPlayerInterface>(Chara->GetController()))
		{
			Inventory->GetInventoryComponent()->BagSet(AffectedSlot, false);
			if (const IInventoryItemAmmoBagInterface* LocalQuiver = Cast<IInventoryItemAmmoBagInterface>(LocalBag);
				LocalQuiver)
				Inventory->GetInventoryComponent()->QuiverSpecificSetup(AffectedSlot, EAmmoType::Unknown);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

float IEquipmentInterface::GetTotalWeight() const
{
	float Sum = 0.f;
	for (const auto& Item : GetAllEquipment())
	{
		if (Item)
		{
			Sum += Item->Weight;
		}
	}

	return Sum;
}

//----------------------------------------------------------------------------------------------------------------------

void IEquipmentInterface::HandleTwoSlotItemEquip(const UInventoryItemEquipable* Item, EEquipmentSlot& InSlot)
{
	if (!EquipmentHasAuthority())
		return;

	if (Item->MultiSlotItem)
	{
		EEquipmentSlot PrimarySlot = InSlot;

		for (int32 i = static_cast<int32>(EEquipmentSlot::Unknown); i < static_cast<int32>(EEquipmentSlot::Last); ++i)
		{
			const int32 localValue = 1 << i;

			if (localValue & Item->EquipableSlotBitMask)
			{
				// put the first slot as the major slot
				PrimarySlot = static_cast<EEquipmentSlot>(i);
				break;
			}
		}

		InSlot = PrimarySlot;
	}
}

//----------------------------------------------------------------------------------------------------------------------

void IEquipmentInterface::HandleTwoSlotItemUnequip(const UInventoryItemEquipable* Item, EEquipmentSlot InSlot)
{
	if (!EquipmentHasAuthority())
		return;

	if (Item->MultiSlotItem)
	{
		TArray<EEquipmentSlot> OtherSlots;
		OtherSlots.Reserve(static_cast<int32>(EEquipmentSlot::Last));
		for (int32 i = static_cast<int32>(EEquipmentSlot::Unknown); i < static_cast<int32>(EEquipmentSlot::Last); ++i)
		{
			const int32 localValue = 1 << i;

			if ((localValue & Item->EquipableSlotBitMask) && InSlot != static_cast<EEquipmentSlot>(i))
			{
				OtherSlots.Emplace(static_cast<EEquipmentSlot>(i));
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

UStaticMesh* IEquipmentInterface::GetPreferedMesh(UStaticMesh* OriginalMesh) const
{
	return OriginalMesh;
}

//----------------------------------------------------------------------------------------------------------------------

bool IEquipmentInterface::HasCompatibleAmmoEquipped(EAmmoType AmmoType) const
{
	const auto PotentialAmmo = GetEquipmentComponentConst()->GetItemAtSlot(EEquipmentSlot::Ammo);

	if (const auto Ammo = Cast<IInventoryItemAmmoInterface>(PotentialAmmo))
	{
		return Ammo->GetAmmoType() == AmmoType;
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------

TScriptInterface<IInventoryItemAmmoInterface> IEquipmentInterface::RemoveAmmoEquipped(EAmmoType AmmoType)
{
	auto* PotentialAmmo = GetEquipmentComponent()->GetItemAtSlot(EEquipmentSlot::Ammo);

	if (const auto Ammo = Cast<IInventoryItemAmmoInterface>(PotentialAmmo))
	{
		if (Ammo->GetAmmoType() == AmmoType)
		{
			GetEquipmentComponent()->RemoveItem(EEquipmentSlot::Ammo);
			// oh man, this sux so much, that const cast from hell
			return TScriptInterface<IInventoryItemAmmoInterface>(const_cast<UInventoryItemEquipable*>(PotentialAmmo));
		}
	}

	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

TArray<FMaterialOverride> IEquipmentInterface::GetMaterialOverridesForSlot(EEquipmentSlot Slot) const
{
	TArray<FMaterialOverride> MaterialOverrides;

	if (const UInventoryItemEquipable* Item = GetEquipmentComponentConst()->GetItemAtSlot(Slot); Item && Item->
		EquipmentMeshMaterialOverride.Num() > 0)
	{
		return Item->EquipmentMeshMaterialOverride;
	}

	return {};
}

//----------------------------------------------------------------------------------------------------------------------

TMap<FString, FMaterialOverride> IEquipmentInterface::GetMaterialOverridesMapForSlot(EEquipmentSlot Slot) const
{
	TMap<FString, FMaterialOverride> MaterialOverrides;

	if (const UInventoryItemEquipable* Item = GetEquipmentComponentConst()->GetItemAtSlot(Slot); Item && Item->
		EquipmentMeshMaterialOverride.Num() > 0)
	{
		auto& MaterialList = Item->EquipmentMesh->GetMaterials();
		for (const FMaterialOverride& Override : Item->EquipmentMeshMaterialOverride)
		{
			if (Override.MaterialID < MaterialList.Num())
				MaterialOverrides.FindOrAdd(MaterialList[Override.MaterialID].MaterialSlotName.ToString(), Override);
		}
	}

	return MaterialOverrides;
}
