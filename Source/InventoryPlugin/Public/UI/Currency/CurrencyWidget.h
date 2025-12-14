// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Definitions.h"
#include "Blueprint/UserWidget.h"
#include "Components/CoinComponent.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "CurrencyWidget.generated.h"

/**
 * @class UCurrencyWidget
 *
 * Widget for displaying a single currency type with icon and value.
 * Uses BindWidget pattern for UI elements.
 * Icon texture is automatically selected based on CurrencyType and GameInstance configuration.
 */
UCLASS()
class INVENTORYPLUGIN_API UCurrencyWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	//------------------------------------------------------------------------------------------------------------------
	// UI Elements (BindWidget)
	//------------------------------------------------------------------------------------------------------------------

	/** Image displaying the currency icon */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Currency|UI")
	UImage* CurrencyIcon = nullptr;

	/** Text displaying the currency amount */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Currency|UI")
	UTextBlock* CurrencyText = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// Configuration & Data
	//------------------------------------------------------------------------------------------------------------------

	/** Type of currency to display (Copper, Silver, Gold, Platinum) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory|Currency")
	ECurrencyType CurrencyType = ECurrencyType::Copper;

	/** Current coin amount */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Currency")
	int32 CoinAmount = 0;

	/** Reference to the coin component */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Currency")
	UCoinComponent* CoinComponent = nullptr;

	/** Allow currency type to be changed at runtime */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Currency")
	bool AllowForCurrencyChange = false;

	//------------------------------------------------------------------------------------------------------------------
	// Internal Functions
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Update the currency icon based on current CurrencyType
	 * Queries GameInstance for appropriate coin texture
	 */
	void UpdateCurrencyIcon();

	/**
	 * @brief Refresh the display with current coin amount
	 */
	UFUNCTION(BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void Refresh();

	/**
	 * @brief Called during widget construction in both editor and runtime
	 * Updates the currency icon based on current CurrencyType
	 */
	virtual void NativePreConstruct() override;

public:
	/**
	 * @brief Update the displayed coin value
	 * @param NewCoinAmount The new amount to display
	 */
	UFUNCTION(BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void UpdateCoinValue(int32 NewCoinAmount);

	/**
	 * @brief Set the currency type to display
	 * Updates the icon to match the new currency type
	 * @param NewCurrencyType The currency type to display
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	void SetCurrencyType(ECurrencyType NewCurrencyType);

	/**
	 * @brief Setup the coin component and configuration
	 * @param OriginCoinComponent The coin component to monitor
	 * @param AllowForCurrencyChangeState Whether currency type can be changed
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|UI")
	void SetupCoinComponent(UCoinComponent* OriginCoinComponent, bool AllowForCurrencyChangeState = false);

	/**
	 * @brief Get the current currency type
	 * @return The currently displayed currency type
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|UI")
	ECurrencyType GetCurrencyType() const { return CurrencyType; }

	/**
	 * @brief Get the current coin amount
	 * @return The currently displayed coin amount
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|UI")
	int32 GetCoinAmount() const { return CoinAmount; }
};
