// Copyright 2022 Maximilien (Synock) Guislain

#include "Interfaces/InventoryPlayerInterface.h"

#include "InventoryUtilities.h"
#include "Components/BankComponent.h"
#include "Components/KeyringComponent.h"
#include "Interfaces/EquipmentInterface.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InventoryHUDInterface.h"
#include "Interfaces/LootableInterface.h"
#include "Items/InventoryItemActionnable.h"
#include "Items/InventoryItemEquipable.h"
#include "Items/InventoryItemKey.h"

UCoinComponent* IInventoryPlayerInterface::GetBankCoin() const
{
	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

UBankComponent* IInventoryPlayerInterface::GetBankComponent() const
{
	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

UKeyringComponent* IInventoryPlayerInterface::GetKeyring() const
{
	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

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

TArray<const UInventoryItemBase*> IInventoryPlayerInterface::GetAllItems() const
{
	TArray<const UInventoryItemBase*> ItemsList;
	for (unsigned int BagID = 1; BagID < static_cast<unsigned int>(EBagSlot::LastValidBag); ++BagID)
	{
		const TArray<FMinimalItemStorage>& Items = GetInventoryComponentConst()->GetBagConst(
			static_cast<EBagSlot>(BagID));
		for (auto& Item : Items)
		{
			const UInventoryItemBase* LocalItem = UInventoryUtilities::GetItemFromID(Item.ItemID,
				GetInventoryOwningActorConst()->GetWorld());
			ItemsList.Add(LocalItem);
		}
	}
	return ItemsList;
}

//----------------------------------------------------------------------------------------------------------------------

const TArray<FMinimalItemStorage>& IInventoryPlayerInterface::GetAllItemsInBag(EBagSlot Slot) const
{
	if (Slot == EBagSlot::BankPool)
	{
		return GetBankComponent()->GetBagConst();
	}

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
                                                          int32& InTopLeft, EBagSlot& PossibleBag)
{
	PossibleEquipment = EEquipmentSlot::Unknown;
	PossibleBag = EBagSlot::Unknown;
	InTopLeft = -1;

	const UInventoryItemBase* LocalItem = UInventoryUtilities::GetItemFromID(
		InItemId, GetInventoryOwningActorConst()->GetWorld());

	if (const IEquipmentInterface* EquipmentInterface = Cast<IEquipmentInterface>(GetInventoryOwningActorConst()))
	{
		if (const UInventoryItemEquipable* Equipable = Cast<UInventoryItemEquipable>(LocalItem))
			PossibleEquipment = EquipmentInterface->FindSuitableSlot(Equipable);

		if (PossibleEquipment != EEquipmentSlot::Unknown)
			return true;
	}

	PossibleBag = GetInventoryComponentConst()->FindSuitableSlot(LocalItem, InTopLeft);

	if (PossibleBag != EBagSlot::Unknown)
		return true;

	return false;
}

//----------------------------------------------------------------------------------------------------------------------

bool IInventoryPlayerInterface::PlayerHasItem(int32 ItemId)
{
	return GetInventoryComponent()->HasItem(ItemId);
}

//----------------------------------------------------------------------------------------------------------------------

bool IInventoryPlayerInterface::PlayerHasItems(int32 ItemId, int32 ItemAmount)
{
	return GetInventoryComponent()->HasItems(ItemId, ItemAmount);
}

//----------------------------------------------------------------------------------------------------------------------

bool IInventoryPlayerInterface::PlayerHasAnyItem(const TArray<int32>& ItemID)
{
	return GetInventoryComponent()->HasAnyItem(ItemID);
}

//----------------------------------------------------------------------------------------------------------------------

bool IInventoryPlayerInterface::PlayerRemoveItemIfPossible(int32 ItemID)
{
	return GetInventoryComponent()->RemoveItemIfPossible(ItemID);
}

//----------------------------------------------------------------------------------------------------------------------

bool IInventoryPlayerInterface::PlayerRemoveAnyItemIfPossible(const TArray<int32>& ItemID)
{
	return GetInventoryComponent()->PlayerRemoveAnyItemIfPossible(ItemID);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerAddItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId)
{
	if (!GetInventoryOwningActor()->HasAuthority())
		return;

	if (InSlot == EBagSlot::BankPool)
		GetBankComponent()->AddItem(InItemId, InTopLeft);
	else
		GetInventoryComponent()->AddItemAt(InSlot, InItemId, InTopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerRemoveItem(int32 TopLeft, EBagSlot Slot)
{
	if (!GetInventoryOwningActor()->HasAuthority())
		return;

	if (Slot == EBagSlot::BankPool)
		GetBankComponent()->RemoveItem(TopLeft);
	else
		GetInventoryComponent()->RemoveItem(Slot, TopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

int32 IInventoryPlayerInterface::PlayerGetItem(int32 TopLeft, EBagSlot Slot) const
{
	if (Slot == EBagSlot::BankPool)
		return GetBankComponent()->GetItemAtIndex(TopLeft);

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

void IInventoryPlayerInterface::TransferCoinTo(UCoinComponent* GivingComponent, UCoinComponent* ReceivingComponent,
                                               const FCoinValue& RemovedCoinValue, const FCoinValue& AddedCoinValue)
{
	//if(ReceivingComponent->GetOwner() != GetInventoryOwningActor())
	//return;

	Server_TransferCoinTo(GivingComponent, ReceivingComponent, RemovedCoinValue, AddedCoinValue);
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

void IInventoryPlayerInterface::HandleActivation(int32 ItemID, int32 TopLeft, EBagSlot BagSlot)
{
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

void IInventoryPlayerInterface::StopLooting(AActor* Actor)
{
	if (GetLootedActor() && (GetLootedActor() == Actor || Actor == nullptr))
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

bool IInventoryPlayerInterface::PlayerCanPutItemSomewhere(int32 ItemID)
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

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::Internal_PlayerAddKeyFromInventory(int32 InTopLeft, EBagSlot InSlot, int32 InItemId)
{
	if (!GetInventoryOwningActor()->HasAuthority())
		return;

	if (!GetKeyring())
		return;

	const UInventoryItemKey* LocalItem = Cast<UInventoryItemKey>(UInventoryUtilities::GetItemFromID(
		InItemId, GetInventoryOwningActorConst()->GetWorld()));

	if (!LocalItem)
		return;

	if (GetKeyring()->TryAddKeyFromItem(LocalItem))
		PlayerRemoveItem(InTopLeft, InSlot);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::Internal_PlayerRemoveKeyToInventory(int32 KeyId)
{
	if (!GetInventoryOwningActor()->HasAuthority())
		return;

	if (!GetKeyring())
		return;

	if (!GetKeyring()->HasKey(KeyId))
		return;

	int32 ItemToGet = GetKeyring()->GetItemFromKey(KeyId);

	EEquipmentSlot PossibleEquipment;
	int32 InTopLeft;
	EBagSlot PossibleBag;

	if (!PlayerTryAutoLootFunction(ItemToGet, PossibleEquipment, InTopLeft, PossibleBag))
		return;

	if (PossibleBag == EBagSlot::Unknown)
		return;

	GetKeyring()->RemoveKey(KeyId);
	PlayerAddItem(InTopLeft, PossibleBag, ItemToGet);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerAddKeyFromInventory(int32 InTopLeft, EBagSlot InSlot, int32 InItemId)
{
	Server_PlayerAddKeyFromInventory(InTopLeft, InSlot, InItemId);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerRemoveKeyToInventory(int32 KeyId)
{
	Server_PlayerRemoveKeyToInventory(KeyId);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::DisplayItemDescription(const UInventoryItemBase* Item, float X, float Y)
{
	GetInventoryHUDInterface()->Execute_DisplayItemDescription(GetInventoryHUDObject(), Item, X, Y);
}

//----------------------------------------------------------------------------------------------------------------------

bool IInventoryPlayerInterface::TryToEat()
{
	UInventoryComponent* Inventory = GetInventoryComponent();
	if (!Inventory)
		return false;

	for (EBagSlot BagSlot = EBagSlot::Unknown; BagSlot < EBagSlot::LastValidBag; ++BagSlot)
	{
		if (!Inventory->IsBagValid(BagSlot))
			continue;

		const TArray<FMinimalItemStorage>& BagItems = Inventory->GetBagConst(BagSlot);
		for (const FMinimalItemStorage& ItemStorage : BagItems)
		{
			const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemStorage.ItemID, GetInventoryOwningActor()->GetWorld());
			if (const UInventoryItemActionnable* ActionnableItem = Cast<UInventoryItemActionnable>(Item))
			{
				if (ActionnableItem->HungerValue > 0.f)
				{
					HandleActivation(ItemStorage.ItemID, ItemStorage.TopLeftID, BagSlot);
					return true;
				}
			}
		}
	}

	return false;
}

bool IInventoryPlayerInterface::TryToDrink()
{
	UInventoryComponent* Inventory = GetInventoryComponent();
	if (!Inventory)
		return false;

	for (EBagSlot BagSlot = EBagSlot::Unknown; BagSlot < EBagSlot::LastValidBag; ++BagSlot)
	{
		if (!Inventory->IsBagValid(BagSlot))
			continue;

		const TArray<FMinimalItemStorage>& BagItems = Inventory->GetBagConst(BagSlot);
		for (const FMinimalItemStorage& ItemStorage : BagItems)
		{
			const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemStorage.ItemID, GetInventoryOwningActor()->GetWorld());
			if (const UInventoryItemActionnable* ActionnableItem = Cast<UInventoryItemActionnable>(Item))
			{
				if (ActionnableItem->ThirstValue > 0.f)
				{
					HandleActivation(ItemStorage.ItemID, ItemStorage.TopLeftID, BagSlot);
					return true;
				}
			}
		}
	}

	return false;
}
