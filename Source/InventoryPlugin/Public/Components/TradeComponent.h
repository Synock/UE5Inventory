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

	FTradeItemSlot() = default;

	FTradeItemSlot(int32 InItemID, EBagSlot InBagSlot, int32 InTopLeft)
		: ItemID(InItemID), SourceBagSlot(InBagSlot), SourceTopLeft(InTopLeft)
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

	// Coin being offered
	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	FCoinValue Coin;

	// Whether this player has accepted the trade
	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	bool bAccepted = false;

	void Reset()
	{
		Items.Empty();
		Coin = FCoinValue();
		bAccepted = false;
	}

	bool IsEmpty() const
	{
		return Items.Num() == 0 && Coin.IsEmpty();
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

	// Coin component for our offer (allows UDynamicPurseWidget binding)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Trade")
	UCoinComponent* OurCoinOffer = nullptr;

	// Coin component for their offer (allows UDynamicPurseWidget binding)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Trade")
	UCoinComponent* TheirCoinOffer = nullptr;

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
	UCoinComponent* GetTheirCoinComponent() const { return TheirCoinOffer; }

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
	 * @brief Set the coin amount we're offering (server-only, validates we have it)
	 * @param CoinAmount The coin value to offer
	 * @return True if coin was set successfully
	 */
	bool SetCoinOffer(const FCoinValue& CoinAmount);

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

private:
	/**
	 * @brief Helper to get the inventory interface from owner
	 */
	IInventoryPlayerInterface* GetInventoryInterface() const;

	/**
	 * @brief Helper to reset all trade state
	 */
	void ResetTradeState();
};

