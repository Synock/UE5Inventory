#include "Interfaces/InventoryPlayerInterface.h"

#include "InventoryUtilities.h"
#include "Components/BankComponent.h"
#include "Components/InventoryNetComponent.h"
#include "Components/InventoryDeliveryComponent.h"
#include "Components/KeyringComponent.h"
#include "Components/StagingAreaComponent.h"
#include "Components/TradeComponent.h"
#include "Interfaces/EquipmentInterface.h"
#include "Interfaces/MerchantInterface.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InventoryHUDInterface.h"
#include "Interfaces/LootableInterface.h"
#include "Items/InventoryItemEquipable.h"
#include "Items/InventoryItemKey.h"
#include "Items/Interfaces/InventoryItemDrinkInterface.h"
#include "Items/Interfaces/InventoryItemFoodInterface.h"

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

	const EBagSlot BagSlot = UInventoryComponent::GetBagSlotFromInventory(Slot);
	return GetInventoryComponentConst()->GetBagConst(BagSlot).Num() == 0 &&
		!GetInventoryComponentConst()->HasReservationsInBag(BagSlot);
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

void IInventoryPlayerInterface::PlayerAddItemWithDurability(int32 InTopLeft, EBagSlot InSlot, int32 InItemId,
                                                            float Durability)
{
	if (!GetInventoryOwningActor()->HasAuthority())
		return;

	if (InSlot == EBagSlot::BankPool)
		GetBankComponent()->AddItem(InItemId, InTopLeft, Durability);
	else
		GetInventoryComponent()->AddItemAt(InSlot, InItemId, InTopLeft, Durability);
}

//----------------------------------------------------------------------------------------------------------------------

float IInventoryPlayerInterface::PlayerRemoveItem(int32 TopLeft, EBagSlot Slot)
{
	if (!GetInventoryOwningActor()->HasAuthority())
		return 100.0f;

	// Capture durability before removing
	float Durability = 100.0f;

	if (Slot == EBagSlot::BankPool)
	{
		const TArray<FMinimalItemStorage>& BankItems = GetBankComponent()->GetBagConst();
		for (const FMinimalItemStorage& Item : BankItems)
		{
			if (Item.TopLeftID == TopLeft)
			{
				Durability = Item.Durability;
				break;
			}
		}
		GetBankComponent()->RemoveItem(TopLeft);
	}
	else
	{
		const TArray<FMinimalItemStorage>& BagItems = GetInventoryComponent()->GetBagConst(Slot);
		for (const FMinimalItemStorage& Item : BagItems)
		{
			if (Item.TopLeftID == TopLeft)
			{
				Durability = Item.Durability;
				break;
			}
		}
		GetInventoryComponent()->RemoveItem(Slot, TopLeft);
	}

	return Durability;
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
	{
		GetInventoryHUDInterface()->Execute_ForceRefreshInventory(GetInventoryHUDObject());
		return;
	}

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

void IInventoryPlayerInterface::ReturnStagingItem(FGuid ReservationId)
{
	Server_ReturnStagingItem(ReservationId);
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

const IEquipmentInterface* IInventoryPlayerInterface::GetConstEquipmentForInventory() const
{
	return Cast<IEquipmentInterface>(GetInventoryOwningActorConst());
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

void IInventoryPlayerInterface::TryPresentEquippedSellItem(EEquipmentSlot Slot, int32 ItemId)
{
	GetInventoryHUDInterface()->Execute_TryPresentEquippedSellItem(GetInventoryHUDObject(), Slot, ItemId);
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

void IInventoryPlayerInterface::PlayerSellEquippedItemToMerchant(EEquipmentSlot Slot, int32 ExpectedItemId)
{
	Server_PlayerSellEquippedItemToMerchant(Slot, ExpectedItemId);
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

void IInventoryPlayerInterface::RepairTrade(AActor* InputRepairerActor)
{
	Server_RepairTrade(InputRepairerActor);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::StopRepairTrade()
{
	Server_StopRepairTrade();
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerRepairEquipment(EEquipmentSlot Slot, const FCoinValue& Price)
{
	Server_PlayerRepairEquipment(Slot, Price);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerRepairAllEquipment(const FCoinValue& TotalPrice)
{
	Server_PlayerRepairAllEquipment(TotalPrice);
}

//----------------------------------------------------------------------------------------------------------------------

float IInventoryPlayerInterface::GetTotalWeight()
{
	float Weight = 0.0f;
	if (const UInventoryComponent* Inventory = GetInventoryComponent())
		Weight += Inventory->GetTotalWeight();
	if (const IEquipmentInterface* Equipment = GetEquipmentForInventory())
		Weight += Equipment->GetTotalWeight();
	if (UCoinComponent* Coin = GetCoinComponent())
		Weight += Coin->GetTotalWeight();
	if (const UStagingAreaComponent* Staging = GetStagingAreaItems())
		Weight += Staging->GetEscrowWeight();
	if (UCoinComponent* StagingCoin = GetStagingAreaCoin())
		Weight += StagingCoin->GetTotalWeight();
	if (const UTradeComponent* Trade = GetLocalTradeComponent())
		Weight += Trade->GetEscrowWeight();
	if (const UInventoryDeliveryComponent* Deliveries = GetInventoryDeliveryComponent())
		Weight += Deliveries->GetPendingDeliveryWeight();
	return Weight;
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
			const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(
				ItemStorage.ItemID, GetInventoryOwningActor()->GetWorld());
			if (const IInventoryItemFoodInterface* FoodItem = Cast<IInventoryItemFoodInterface>(Item))
			{
				const float HungerValue = FoodItem->Execute_GetHungerRestoration(Item);
				if (HungerValue > 0.f)
				{
					HandleActivation(ItemStorage.ItemID, ItemStorage.TopLeftID, BagSlot);
					return true;
				}
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------

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
			const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(
				ItemStorage.ItemID, GetInventoryOwningActor()->GetWorld());
			if (const IInventoryItemDrinkInterface* DrinkItem = Cast<IInventoryItemDrinkInterface>(Item))
			{
				const float ThirstValue = DrinkItem->Execute_GetThirstRestoration(Item);
				if (ThirstValue > 0.f)
				{
					HandleActivation(ItemStorage.ItemID, ItemStorage.TopLeftID, BagSlot);
					return true;
				}
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------

bool IInventoryPlayerInterface::CanSpendAmmo(EAmmoType AmmoType) const
{
	if (GetConstEquipmentForInventory()->HasCompatibleAmmoEquipped(AmmoType))
		return true;

	return GetInventoryComponentConst()->HasCompatibleAmmoInQuiver(AmmoType);
}

//----------------------------------------------------------------------------------------------------------------------

TScriptInterface<IInventoryItemAmmoInterface> IInventoryPlayerInterface::SpendAmmo(EAmmoType AmmoType)
{
	// first if we have ammo directly in the ammo slot we remove it
	if (auto EquippedAmmo = GetEquipmentForInventory()->RemoveAmmoEquipped(AmmoType))
		return EquippedAmmo;
	// Otherwise, if we have a quiver, we try to remove ammo from it
	if (GetInventoryComponent()->HasCompatibleAmmoInQuiver(AmmoType))
	{
		return GetInventoryComponent()->RemoveAmmoFromQuiver(AmmoType);
	}

	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
// Trade related functions -- Client (Wrappers for Server RPCs)
//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerRequestTrade(ACharacter* OtherPlayerCharacter)
{
	Server_PlayerRequestTrade(OtherPlayerCharacter);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerRequestTradeWithItem(ACharacter* OtherPlayerCharacter, int32 ItemID,
                                                           EBagSlot BagSlot, int32 TopLeft)
{
	Server_PlayerRequestTradeWithItem(OtherPlayerCharacter, ItemID, BagSlot, TopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerAcceptTradeRequest(ACharacter* RequestingPlayerCharacter)
{
	Server_PlayerAcceptTradeRequest(RequestingPlayerCharacter);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerDeclineTradeRequest(ACharacter* RequestingPlayerCharacter)
{
	Server_PlayerDeclineTradeRequest(RequestingPlayerCharacter);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerAddItemToTrade(int32 ItemID, EBagSlot BagSlot, int32 TopLeft)
{
	Server_PlayerAddItemToTrade(ItemID, BagSlot, TopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerRemoveItemFromTrade(int32 SlotIndex)
{
	Server_PlayerRemoveItemFromTrade(SlotIndex);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerSetTradeCoin(const FCoinValue& CoinAmount)
{
	Server_PlayerSetTradeCoin(CoinAmount);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerToggleTradeAcceptance(bool bAccept)
{
	Server_PlayerToggleTradeAcceptance(bAccept);
}

//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::PlayerCancelTrade()
{
	Server_PlayerCancelTrade();
}

//======================================================================================================================
// Default Server_ implementations - forward to UInventoryNetComponent
//======================================================================================================================

void IInventoryPlayerInterface::Server_MerchantTrade(AActor* InputMerchantActor)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_MerchantTrade(InputMerchantActor);
}

void IInventoryPlayerInterface::Server_StopMerchantTrade()
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_StopMerchantTrade();
}

void IInventoryPlayerInterface::Server_PlayerBuyFromMerchant(int32 ItemId, const FCoinValue& Price)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerBuyFromMerchant(ItemId, Price);
}

void IInventoryPlayerInterface::Server_PlayerSellToMerchant(EBagSlot OutSlot, int32 ItemId, int32 TopLeft,
                                                             const FCoinValue& Price)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerSellToMerchant(OutSlot, ItemId, TopLeft, Price);
}

void IInventoryPlayerInterface::Server_PlayerSellEquippedItemToMerchant(EEquipmentSlot Slot, int32 ExpectedItemId)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerSellEquippedItemToMerchant(Slot, ExpectedItemId);
}

void IInventoryPlayerInterface::Server_RepairTrade(AActor* InputRepairerActor)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_RepairTrade(InputRepairerActor);
}

void IInventoryPlayerInterface::Server_StopRepairTrade()
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_StopRepairTrade();
}

void IInventoryPlayerInterface::Server_PlayerRepairEquipment(EEquipmentSlot Slot, const FCoinValue& Price)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerRepairEquipment(Slot, Price);
}

void IInventoryPlayerInterface::Server_PlayerRepairAllEquipment(const FCoinValue& TotalPrice)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerRepairAllEquipment(TotalPrice);
}

void IInventoryPlayerInterface::Server_PlayerAutoEquipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerAutoEquipItem(InTopLeft, InSlot, InItemId);
}

void IInventoryPlayerInterface::Server_LootActor(AActor* InputLootedActor)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_LootActor(InputLootedActor);
}

void IInventoryPlayerInterface::Server_StopLooting()
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_StopLooting();
}

void IInventoryPlayerInterface::Server_PlayerLootItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId,
                                                       int32 OutTopLeft)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerLootItem(InTopLeft, InSlot, InItemId, OutTopLeft);
}

void IInventoryPlayerInterface::Server_PlayerEquipItemFromLoot(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerEquipItemFromLoot(InItemId, InSlot, OutTopLeft);
}

void IInventoryPlayerInterface::Server_PlayerAutoLootAll()
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerAutoLootAll();
}

void IInventoryPlayerInterface::Server_PlayerMoveItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId,
                                                       int32 OutTopLeft, EBagSlot OutSlot)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerMoveItem(InTopLeft, InSlot, InItemId, OutTopLeft, OutSlot);
}

void IInventoryPlayerInterface::Server_PlayerUnequipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId,
                                                          EEquipmentSlot OutSlot)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerUnequipItem(InTopLeft, InSlot, InItemId, OutSlot);
}

void IInventoryPlayerInterface::Server_PlayerEquipItemFromInventory(int32 InItemId, EEquipmentSlot InSlot,
                                                                     int32 OutTopLeft, EBagSlot OutSlot)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerEquipItemFromInventory(InItemId, InSlot, OutTopLeft, OutSlot);
}

void IInventoryPlayerInterface::Server_PlayerSwapEquipment(int32 DroppedItemId, EEquipmentSlot DroppedInSlot,
                                                            int32 SwappedItemId, EEquipmentSlot DraggedOutSlot)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerSwapEquipment(DroppedItemId, DroppedInSlot, SwappedItemId, DraggedOutSlot);
}

void IInventoryPlayerInterface::Server_TransferCoinTo(UCoinComponent* GivingComponent,
                                                       UCoinComponent* ReceivingComponent,
                                                       const FCoinValue& RemovedCoinValue,
                                                       const FCoinValue& AddedCoinValue)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_TransferCoinTo(GivingComponent, ReceivingComponent, RemovedCoinValue, AddedCoinValue);
}

void IInventoryPlayerInterface::Server_DropItemFromInventory(int32 TopLeft, EBagSlot Slot, FVector DropLocation)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_DropItemFromInventory(TopLeft, Slot, DropLocation);
}

void IInventoryPlayerInterface::Server_DropItemFromEquipment(EEquipmentSlot Slot, FVector DropLocation)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_DropItemFromEquipment(Slot, DropLocation);
}

void IInventoryPlayerInterface::Server_CancelStagingArea()
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_CancelStagingArea();
}

void IInventoryPlayerInterface::Server_ReturnStagingItem(FGuid ReservationId)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_ReturnStagingItem(ReservationId);
}

void IInventoryPlayerInterface::Server_TransferStagingToActor(AActor* TargetActor)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_TransferStagingToActor(TargetActor);
}

void IInventoryPlayerInterface::Server_MoveEquipmentToStagingArea(int32 InItemId, EEquipmentSlot OutSlot)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_MoveEquipmentToStagingArea(InItemId, OutSlot);
}

void IInventoryPlayerInterface::Server_MoveInventoryItemToStagingArea(int32 InItemId, int32 OutTopLeft,
                                                                       EBagSlot OutSlot)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_MoveInventoryItemToStagingArea(InItemId, OutTopLeft, OutSlot);
}

void IInventoryPlayerInterface::Server_PlayerAddKeyFromInventory(int32 InTopLeft, EBagSlot InSlot, int32 InItemId)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerAddKeyFromInventory(InTopLeft, InSlot, InItemId);
}

void IInventoryPlayerInterface::Server_PlayerRemoveKeyToInventory(int32 KeyId)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerRemoveKeyToInventory(KeyId);
}

void IInventoryPlayerInterface::Server_PlayerRequestTrade(ACharacter* OtherPlayerCharacter)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerRequestTrade(OtherPlayerCharacter);
}

void IInventoryPlayerInterface::Server_PlayerRequestTradeWithItem(ACharacter* OtherPlayerCharacter, int32 ItemID,
                                                                   EBagSlot BagSlot, int32 TopLeft)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerRequestTradeWithItem(OtherPlayerCharacter, ItemID, BagSlot, TopLeft);
}

void IInventoryPlayerInterface::Server_PlayerAcceptTradeRequest(ACharacter* RequestingPlayerCharacter)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerAcceptTradeRequest(RequestingPlayerCharacter);
}

void IInventoryPlayerInterface::Server_PlayerDeclineTradeRequest(ACharacter* RequestingPlayerCharacter)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerDeclineTradeRequest(RequestingPlayerCharacter);
}

void IInventoryPlayerInterface::Server_PlayerAddItemToTrade(int32 ItemID, EBagSlot BagSlot, int32 TopLeft)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerAddItemToTrade(ItemID, BagSlot, TopLeft);
}

void IInventoryPlayerInterface::Server_PlayerRemoveItemFromTrade(int32 SlotIndex)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerRemoveItemFromTrade(SlotIndex);
}

void IInventoryPlayerInterface::Server_PlayerSetTradeCoin(const FCoinValue& CoinAmount)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerSetTradeCoin(CoinAmount);
}

void IInventoryPlayerInterface::Server_PlayerToggleTradeAcceptance(bool bAccept)
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerToggleTradeAcceptance(bAccept);
}

void IInventoryPlayerInterface::Server_PlayerCancelTrade()
{
	if (UInventoryNetComponent* Comp = GetInventoryNetComponent())
		Comp->Server_PlayerCancelTrade();
}

//----------------------------------------------------------------------------------------------------------------------
// Drop wrappers
//----------------------------------------------------------------------------------------------------------------------

void IInventoryPlayerInterface::DropItemFromInventory(int32 TopLeft, EBagSlot Slot, FVector DropLocation)
{
	Server_DropItemFromInventory(TopLeft, Slot, DropLocation);
}

void IInventoryPlayerInterface::DropItemFromEquipment(EEquipmentSlot Slot, FVector DropLocation)
{
	Server_DropItemFromEquipment(Slot, DropLocation);
}

