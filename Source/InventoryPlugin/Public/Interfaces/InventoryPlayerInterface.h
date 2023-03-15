// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "EquipmentInterface.h"
#include "InventoryHUDInterface.h"
#include "Components/CoinComponent.h"
#include "Components/InventoryComponent.h"
#include "CoinValue.h"
#include "Components/StagingAreaComponent.h"
#include "UI/WeightWidget.h"
#include "UObject/Interface.h"
#include "Items/InventoryItemBase.h"
#include "InventoryPlayerInterface.generated.h"

class UBankComponent;
// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInventoryPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYPLUGIN_API IInventoryPlayerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual UInventoryComponent* GetInventoryComponent() = 0;
	virtual const UInventoryComponent* GetInventoryComponentConst() const = 0;

	virtual UCoinComponent* GetCoinComponent() = 0;
	virtual const UCoinComponent* GetCoinComponentConst() const = 0;

	virtual AActor* GetInventoryOwningActor() = 0;
	virtual AActor const* GetInventoryOwningActorConst() const = 0;

	virtual bool GetTransactionBoolean() = 0;
	virtual void SetTransactionBoolean(bool Value) = 0;

	virtual AActor* GetMerchantActor() = 0;
	virtual const AActor* GetMerchantActorConst() const = 0;
	virtual void SetMerchantActor(AActor* Actor) = 0;

	virtual AActor* GetLootedActor() = 0;
	virtual const AActor* GetLootedActorConst() const = 0;
	virtual void SetLootedActor(AActor* Actor) = 0;

	virtual IInventoryHUDInterface* GetInventoryHUDInterface() = 0;
	virtual UObject* GetInventoryHUDObject() = 0;

	virtual UCoinComponent* GetStagingAreaCoin() = 0;
	virtual UStagingAreaComponent* GetStagingAreaItems() = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bank")
	virtual UCoinComponent* GetBankCoin() const;
	UFUNCTION(BlueprintCallable, Category = "Inventory|Bank")
	virtual UBankComponent* GetBankComponent() const;

	//------------------------------------------------------------------------------------------------------------------
	// Equipment
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	virtual void PlayerUnequipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, EEquipmentSlot OutSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	virtual void PlayerEquipItemFromInventory(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft,
	                                          EBagSlot OutSlot);

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Equipment")
	virtual void PlayerSwapEquipment(int32 DroppedItemId, EEquipmentSlot DroppedInSlot, int32 SwappedItemId,
	                                 EEquipmentSlot DraggedOutSlot);

	//------------------------------------------------------------------------------------------------------------------
	// Helpers
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool PlayerCanPutItemSomewhere(int32 ItemID) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool PlayerCanPayAmount(const FCoinValue& CoinValue) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool PlayerTryAutoEquip(int32 InItemId, EEquipmentSlot& PossibleEquipment);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void PlayerAutoEquipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId);

	void ResetTransaction();

	IEquipmentInterface* GetEquipmentForInventory();

	virtual FOnWeightChanged& GetWeightChangedDelegate() = 0;

	//------------------------------------------------------------------------------------------------------------------
	// Inventory
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual TArray<const UInventoryItemBase*> GetAllItems() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual const TArray<FMinimalItemStorage>& GetAllItemsInBag(EBagSlot Slot) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool CanUnequipBag(EEquipmentSlot Slot) const;

	UFUNCTION()
	virtual bool PlayerTryAutoLootFunction(int32 InItemId, EEquipmentSlot& PossibleEquipment, int32& InTopLeft,
	                                       EBagSlot& PossibleBag) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool PlayerHasItem(int32 ItemId);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool PlayerRemoveItemIfPossible(int32 ItemID);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool PlayerRemoveAnyItemIfPossible(const TArray<int32>& ItemID);

	UFUNCTION()
	virtual void PlayerAddItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId);

	UFUNCTION()
	virtual void PlayerRemoveItem(int32 TopLeft, EBagSlot Slot);

	UFUNCTION()
	virtual int32 PlayerGetItem(int32 TopLeft, EBagSlot Slot) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void PlayerMoveItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft, EBagSlot OutSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void TransferCoinTo(UCoinComponent* GivingComponent, UCoinComponent* ReceivingComponent,
	                            const FCoinValue& RemovedCoinValue, const FCoinValue& AddedCoinValue);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void CancelStagingArea();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void TransferStagingToActor(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void MoveEquipmentToStagingArea(int32 InItemId, EEquipmentSlot OutSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void MoveInventoryItemToStagingArea(int32 InItemId, int32 OutTopLeft, EBagSlot OutSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void HandleActivation(int32 ItemID, int32 TopLeft, EBagSlot BagSlot);

	//------------------------------------------------------------------------------------------------------------------
	// Loot -- Client
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual bool IsLooting() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void StartLooting(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void StopLooting();

	//Try to loot an items from the lootpool in equipment then in bags
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void PlayerAutoLootItem(int32 InItemId, int32 OutTopLeft);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void PlayerAutoLootAll();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void PlayerLootItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void PlayerEquipItemFromLoot(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft);

	//------------------------------------------------------------------------------------------------------------------
	// Merchant -- Client
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual bool IsTrading() const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	virtual void TryPresentSellItem(EBagSlot OutSlot, int32 ItemId, int32 TopLeft);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	virtual void ResetSellItem();

	//Buy selected stuff from merchant
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void PlayerBuyFromMerchant(int32 ItemId, const FCoinValue& Price);

	//Sell selected stuff to merchant
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void PlayerSellToMerchant(EBagSlot OutSlot, int32 ItemId, int32 TopLeft, const FCoinValue& Price);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void MerchantTrade(AActor* InputMerchantActor);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void StopMerchantTrade();

	float GetTotalWeight();

protected:
	//------------------------------------------------------------------------------------------------------------------
	// Merchant -- Server
	//------------------------------------------------------------------------------------------------------------------
	//UFUNCTION(Server, Reliable, Category = "Inventory|Merchant")
	virtual void Server_MerchantTrade(AActor* InputMerchantActor) = 0;

	//UFUNCTION(Server, Reliable, Category = "Inventory|Merchant")
	virtual void Server_StopMerchantTrade() = 0;

	//Buy selected stuff from merchant
	//UFUNCTION(Server, Reliable, WithValidation,  Category = "Inventory|Merchant")
	virtual void Server_PlayerBuyFromMerchant(int32 ItemId, const FCoinValue& Price) = 0;

	//Sell selected stuff to merchant
	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Merchant")
	virtual void Server_PlayerSellToMerchant(EBagSlot OutSlot, int32 ItemId, int32 TopLeft, const FCoinValue& Price) =
	0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	virtual void Server_PlayerAutoEquipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId) = 0;

	//------------------------------------------------------------------------------------------------------------------
	// Loot -- Server
	//------------------------------------------------------------------------------------------------------------------
	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Loot")
	virtual void Server_LootActor(AActor* InputLootedActor) = 0;

	//UFUNCTION(Server, Reliable, Category = "Inventory|Loot")
	virtual void Server_StopLooting() = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Loot")
	virtual void Server_PlayerLootItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft) = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Loot")
	virtual void Server_PlayerEquipItemFromLoot(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft) = 0;

	//Try to loot all items in the lootPool, return false is at least one item cannot be looted
	//UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory|Loot")
	virtual void Server_PlayerAutoLootAll() = 0;

	//------------------------------------------------------------------------------------------------------------------
	// Inventory -- Server
	//------------------------------------------------------------------------------------------------------------------
	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	virtual void Server_PlayerMoveItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft,
	                                   EBagSlot OutSlot) = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Equipment")
	virtual void Server_PlayerUnequipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, EEquipmentSlot OutSlot) = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Equipment")
	virtual void Server_PlayerEquipItemFromInventory(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft,
	                                                 EBagSlot OutSlot) = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Equipment")
	virtual void Server_PlayerSwapEquipment(int32 DroppedItemId, EEquipmentSlot DroppedInSlot, int32 SwappedItemId,
	                                        EEquipmentSlot DraggedOutSlot) = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	virtual void Server_TransferCoinTo(UCoinComponent* GivingComponent, UCoinComponent* ReceivingComponent,
	                                   const FCoinValue& RemovedCoinValue, const FCoinValue& AddedCoinValue) = 0;

	//------------------------------------------------------------------------------------------------------------------
	// Staging -- Server
	//------------------------------------------------------------------------------------------------------------------

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Staging")
	virtual void Server_CancelStagingArea() = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Staging")
	virtual void Server_TransferStagingToActor(AActor* TargetActor) = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Staging")
	virtual void Server_MoveEquipmentToStagingArea(int32 InItemId, EEquipmentSlot OutSlot) = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Staging")
	virtual void Server_MoveInventoryItemToStagingArea(int32 InItemId, int32 OutTopLeft, EBagSlot OutSlot) = 0;
};
