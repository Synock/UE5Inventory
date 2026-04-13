// Copyright 2023 Maximilien (Synock) Guislain

#include "Components/InventoryNetComponent.h"
#include "InventoryPlugin.h"

#include "InventoryUtilities.h"
#include "InventoryPlugin.h"
#include "Components/CoinComponent.h"
#include "InventoryPlugin.h"
#include "Components/EquipmentComponent.h"
#include "InventoryPlugin.h"
#include "Components/InventoryComponent.h"
#include "InventoryPlugin.h"
#include "Components/LootPoolComponent.h"
#include "InventoryPlugin.h"
#include "Components/StagingAreaComponent.h"
#include "InventoryPlugin.h"
#include "Components/TradeComponent.h"
#include "InventoryPlugin.h"
#include "Interfaces/EquipmentInterface.h"
#include "InventoryPlugin.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "InventoryPlugin.h"
#include "Interfaces/LootableInterface.h"
#include "InventoryPlugin.h"
#include "Interfaces/MerchantInterface.h"
#include "InventoryPlugin.h"
#include "Items/InventoryItemBase.h"
#include "InventoryPlugin.h"
#include "Items/InventoryItemEquipable.h"
#include "InventoryPlugin.h"
#include "Items/Interfaces/InventoryItemBagInterface.h"
#include "InventoryPlugin.h"
#include "Net/UnrealNetwork.h"
#include "InventoryPlugin.h"

UInventoryNetComponent::UInventoryNetComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		PlayerInterface = Cast<IInventoryPlayerInterface>(Owner);
		if (!PlayerInterface)
		{
			UE_LOG(LogInventoryPlugin, Error, TEXT("UInventoryNetComponent: Owner %s does not implement IInventoryPlayerInterface"),
			       *Owner->GetName());
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UInventoryNetComponent, LootedActor, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryNetComponent, MerchantActor, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryNetComponent, RepairerActor, COND_OwnerOnly);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::OnRep_LootedActor()
{
}

void UInventoryNetComponent::OnRep_MerchantActor()
{
}

void UInventoryNetComponent::OnRep_RepairerActor()
{
}

//----------------------------------------------------------------------------------------------------------------------
// Helpers
//----------------------------------------------------------------------------------------------------------------------

IEquipmentInterface* UInventoryNetComponent::GetEquipmentInterface() const
{
	if (!PlayerInterface)
		return nullptr;
	return Cast<IEquipmentInterface>(PlayerInterface->GetInventoryOwningActor());
}

//----------------------------------------------------------------------------------------------------------------------

UTradeComponent* UInventoryNetComponent::GetTradeComponent() const
{
	if (AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UTradeComponent>();
	}
	return nullptr;
}

//======================================================================================================================
// RPC _Implementation / _Validate dispatch
//======================================================================================================================

// --- Inventory ---

void UInventoryNetComponent::Server_PlayerMoveItem_Implementation(int32 InTopLeft, EBagSlot InSlot, int32 InItemId,
                                                                   int32 OutTopLeft, EBagSlot OutSlot)
{
	HandlePlayerMoveItem(InTopLeft, InSlot, InItemId, OutTopLeft, OutSlot);
}

bool UInventoryNetComponent::Server_PlayerMoveItem_Validate(int32 InTopLeft, EBagSlot InSlot, int32 InItemId,
                                                             int32 OutTopLeft, EBagSlot OutSlot)
{
	return ValidatePlayerMoveItem(InTopLeft, InSlot, InItemId, OutTopLeft, OutSlot);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::Server_PlayerUnequipItem_Implementation(int32 InTopLeft, EBagSlot InSlot, int32 InItemId,
                                                                      EEquipmentSlot OutSlot)
{
	HandlePlayerUnequipItem(InTopLeft, InSlot, InItemId, OutSlot);
}

bool UInventoryNetComponent::Server_PlayerUnequipItem_Validate(int32 InTopLeft, EBagSlot InSlot, int32 InItemId,
                                                                EEquipmentSlot OutSlot)
{
	return ValidatePlayerUnequipItem(InTopLeft, InSlot, InItemId, OutSlot);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::Server_PlayerEquipItemFromInventory_Implementation(int32 InItemId, EEquipmentSlot InSlot,
                                                                                 int32 OutTopLeft, EBagSlot OutSlot)
{
	HandlePlayerEquipItemFromInventory(InItemId, InSlot, OutTopLeft, OutSlot);
}

bool UInventoryNetComponent::Server_PlayerEquipItemFromInventory_Validate(int32 InItemId, EEquipmentSlot InSlot,
                                                                           int32 OutTopLeft, EBagSlot OutSlot)
{
	return ValidatePlayerEquipItemFromInventory(InItemId, InSlot, OutTopLeft, OutSlot);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::Server_PlayerSwapEquipment_Implementation(int32 DroppedItemId,
                                                                        EEquipmentSlot DroppedInSlot,
                                                                        int32 SwappedItemId,
                                                                        EEquipmentSlot DraggedOutSlot)
{
	HandlePlayerSwapEquipment(DroppedItemId, DroppedInSlot, SwappedItemId, DraggedOutSlot);
}

bool UInventoryNetComponent::Server_PlayerSwapEquipment_Validate(int32 DroppedItemId, EEquipmentSlot DroppedInSlot,
                                                                  int32 SwappedItemId, EEquipmentSlot DraggedOutSlot)
{
	return ValidatePlayerSwapEquipment(DroppedItemId, DroppedInSlot, SwappedItemId, DraggedOutSlot);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::Server_PlayerAutoEquipItem_Implementation(int32 InTopLeft, EBagSlot InSlot,
                                                                        int32 InItemId)
{
	HandlePlayerAutoEquipItem(InTopLeft, InSlot, InItemId);
}

bool UInventoryNetComponent::Server_PlayerAutoEquipItem_Validate(int32 InTopLeft, EBagSlot InSlot, int32 InItemId)
{
	return ValidatePlayerAutoEquipItem(InTopLeft, InSlot, InItemId);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::Server_TransferCoinTo_Implementation(UCoinComponent* GivingComponent,
                                                                   UCoinComponent* ReceivingComponent,
                                                                   const FCoinValue& RemovedCoinValue,
                                                                   const FCoinValue& AddedCoinValue)
{
	HandleTransferCoinTo(GivingComponent, ReceivingComponent, RemovedCoinValue, AddedCoinValue);
}

bool UInventoryNetComponent::Server_TransferCoinTo_Validate(UCoinComponent* GivingComponent,
                                                             UCoinComponent* ReceivingComponent,
                                                             const FCoinValue& RemovedCoinValue,
                                                             const FCoinValue& AddedCoinValue)
{
	return ValidateTransferCoinTo(GivingComponent, ReceivingComponent, RemovedCoinValue, AddedCoinValue);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::Server_DropItemFromInventory_Implementation(int32 TopLeft, EBagSlot Slot,
                                                                          FVector DropLocation)
{
	HandleDropItemFromInventory(TopLeft, Slot, DropLocation);
}

bool UInventoryNetComponent::Server_DropItemFromInventory_Validate(int32 TopLeft, EBagSlot Slot, FVector DropLocation)
{
	return ValidateDropItemFromInventory(TopLeft, Slot, DropLocation);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::Server_DropItemFromEquipment_Implementation(EEquipmentSlot Slot, FVector DropLocation)
{
	HandleDropItemFromEquipment(Slot, DropLocation);
}

bool UInventoryNetComponent::Server_DropItemFromEquipment_Validate(EEquipmentSlot Slot, FVector DropLocation)
{
	return ValidateDropItemFromEquipment(Slot, DropLocation);
}

// --- Loot ---

void UInventoryNetComponent::Server_LootActor_Implementation(AActor* InputLootedActor)
{
	HandleLootActor(InputLootedActor);
}

bool UInventoryNetComponent::Server_LootActor_Validate(AActor* InputLootedActor)
{
	return ValidateLootActor(InputLootedActor);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::Server_StopLooting_Implementation()
{
	HandleStopLooting();
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::Server_PlayerLootItem_Implementation(int32 InTopLeft, EBagSlot InSlot, int32 InItemId,
                                                                    int32 OutTopLeft)
{
	HandlePlayerLootItem(InTopLeft, InSlot, InItemId, OutTopLeft);
}

bool UInventoryNetComponent::Server_PlayerLootItem_Validate(int32 InTopLeft, EBagSlot InSlot, int32 InItemId,
                                                              int32 OutTopLeft)
{
	return ValidatePlayerLootItem(InTopLeft, InSlot, InItemId, OutTopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::Server_PlayerEquipItemFromLoot_Implementation(int32 InItemId, EEquipmentSlot InSlot,
                                                                            int32 OutTopLeft)
{
	HandlePlayerEquipItemFromLoot(InItemId, InSlot, OutTopLeft);
}

bool UInventoryNetComponent::Server_PlayerEquipItemFromLoot_Validate(int32 InItemId, EEquipmentSlot InSlot,
                                                                      int32 OutTopLeft)
{
	return ValidatePlayerEquipItemFromLoot(InItemId, InSlot, OutTopLeft);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::Server_PlayerAutoLootAll_Implementation()
{
	HandlePlayerAutoLootAll();
}

// --- Merchant ---

void UInventoryNetComponent::Server_MerchantTrade_Implementation(AActor* InputMerchantActor)
{
	HandleMerchantTrade(InputMerchantActor);
}

void UInventoryNetComponent::Server_StopMerchantTrade_Implementation()
{
	HandleStopMerchantTrade();
}

void UInventoryNetComponent::Server_PlayerBuyFromMerchant_Implementation(int32 ItemId, const FCoinValue& Price)
{
	HandlePlayerBuyFromMerchant(ItemId, Price);
}

bool UInventoryNetComponent::Server_PlayerBuyFromMerchant_Validate(int32 ItemId, const FCoinValue& Price)
{
	return ValidatePlayerBuyFromMerchant(ItemId, Price);
}

void UInventoryNetComponent::Server_PlayerSellToMerchant_Implementation(EBagSlot OutSlot, int32 ItemId, int32 TopLeft,
                                                                         const FCoinValue& Price)
{
	HandlePlayerSellToMerchant(OutSlot, ItemId, TopLeft, Price);
}

bool UInventoryNetComponent::Server_PlayerSellToMerchant_Validate(EBagSlot OutSlot, int32 ItemId, int32 TopLeft,
                                                                    const FCoinValue& Price)
{
	return ValidatePlayerSellToMerchant(OutSlot, ItemId, TopLeft, Price);
}

// --- Repair ---

void UInventoryNetComponent::Server_RepairTrade_Implementation(AActor* InputRepairerActor)
{
	HandleRepairTrade(InputRepairerActor);
}

void UInventoryNetComponent::Server_StopRepairTrade_Implementation()
{
	HandleStopRepairTrade();
}

void UInventoryNetComponent::Server_PlayerRepairEquipment_Implementation(EEquipmentSlot Slot, const FCoinValue& Price)
{
	HandlePlayerRepairEquipment(Slot, Price);
}

bool UInventoryNetComponent::Server_PlayerRepairEquipment_Validate(EEquipmentSlot Slot, const FCoinValue& Price)
{
	return ValidatePlayerRepairEquipment(Slot, Price);
}

void UInventoryNetComponent::Server_PlayerRepairAllEquipment_Implementation(const FCoinValue& TotalPrice)
{
	HandlePlayerRepairAllEquipment(TotalPrice);
}

bool UInventoryNetComponent::Server_PlayerRepairAllEquipment_Validate(const FCoinValue& TotalPrice)
{
	return ValidatePlayerRepairAllEquipment(TotalPrice);
}

// --- Staging ---

void UInventoryNetComponent::Server_CancelStagingArea_Implementation()
{
	HandleCancelStagingArea();
}

void UInventoryNetComponent::Server_TransferStagingToActor_Implementation(AActor* TargetActor)
{
	HandleTransferStagingToActor(TargetActor);
}

bool UInventoryNetComponent::Server_TransferStagingToActor_Validate(AActor* TargetActor)
{
	return ValidateTransferStagingToActor(TargetActor);
}

void UInventoryNetComponent::Server_MoveEquipmentToStagingArea_Implementation(int32 InItemId, EEquipmentSlot OutSlot)
{
	HandleMoveEquipmentToStagingArea(InItemId, OutSlot);
}

bool UInventoryNetComponent::Server_MoveEquipmentToStagingArea_Validate(int32 InItemId, EEquipmentSlot OutSlot)
{
	return ValidateMoveEquipmentToStagingArea(InItemId, OutSlot);
}

void UInventoryNetComponent::Server_MoveInventoryItemToStagingArea_Implementation(int32 InItemId, int32 OutTopLeft,
                                                                                    EBagSlot OutSlot)
{
	HandleMoveInventoryItemToStagingArea(InItemId, OutTopLeft, OutSlot);
}

bool UInventoryNetComponent::Server_MoveInventoryItemToStagingArea_Validate(int32 InItemId, int32 OutTopLeft,
                                                                              EBagSlot OutSlot)
{
	return ValidateMoveInventoryItemToStagingArea(InItemId, OutTopLeft, OutSlot);
}

// --- Keys ---

void UInventoryNetComponent::Server_PlayerAddKeyFromInventory_Implementation(int32 InTopLeft, EBagSlot InSlot,
                                                                              int32 InItemId)
{
	HandlePlayerAddKeyFromInventory(InTopLeft, InSlot, InItemId);
}

bool UInventoryNetComponent::Server_PlayerAddKeyFromInventory_Validate(int32 InTopLeft, EBagSlot InSlot,
                                                                        int32 InItemId)
{
	return ValidatePlayerAddKeyFromInventory(InTopLeft, InSlot, InItemId);
}

void UInventoryNetComponent::Server_PlayerRemoveKeyToInventory_Implementation(int32 KeyId)
{
	HandlePlayerRemoveKeyToInventory(KeyId);
}

bool UInventoryNetComponent::Server_PlayerRemoveKeyToInventory_Validate(int32 KeyId)
{
	return ValidatePlayerRemoveKeyToInventory(KeyId);
}

// --- Trade ---

void UInventoryNetComponent::Server_PlayerRequestTrade_Implementation(ACharacter* OtherPlayerCharacter)
{
	HandlePlayerRequestTrade(OtherPlayerCharacter);
}

bool UInventoryNetComponent::Server_PlayerRequestTrade_Validate(ACharacter* OtherPlayerCharacter)
{
	return ValidatePlayerRequestTrade(OtherPlayerCharacter);
}

void UInventoryNetComponent::Server_PlayerRequestTradeWithItem_Implementation(ACharacter* OtherPlayerCharacter,
                                                                               int32 ItemID, EBagSlot BagSlot,
                                                                               int32 TopLeft)
{
	HandlePlayerRequestTradeWithItem(OtherPlayerCharacter, ItemID, BagSlot, TopLeft);
}

bool UInventoryNetComponent::Server_PlayerRequestTradeWithItem_Validate(ACharacter* OtherPlayerCharacter, int32 ItemID,
                                                                         EBagSlot BagSlot, int32 TopLeft)
{
	return ValidatePlayerRequestTradeWithItem(OtherPlayerCharacter, ItemID, BagSlot, TopLeft);
}

void UInventoryNetComponent::Server_PlayerAcceptTradeRequest_Implementation(ACharacter* RequestingPlayerCharacter)
{
	HandlePlayerAcceptTradeRequest(RequestingPlayerCharacter);
}

bool UInventoryNetComponent::Server_PlayerAcceptTradeRequest_Validate(ACharacter* RequestingPlayerCharacter)
{
	return ValidatePlayerAcceptTradeRequest(RequestingPlayerCharacter);
}

void UInventoryNetComponent::Server_PlayerDeclineTradeRequest_Implementation(ACharacter* RequestingPlayerCharacter)
{
	HandlePlayerDeclineTradeRequest(RequestingPlayerCharacter);
}

void UInventoryNetComponent::Server_PlayerAddItemToTrade_Implementation(int32 ItemID, EBagSlot BagSlot, int32 TopLeft)
{
	HandlePlayerAddItemToTrade(ItemID, BagSlot, TopLeft);
}

bool UInventoryNetComponent::Server_PlayerAddItemToTrade_Validate(int32 ItemID, EBagSlot BagSlot, int32 TopLeft)
{
	return ValidatePlayerAddItemToTrade(ItemID, BagSlot, TopLeft);
}

void UInventoryNetComponent::Server_PlayerRemoveItemFromTrade_Implementation(int32 SlotIndex)
{
	HandlePlayerRemoveItemFromTrade(SlotIndex);
}

void UInventoryNetComponent::Server_PlayerSetTradeCoin_Implementation(const FCoinValue& CoinAmount)
{
	HandlePlayerSetTradeCoin(CoinAmount);
}

bool UInventoryNetComponent::Server_PlayerSetTradeCoin_Validate(const FCoinValue& CoinAmount)
{
	return ValidatePlayerSetTradeCoin(CoinAmount);
}

void UInventoryNetComponent::Server_PlayerToggleTradeAcceptance_Implementation(bool bAccept)
{
	HandlePlayerToggleTradeAcceptance(bAccept);
}

void UInventoryNetComponent::Server_PlayerCancelTrade_Implementation()
{
	HandlePlayerCancelTrade();
}

//======================================================================================================================
// Default Handle* implementations
//======================================================================================================================

void UInventoryNetComponent::HandlePlayerMoveItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft,
                                                   EBagSlot OutSlot)
{
	if (!PlayerInterface)
		return;

	const float ItemDurability = PlayerInterface->PlayerRemoveItem(OutTopLeft, OutSlot);
	PlayerInterface->PlayerAddItemWithDurability(InTopLeft, InSlot, InItemId, ItemDurability);
	PlayerInterface->ResetTransaction();
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandlePlayerUnequipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId,
                                                      EEquipmentSlot OutSlot)
{
	if (!PlayerInterface)
		return;

	IEquipmentInterface* Equipment = GetEquipmentInterface();
	if (!Equipment)
		return;

	float ItemDurability = 100.0f;
	Equipment->GetEquipmentComponent()->GetEquipmentDurability(OutSlot, ItemDurability);
	Equipment->UnequipItem(OutSlot);
	PlayerInterface->PlayerAddItemWithDurability(InTopLeft, InSlot, InItemId, ItemDurability);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandlePlayerEquipItemFromInventory(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft,
                                                                 EBagSlot OutSlot)
{
	if (!PlayerInterface)
		return;

	IEquipmentInterface* Equipment = GetEquipmentInterface();
	if (!Equipment)
		return;

	const float ItemDurability = PlayerInterface->PlayerRemoveItem(OutTopLeft, OutSlot);
	Equipment->EquipItemWithDurability(InSlot, InItemId, ItemDurability);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandlePlayerSwapEquipment(int32 DroppedItemId, EEquipmentSlot DroppedInSlot,
                                                        int32 SwappedItemId, EEquipmentSlot DraggedOutSlot)
{
	IEquipmentInterface* Equipment = GetEquipmentInterface();
	if (!Equipment)
		return;

	const UInventoryItemEquipable* ItemToMove = Equipment->GetEquippedItem(DroppedInSlot);
	const UInventoryItemEquipable* DroppedItem = Equipment->GetEquippedItem(DraggedOutSlot);

	Equipment->GetEquipmentComponent()->RemoveItem(DroppedInSlot);
	if (ItemToMove)
		Equipment->HandleUnEquipmentEffect(DroppedInSlot, ItemToMove);
	Equipment->GetEquipmentComponent()->EquipItem(DroppedItem, DroppedInSlot);
	Equipment->HandleEquipmentEffect(DroppedInSlot, DroppedItem);

	Equipment->GetEquipmentComponent()->RemoveItem(DraggedOutSlot);
	Equipment->HandleUnEquipmentEffect(DraggedOutSlot, DroppedItem);
	if (ItemToMove)
	{
		Equipment->GetEquipmentComponent()->EquipItem(ItemToMove, DraggedOutSlot);
		Equipment->HandleEquipmentEffect(DraggedOutSlot, ItemToMove);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandlePlayerAutoEquipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId)
{
	if (!PlayerInterface)
		return;

	EEquipmentSlot SlotToEquip;
	if (!PlayerInterface->PlayerTryAutoEquip(InItemId, SlotToEquip))
		return;

	// Re-use the equip-from-inventory path
	HandlePlayerEquipItemFromInventory(InItemId, SlotToEquip, InTopLeft, InSlot);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandleTransferCoinTo(UCoinComponent* GivingComponent, UCoinComponent* ReceivingComponent,
                                                   const FCoinValue& RemovedCoinValue, const FCoinValue& AddedCoinValue)
{
	if (GivingComponent)
		GivingComponent->PayAndAdjust(RemovedCoinValue);

	if (ReceivingComponent)
		ReceivingComponent->AddCoins(AddedCoinValue);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandleDropItemFromInventory(int32 TopLeft, EBagSlot Slot, FVector DropLocation)
{
	// No sensible generic default - game subclass must override to spawn a world actor.
	UE_LOG(LogInventoryPlugin, Warning,
	       TEXT(
		       "UInventoryNetComponent::HandleDropItemFromInventory: No override provided. Item removed but not spawned in world."
	       ));
	if (PlayerInterface)
		PlayerInterface->PlayerRemoveItem(TopLeft, Slot);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandleDropItemFromEquipment(EEquipmentSlot Slot, FVector DropLocation)
{
	// No sensible generic default - game subclass must override to spawn a world actor.
	UE_LOG(LogInventoryPlugin, Warning,
	       TEXT(
		       "UInventoryNetComponent::HandleDropItemFromEquipment: No override provided. Item unequipped but not spawned in world."
	       ));
	if (IEquipmentInterface* Equipment = GetEquipmentInterface())
		Equipment->UnequipItem(Slot);
}

//----------------------------------------------------------------------------------------------------------------------
// Loot
//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandleLootActor(AActor* InputLootedActor)
{
	if (!LootedActor)
	{
		if (ILootableInterface* LootInterface = Cast<ILootableInterface>(InputLootedActor))
		{
			LootedActor = InputLootedActor;
			LootInterface->StartLooting(PlayerInterface ? PlayerInterface->GetInventoryOwningActor() : GetOwner());
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandleStopLooting()
{
	if (LootedActor)
	{
		if (ILootableInterface* Lootable = Cast<ILootableInterface>(LootedActor))
			Lootable->StopLooting(PlayerInterface ? PlayerInterface->GetInventoryOwningActor() : GetOwner());
		LootedActor = nullptr;
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandlePlayerLootItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft)
{
	if (!PlayerInterface || !LootedActor)
		return;

	ILootableInterface* Loot = Cast<ILootableInterface>(LootedActor);
	if (!Loot)
		return;

	// Preserve durability from loot pool
	float Durability = 100.0f;
	if (ULootPoolComponent* LootPool = Loot->GetLootPoolComponent())
	{
		for (const FMinimalItemStorage& ItemStorage : LootPool->GetBagConst())
		{
			if (ItemStorage.TopLeftID == OutTopLeft && ItemStorage.ItemID == InItemId)
			{
				Durability = ItemStorage.Durability;
				break;
			}
		}
	}

	Loot->RemoveItem(OutTopLeft);
	PlayerInterface->PlayerAddItemWithDurability(InTopLeft, InSlot, InItemId, Durability);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandlePlayerEquipItemFromLoot(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft)
{
	if (!LootedActor)
		return;

	ILootableInterface* Loot = Cast<ILootableInterface>(LootedActor);
	IEquipmentInterface* Equipment = GetEquipmentInterface();
	if (!Loot || !Equipment)
		return;

	// Preserve durability from loot pool
	float Durability = 100.0f;
	if (ULootPoolComponent* LootPool = Loot->GetLootPoolComponent())
	{
		for (const FMinimalItemStorage& ItemStorage : LootPool->GetBagConst())
		{
			if (ItemStorage.TopLeftID == OutTopLeft && ItemStorage.ItemID == InItemId)
			{
				Durability = ItemStorage.Durability;
				break;
			}
		}
	}

	Loot->RemoveItem(OutTopLeft);
	Equipment->EquipItemWithDurability(InSlot, InItemId, Durability);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandlePlayerAutoLootAll()
{
	if (!PlayerInterface || !LootedActor)
		return;

	ILootableInterface* Loot = Cast<ILootableInterface>(LootedActor);
	if (!Loot || !Loot->GetLootPoolComponent())
		return;

	TArray<FMinimalItemStorage> ItemsCopy = Loot->GetLootPoolComponent()->GetBagConst();
	for (const FMinimalItemStorage& ItemStorage : ItemsCopy)
	{
		EEquipmentSlot TriedSlot = EEquipmentSlot::Unknown;
		EBagSlot TriedBag = EBagSlot::Unknown;
		int32 InTopLeft = -1;

		if (PlayerInterface->PlayerTryAutoLootFunction(ItemStorage.ItemID, TriedSlot, InTopLeft, TriedBag))
		{
			if (TriedSlot != EEquipmentSlot::Unknown)
			{
				HandlePlayerEquipItemFromLoot(ItemStorage.ItemID, TriedSlot, ItemStorage.TopLeftID);
			}
			else if (TriedBag != EBagSlot::Unknown)
			{
				HandlePlayerLootItem(InTopLeft, TriedBag, ItemStorage.ItemID, ItemStorage.TopLeftID);
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
// Merchant
//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandleMerchantTrade(AActor* InputMerchantActor)
{
	if (InputMerchantActor && InputMerchantActor->Implements<UMerchantInterface>())
	{
		MerchantActor = InputMerchantActor;
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandleStopMerchantTrade()
{
	MerchantActor = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandlePlayerBuyFromMerchant(int32 ItemId, const FCoinValue& Price)
{
	if (!PlayerInterface || !MerchantActor)
		return;

	IMerchantInterface* Merchant = Cast<IMerchantInterface>(MerchantActor);
	if (!Merchant || !Merchant->HasItem(ItemId))
		return;

	// Deduct coins
	PlayerInterface->GetCoinComponent()->PayAndAdjust(Price);

	// Remove from merchant
	Merchant->RemoveItemAmountIfNeeded(ItemId);

	// Add to player
	EEquipmentSlot TriedSlot = EEquipmentSlot::Unknown;
	EBagSlot TriedBag = EBagSlot::Unknown;
	int32 InTopLeft = -1;

	if (PlayerInterface->PlayerTryAutoLootFunction(ItemId, TriedSlot, InTopLeft, TriedBag))
	{
		IEquipmentInterface* Equipment = GetEquipmentInterface();
		if (TriedSlot != EEquipmentSlot::Unknown && Equipment)
		{
			Equipment->EquipItem(TriedSlot, ItemId);
		}
		else if (TriedBag != EBagSlot::Unknown)
		{
			PlayerInterface->PlayerAddItem(InTopLeft, TriedBag, ItemId);
		}
	}

	// Pay merchant
	Merchant->ReceiveCoin(Price);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandlePlayerSellToMerchant(EBagSlot OutSlot, int32 ItemId, int32 TopLeft,
                                                         const FCoinValue& Price)
{
	if (!PlayerInterface || !MerchantActor)
		return;

	IMerchantInterface* Merchant = Cast<IMerchantInterface>(MerchantActor);
	if (!Merchant || !Merchant->CanPayAmount(Price))
		return;

	PlayerInterface->PlayerRemoveItem(TopLeft, OutSlot);
	Merchant->PayCoin(Price);
	Merchant->AddDynamicItem(ItemId);
	PlayerInterface->GetCoinComponent()->AddCoins(Price);
}

//----------------------------------------------------------------------------------------------------------------------
// Repair
//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandleRepairTrade(AActor* InputRepairerActor)
{
	if (InputRepairerActor)
		RepairerActor = InputRepairerActor;
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandleStopRepairTrade()
{
	RepairerActor = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandlePlayerRepairEquipment(EEquipmentSlot Slot, const FCoinValue& Price)
{
	if (!PlayerInterface || !RepairerActor)
		return;

	IEquipmentInterface* Equipment = GetEquipmentInterface();
	if (!Equipment)
		return;

	UEquipmentComponent* EquipComp = Equipment->GetEquipmentComponent();
	if (!EquipComp)
		return;

	const UInventoryItemEquipable* Item = EquipComp->GetItemAtSlot(Slot);
	if (!Item)
		return;

	float CurrentDurability = 0.0f;
	if (!EquipComp->GetEquipmentDurability(Slot, CurrentDurability))
		return;

	if (CurrentDurability >= Item->GetTotalDurability())
		return;

	if (!PlayerInterface->PlayerCanPayAmount(Price))
		return;

	PlayerInterface->GetCoinComponent()->PayAndAdjust(Price);

	if (IMerchantInterface* MerchantInterface = Cast<IMerchantInterface>(RepairerActor))
		MerchantInterface->ReceiveCoin(Price);

	EquipComp->SetEquipmentDurability(Slot, Item->GetTotalDurability());
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandlePlayerRepairAllEquipment(const FCoinValue& TotalPrice)
{
	if (!PlayerInterface || !RepairerActor)
		return;

	IEquipmentInterface* Equipment = GetEquipmentInterface();
	if (!Equipment)
		return;

	UEquipmentComponent* EquipComp = Equipment->GetEquipmentComponent();
	if (!EquipComp)
		return;

	if (!PlayerInterface->PlayerCanPayAmount(TotalPrice))
		return;

	PlayerInterface->GetCoinComponent()->PayAndAdjust(TotalPrice);

	if (IMerchantInterface* MerchantInterface = Cast<IMerchantInterface>(RepairerActor))
		MerchantInterface->ReceiveCoin(TotalPrice);

	const TArray<const UInventoryItemEquipable*>& AllEquipment = EquipComp->GetAllEquipment();
	for (int32 i = 0; i < AllEquipment.Num(); ++i)
	{
		if (const UInventoryItemEquipable* Item = AllEquipment[i])
		{
			const EEquipmentSlot ItemSlot = static_cast<EEquipmentSlot>(i);
			float CurrentDurability = 0.0f;
			if (EquipComp->GetEquipmentDurability(ItemSlot, CurrentDurability))
			{
				if (CurrentDurability < Item->GetTotalDurability())
					EquipComp->SetEquipmentDurability(ItemSlot, Item->GetTotalDurability());
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
// Staging
//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandleCancelStagingArea()
{
	if (!PlayerInterface)
		return;

	UCoinComponent* StagingCoin = PlayerInterface->GetStagingAreaCoin();
	UStagingAreaComponent* StagingItems = PlayerInterface->GetStagingAreaItems();

	if (StagingCoin)
	{
		PlayerInterface->GetCoinComponent()->AddCoins(StagingCoin->GetPurseContent());
		StagingCoin->ClearPurse();
	}

	if (StagingItems)
	{
		for (const FMinimalItemStorage& ItemStorage : StagingItems->GetStagingAreaItems())
		{
			EEquipmentSlot TriedSlot = EEquipmentSlot::Unknown;
			EBagSlot TriedBag = EBagSlot::Unknown;
			int32 InTopLeft = -1;

			if (!PlayerInterface->PlayerTryAutoLootFunction(ItemStorage.ItemID, TriedSlot, InTopLeft, TriedBag))
			{
				UE_LOG(LogInventoryPlugin, Error, TEXT("UInventoryNetComponent: Cannot put item %d back from staging"),
				       ItemStorage.ItemID);
				continue;
			}

			IEquipmentInterface* Equipment = GetEquipmentInterface();
			if (TriedSlot != EEquipmentSlot::Unknown && Equipment)
			{
				Equipment->EquipItemWithDurability(TriedSlot, ItemStorage.ItemID, ItemStorage.Durability);
			}
			else if (TriedBag != EBagSlot::Unknown)
			{
				PlayerInterface->PlayerAddItemWithDurability(InTopLeft, TriedBag, ItemStorage.ItemID,
				                                            ItemStorage.Durability);
			}
		}
		StagingItems->ClearStagingArea();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandleTransferStagingToActor(AActor* TargetActor)
{
	// No sensible generic default for transferring to an NPC/actor.
	// Game subclass should override this to handle NPC-specific logic (e.g. HandlePlayerGive).
	UE_LOG(LogInventoryPlugin, Warning,
	       TEXT(
		       "UInventoryNetComponent::HandleTransferStagingToActor: No override provided. Staging area not transferred."
	       ));
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandleMoveEquipmentToStagingArea(int32 InItemId, EEquipmentSlot OutSlot)
{
	if (!PlayerInterface)
		return;

	IEquipmentInterface* Equipment = GetEquipmentInterface();
	UStagingAreaComponent* StagingItems = PlayerInterface->GetStagingAreaItems();
	if (!Equipment || !StagingItems)
		return;

	float ItemDurability = 100.0f;
	if (UEquipmentComponent* EquipComp = Equipment->GetEquipmentComponent())
		EquipComp->GetEquipmentDurability(OutSlot, ItemDurability);

	FMinimalItemStorage ItemStorage;
	ItemStorage.ItemID = InItemId;
	ItemStorage.TopLeftID = 0;
	ItemStorage.Durability = ItemDurability;
	ItemStorage.bIsLocked = false;

	StagingItems->AddItemToStagingArea(ItemStorage);
	Equipment->UnequipItem(OutSlot);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandleMoveInventoryItemToStagingArea(int32 InItemId, int32 OutTopLeft, EBagSlot OutSlot)
{
	if (!PlayerInterface)
		return;

	UStagingAreaComponent* StagingItems = PlayerInterface->GetStagingAreaItems();
	if (!StagingItems)
		return;

	// Get current durability from inventory before moving
	float ItemDurability = 100.0f;
	if (UInventoryComponent* InventoryComp = PlayerInterface->GetInventoryComponent())
	{
		const TArray<FMinimalItemStorage>& BagContents = InventoryComp->GetBagConst(OutSlot);
		for (const FMinimalItemStorage& Item : BagContents)
		{
			if (Item.TopLeftID == OutTopLeft && Item.ItemID == InItemId)
			{
				ItemDurability = Item.Durability;
				break;
			}
		}
	}

	FMinimalItemStorage ItemStorage;
	ItemStorage.ItemID = InItemId;
	ItemStorage.TopLeftID = 0;
	ItemStorage.Durability = ItemDurability;
	ItemStorage.bIsLocked = false;

	StagingItems->AddItemToStagingArea(ItemStorage);
	PlayerInterface->PlayerRemoveItem(OutTopLeft, OutSlot);
}

//----------------------------------------------------------------------------------------------------------------------
// Keys
//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandlePlayerAddKeyFromInventory(int32 InTopLeft, EBagSlot InSlot, int32 InItemId)
{
	if (PlayerInterface)
		PlayerInterface->Internal_PlayerAddKeyFromInventory(InTopLeft, InSlot, InItemId);
}

void UInventoryNetComponent::HandlePlayerRemoveKeyToInventory(int32 KeyId)
{
	if (PlayerInterface)
		PlayerInterface->Internal_PlayerRemoveKeyToInventory(KeyId);
}

//----------------------------------------------------------------------------------------------------------------------
// Trade
//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandlePlayerRequestTrade(ACharacter* OtherPlayerCharacter)
{
	UTradeComponent* Trade = GetTradeComponent();
	if (Trade)
		Trade->StartTrade(OtherPlayerCharacter);
}

void UInventoryNetComponent::HandlePlayerRequestTradeWithItem(ACharacter* OtherPlayerCharacter, int32 ItemID,
                                                               EBagSlot BagSlot, int32 TopLeft)
{
	UTradeComponent* Trade = GetTradeComponent();
	if (Trade)
	{
		if (Trade->StartTrade(OtherPlayerCharacter))
			Trade->AddItemToOffer(ItemID, BagSlot, TopLeft);
	}
}

void UInventoryNetComponent::HandlePlayerAcceptTradeRequest(ACharacter* RequestingPlayerCharacter)
{
	// No-op by default - deprecated pattern. Override if needed.
}

void UInventoryNetComponent::HandlePlayerDeclineTradeRequest(ACharacter* RequestingPlayerCharacter)
{
	// No-op by default. Override to add chat notifications.
}

void UInventoryNetComponent::HandlePlayerAddItemToTrade(int32 ItemID, EBagSlot BagSlot, int32 TopLeft)
{
	if (UTradeComponent* Trade = GetTradeComponent(); Trade && Trade->IsTrading())
		Trade->AddItemToOffer(ItemID, BagSlot, TopLeft);
}

void UInventoryNetComponent::HandlePlayerRemoveItemFromTrade(int32 SlotIndex)
{
	if (UTradeComponent* Trade = GetTradeComponent(); Trade && Trade->IsTrading())
		Trade->RemoveItemFromOffer(SlotIndex);
}

void UInventoryNetComponent::HandlePlayerSetTradeCoin(const FCoinValue& CoinAmount)
{
	// Default: no-op. Override to set coin on trade component.
}

void UInventoryNetComponent::HandlePlayerToggleTradeAcceptance(bool bAccept)
{
	UTradeComponent* Trade = GetTradeComponent();
	if (Trade && Trade->IsTrading())
		Trade->SetAcceptance(bAccept);
}

void UInventoryNetComponent::HandlePlayerCancelTrade()
{
	UTradeComponent* Trade = GetTradeComponent();
	if (Trade && Trade->IsTrading())
		Trade->CancelTrade();
}

//======================================================================================================================
// Default Validate* implementations - built-in anti-cheat
//======================================================================================================================

bool UInventoryNetComponent::ValidatePlayerMoveItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft,
                                                     EBagSlot OutSlot)
{
	if (!PlayerInterface)
		return false;

	const int32 SourceItemId = PlayerInterface->PlayerGetItem(OutTopLeft, OutSlot);
	if (SourceItemId != InItemId)
		return false;

	if ((InSlot == EBagSlot::Unknown || InSlot >= EBagSlot::LastValidBag) && InSlot != EBagSlot::BankPool)
		return false;

	if (InItemId <= 0)
		return false;

	const UInventoryItemBase* ItemBase = UInventoryUtilities::GetItemFromID(InItemId, GetWorld());
	return ItemBase != nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerUnequipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId,
                                                        EEquipmentSlot OutSlot)
{
	IEquipmentInterface* Equipment = GetEquipmentInterface();
	if (!Equipment || !PlayerInterface)
		return false;

	const UInventoryItemEquipable* ConsideredItem = Equipment->GetEquippedItem(OutSlot);
	if (!ConsideredItem || ConsideredItem->ItemID <= 0)
		return false;

	if (Cast<IInventoryItemBagInterface>(ConsideredItem))
	{
		if (PlayerInterface->GetInventoryComponent()->GetBagConst(
			UInventoryComponent::GetBagSlotFromInventory(OutSlot)).Num() > 0)
			return false;
	}

	return ConsideredItem->ItemID == InItemId;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerEquipItemFromInventory(int32 InItemId, EEquipmentSlot InSlot,
                                                                    int32 OutTopLeft, EBagSlot OutSlot)
{
	if (!PlayerInterface)
		return false;

	if (InItemId <= 0)
		return false;

	if (PlayerInterface->PlayerGetItem(OutTopLeft, OutSlot) != InItemId)
		return false;

	IEquipmentInterface* Equipment = GetEquipmentInterface();
	if (!Equipment)
		return false;

	if (Equipment->GetEquippedItem(InSlot) != nullptr)
		return false;

	const UInventoryItemEquipable* LocalItem = Cast<UInventoryItemEquipable>(
		UInventoryUtilities::GetItemFromID(InItemId, GetWorld()));
	return LocalItem && LocalItem->ItemID > 0;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerSwapEquipment(int32 DroppedItemId, EEquipmentSlot DroppedInSlot,
                                                          int32 SwappedItemId, EEquipmentSlot DraggedOutSlot)
{
	IEquipmentInterface* Equipment = GetEquipmentInterface();
	if (!Equipment)
		return false;

	if (DroppedInSlot == EEquipmentSlot::Unknown || DraggedOutSlot == EEquipmentSlot::Unknown)
		return false;

	const UInventoryItemEquipable* DraggedItem = Equipment->GetEquippedItem(DraggedOutSlot);
	if (DraggedItem && DraggedItem->ItemID != DroppedItemId)
		return false;

	const UInventoryItemEquipable* DroppedInItem = Equipment->GetEquippedItem(DroppedInSlot);
	if (DroppedInItem && DroppedInItem->ItemID != SwappedItemId)
		return false;

	if (DraggedItem && DraggedItem->MultiSlotItem)
		return DroppedInItem && DroppedInItem->MultiSlotItem;

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerAutoEquipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId)
{
	if (!PlayerInterface)
		return false;

	return PlayerInterface->PlayerGetItem(InTopLeft, InSlot) == InItemId;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidateTransferCoinTo(UCoinComponent* GivingComponent,
                                                     UCoinComponent* ReceivingComponent,
                                                     const FCoinValue& RemovedCoinValue,
                                                     const FCoinValue& AddedCoinValue)
{
	if (!PlayerInterface)
		return false;

	if (GivingComponent && ReceivingComponent)
	{
		AActor* OwningActor = PlayerInterface->GetInventoryOwningActor();
		if (ReceivingComponent->GetOwner() != GetOwner() && ReceivingComponent->GetOwner() != OwningActor)
			return false;
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidateDropItemFromInventory(int32 TopLeft, EBagSlot Slot, FVector DropLocation)
{
	if (!PlayerInterface)
		return false;

	return PlayerInterface->PlayerGetItem(TopLeft, Slot) > 0;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidateDropItemFromEquipment(EEquipmentSlot Slot, FVector DropLocation)
{
	IEquipmentInterface* Equipment = GetEquipmentInterface();
	if (!Equipment)
		return false;

	return Equipment->GetEquippedItem(Slot) != nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidateLootActor(AActor* InputLootedActor)
{
	return InputLootedActor != nullptr && Cast<ILootableInterface>(InputLootedActor) != nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerLootItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft)
{
	if (!LootedActor)
		return false;

	const ILootableInterface* Loot = Cast<ILootableInterface>(LootedActor);
	if (!Loot)
		return false;

	return Loot->GetItemData(OutTopLeft) == InItemId;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerEquipItemFromLoot(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft)
{
	if (!LootedActor)
		return false;

	const ILootableInterface* Loot = Cast<ILootableInterface>(LootedActor);
	if (!Loot)
		return false;

	return Loot->GetItemData(OutTopLeft) == InItemId;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerBuyFromMerchant(int32 ItemId, const FCoinValue& Price)
{
	if (!MerchantActor || !PlayerInterface)
		return false;

	return PlayerInterface->PlayerCanPayAmount(Price);
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerSellToMerchant(EBagSlot OutSlot, int32 ItemId, int32 TopLeft,
                                                            const FCoinValue& Price)
{
	if (!MerchantActor || !PlayerInterface)
		return false;

	return PlayerInterface->PlayerGetItem(TopLeft, OutSlot) == ItemId;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerRepairEquipment(EEquipmentSlot Slot, const FCoinValue& Price)
{
	if (!RepairerActor)
		return false;

	IEquipmentInterface* Equipment = GetEquipmentInterface();
	if (!Equipment)
		return false;

	return Equipment->GetEquippedItem(Slot) != nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerRepairAllEquipment(const FCoinValue& TotalPrice)
{
	if (!RepairerActor)
		return false;

	IEquipmentInterface* Equipment = GetEquipmentInterface();
	return Equipment && Equipment->GetEquipmentComponent() != nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidateTransferStagingToActor(AActor* TargetActor)
{
	return TargetActor != nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidateMoveEquipmentToStagingArea(int32 InItemId, EEquipmentSlot OutSlot)
{
	IEquipmentInterface* Equipment = GetEquipmentInterface();
	if (!Equipment)
		return false;

	const UInventoryItemEquipable* Item = Equipment->GetEquippedItem(OutSlot);
	return Item && Item->ItemID == InItemId;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidateMoveInventoryItemToStagingArea(int32 InItemId, int32 OutTopLeft, EBagSlot OutSlot)
{
	if (!PlayerInterface)
		return false;

	return PlayerInterface->PlayerGetItem(OutTopLeft, OutSlot) == InItemId;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerAddKeyFromInventory(int32 InTopLeft, EBagSlot InSlot, int32 InItemId)
{
	if (!PlayerInterface)
		return false;

	return PlayerInterface->PlayerGetItem(InTopLeft, InSlot) == InItemId;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerRemoveKeyToInventory(int32 KeyId)
{
	return PlayerInterface != nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerRequestTrade(ACharacter* OtherPlayerCharacter)
{
	return OtherPlayerCharacter != nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerRequestTradeWithItem(ACharacter* OtherPlayerCharacter, int32 ItemID,
                                                                  EBagSlot BagSlot, int32 TopLeft)
{
	if (!OtherPlayerCharacter || !PlayerInterface)
		return false;

	return PlayerInterface->PlayerGetItem(TopLeft, BagSlot) == ItemID;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerAcceptTradeRequest(ACharacter* RequestingPlayerCharacter)
{
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerAddItemToTrade(int32 ItemID, EBagSlot BagSlot, int32 TopLeft)
{
	if (BagSlot == EBagSlot::Unknown || !PlayerInterface)
		return false;

	return PlayerInterface->PlayerGetItem(TopLeft, BagSlot) == ItemID;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerSetTradeCoin(const FCoinValue& CoinAmount)
{
	if (!PlayerInterface)
		return false;

	return PlayerInterface->PlayerCanPayAmount(CoinAmount);
}

