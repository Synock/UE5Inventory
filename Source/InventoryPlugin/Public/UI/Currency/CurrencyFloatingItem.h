#pragma once

#include "CoreMinimal.h"
#include "Definitions.h"
#include "Blueprint/UserWidget.h"
#include "Components/CoinComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "CurrencyFloatingItem.generated.h"

/**
 * @class UCurrencyFloatingItem
 *
 * Floating widget displaying a single currency type with icon and amount.
 * Typically used for drag-and-drop currency operations or floating indicators.
 * Uses BindWidget pattern with automatic currency icon selection from GameInstance.
 */
UCLASS()
class INVENTORYPLUGIN_API UCurrencyFloatingItem : public UUserWidget
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
	// Data
	//------------------------------------------------------------------------------------------------------------------

	/** Type of currency being displayed */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Currency")
	ECurrencyType CurrencyType = ECurrencyType::Copper;

	/** Amount of currency being displayed */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Currency")
	int32 CoinAmount = 0;

	/** Origin coin component (optional reference) */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Currency")
	UCoinComponent* CoinOrigin = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// Internal Functions
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Update the currency icon based on CurrencyType
	 * Queries GameInstance for appropriate texture
	 */
	void UpdateCurrencyIcon();

	/**
	 * @brief Update the currency text with current amount
	 */
	void UpdateCurrencyText();

	/**
	 * @brief Setup UI with current currency data
	 */
	void SetupUI();

	virtual void NativePreConstruct() override;

public:
	/**
	 * @brief Initialize the widget with currency data
	 * @param CoinOriginPointer Optional reference to origin coin component
	 * @param InputCoinAmount Amount of currency to display
	 * @param InputCurrencyType Type of currency to display
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Currency")
	void InitWidget(UCoinComponent* CoinOriginPointer, int32 InputCoinAmount, ECurrencyType InputCurrencyType);

	/**
	 * @brief Get the current currency type
	 * @return The displayed currency type
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Currency")
	ECurrencyType GetCurrencyType() const { return CurrencyType; }

	/**
	 * @brief Get the current coin amount
	 * @return The displayed coin amount
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Currency")
	int32 GetCoinAmount() const { return CoinAmount; }
};
