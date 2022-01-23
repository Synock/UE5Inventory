// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/PlayerController.h"
#include "UI/HUDWidget.h"
#include "Inventory/CoinValue.h"
#include "MainPlayerController.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYPOC_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory|UI")
	TSubclassOf<UHUDWidget> UIHUDWidgetClass;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|UI")
	UHUDWidget* UIHUDWidget;

	UPROPERTY(ReplicatedUsing=OnRep_LootedActor, BlueprintReadOnly, Category = "Inventory|Loot")
	class ALootableActor* LootedActor = nullptr;

	UPROPERTY(ReplicatedUsing=OnRep_MerchantActor, BlueprintReadOnly, Category = "Inventory|Merchant")
	class AMerchantActor* MerchantActor = nullptr;

public:
	void CreateHUD();

	UFUNCTION(BlueprintCallable)
	UHUDWidget* GetMainHUD();

	//------------------------------------------------------------------------------------------------------------------
	// Helpers
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Inventory|Inventory")
	bool PlayerCanPutItemSomewhere(int64 ItemID) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Inventory")
	bool PlayerCanPayAmount(const FCoinValue& CoinValue) const;

	UFUNCTION()
	void OnRep_LootedActor();

	UFUNCTION()
	void OnRep_MerchantActor();

	UFUNCTION(BlueprintCallable)
	bool PlayerTryAutoEquip(int64 InItemId,InventorySlot& PossibleEquipment);

	UFUNCTION(BlueprintCallable)
	void PlayerAutoEquipItem(int32 InTopLeft, BagSlot InSlot, int64 InItemId);

	//------------------------------------------------------------------------------------------------------------------
	// Loot -- Client
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	bool IsLooting() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	void StopLooting();

	//Try to loot all items in the lootpool, return false is at least one item cannot be looted
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory|Loot")
	void PlayerAutoLootAll();

	//Try to loot an items from the lootpool in equipment then in bags
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	void PlayerAutoLootItem(int64 InItemId, int32 OutTopLeft);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	void PlayerLootItem(int32 InTopLeft, BagSlot InSlot, int64 InItemId, int32 OutTopLeft);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	void PlayerEquipItemFromLoot(int64 InItemId, InventorySlot InSlot, int32 OutTopLeft);


	//------------------------------------------------------------------------------------------------------------------
	// Merchant -- Client
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	bool IsTrading() const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void TryPresentSellItem(BagSlot OutSlot, int64 ItemId, int32 TopLeft);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void ResetSellItem();

	//Buy selected stuff from merchant
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	void PlayerBuyFromMerchant(int64 ItemId, const FCoinValue& Price);

	//Sell selected stuff to merchant
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	void PlayerSellToMerchant(BagSlot OutSlot, int64 ItemId, int32 TopLeft, const FCoinValue& Price);

protected:

	//------------------------------------------------------------------------------------------------------------------
	// Loot -- Server
	//------------------------------------------------------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation, Category = "Inventory|Loot")
	void Server_LootActor(ALootableActor* InputLootedActor);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Loot")
	void Server_StopLooting();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Loot")
	void Server_PlayerLootItem(int32 InTopLeft, BagSlot InSlot, int64 InItemId, int32 OutTopLeft);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Loot")
	void Server_PlayerEquipItemFromLoot(int64 InItemId, InventorySlot InSlot, int32 OutTopLeft);


	//------------------------------------------------------------------------------------------------------------------
	// Merchant -- Server
	//------------------------------------------------------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Merchant")
	void Server_MerchantTrade(AMerchantActor* InputMerchantActor);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Inventory|Merchant")
	void Server_StopMerchantTrade();

	//Buy selected stuff from merchant
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Inventory|Merchant")
	void Server_PlayerBuyFromMerchant(int64 ItemId, const FCoinValue& Price);

	//Sell selected stuff to merchant
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Inventory|Merchant")
	void Server_PlayerSellToMerchant(BagSlot OutSlot, int64 ItemId, int32 TopLeft, const FCoinValue& Price);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Inventory|Inventory")
	void Server_PlayerAutoEquipItem(int32 InTopLeft, BagSlot InSlot, int64 InItemId);
};
