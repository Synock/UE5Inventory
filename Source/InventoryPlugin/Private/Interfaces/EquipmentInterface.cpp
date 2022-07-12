// Copyright 2022 Maximilien (Synock) Guislain


#include "Interfaces/EquipmentInterface.h"

#include "InventoryUtilities.h"
#include "Components/EquipmentComponent.h"
#include "Components/InventoryComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/InventoryPlayerInterface.h"


bool IEquipmentInterface::EquipmentHasAuthority()
{
	return GetEquipmentOwningActor()->HasAuthority();
}

//----------------------------------------------------------------------------------------------------------------------

UWorld* IEquipmentInterface::EquipmentGetWorldContext() const
{
	return GetEquipmentOwningActorConst()->GetWorld();
}

//----------------------------------------------------------------------------------------------------------------------

// Add default functionality here for any IEquipmentInterface functions that are not pure virtual.
TArray<FInventoryItem> IEquipmentInterface::GetAllEquipment() const
{
	return GetEquipmentComponentConst()->GetAllEquipment();
}

//----------------------------------------------------------------------------------------------------------------------

const FInventoryItem& IEquipmentInterface::GetEquippedItem(EEquipmentSlot Slot) const
{
	check(GetEquipmentComponentConst());
	return GetEquipmentComponentConst()->GetItemAtSlot(Slot);
}

//----------------------------------------------------------------------------------------------------------------------

void IEquipmentInterface::EquipItem(EEquipmentSlot InSlot, int32 InItemId)
{
	if (!EquipmentHasAuthority())
		return;

	const FInventoryItem LocalItem = UInventoryUtilities::GetItemFromID(InItemId, EquipmentGetWorldContext());

	GetEquipmentComponent()->EquipItem(LocalItem, InSlot);
	HandleEquipmentEffect(InSlot, LocalItem);
}

//----------------------------------------------------------------------------------------------------------------------

bool IEquipmentInterface::TryAutoEquip(int32 InItemId, EEquipmentSlot& PossibleEquipment) const
{
	const FInventoryItem LocalItem = UInventoryUtilities::GetItemFromID(InItemId, EquipmentGetWorldContext());
	PossibleEquipment = EEquipmentSlot::Unknown;

	if (LocalItem.Equipable)
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

	FInventoryItem ItemToMove = GetEquippedItem(DroppedInSlot);
	FInventoryItem DroppedItem = GetEquippedItem(DraggedOutSlot);

	GetEquipmentComponent()->RemoveItem(DroppedInSlot);
	HandleUnEquipmentEffect(DroppedInSlot, ItemToMove);
	GetEquipmentComponent()->EquipItem(DroppedItem, DroppedInSlot);
	HandleEquipmentEffect(DroppedInSlot, DroppedItem);

	GetEquipmentComponent()->RemoveItem(DraggedOutSlot);
	HandleUnEquipmentEffect(DraggedOutSlot, DroppedItem);
	GetEquipmentComponent()->EquipItem(ItemToMove, DraggedOutSlot);
	HandleEquipmentEffect(DraggedOutSlot, ItemToMove);
}

EEquipmentSlot IEquipmentInterface::FindSuitableSlot(const FInventoryItem& Item) const
{
	return GetEquipmentComponentConst()->FindSuitableSlot(Item);
}

//----------------------------------------------------------------------------------------------------------------------

void IEquipmentInterface::HandleEquipmentEffect(EEquipmentSlot InSlot, const FInventoryItem& LocalItem)
{
	if (!EquipmentHasAuthority())
		return;

	HandleTwoSlotItemEquip(LocalItem, InSlot);
	//do equipment specific stuff here

	if (LocalItem.Bag)
	{
		const ACharacter* Chara = Cast<ACharacter>(GetEquipmentOwningActor());
		if (!Chara)
			return;

		const EBagSlot AffectedSlot = UInventoryComponent::GetBagSlotFromInventory(InSlot);
		if (IInventoryPlayerInterface* Inventory = Cast<IInventoryPlayerInterface>(Chara->GetController()))
			Inventory->GetInventoryComponent()->BagSet(AffectedSlot, true, LocalItem.BagWidth, LocalItem.BagHeight,
			                                           LocalItem.BagSize);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void IEquipmentInterface::HandleUnEquipmentEffect(EEquipmentSlot InSlot, const FInventoryItem& LocalItem)
{
	if (!EquipmentHasAuthority())
		return;

	HandleTwoSlotItemUnequip(LocalItem, InSlot);
	//do equipment specific stuff here

	if (LocalItem.Bag)
	{
		const ACharacter* Chara = Cast<ACharacter>(GetEquipmentOwningActor());
		if (!Chara)
			return;

		const EBagSlot AffectedSlot = UInventoryComponent::GetBagSlotFromInventory(InSlot);
		if (IInventoryPlayerInterface* Inventory = Cast<IInventoryPlayerInterface>(Chara->GetController()))
			Inventory->GetInventoryComponent()->BagSet(AffectedSlot, false);
	}
}

//----------------------------------------------------------------------------------------------------------------------

float IEquipmentInterface::GetTotalWeight() const
{
	float Sum = 0.f;
	for(const auto& Item :  GetAllEquipment())
	{
		if(Item.ItemID > 0)
		{
			Sum+= Item.Weight;
		}
	}

	return Sum;
}

//----------------------------------------------------------------------------------------------------------------------

void IEquipmentInterface::HandleTwoSlotItemEquip(const FInventoryItem& Item, EEquipmentSlot& InSlot)
{
	if (!EquipmentHasAuthority())
		return;

	if (Item.TwoSlotsItem)
	{
		EEquipmentSlot PrimarySlot = InSlot;
		EEquipmentSlot SecondarySlot = InSlot;

		if (PrimarySlot == EEquipmentSlot::BackPack1 || PrimarySlot == EEquipmentSlot::BackPack2)
		{
			PrimarySlot = EEquipmentSlot::BackPack1;
			SecondarySlot = EEquipmentSlot::BackPack2;
		}
		else if (PrimarySlot == EEquipmentSlot::WaistBag1 || PrimarySlot == EEquipmentSlot::WaistBag2)
		{
			PrimarySlot = EEquipmentSlot::WaistBag1;
			SecondarySlot = EEquipmentSlot::WaistBag2;
		}

		FInventoryItem GhostItem;
		GhostItem.ItemID = 0;
		GhostItem.IconName = Item.IconName;

		GetEquipmentComponent()->EquipItem(GhostItem, SecondarySlot);
		InSlot = PrimarySlot;
	}
}

//----------------------------------------------------------------------------------------------------------------------

void IEquipmentInterface::HandleTwoSlotItemUnequip(const FInventoryItem& Item, EEquipmentSlot InSlot)
{
	if (!EquipmentHasAuthority())
		return;

	if (Item.TwoSlotsItem)
	{
		const EEquipmentSlot PrimarySlot = InSlot;
		EEquipmentSlot SecondarySlot = InSlot;

		if (PrimarySlot == EEquipmentSlot::BackPack1)
			SecondarySlot = EEquipmentSlot::BackPack2;

		if (PrimarySlot == EEquipmentSlot::WaistBag1)
			SecondarySlot = EEquipmentSlot::WaistBag2;

		GetEquipmentComponent()->RemoveItem(SecondarySlot);
	}
}
