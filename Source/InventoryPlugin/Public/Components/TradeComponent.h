#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CoinValue.h"
#include "Definitions.h"
#include "TradeComponent.generated.h"

class IInventoryPlayerInterface;
class UCoinComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTradeStateChangedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTradeItemsChangedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTradeCoinChangedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTradeAcceptanceChangedDelegate);

// Delegates for trade action notifications (for chat messages)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTradeItemAddedDelegate, FString, PlayerName, class UInventoryItemBase*, Item, bool, bIsOurOffer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTradeItemRemovedDelegate, FString, PlayerName, class UInventoryItemBase*, Item, bool, bIsOurOffer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTradeCoinAddedDelegate, FString, PlayerName, FCoinValue, CoinAmount, bool, bIsOurOffer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTradeCoinRemovedDelegate, FString, PlayerName, FCoinValue, CoinAmount, bool, bIsOurOffer);

// Delegate for trade cancellation with reason (for error messages on game side)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTradeCancelledDelegate, APlayerController*, PlayerController, FString, CancellationReason);

/**
 * @brief Represents a single item in a trade offer
 */
USTRUCT(BlueprintType)
struct FTradeItemSlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	int32 ItemID = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	EBagSlot SourceBagSlot = EBagSlot::Unknown;

	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	int32 SourceTopLeft = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	float Durability = 100.0f;

	FTradeItemSlot() = default;

	FTradeItemSlot(int32 InItemID, EBagSlot InBagSlot, int32 InTopLeft, float InDurability)
		: ItemID(InItemID), SourceBagSlot(InBagSlot), SourceTopLeft(InTopLeft), Durability(InDurability)
	{
	}

	bool IsValid() const { return ItemID > 0 && SourceTopLeft >= 0; }
};

/**
 * @brief Represents a complete trade offer from one player
 */
USTRUCT(BlueprintType)
struct FTradeOffer
{
	GENERATED_BODY()

	// Items being offered (max 8 slots like staging area)
	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	TArray<FTradeItemSlot> Items;

	// Whether this player has accepted the trade
	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	bool bAccepted = false;

	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	FCoinValue CoinOffer;

	void Reset()
	{
		Items.Empty();
		bAccepted = false;
		CoinOffer = {};
	}

	bool IsEmpty() const
	{
		return Items.Num() == 0 && CoinOffer.IsEmpty();
	}
};

/**
 * @class UTradeComponent
 *
 * Component attached to player controllers to manage player-to-player trading.
 * Handles trade offers, acceptance, validation, and execution.
 * Server-authoritative with replicated state for client UI.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API UTradeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTradeComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//------------------------------------------------------------------------------------------------------------------
	// Replicated State
	//------------------------------------------------------------------------------------------------------------------

	// Actor we're currently trading with (nullptr if not trading)
	UPROPERTY(ReplicatedUsing=OnRep_TradePartner, BlueprintReadOnly, Category = "Trade")
	AActor* TradePartner = nullptr;

	// Our current offer
	UPROPERTY(ReplicatedUsing=OnRep_OurOffer, BlueprintReadOnly, Category = "Trade")
	FTradeOffer OurOffer;

	// Their current offer
	UPROPERTY(ReplicatedUsing=OnRep_TheirOffer, BlueprintReadOnly, Category = "Trade")
	FTradeOffer TheirOffer;

	// Trade session active
	UPROPERTY(ReplicatedUsing=OnRep_IsTrading, BlueprintReadOnly, Category = "Trade")
	bool bIsTrading = false;

	// Coin component for our offer (allows UDynamicPurseWidget binding and direct manipulation)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Trade")
	UCoinComponent* OurCoinOffer = nullptr;

	// Track previous coin value to detect add/remove operations
	FCoinValue PreviousCoinValue;


	//------------------------------------------------------------------------------------------------------------------
	// Replication Callbacks
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION()
	void OnRep_TradePartner();

	UFUNCTION()
	void OnRep_OurOffer();

	UFUNCTION()
	void OnRep_TheirOffer();

	UFUNCTION()
	void OnRep_IsTrading();

	// Called when OurCoinOffer component changes (from user interaction)
	UFUNCTION()
	void OnOurCoinOfferChanged();

	// Server RPC to notify of coin changes
	UFUNCTION(Server, Reliable)
	void Server_NotifyCoinOfferChanged();


public:
	//------------------------------------------------------------------------------------------------------------------
	// Delegates for UI Updates
	//------------------------------------------------------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Trade")
	FOnTradeStateChangedDelegate OnTradeStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Trade")
	FOnTradeItemsChangedDelegate OnOurItemsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Trade")
	FOnTradeItemsChangedDelegate OnTheirItemsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Trade")
	FOnTradeCoinChangedDelegate OnOurCoinChanged;

	UPROPERTY(BlueprintAssignable, Category = "Trade")
	FOnTradeCoinChangedDelegate OnTheirCoinChanged;

	UPROPERTY(BlueprintAssignable, Category = "Trade")
	FOnTradeAcceptanceChangedDelegate OnAcceptanceChanged;

	//------------------------------------------------------------------------------------------------------------------
	// Trade Action Notification Delegates (for chat messages)
	//------------------------------------------------------------------------------------------------------------------

	/** Broadcast when an item is added to trade (visible to both players) */
	UPROPERTY(BlueprintAssignable, Category = "Trade|Notifications")
	FOnTradeItemAddedDelegate OnTradeItemAdded;

	/** Broadcast when an item is removed from trade (visible to both players) */
	UPROPERTY(BlueprintAssignable, Category = "Trade|Notifications")
	FOnTradeItemRemovedDelegate OnTradeItemRemoved;

	/** Broadcast when coins are added to trade (visible to both players) */
	UPROPERTY(BlueprintAssignable, Category = "Trade|Notifications")
	FOnTradeCoinAddedDelegate OnTradeCoinAdded;

	/** Broadcast when coins are removed from trade (visible to both players) */
	UPROPERTY(BlueprintAssignable, Category = "Trade|Notifications")
	FOnTradeCoinRemovedDelegate OnTradeCoinRemoved;

	/** Broadcast when trade is cancelled with a reason (for error messages on game side) */
	UPROPERTY(BlueprintAssignable, Category = "Trade|Notifications")
	FOnTradeCancelledDelegate OnTradeCancelled;

	//------------------------------------------------------------------------------------------------------------------
	// Public Getters (Blueprint & Code)
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trade")
	bool IsTrading() const { return bIsTrading; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trade")
	AActor* GetTradePartner() const { return TradePartner; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trade")
	const FTradeOffer& GetOurOffer() const { return OurOffer; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trade")
	const FTradeOffer& GetTheirOffer() const { return TheirOffer; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trade")
	bool HaveWeAccepted() const { return OurOffer.bAccepted; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trade")
	bool HaveTheyAccepted() const { return TheirOffer.bAccepted; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trade")
	bool BothAccepted() const { return OurOffer.bAccepted && TheirOffer.bAccepted; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trade")
	UCoinComponent* GetOurCoinComponent() const { return OurCoinOffer; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trade")
	FCoinValue GetTheirCoinOffer() const { return TheirOffer.CoinOffer; }

	//------------------------------------------------------------------------------------------------------------------
	// Server-Only Functions (Called by PlayerController RPCs)
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Start a trade session with another player (server-only)
	 * @param OtherTrader The actor to trade with
	 * @return True if trade started successfully
	 */
	bool StartTrade(ACharacter* OtherTrader);

	/**
	 * @brief Cancel the current trade session (server-only)
	 */
	void CancelTrade();

	/**
	 * @brief Add an item to our trade offer (server-only, validates ownership)
	 * @param ItemID The item ID
	 * @param BagSlot The bag containing the item
	 * @param TopLeft The position in the bag
	 * @return True if item was added successfully
	 */
	bool AddItemToOffer(int32 ItemID, EBagSlot BagSlot, int32 TopLeft);

	/**
	 * @brief Remove an item from our trade offer (server-only)
	 * @param SlotIndex The slot index (0-7) to remove
	 * @return True if item was removed
	 */
	bool RemoveItemFromOffer(int32 SlotIndex);

	/**
	 * @brief Toggle our acceptance of the current trade (server-only)
	 * @param bAccept True to accept, false to unaccept
	 */
	void SetAcceptance(bool bAccept);

	/**
	 * @brief Update the partner's offer (called by their controller, server-only)
	 * @param PartnerOffer Their complete offer
	 */
	void UpdatePartnerOffer(const FTradeOffer& PartnerOffer);

	/**
	 * @brief Execute the trade (server-only, called when both accepted)
	 * @return True if trade completed successfully
	 */
	bool ExecuteTrade();

	//------------------------------------------------------------------------------------------------------------------
	// Validation Helpers (Server-Only)
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Validates that we actually own the items we're offering
	 * @return True if all items are valid
	 */
	bool ValidateOurItems() const;

	/**
	 * @brief Validates that we have the coin we're offering
	 * @return True if we have enough coin
	 */
	bool ValidateOurCoin() const;

	/**
	 * @brief Validates that the partner has inventory space for our items
	 * @return True if they can receive our items
	 */
	bool ValidatePartnerHasSpace() const;

	/**
	 * @brief Validates that both traders are within acceptable distance
	 * @return True if distance is valid, false if too far
	 */
	bool ValidateTradeDistance() const;

private:
	/**
	 * Maximum distance in units between traders to allow trade execution
	 * Server-only, not replicated
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Trade|Settings")
	float MaxTradeDistance = 500.0f;

	/**
	 * Sphere component for efficient collision-based distance detection
	 * Attached to player when trade starts
	 */
	UPROPERTY()
	class USphereComponent* TradeRangeSphere;

	/**
	 * Called when the trade partner exits the interaction sphere
	 */
	UFUNCTION()
	void OnTradePartnerExitSphere(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	/**
	 * @brief Helper to get the inventory interface from owner
	 */
	IInventoryPlayerInterface* GetInventoryInterface() const;

	/**
	 * @brief Helper to get the partner's trade component
	 * Handles the conversion from TradePartner (Character/Pawn) to Controller to TradeComponent
	 */
	UTradeComponent* GetPartnerTradeComponent() const;

	/**
	 * @brief Helper to reset all trade state
	 * @param bReturnItems If true, returns items to owner's inventory (used on cancel). If false, items are not returned (used on successful trade).
	 */
	void ResetTradeState(bool bReturnItems = true);
	bool ReturnEscrowedItem(const FTradeItemSlot& ItemSlot);

	/**
	 * @brief Broadcast a trade action notification to both trading parties
	 * @param PlayerName The name of the player performing the action
	 * @param Item The item being added/removed (can be nullptr for coin-only notifications)
	 * @param CoinValue The coin amount being added/removed
	 * @param bIsAdd True if adding, false if removing
	 * @param bIsItem True if item notification, false if coin notification
	 * @param bIsOurAction True if this is our action, false if partner's action
	 */
	void BroadcastTradeNotification(const FString& PlayerName, UInventoryItemBase* Item, const FCoinValue& CoinValue, bool bIsAdd, bool bIsItem, bool bIsOurAction);

	/**
	 * @brief Get the player name for notifications
	 */
	FString GetOwnerPlayerName() const;
};
