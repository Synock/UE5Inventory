// Copyright 2022 Maximilien (Synock) Guislain

#include "Interfaces/InventoryPlayerInterface.h"

#include "InventoryUtilities.h"
#include "Interfaces/EquipmentInterface.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InventoryHUDInterface.h"
#include "Interfaces/LootableInterface.h"

void IInventoryPlayerInterface::PlayerUnequipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId,
                                                  EEquipmentSlot OutSlot)
{
	if (GetTransactionBoolean())
		return;

	SetTransactionBoolean(true);
	Server_PlayerUnequipItem(InTopLeft, InSlot, InItemId, OutSlot);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerEquipItemFromInventory(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft,
                                                             EBagSlot OutSlot)
{
	if (GetTransactionBoolean())
		return;

	SetTransactionBoolean(true);
	Server_PlayerEquipItemFromInventory(InItemId, InSlot, OutTopLeft, OutSlot);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerSwapEquipment(int32 DroppedItemId, EEquipmentSlot DroppedInSlot,
                                                    int32 SwappedItemId, EEquipmentSlot DraggedOutSlot)
{
	Server_PlayerSwapEquipment(DroppedItemId, DroppedInSlot, SwappedItemId, DraggedOutSlot);
}

//----------------------------------------------------------------------------------------------------------------------

// Add default functionality here for any IInventoryPlayerInterface functions that are not pure virtual.
TArray<FInventoryItem> IInventoryPlayerInterface::GetAllItems() const
{
	TArray<FInventoryItem> ItemsList;
	for (unsigned int BagID = 1; BagID < static_cast<unsigned int>(EBagSlot::LastValidBag); ++BagID)
	{
		const TArray<FMinimalItemStorage>& Items = GetInventoryComponentConst()->GetBagConst(
			static_cast<EBagSlot>(BagID));
		for (auto& Item : Items)
		{
			FInventoryItem LocalItem = UInventoryUtilities::GetItemFromID(Item.ItemID,
			                                                              GetInventoryOwningActorConst()->GetWorld());
			ItemsList.Add(LocalItem);
		}
	}
	return ItemsList;
}

//----------------------------------------------------------------------------------------------------------------------

const TArray<FMinimalItemStorage>& IInventoryPlayerInterface::GetAllItemsInBag(EBagSlot Slot) const
{
	return GetInventoryComponentConst()->GetBagConst(Slot);
}

//----------------------------------------------------------------------------------------------------------------------

bool IInventoryPlayerInterface::CanUnequipBag(EEquipmentSlot Slot) const
{
	if (Slot != EEquipmentSlot::WaistBag1 && Slot != EEquipmentSlot::WaistBag2 &&
		Slot != EEquipmentSlot::BackPack1 && Slot != EEquipmentSlot::BackPack2)
		return true;

	return GetInventoryComponentConst()->GetBagConst(UInventoryComponent::GetBagSlotFromInventory(Slot)).Num() == 0;
}

//----------------------------------------------------------------------------------------------------------------------

bool IInventoryPlayerInterface::PlayerTryAutoLootFunction(int32 InItemId, EEquipmentSlot& PossibleEquipment,
                                                          int32& InTopLeft, EBagSlot& PossibleBag) const
{
	PossibleEquipment = EEquipmentSlot::Unknown;
	PossibleBag = EBagSlot::Unknown;
	InTopLeft = -1;

	const FInventoryItem LocalItem = UInventoryUtilities::GetItemFromID(
		InItemId, GetInventoryOwningActorConst()->GetWorld());

	if (const IEquipmentInterface* EquipmentInterface = Cast<IEquipmentInterface>(GetInventoryOwningActorConst()))
	{
		if (LocalItem.Equipable)
			PossibleEquipment = EquipmentInterface->FindSuitableSlot(LocalItem);

		if (PossibleEquipment != EEquipmentSlot::Unknown)
			return true;
	}

	PossibleBag = GetInventoryComponentConst()->FindSuitableSlot(LocalItem, InTopLeft);

	if (PossibleBag != EBagSlot::Unknown)
		return true;

	return false;
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerAddItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId)
{
	if (GetInventoryOwningActor()->HasAuthority())
		GetInventoryComponent()->AddItemAt(InSlot, InItemId, InTopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerRemoveItem(int32 TopLeft, EBagSlot Slot)
{
	if (GetInventoryOwningActor()->HasAuthority())
		GetInventoryComponent()->RemoveItem(Slot, TopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

int32 IInventoryPlayerInterface::PlayerGetItem(int32 TopLeft, EBagSlot Slot) const
{
	return GetInventoryComponentConst()->GetItemAtIndex(Slot, TopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerMoveItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft,
                                               EBagSlot OutSlot)
{
	if (GetTransactionBoolean())
		return;

	SetTransactionBoolean(true);

	Server_PlayerMoveItem(InTopLeft, InSlot, InItemId, OutTopLeft, OutSlot);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::TransferCoinTo(UCoinComponent* ReceivingComponent, const FCoinValue& CoinValue)
{
	//if(ReceivingComponent->GetOwner() != GetInventoryOwningActor())
		//return;

	Server_TransferCoinTo(ReceivingComponent, CoinValue);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::CancelStagingArea()
{
	Server_CancelStagingArea();
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::TransferStagingToActor(AActor* TargetActor)
{
	Server_TransferStagingToActor(TargetActor);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::MoveEquipmentToStagingArea(int32 InItemId, EEquipmentSlot OutSlot)
{
	Server_MoveEquipmentToStagingArea(InItemId, OutSlot);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::MoveInventoryItemToStagingArea(int32 InItemId, int32 OutTopLeft, EBagSlot OutSlot)
{
	Server_MoveInventoryItemToStagingArea(InItemId, OutTopLeft, OutSlot);
}

//----------------------------------------------------------------------------------------------------------------------

bool IInventoryPlayerInterface::IsLooting() const
{
	return GetLootedActorConst() != nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::StartLooting(AActor* Actor)
{
	if (const ILootableInterface* LootInterface = Cast<ILootableInterface>(Actor))
		if (!GetLootedActor() && Actor && !LootInterface->GetIsBeingLooted())
			Server_LootActor(Actor);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::StopLooting()
{
	if (GetLootedActor())
		Server_StopLooting();
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerAutoLootItem(int32 InItemId, int32 OutTopLeft)
{
	if (GetTransactionBoolean())
		return;

	EEquipmentSlot TriedSlot = EEquipmentSlot::Unknown;
	EBagSlot TriedBag = EBagSlot::Unknown;
	int32 InTopLeft = -1;

	if (PlayerTryAutoLootFunction(InItemId, TriedSlot, InTopLeft, TriedBag))
	{
		if (TriedSlot != EEquipmentSlot::Unknown)
		{
			SetTransactionBoolean(true);
			Server_PlayerEquipItemFromLoot(InItemId, TriedSlot, OutTopLeft);
		}
		else if (TriedBag != EBagSlot::Unknown)
		{
			SetTransactionBoolean(true);
			Server_PlayerLootItem(InTopLeft, TriedBag, InItemId, OutTopLeft);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerAutoLootAll()
{
	Server_PlayerAutoLootAll();
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerLootItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft)
{
	if (GetTransactionBoolean())
		return;

	SetTransactionBoolean(true);
	Server_PlayerLootItem(InTopLeft, InSlot, InItemId, OutTopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerEquipItemFromLoot(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft)
{
	if (GetTransactionBoolean())
		return;

	SetTransactionBoolean(true);
	Server_PlayerEquipItemFromLoot(InItemId, InSlot, OutTopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

bool IInventoryPlayerInterface::PlayerCanPutItemSomewhere(int32 ItemID) const
{
	EEquipmentSlot TriedSlot = EEquipmentSlot::Unknown;
	EBagSlot TriedBag = EBagSlot::Unknown;
	int32 InTopLeft = -1;
	return PlayerTryAutoLootFunction(ItemID, TriedSlot, InTopLeft, TriedBag);
}

//----------------------------------------------------------------------------------------------------------------------

bool IInventoryPlayerInterface::PlayerCanPayAmount(const FCoinValue& CoinValue) const
{
	return FCoinValue::CanPayWithChange(GetCoinComponentConst()->GetPurseContent(), CoinValue);
}

//----------------------------------------------------------------------------------------------------------------------

bool IInventoryPlayerInterface::PlayerTryAutoEquip(int32 InItemId, EEquipmentSlot& PossibleEquipment)
{
	return GetEquipmentForInventory()->TryAutoEquip(InItemId, PossibleEquipment);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerAutoEquipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId)
{
	if (GetTransactionBoolean())
		return;

	SetTransactionBoolean(true);
	Server_PlayerAutoEquipItem(InTopLeft, InSlot, InItemId);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::ResetTransaction()
{
	SetTransactionBoolean(false);
}

//----------------------------------------------------------------------------------------------------------------------

IEquipmentInterface* IInventoryPlayerInterface::GetEquipmentForInventory()
{
	return Cast<IEquipmentInterface>(GetInventoryOwningActor());
}

//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
// Merchant related functions -- Client
//----------------------------------------------------------------------------------------------------------------------

bool IInventoryPlayerInterface::IsTrading() const
{
	return GetMerchantActorConst() != nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::TryPresentSellItem(EBagSlot OutSlot, int32 ItemId, int32 TopLeft)
{
	GetInventoryHUDInterface()->Execute_TryPresentSellItem(GetInventoryHUDObject(), OutSlot,
	                                                       ItemId, TopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::ResetSellItem()
{
	GetInventoryHUDInterface()->Execute_ResetSellItem(GetInventoryHUDObject());
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerBuyFromMerchant(int32 ItemId, const FCoinValue& Price)
{
	Server_PlayerBuyFromMerchant(ItemId, Price);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerSellToMerchant(EBagSlot OutSlot, int32 ItemId, int32 TopLeft,
                                                     const FCoinValue& Price)
{
	Server_PlayerSellToMerchant(OutSlot, ItemId, TopLeft, Price);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::MerchantTrade(AActor* InputMerchantActor)
{
	Server_MerchantTrade(InputMerchantActor);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::StopMerchantTrade()
{
	Server_StopMerchantTrade();
}

//----------------------------------------------------------------------------------------------------------------------

float IInventoryPlayerInterface::GetTotalWeight()
{
	return GetInventoryComponent()->GetTotalWeight() + GetEquipmentForInventory()->GetTotalWeight() +
		GetCoinComponent()->GetTotalWeight();
}
