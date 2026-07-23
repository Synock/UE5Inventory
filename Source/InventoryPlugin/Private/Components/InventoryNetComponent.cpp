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
#include "Components/InventoryDeliveryComponent.h"
#include "InventoryPlugin.h"
#include "Components/LootPoolComponent.h"
#include "InventoryPlugin.h"
#include "Components/StagingAreaComponent.h"
#include "Components/StagingReturnRouting.h"
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
#include "Interfaces/RepairInterface.h"
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

void UInventoryNetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		HandleCancelStagingArea();
		HandleStopLooting();
	}

	Super::EndPlay(EndPlayReason);
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

void UInventoryNetComponent::Client_LootRequestRejected_Implementation()
{
	if (!PlayerInterface)
	{
		PlayerInterface = Cast<IInventoryPlayerInterface>(GetOwner());
	}

	if (PlayerInterface)
	{
		PlayerInterface->ResetTransaction();
	}

	if (LootedActor)
	{
		if (ILootableInterface* Lootable = Cast<ILootableInterface>(LootedActor))
		{
			Lootable->GetLootPoolDelegate().Broadcast();
		}
	}
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

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::OwnsActiveLootSession(const ILootableInterface* Lootable) const
{
	return Lootable
		&& LootedActor
		&& LootedActor->GetOwner() == GetOwner()
		&& Lootable->GetIsBeingLooted();
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::RejectLootRequest()
{
#if WITH_AUTOMATION_WORKER
	++RejectedLootRequestCountForTests;
#endif

	if (LootedActor)
	{
		LootedActor->FlushNetDormancy();
		LootedActor->ForceNetUpdate();
	}

	Client_LootRequestRejected();
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

void UInventoryNetComponent::Server_ClaimPendingDelivery_Implementation(FGuid DeliveryId)
{
	HandleClaimPendingDelivery(DeliveryId);
}

bool UInventoryNetComponent::Server_ClaimPendingDelivery_Validate(FGuid DeliveryId)
{
	return ValidateClaimPendingDelivery(DeliveryId);
}

void UInventoryNetComponent::Server_ClaimAllPendingDeliveries_Implementation()
{
	HandleClaimAllPendingDeliveries();
}

void UInventoryNetComponent::Server_ClaimPendingDeliveryAt_Implementation(FGuid DeliveryId,
	FInventoryDeliveryDestination Destination)
{
	HandleClaimPendingDeliveryAt(DeliveryId, Destination);
}

bool UInventoryNetComponent::Server_ClaimPendingDeliveryAt_Validate(FGuid DeliveryId,
	FInventoryDeliveryDestination Destination)
{
	return ValidateClaimPendingDeliveryAt(DeliveryId, Destination);
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
	if (LootedActor || !InputLootedActor || !GetOwner())
		return;

	ILootableInterface* LootInterface = Cast<ILootableInterface>(InputLootedActor);
	if (!LootInterface || LootInterface->GetIsBeingLooted())
		return;

	AActor* Looter = PlayerInterface ? PlayerInterface->GetInventoryOwningActor() : GetOwner();
	if (!Looter)
		return;

	PreviousLootOwner = InputLootedActor->GetOwner();
	LootInterface->StartLooting(Looter);

	// Custom lootables may reject the looter. Do not publish a session unless the target accepted it.
	if (!LootInterface->GetIsBeingLooted())
	{
		if (InputLootedActor->GetOwner() != PreviousLootOwner)
			InputLootedActor->SetOwner(PreviousLootOwner);
		PreviousLootOwner = nullptr;
		return;
	}

	InputLootedActor->FlushNetDormancy();
	InputLootedActor->SetOwner(GetOwner());
	InputLootedActor->ForceNetUpdate();
	LootedActor = InputLootedActor;
	GetOwner()->ForceNetUpdate();
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandleStopLooting()
{
	AActor* SessionActor = LootedActor.Get();
	LootedActor = nullptr;

	if (!SessionActor)
	{
		PreviousLootOwner = nullptr;
		return;
	}

	// If ownership changed unexpectedly, never unlock or retarget another player's session.
	if (SessionActor->GetOwner() != GetOwner())
	{
		PreviousLootOwner = nullptr;
		GetOwner()->ForceNetUpdate();
		return;
	}

	AActor* Looter = PlayerInterface ? PlayerInterface->GetInventoryOwningActor() : GetOwner();
	if (ILootableInterface* Lootable = Cast<ILootableInterface>(SessionActor))
	{
		Lootable->StopLooting(Looter);
	}

	if (IsValid(SessionActor))
	{
		SessionActor->FlushNetDormancy();
		SessionActor->SetOwner(PreviousLootOwner);
		SessionActor->ForceNetUpdate();
	}

	PreviousLootOwner = nullptr;
	if (GetOwner())
		GetOwner()->ForceNetUpdate();
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandlePlayerLootItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft)
{
	if (!PlayerInterface || !LootedActor)
	{
		RejectLootRequest();
		return;
	}

	ILootableInterface* Loot = Cast<ILootableInterface>(LootedActor);
	if (!OwnsActiveLootSession(Loot) || Loot->GetItemData(OutTopLeft) != InItemId)
	{
		RejectLootRequest();
		return;
	}

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
	if (LootedActor)
	{
		LootedActor->FlushNetDormancy();
		LootedActor->ForceNetUpdate();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandlePlayerEquipItemFromLoot(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft)
{
	if (!LootedActor)
	{
		RejectLootRequest();
		return;
	}

	ILootableInterface* Loot = Cast<ILootableInterface>(LootedActor);
	IEquipmentInterface* Equipment = GetEquipmentInterface();
	if (!OwnsActiveLootSession(Loot) || !Equipment || Loot->GetItemData(OutTopLeft) != InItemId)
	{
		RejectLootRequest();
		return;
	}

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
	if (LootedActor)
	{
		LootedActor->FlushNetDormancy();
		LootedActor->ForceNetUpdate();
	}
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

	const FCoinValue ServerPrice = Merchant->GetItemPriceSell(ItemId);
	if (!Price.IsNonNegative() || !PlayerInterface->PlayerCanPayAmount(ServerPrice))
		return;

	// Deduct coins
	PlayerInterface->GetCoinComponent()->PayAndAdjust(ServerPrice);

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
	Merchant->ReceiveCoin(ServerPrice);
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandlePlayerSellToMerchant(EBagSlot OutSlot, int32 ItemId, int32 TopLeft,
                                                         const FCoinValue& Price)
{
	if (!PlayerInterface || !MerchantActor)
		return;

	if (PlayerInterface->PlayerGetItem(TopLeft, OutSlot) != ItemId)
		return;

	IMerchantInterface* Merchant = Cast<IMerchantInterface>(MerchantActor);
	if (!Merchant)
		return;

	const FCoinValue ServerPrice = Merchant->GetItemPriceBuy(ItemId);
	if (!Price.IsNonNegative() || !Merchant->CanPayAmount(ServerPrice))
		return;

	PlayerInterface->PlayerRemoveItem(TopLeft, OutSlot);
	Merchant->PayCoin(ServerPrice);
	Merchant->AddDynamicItem(ItemId);
	PlayerInterface->GetCoinComponent()->AddCoins(ServerPrice);
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

	const IRepairInterface* RepairInterface = Cast<IRepairInterface>(RepairerActor);
	if (!RepairInterface)
		return;

	const FCoinValue ServerPrice = RepairInterface->CalculateRepairCost(Item->ItemID, CurrentDurability,
	                                                                    Item->GetTotalDurability());
	if (!Price.IsNonNegative() || !PlayerInterface->PlayerCanPayAmount(ServerPrice))
		return;

	PlayerInterface->GetCoinComponent()->PayAndAdjust(ServerPrice);

	if (IMerchantInterface* MerchantInterface = Cast<IMerchantInterface>(RepairerActor))
		MerchantInterface->ReceiveCoin(ServerPrice);

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

	const IRepairInterface* RepairInterface = Cast<IRepairInterface>(RepairerActor);
	if (!RepairInterface)
		return;

	FCoinValue ServerPrice{0, 0, 0, 0};
	const TArray<const UInventoryItemEquipable*>& AllEquipment = EquipComp->GetAllEquipment();
	for (int32 i = 0; i < AllEquipment.Num(); ++i)
	{
		if (const UInventoryItemEquipable* Item = AllEquipment[i])
		{
			const EEquipmentSlot ItemSlot = static_cast<EEquipmentSlot>(i);
			float CurrentDurability = 0.0f;
			if (EquipComp->GetEquipmentDurability(ItemSlot, CurrentDurability)
				&& CurrentDurability < Item->GetTotalDurability())
			{
				ServerPrice += RepairInterface->CalculateRepairCost(Item->ItemID, CurrentDurability,
				                                                    Item->GetTotalDurability());
			}
		}
	}

	if (!TotalPrice.IsNonNegative() || !PlayerInterface->PlayerCanPayAmount(ServerPrice))
		return;

	PlayerInterface->GetCoinComponent()->PayAndAdjust(ServerPrice);

	if (IMerchantInterface* MerchantInterface = Cast<IMerchantInterface>(RepairerActor))
		MerchantInterface->ReceiveCoin(ServerPrice);

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

	if (StagingCoin && PlayerInterface->GetCoinComponent())
	{
		PlayerInterface->GetCoinComponent()->AddCoins(StagingCoin->GetPurseContent());
		StagingCoin->ClearPurse();
	}

	if (StagingItems)
	{
		TArray<FInventoryEscrowItem> UnresolvedItems;
		for (const FInventoryEscrowItem& ItemStorage : StagingItems->GetStagingAreaItems())
		{
			const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(ItemStorage.ItemID, GetWorld());
			const auto ReestablishReservation = [this, &ItemStorage, Item]()
			{
				if (!Item)
					return;
				if (ItemStorage.Source.Kind == EInventoryDeliveryDestinationKind::Bag)
				{
					if (UInventoryComponent* Inventory = PlayerInterface->GetInventoryComponent())
						Inventory->ReserveItemFootprint(ItemStorage.ReservationId, ItemStorage.Source.Bag, Item,
							ItemStorage.Source.TopLeft);
				}
				else if (const UInventoryItemEquipable* Equipable = Cast<UInventoryItemEquipable>(Item);
					Equipable && ItemStorage.Source.Kind == EInventoryDeliveryDestinationKind::Equipment)
				{
					if (IEquipmentInterface* Equipment = GetEquipmentInterface())
						if (UEquipmentComponent* Component = Equipment->GetEquipmentComponent())
							Component->ReservePendingDelivery(ItemStorage.ReservationId, Equipable,
								ItemStorage.Source.EquipmentSlot);
				}
			};
			bool bRestored = false;
			if (Item && ItemStorage.Source.Kind == EInventoryDeliveryDestinationKind::Bag)
			{
				if (UInventoryComponent* Inventory = PlayerInterface->GetInventoryComponent())
				{
					Inventory->ReleaseItemFootprint(ItemStorage.ReservationId);
					if (Inventory->CanPlaceItemAt(ItemStorage.Source.Bag, Item, ItemStorage.Source.TopLeft))
					{
						PlayerInterface->PlayerAddItemWithDurability(ItemStorage.Source.TopLeft,
							ItemStorage.Source.Bag, ItemStorage.ItemID, ItemStorage.Durability);
						bRestored = PlayerInterface->PlayerGetItem(ItemStorage.Source.TopLeft,
							ItemStorage.Source.Bag) == ItemStorage.ItemID;
					}
				}
			}
			else if (const UInventoryItemEquipable* Equipable = Cast<UInventoryItemEquipable>(Item);
				Equipable && ItemStorage.Source.Kind == EInventoryDeliveryDestinationKind::Equipment)
			{
				if (IEquipmentInterface* Equipment = GetEquipmentInterface())
				{
					UEquipmentComponent* EquipmentComponent = Equipment->GetEquipmentComponent();
					if (EquipmentComponent)
						EquipmentComponent->ReleasePendingDeliveryReservation(ItemStorage.ReservationId);
					if (EquipmentComponent && EquipmentComponent->CanEquipItemAt(Equipable,
						ItemStorage.Source.EquipmentSlot))
					{
						Equipment->EquipItemWithDurability(ItemStorage.Source.EquipmentSlot,
							ItemStorage.ItemID, ItemStorage.Durability);
						bRestored = Equipment->GetEquippedItem(ItemStorage.Source.EquipmentSlot) == Equipable;
					}
				}
			}

			if (bRestored)
				continue;

			UInventoryDeliveryComponent* DeliveryComponent = PlayerInterface->GetInventoryDeliveryComponent();
			if (!DeliveryComponent)
			{
				ReestablishReservation();
				UnresolvedItems.Add(ItemStorage);
				continue;
			}

			if (!InventoryPlugin::StagingReturn::Route(ItemStorage,
				[DeliveryComponent](const FInventoryDeliveryRequest& Request)
				{
					return DeliveryComponent->TryDeliverOrQueue(Request);
				}))
			{
				ReestablishReservation();
				UnresolvedItems.Add(ItemStorage);
			}
		}
		StagingItems->SetStagingAreaItems(UnresolvedItems);
	}
}

void UInventoryNetComponent::HandleClaimPendingDelivery(FGuid DeliveryId)
{
	HandleClaimPendingDeliveryAt(DeliveryId, FInventoryDeliveryDestination());
}

void UInventoryNetComponent::HandleClaimPendingDeliveryAt(FGuid DeliveryId,
	FInventoryDeliveryDestination Destination)
{
	if (!PlayerInterface)
		return;
	if (UInventoryDeliveryComponent* DeliveryComponent = PlayerInterface->GetInventoryDeliveryComponent())
		if (DeliveryComponent->ReserveDestination(DeliveryId, Destination))
			DeliveryComponent->CommitClaim(DeliveryId, Destination);
}

void UInventoryNetComponent::HandleClaimAllPendingDeliveries()
{
	if (!PlayerInterface)
		return;
	UInventoryDeliveryComponent* DeliveryComponent = PlayerInterface->GetInventoryDeliveryComponent();
	if (!DeliveryComponent)
		return;

	const TArray<FPendingInventoryDelivery> Snapshot = DeliveryComponent->GetPendingDeliveries();
	for (const FPendingInventoryDelivery& Delivery : Snapshot)
	{
		FInventoryDeliveryDestination Destination;
		if (DeliveryComponent->ReserveDestination(Delivery.DeliveryId, Destination))
			DeliveryComponent->CommitClaim(Delivery.DeliveryId, Destination);
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

	if (!StagingItems->HasCapacity())
		return;
	const UInventoryItemEquipable* Item = Equipment->GetEquippedItem(OutSlot);
	UEquipmentComponent* EquipComp = Equipment->GetEquipmentComponent();
	if (!Item || !EquipComp)
		return;

	FInventoryEscrowItem Escrow;
	Escrow.ItemID = InItemId;
	Escrow.Source = FInventoryDeliveryDestination::MakeEquipment(OutSlot);
	Escrow.ReservationId = FGuid::NewGuid();
	Escrow.EffectiveWeight = FMath::Max(0.0f, Item->GetWeight());
	EquipComp->GetEquipmentDurability(OutSlot, Escrow.Durability);

	Equipment->UnequipItem(OutSlot);
	if (!EquipComp->ReservePendingDelivery(Escrow.ReservationId, Item, OutSlot) ||
		!StagingItems->AddItemToStagingArea(Escrow))
	{
		EquipComp->ReleasePendingDeliveryReservation(Escrow.ReservationId);
		Equipment->EquipItemWithDurability(OutSlot, InItemId, Escrow.Durability);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UInventoryNetComponent::HandleMoveInventoryItemToStagingArea(int32 InItemId, int32 OutTopLeft, EBagSlot OutSlot)
{
	if (!PlayerInterface)
		return;

	UStagingAreaComponent* StagingItems = PlayerInterface->GetStagingAreaItems();
	UInventoryComponent* InventoryComp = PlayerInterface->GetInventoryComponent();
	if (!StagingItems || !InventoryComp || !StagingItems->HasCapacity())
		return;

	// Get current durability from inventory before moving
	float ItemDurability = 100.0f;
	if (InventoryComp)
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

	const UInventoryItemBase* Item = UInventoryUtilities::GetItemFromID(InItemId, GetWorld());
	if (!Item)
		return;

	FInventoryEscrowItem Escrow;
	Escrow.ItemID = InItemId;
	Escrow.Durability = ItemDurability;
	Escrow.Source = FInventoryDeliveryDestination::MakeBag(OutSlot, OutTopLeft);
	Escrow.ReservationId = FGuid::NewGuid();
	Escrow.EffectiveWeight = InventoryComp->GetEffectiveItemWeight(OutSlot, Item);

	PlayerInterface->PlayerRemoveItem(OutTopLeft, OutSlot);
	if (!InventoryComp->ReserveItemFootprint(Escrow.ReservationId, OutSlot, Item, OutTopLeft) ||
		!StagingItems->AddItemToStagingArea(Escrow))
	{
		InventoryComp->ReleaseItemFootprint(Escrow.ReservationId);
		PlayerInterface->PlayerAddItemWithDurability(OutTopLeft, OutSlot, InItemId, ItemDurability);
	}
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
		const EBagSlot BagSlot = UInventoryComponent::GetBagSlotFromInventory(OutSlot);
		if (PlayerInterface->GetInventoryComponent()->GetBagConst(BagSlot).Num() > 0 ||
			PlayerInterface->GetInventoryComponent()->HasReservationsInBag(BagSlot))
			return false;
	}
	if (Equipment->GetEquipmentComponent()->IsEquipmentSlotReserved(OutSlot))
		return false;
	if (!PlayerInterface->GetInventoryComponent()->CanPlaceItemAt(InSlot, ConsideredItem, InTopLeft))
		return false;

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

	const UInventoryItemEquipable* LocalItem = Cast<UInventoryItemEquipable>(
		UInventoryUtilities::GetItemFromID(InItemId, GetWorld()));
	return LocalItem && LocalItem->ItemID > 0 &&
		Equipment->GetEquipmentComponent()->CanEquipItemAt(LocalItem, InSlot);
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
	if (Equipment->GetEquipmentComponent()->IsEquipmentSlotReserved(DroppedInSlot) ||
		Equipment->GetEquipmentComponent()->IsEquipmentSlotReserved(DraggedOutSlot))
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

	if (!GivingComponent || !ReceivingComponent)
		return false;

	if (!RemovedCoinValue.IsNonNegative() || !AddedCoinValue.IsNonNegative())
		return false;

	if (!RemovedCoinValue.HasSameValue(AddedCoinValue))
		return false;

	const AActor* OwningActor = PlayerInterface->GetInventoryOwningActor();
	const auto IsPlayerOwnedCoin = [this, OwningActor](const UCoinComponent* CoinComponent)
	{
		const AActor* CoinOwner = CoinComponent ? CoinComponent->GetOwner() : nullptr;
		return CoinOwner && (CoinOwner == GetOwner() || CoinOwner == OwningActor);
	};

	if (!IsPlayerOwnedCoin(GivingComponent) || !IsPlayerOwnedCoin(ReceivingComponent))
		return false;

	return FCoinValue::CanPayWithChange(GivingComponent->GetPurseContent(), RemovedCoinValue);
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

	const UInventoryItemEquipable* Item = Equipment->GetEquippedItem(Slot);
	if (!Item || Equipment->GetEquipmentComponent()->IsEquipmentSlotReserved(Slot))
		return false;
	if (Cast<IInventoryItemBagInterface>(Item) && PlayerInterface)
	{
		const EBagSlot BagSlot = UInventoryComponent::GetBagSlotFromInventory(Slot);
		if (PlayerInterface->GetInventoryComponent()->GetBagConst(BagSlot).Num() > 0 ||
			PlayerInterface->GetInventoryComponent()->HasReservationsInBag(BagSlot))
			return false;
	}
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidateLootActor(AActor* InputLootedActor)
{
	return IsValid(InputLootedActor) && Cast<ILootableInterface>(InputLootedActor) != nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerLootItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft)
{
	return InTopLeft >= 0
		&& OutTopLeft >= 0
		&& InItemId > 0
		&& InSlot > EBagSlot::Unknown
		&& InSlot < EBagSlot::LastValidBag;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerEquipItemFromLoot(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft)
{
	return InItemId > 0
		&& OutTopLeft >= 0
		&& InSlot > EEquipmentSlot::Unknown
		&& InSlot < EEquipmentSlot::Last;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerBuyFromMerchant(int32 ItemId, const FCoinValue& Price)
{
	return ItemId >= 0 && Price.IsNonNegative();
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerSellToMerchant(EBagSlot OutSlot, int32 ItemId, int32 TopLeft,
                                                            const FCoinValue& Price)
{
	return ItemId >= 0 && TopLeft >= 0 && Price.IsNonNegative();
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerRepairEquipment(EEquipmentSlot Slot, const FCoinValue& Price)
{
	return Slot != EEquipmentSlot::Unknown && Price.IsNonNegative();
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidatePlayerRepairAllEquipment(const FCoinValue& TotalPrice)
{
	return TotalPrice.IsNonNegative();
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidateTransferStagingToActor(AActor* TargetActor)
{
	return TargetActor != nullptr;
}

bool UInventoryNetComponent::ValidateClaimPendingDelivery(FGuid DeliveryId)
{
	return PlayerInterface && DeliveryId.IsValid() && PlayerInterface->GetInventoryDeliveryComponent();
}

bool UInventoryNetComponent::ValidateClaimPendingDeliveryAt(FGuid DeliveryId,
	const FInventoryDeliveryDestination& Destination)
{
	if (!ValidateClaimPendingDelivery(DeliveryId))
		return false;
	if (Destination.Kind == EInventoryDeliveryDestinationKind::Bag)
		return Destination.Bag > EBagSlot::Unknown && Destination.Bag < EBagSlot::LastValidBag &&
			Destination.TopLeft >= 0 && Destination.TopLeft <= 4095;
	if (Destination.Kind == EInventoryDeliveryDestinationKind::Equipment)
		return Destination.EquipmentSlot > EEquipmentSlot::Unknown &&
			Destination.EquipmentSlot < EEquipmentSlot::Last;
	return false;
}

//----------------------------------------------------------------------------------------------------------------------

bool UInventoryNetComponent::ValidateMoveEquipmentToStagingArea(int32 InItemId, EEquipmentSlot OutSlot)
{
	IEquipmentInterface* Equipment = GetEquipmentInterface();
	if (!Equipment)
		return false;

	const UInventoryItemEquipable* Item = Equipment->GetEquippedItem(OutSlot);
	return Item && Item->ItemID == InItemId &&
		!Equipment->GetEquipmentComponent()->IsEquipmentSlotReserved(OutSlot);
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
