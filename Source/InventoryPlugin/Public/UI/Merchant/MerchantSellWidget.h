#pragma once

#include <CoreMinimal.h>
#include <Blueprint/UserWidget.h>
#include <TimerManager.h>

#include "MerchantItemListWidget.h"
#include "MerchantItemWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "InventoryPlugin/Public/Interfaces/MerchantInterface.h"
#include "InventoryItem.h"
#include "UI/InventoryMerchantWindowInterface.h"
#include "UI/PurseWidget.h"
#include "MerchantSellWidget.generated.h"

class UCoinDisplayWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNotEnoughPlayerMoney);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNotEnoughPlayerSpace);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNotEnoughMerchantMoney);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMerchantRejectsItemType, const FString&, MerchantName, const FString&, ItemName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMerchantOffersPriceQuote, const FString&, MerchantName, const FString&, ItemName, const FCoinValue&, OfferPrice);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMerchantOffersItemForSale, const FString&, MerchantName, const FString&, ItemName, const FCoinValue&, SalePrice);

UENUM(BlueprintType)
enum class EMerchantWindowMode : uint8
{
	Sell = 0,
	Buy = 1,
};

/**
 * @class UMerchantSellWidget
 *
 * Modernized merchant sell/buy widget that uses BindWidget pattern.
 * Displays merchant inventory, handles buy/sell transactions, and shows item preview.
 */
UCLASS()
class INVENTORYPLUGIN_API UMerchantSellWidget : public UUserWidget, public IInventoryMerchantWindowInterface
{
	GENERATED_BODY()

protected:
	//------------------------------------------------------------------------------------------------------------------
	// UI Elements (BindWidget)
	//------------------------------------------------------------------------------------------------------------------

	/** Button to execute buy/sell transaction */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Merchant|UI")
	UButton* BuySellButton = nullptr;

	/** Optional button to close merchant window */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory|Merchant|UI")
	UButton* DoneButton = nullptr;

	/** Preview image of selected item */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Merchant|UI")
	UImage* ItemIconPreview = nullptr;

	/** Name of selected item */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Merchant|UI")
	UTextBlock* ItemName = nullptr;

	/** Text on buy/sell button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Merchant|UI")
	UTextBlock* BuySellButtonText = nullptr;

	/** Price display for selected item (can be TextBlock or CoinDisplayWidget) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory|Merchant|UI")
	UTextBlock* ItemPriceText = nullptr;

	/** Optional: Coin display widget for item price (alternative to ItemPriceText) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory|Merchant|UI")
	UCoinDisplayWidget* ItemPrice = nullptr;

	/** Merchant's name display */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Merchant|UI")
	UTextBlock* MerchantNameText = nullptr;

	/** List widget showing merchant's items */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Merchant|UI")
	UMerchantItemListWidget* ItemList = nullptr;

	/** Purse widget showing merchant's money */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Merchant|UI")
	UPurseWidget* MerchantPurse = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// Internal Data
	//------------------------------------------------------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Merchant")
	TScriptInterface<IMerchantInterface> MerchantActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Merchant")
	int32 SelectedItemId = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Merchant|Sell")
	int32 DynamicStartID = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Merchant|Buy")
	bool MerchantCanBuy = true;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Merchant|Buy")
	EBagSlot MerchantBuyOriginSlot = EBagSlot::Unknown;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Merchant|Buy")
	int32 MerchantBuyOriginTopLeft = -1;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Merchant")
	EMerchantWindowMode MerchantMode = EMerchantWindowMode::Sell;

	/** Pending post-transaction refresh. Cleared when the merchant session closes. */
	FTimerHandle TransactionRefreshTimer;

	//------------------------------------------------------------------------------------------------------------------
	// Internal Functions
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Handle BuySellButton click - calls HandleBuyClick or HandleSellClick based on merchant mode
	 */
	UFUNCTION()
	void OnBuySellButtonClicked();

	/**
	 * @brief Handle DoneButton click - calls StopTrading
	 */
	UFUNCTION()
	void OnDoneButtonClicked();

	/**
	 * @brief Update the item preview display
	 */
	void UpdateItemPreview();

	/**
	 * @brief Hide the item preview display
	 */
	void HideItemPreview();

	/**
	 * @brief Update the price preview display
	 */
	void UpdatePricePreview();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	TArray<FMerchantItemDataStruct> GetStaticDataDisplayable();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	TArray<FMerchantItemDataStruct> GetDynamicDataDisplayable();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void InitListFromStatic(UMerchantItemListWidget* InputListWidget);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void InitListFromDynamic(UMerchantItemListWidget* InputListWidget);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	bool MerchantCanSell(int32 ItemID) const;

	/**
	 * @brief Check if merchant accepts this item type for purchase from player
	 * @param ItemID The ID of the item player wants to sell
	 * @return True if merchant accepts this item type, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	bool CanMerchantAcceptItem(const UInventoryItemBase* Item) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	FCoinValue GetCorrectPrice(float FloatValue) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	void HandleBuyClick();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	void HandleSellClick();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	FCoinValue GetSelectedItemPrice() const;

	bool IsWorthless();

	/** Clear transient transaction state and all merchant-facing UI. */
	void ResetMerchantSessionState();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	void StopTrading();

public:
	//------------------------------------------------------------------------------------------------------------------
	// Public Interface
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void InitMerchantData(AActor* InputMerchantActor);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void DeInitMerchantData();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void Refresh();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	void ResetSellData();

	/** Returns the optional Done button so external owners can rebind it. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Merchant")
	UButton* GetDoneButton() const { return DoneButton; }

#if WITH_AUTOMATION_WORKER
	bool MerchantCanSellForTests(int32 ItemID) const { return MerchantCanSell(ItemID); }
	void SetMerchantSessionStateForTests(int32 ItemID, int32 TopLeft, EBagSlot OriginBag, EMerchantWindowMode Mode)
	{
		SelectedItemId = ItemID;
		MerchantBuyOriginTopLeft = TopLeft;
		MerchantBuyOriginSlot = OriginBag;
		MerchantMode = Mode;
	}
	bool HasMerchantSessionStateForTests() const
	{
		return SelectedItemId != 0 || MerchantBuyOriginTopLeft != -1 ||
			MerchantBuyOriginSlot != EBagSlot::Unknown || MerchantMode != EMerchantWindowMode::Sell;
	}
#endif

	// ---- IInventoryMerchantWindowInterface ----------------------------------

	virtual void InitMerchantWindow_Implementation(AActor* NewMerchantActor) override;
	virtual void DeInitMerchantWindow_Implementation() override;
	virtual void ShowMerchantWindow_Implementation() override;
	virtual void HideMerchantWindow_Implementation() override;
	virtual void RefreshMerchantWindow_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant|Buy")
	void AssignSellData(int32 ItemID, int32 TopLeft, EBagSlot OriginBag);

	/**
	 * @brief Handle item selection from merchant list
	 * Sets the selected item, merchant mode to Sell, and updates preview
	 * @param ItemID The ID of the selected item
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	void OnItemListSelectionChanged(int32 ItemID);

	//------------------------------------------------------------------------------------------------------------------
	// Notification Callbacks
	//------------------------------------------------------------------------------------------------------------------

	// These functions are intended for player notification of the possible results
	virtual void OnNotEnoughPlayerMoney();
	virtual void OnNotEnoughPlayerSpace();
	virtual void OnNotEnoughMerchantMoney();
	virtual void OnMerchantRejectsItemType(const FString& MerchantName,const FString& RefusedItemName);
	virtual void OnMerchantOffersPriceQuote(const FString& MerchantName, const FString& ItemName, const FCoinValue& OfferPrice);
	virtual void OnMerchantOffersItemForSale(const FString& MerchantName, const FString& ItemName, const FCoinValue& SalePrice);

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Merchant")
	FNotEnoughPlayerMoney OnNotEnoughPlayerMoneyDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Merchant")
	FNotEnoughPlayerSpace OnNotEnoughPlayerSpaceDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Merchant")
	FNotEnoughMerchantMoney OnNotEnoughMerchantMoneyDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Merchant")
	FMerchantRejectsItemType OnMerchantRejectsItemTypeDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Merchant")
	FMerchantOffersPriceQuote OnMerchantOffersPriceQuoteDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Merchant")
	FMerchantOffersItemForSale OnMerchantOffersItemForSaleDelegate;
};
