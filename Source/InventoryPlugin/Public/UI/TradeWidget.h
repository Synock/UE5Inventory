#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "TradeWidget.generated.h"

class UDynamicPurseWidget;
class UPurseWidget;
class UTradeComponent;
class UTradeSlotWidget;
class UCoinDisplayWidget;
class IInventoryPlayerInterface;

/**
 * @class UTradeWidget
 *
 * Main UI widget for player-to-player trading.
 * Displays both players' offers, handles item/coin placement, and manages trade acceptance.
 * Follows the pattern of MerchantSellWidget with BindWidget for UMG integration.
 */
UCLASS()
class INVENTORYPLUGIN_API UTradeWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	//------------------------------------------------------------------------------------------------------------------
	// UI Elements (BindWidget) - Left Side (Our Offer)
	//------------------------------------------------------------------------------------------------------------------

	/** Container for our trade slots (0-7) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Our")
	UTradeSlotWidget* OurSlot0 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Our")
	UTradeSlotWidget* OurSlot1 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Our")
	UTradeSlotWidget* OurSlot2 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Our")
	UTradeSlotWidget* OurSlot3 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Our")
	UTradeSlotWidget* OurSlot4 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Our")
	UTradeSlotWidget* OurSlot5 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Our")
	UTradeSlotWidget* OurSlot6 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Our")
	UTradeSlotWidget* OurSlot7 = nullptr;

	/** Display our offered coin */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Our")
	UDynamicPurseWidget* OurCoinOffer = nullptr;

	/** Our player name */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Our")
	UTextBlock* OurNameText = nullptr;

	/** Indicator that we've accepted */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Trade|UI|Our")
	UTextBlock* OurAcceptedText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Trade|UI|Our")
	UImage* OurAcceptedIcon = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// UI Elements (BindWidget) - Right Side (Their Offer)
	//------------------------------------------------------------------------------------------------------------------

	/** Container for their trade slots (0-7) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Their")
	UTradeSlotWidget* TheirSlot0 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Their")
	UTradeSlotWidget* TheirSlot1 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Their")
	UTradeSlotWidget* TheirSlot2 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Their")
	UTradeSlotWidget* TheirSlot3 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Their")
	UTradeSlotWidget* TheirSlot4 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Their")
	UTradeSlotWidget* TheirSlot5 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Their")
	UTradeSlotWidget* TheirSlot6 = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Their")
	UTradeSlotWidget* TheirSlot7 = nullptr;

	/** Display their offered coin */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Their")
	UPurseWidget* TheirCoinOffer = nullptr;

	/** Their player name */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI|Their")
	UTextBlock* TheirNameText = nullptr;

	/** Indicator that they've accepted */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Trade|UI|Their")
	UTextBlock* TheirAcceptedText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Trade|UI|Their")
	UImage* TheirAcceptedIcon = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// UI Elements (BindWidget) - Center Controls
	//------------------------------------------------------------------------------------------------------------------

	/** Button to accept/unaccept trade */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI")
	UButton* AcceptButton = nullptr;

	/** Text on accept button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI")
	UTextBlock* AcceptButtonText = nullptr;

	/** Button to cancel trade */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Trade|UI")
	UButton* CancelButton = nullptr;

	/** Status message (e.g., "Waiting for other player...") */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Trade|UI")
	UTextBlock* StatusText = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// Internal Data
	//------------------------------------------------------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	UTradeComponent* TradeComponent = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// Button Callbacks
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION()
	void OnAcceptButtonClicked();

	UFUNCTION()
	void OnCancelButtonClicked();

	//------------------------------------------------------------------------------------------------------------------
	// Delegate Handlers (bound to TradeComponent)
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION()
	void OnTradeStateChanged();

	UFUNCTION()
	void OnOurItemsChanged();

	UFUNCTION()
	void OnTheirItemsChanged();

	UFUNCTION()
	void OnOurCoinChanged();

	UFUNCTION()
	void OnTheirCoinChanged();

	UFUNCTION()
	void OnAcceptanceChanged();

	//------------------------------------------------------------------------------------------------------------------
	// Refresh Functions
	//------------------------------------------------------------------------------------------------------------------

	void RefreshOurOffer();
	void RefreshTheirOffer();
	void RefreshAcceptanceState();
	void RefreshStatusText();

	//------------------------------------------------------------------------------------------------------------------
	// Helper Functions
	//------------------------------------------------------------------------------------------------------------------

	UTradeSlotWidget* GetOurSlot(int32 Index);
	UTradeSlotWidget* GetTheirSlot(int32 Index);

	IInventoryPlayerInterface* GetInventoryInterface() const;

public:
	/**
	 * @brief Initialize the trade widget with the local player's trade component
	 * @param InTradeComponent The trade component to bind to
	 */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	void InitializeTrade(UTradeComponent* InTradeComponent);

	/**
	 * @brief Close the trade window and clean up
	 */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	void CloseTrade();
};

