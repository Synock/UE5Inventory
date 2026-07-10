#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CoinValue.h"
#include "Definitions.h"
#include "InventoryNetComponent.generated.h"

class IInventoryPlayerInterface;
class IEquipmentInterface;
class ILootableInterface;
class IMerchantInterface;
class UCoinComponent;
class UStagingAreaComponent;
class UTradeComponent;
class UInventoryItemBase;
class UInventoryItemEquipable;

/**
 * @class UInventoryNetComponent
 *
 * Replicated ActorComponent that owns all inventory-related Server RPCs.
 * Attach to a PlayerController (or Character) that implements IInventoryPlayerInterface.
 *
 * Each Server RPC dispatches to a virtual Handle / Validate pair.
 * Game projects subclass this component and override only the methods they need
 * to customize - no need to ever redeclare UFUNCTION(Server, Reliable).
 *
 * Default Handle implementations perform the core inventory mutation (move, equip, loot, etc.)
 * using the owning actor IInventoryPlayerInterface.
 * Default Validate implementations provide anti-cheat checks (item existence, slot validity, etc.).
 */
UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API UInventoryNetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryNetComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Cached reference to the owner's IInventoryPlayerInterface. Set in BeginPlay. */
	IInventoryPlayerInterface* GetPlayerInterface() const { return PlayerInterface; }

#if WITH_AUTOMATION_WORKER
	bool ValidatePlayerBuyFromMerchantForTests(int32 ItemId, const FCoinValue& Price)
	{
		return ValidatePlayerBuyFromMerchant(ItemId, Price);
	}

	bool ValidatePlayerSellToMerchantForTests(EBagSlot OutSlot, int32 ItemId, int32 TopLeft, const FCoinValue& Price)
	{
		return ValidatePlayerSellToMerchant(OutSlot, ItemId, TopLeft, Price);
	}

	bool ValidatePlayerRepairEquipmentForTests(EEquipmentSlot Slot, const FCoinValue& Price)
	{
		return ValidatePlayerRepairEquipment(Slot, Price);
	}

	bool ValidatePlayerRepairAllEquipmentForTests(const FCoinValue& TotalPrice)
	{
		return ValidatePlayerRepairAllEquipment(TotalPrice);
	}

	bool ValidateLootActorForTests(AActor* InputLootedActor)
	{
		return ValidateLootActor(InputLootedActor);
	}

	bool ValidatePlayerLootItemForTests(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft)
	{
		return ValidatePlayerLootItem(InTopLeft, InSlot, InItemId, OutTopLeft);
	}

	bool ValidatePlayerEquipItemFromLootForTests(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft)
	{
		return ValidatePlayerEquipItemFromLoot(InItemId, InSlot, OutTopLeft);
	}

	void HandleLootActorForTests(AActor* InputLootedActor) { HandleLootActor(InputLootedActor); }
	void HandleStopLootingForTests() { HandleStopLooting(); }
	void HandleEndPlayCleanupForTests() { HandleStopLooting(); }
	void HandlePlayerLootItemForTests(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft)
	{
		HandlePlayerLootItem(InTopLeft, InSlot, InItemId, OutTopLeft);
	}
	AActor* GetPreviousLootOwnerForTests() const { return PreviousLootOwner.Get(); }
	int32 GetRejectedLootRequestCountForTests() const { return RejectedLootRequestCountForTests; }
#endif

	//==================================================================================================================
	// Replicated State
	//==================================================================================================================

	UPROPERTY(ReplicatedUsing=OnRep_LootedActor, BlueprintReadOnly, Category = "Inventory|Loot")
	TObjectPtr<AActor> LootedActor;

	UPROPERTY(ReplicatedUsing=OnRep_MerchantActor, BlueprintReadOnly, Category = "Inventory|Merchant")
	TObjectPtr<AActor> MerchantActor;

	UPROPERTY(ReplicatedUsing=OnRep_RepairerActor, BlueprintReadOnly, Category = "Inventory|Repair")
	TObjectPtr<AActor> RepairerActor;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	bool TransactionBoolean = false;

	UFUNCTION()
	virtual void OnRep_LootedActor();

	UFUNCTION()
	virtual void OnRep_MerchantActor();

	UFUNCTION()
	virtual void OnRep_RepairerActor();

	//==================================================================================================================
	// Inventory RPCs
	//==================================================================================================================

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void Server_PlayerMoveItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft, EBagSlot OutSlot);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Equipment")
	void Server_PlayerUnequipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, EEquipmentSlot OutSlot);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Equipment")
	void Server_PlayerEquipItemFromInventory(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft, EBagSlot OutSlot);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Equipment")
	void Server_PlayerSwapEquipment(int32 DroppedItemId, EEquipmentSlot DroppedInSlot, int32 SwappedItemId,
	                                EEquipmentSlot DraggedOutSlot);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void Server_PlayerAutoEquipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void Server_TransferCoinTo(UCoinComponent* GivingComponent, UCoinComponent* ReceivingComponent,
	                           const FCoinValue& RemovedCoinValue, const FCoinValue& AddedCoinValue);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void Server_DropItemFromInventory(int32 TopLeft, EBagSlot Slot, FVector DropLocation);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void Server_DropItemFromEquipment(EEquipmentSlot Slot, FVector DropLocation);

	//==================================================================================================================
	// Loot RPCs
	//==================================================================================================================

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Loot")
	void Server_LootActor(AActor* InputLootedActor);

	UFUNCTION(Server, Reliable, Category = "Inventory|Loot")
	void Server_StopLooting();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Loot")
	void Server_PlayerLootItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Loot")
	void Server_PlayerEquipItemFromLoot(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft);

	UFUNCTION(Server, Reliable, Category = "Inventory|Loot")
	void Server_PlayerAutoLootAll();

	/** Clears a client-side transaction after the server softly rejects stale loot state. */
	UFUNCTION(Client, Reliable, Category = "Inventory|Loot")
	void Client_LootRequestRejected();

	//==================================================================================================================
	// Merchant RPCs
	//==================================================================================================================

	UFUNCTION(Server, Reliable, Category = "Inventory|Merchant")
	void Server_MerchantTrade(AActor* InputMerchantActor);

	UFUNCTION(Server, Reliable, Category = "Inventory|Merchant")
	void Server_StopMerchantTrade();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Merchant")
	void Server_PlayerBuyFromMerchant(int32 ItemId, const FCoinValue& Price);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Merchant")
	void Server_PlayerSellToMerchant(EBagSlot OutSlot, int32 ItemId, int32 TopLeft, const FCoinValue& Price);

	//==================================================================================================================
	// Repair RPCs
	//==================================================================================================================

	UFUNCTION(Server, Reliable, Category = "Inventory|Repair")
	void Server_RepairTrade(AActor* InputRepairerActor);

	UFUNCTION(Server, Reliable, Category = "Inventory|Repair")
	void Server_StopRepairTrade();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Repair")
	void Server_PlayerRepairEquipment(EEquipmentSlot Slot, const FCoinValue& Price);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Repair")
	void Server_PlayerRepairAllEquipment(const FCoinValue& TotalPrice);

	//==================================================================================================================
	// Staging RPCs
	//==================================================================================================================

	UFUNCTION(Server, Reliable, Category = "Inventory|Staging")
	void Server_CancelStagingArea();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Staging")
	void Server_TransferStagingToActor(AActor* TargetActor);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Staging")
	void Server_MoveEquipmentToStagingArea(int32 InItemId, EEquipmentSlot OutSlot);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Staging")
	void Server_MoveInventoryItemToStagingArea(int32 InItemId, int32 OutTopLeft, EBagSlot OutSlot);

	//==================================================================================================================
	// Key RPCs
	//==================================================================================================================

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Key")
	void Server_PlayerAddKeyFromInventory(int32 InTopLeft, EBagSlot InSlot, int32 InItemId);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Key")
	void Server_PlayerRemoveKeyToInventory(int32 KeyId);

	//==================================================================================================================
	// Trade RPCs
	//==================================================================================================================

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Trade")
	void Server_PlayerRequestTrade(ACharacter* OtherPlayerCharacter);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Trade")
	void Server_PlayerRequestTradeWithItem(ACharacter* OtherPlayerCharacter, int32 ItemID, EBagSlot BagSlot,
	                                       int32 TopLeft);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Trade")
	void Server_PlayerAcceptTradeRequest(ACharacter* RequestingPlayerCharacter);

	UFUNCTION(Server, Reliable, Category = "Inventory|Trade")
	void Server_PlayerDeclineTradeRequest(ACharacter* RequestingPlayerCharacter);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Trade")
	void Server_PlayerAddItemToTrade(int32 ItemID, EBagSlot BagSlot, int32 TopLeft);

	UFUNCTION(Server, Reliable, Category = "Inventory|Trade")
	void Server_PlayerRemoveItemFromTrade(int32 SlotIndex);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Trade")
	void Server_PlayerSetTradeCoin(const FCoinValue& CoinAmount);

	UFUNCTION(Server, Reliable, Category = "Inventory|Trade")
	void Server_PlayerToggleTradeAcceptance(bool bAccept);

	UFUNCTION(Server, Reliable, Category = "Inventory|Trade")
	void Server_PlayerCancelTrade();

protected:
	//==================================================================================================================
	// Overridable Handle* methods - override in game subclass for custom behavior.
	// Default implementations perform the core inventory mutation.
	//==================================================================================================================

	// -- Inventory --
	virtual void HandlePlayerMoveItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft,
	                                  EBagSlot OutSlot);
	virtual void HandlePlayerUnequipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, EEquipmentSlot OutSlot);
	virtual void HandlePlayerEquipItemFromInventory(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft,
	                                                EBagSlot OutSlot);
	virtual void HandlePlayerSwapEquipment(int32 DroppedItemId, EEquipmentSlot DroppedInSlot, int32 SwappedItemId,
	                                       EEquipmentSlot DraggedOutSlot);
	virtual void HandlePlayerAutoEquipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId);
	virtual void HandleTransferCoinTo(UCoinComponent* GivingComponent, UCoinComponent* ReceivingComponent,
	                                  const FCoinValue& RemovedCoinValue, const FCoinValue& AddedCoinValue);
	virtual void HandleDropItemFromInventory(int32 TopLeft, EBagSlot Slot, FVector DropLocation);
	virtual void HandleDropItemFromEquipment(EEquipmentSlot Slot, FVector DropLocation);

	// -- Loot --
	virtual void HandleLootActor(AActor* InputLootedActor);
	virtual void HandleStopLooting();
	virtual void HandlePlayerLootItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft);
	virtual void HandlePlayerEquipItemFromLoot(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft);
	virtual void HandlePlayerAutoLootAll();

	// -- Merchant --
	virtual void HandleMerchantTrade(AActor* InputMerchantActor);
	virtual void HandleStopMerchantTrade();
	virtual void HandlePlayerBuyFromMerchant(int32 ItemId, const FCoinValue& Price);
	virtual void HandlePlayerSellToMerchant(EBagSlot OutSlot, int32 ItemId, int32 TopLeft, const FCoinValue& Price);

	// -- Repair --
	virtual void HandleRepairTrade(AActor* InputRepairerActor);
	virtual void HandleStopRepairTrade();
	virtual void HandlePlayerRepairEquipment(EEquipmentSlot Slot, const FCoinValue& Price);
	virtual void HandlePlayerRepairAllEquipment(const FCoinValue& TotalPrice);

	// -- Staging --
	virtual void HandleCancelStagingArea();
	virtual void HandleTransferStagingToActor(AActor* TargetActor);
	virtual void HandleMoveEquipmentToStagingArea(int32 InItemId, EEquipmentSlot OutSlot);
	virtual void HandleMoveInventoryItemToStagingArea(int32 InItemId, int32 OutTopLeft, EBagSlot OutSlot);

	// -- Keys --
	virtual void HandlePlayerAddKeyFromInventory(int32 InTopLeft, EBagSlot InSlot, int32 InItemId);
	virtual void HandlePlayerRemoveKeyToInventory(int32 KeyId);

	// -- Trade --
	virtual void HandlePlayerRequestTrade(ACharacter* OtherPlayerCharacter);
	virtual void HandlePlayerRequestTradeWithItem(ACharacter* OtherPlayerCharacter, int32 ItemID, EBagSlot BagSlot,
	                                              int32 TopLeft);
	virtual void HandlePlayerAcceptTradeRequest(ACharacter* RequestingPlayerCharacter);
	virtual void HandlePlayerDeclineTradeRequest(ACharacter* RequestingPlayerCharacter);
	virtual void HandlePlayerAddItemToTrade(int32 ItemID, EBagSlot BagSlot, int32 TopLeft);
	virtual void HandlePlayerRemoveItemFromTrade(int32 SlotIndex);
	virtual void HandlePlayerSetTradeCoin(const FCoinValue& CoinAmount);
	virtual void HandlePlayerToggleTradeAcceptance(bool bAccept);
	virtual void HandlePlayerCancelTrade();

	//==================================================================================================================
	// Overridable Validate* methods - override in game subclass to add extra anti-cheat.
	// Default implementations provide standard inventory validation.
	// Call Super::Validate*() to keep built-in checks when overriding.
	//==================================================================================================================

	// -- Inventory --
	virtual bool ValidatePlayerMoveItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft,
	                                    EBagSlot OutSlot);
	virtual bool ValidatePlayerUnequipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, EEquipmentSlot OutSlot);
	virtual bool ValidatePlayerEquipItemFromInventory(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft,
	                                                  EBagSlot OutSlot);
	virtual bool ValidatePlayerSwapEquipment(int32 DroppedItemId, EEquipmentSlot DroppedInSlot, int32 SwappedItemId,
	                                         EEquipmentSlot DraggedOutSlot);
	virtual bool ValidatePlayerAutoEquipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId);
	virtual bool ValidateTransferCoinTo(UCoinComponent* GivingComponent, UCoinComponent* ReceivingComponent,
	                                    const FCoinValue& RemovedCoinValue, const FCoinValue& AddedCoinValue);
	virtual bool ValidateDropItemFromInventory(int32 TopLeft, EBagSlot Slot, FVector DropLocation);
	virtual bool ValidateDropItemFromEquipment(EEquipmentSlot Slot, FVector DropLocation);

	// -- Loot --
	virtual bool ValidateLootActor(AActor* InputLootedActor);
	virtual bool ValidatePlayerLootItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft);
	virtual bool ValidatePlayerEquipItemFromLoot(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft);

	// -- Merchant --
	virtual bool ValidatePlayerBuyFromMerchant(int32 ItemId, const FCoinValue& Price);
	virtual bool ValidatePlayerSellToMerchant(EBagSlot OutSlot, int32 ItemId, int32 TopLeft, const FCoinValue& Price);

	// -- Repair --
	virtual bool ValidatePlayerRepairEquipment(EEquipmentSlot Slot, const FCoinValue& Price);
	virtual bool ValidatePlayerRepairAllEquipment(const FCoinValue& TotalPrice);

	// -- Staging --
	virtual bool ValidateTransferStagingToActor(AActor* TargetActor);
	virtual bool ValidateMoveEquipmentToStagingArea(int32 InItemId, EEquipmentSlot OutSlot);
	virtual bool ValidateMoveInventoryItemToStagingArea(int32 InItemId, int32 OutTopLeft, EBagSlot OutSlot);

	// -- Keys --
	virtual bool ValidatePlayerAddKeyFromInventory(int32 InTopLeft, EBagSlot InSlot, int32 InItemId);
	virtual bool ValidatePlayerRemoveKeyToInventory(int32 KeyId);

	// -- Trade --
	virtual bool ValidatePlayerRequestTrade(ACharacter* OtherPlayerCharacter);
	virtual bool ValidatePlayerRequestTradeWithItem(ACharacter* OtherPlayerCharacter, int32 ItemID, EBagSlot BagSlot,
	                                                int32 TopLeft);
	virtual bool ValidatePlayerAcceptTradeRequest(ACharacter* RequestingPlayerCharacter);
	virtual bool ValidatePlayerAddItemToTrade(int32 ItemID, EBagSlot BagSlot, int32 TopLeft);
	virtual bool ValidatePlayerSetTradeCoin(const FCoinValue& CoinAmount);

	//==================================================================================================================
	// Helpers
	//==================================================================================================================

	/** Resolve the owning actor's IEquipmentInterface. */
	IEquipmentInterface* GetEquipmentInterface() const;

	/** Get the TradeComponent from the owner, if any. */
	UTradeComponent* GetTradeComponent() const;

	/** True only while this component owns the target's exclusive loot session. */
	bool OwnsActiveLootSession(const ILootableInterface* Lootable) const;

	/** Softly reject stale mutable state without failing RPC validation and disconnecting the client. */
	void RejectLootRequest();

private:
	UPROPERTY()
	TScriptInterface<IInventoryPlayerInterface> CachedPlayerInterfaceObject;

	IInventoryPlayerInterface* PlayerInterface = nullptr;

	/** Actor owner to restore when the exclusive loot session ends. */
	UPROPERTY(Transient)
	TObjectPtr<AActor> PreviousLootOwner;

#if WITH_AUTOMATION_WORKER
	int32 RejectedLootRequestCountForTests = 0;
#endif
};
