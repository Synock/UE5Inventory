#pragma once

#include "CoreMinimal.h"
#include "Definitions.h"
#include "Blueprint/UserWidget.h"
#include "Components/CoinComponent.h"
#include "Components/Image.h"
#include "Components/SpinBox.h"
#include "Components/Slider.h"
#include "Components/Button.h"
#include "CurrencySelectionWidget.generated.h"

/**
 * @class UCurrencySelectionWidget
 *
 * Widget for selecting and transferring a specific amount of currency between two coin components.
 * Uses BindWidget pattern with synchronized spinbox and slider controls.
 * Displays currency icon based on CurrencyType via GameInstance interface.
 */
UCLASS()
class INVENTORYPLUGIN_API UCurrencySelectionWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	//------------------------------------------------------------------------------------------------------------------
	// UI Elements (BindWidget)
	//------------------------------------------------------------------------------------------------------------------

	/** Image displaying the currency icon */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Currency|UI")
	UImage* CurrencyIcon = nullptr;

	/** Spinbox for precise currency value input */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Currency|UI")
	USpinBox* CurrencySpinbox = nullptr;

	/** Slider for quick currency value selection */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Currency|UI")
	USlider* CurrencySlider = nullptr;

	/** Button to confirm and execute the transfer */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Currency|UI")
	UButton* OkButton = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// Configuration & Data
	//------------------------------------------------------------------------------------------------------------------

	/** Source coin component for the transfer */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Currency")
	UCoinComponent* Origin = nullptr;

	/** Destination coin component for the transfer */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Currency")
	UCoinComponent* Destination = nullptr;

	/** Type of currency being transferred from origin */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Currency")
	ECurrencyType CurrencyType = ECurrencyType::Copper;

	/** Type of currency being converted to at destination */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Currency")
	ECurrencyType DesiredCurrencyType = ECurrencyType::Copper;

	/** Maximum amount of coins that can be transferred */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Currency")
	int32 MaximumCoinCapacity = 0;

	/** Allow currency type to be changed at runtime */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Currency")
	bool AllowForCurrencyChange = false;

	//------------------------------------------------------------------------------------------------------------------
	// Internal Functions
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Update the currency icon based on CurrencyType
	 * Queries GameInstance for appropriate texture
	 */
	void UpdateCurrencyIcon();

	/**
	 * @brief Setup UI elements with initial values and bindings
	 */
	void SetupUI();

	/**
	 * @brief Handle spinbox value changed
	 * Synchronizes slider with spinbox value
	 */
	UFUNCTION()
	void OnSpinboxValueChanged(float Value);

	/**
	 * @brief Handle slider value changed
	 * Synchronizes spinbox with slider value
	 */
	UFUNCTION()
	void OnSliderValueChanged(float Value);

	/**
	 * @brief Handle OK button clicked
	 * Executes transfer and removes widget
	 */
	UFUNCTION()
	void OnOkButtonClicked();

	virtual void NativePreConstruct() override;

public:
	/**
	 * @brief Initialize the widget with coin components and currency types
	 * @param OriginCoinComponent Source of coins
	 * @param DestinationCoinComponent Destination for coins
	 * @param InputCurrencyType Currency type to transfer from origin
	 * @param OutputCurrencyType Currency type to convert to at destination
	 * @param AllowForCurrencyChangeState Whether currency type can be changed
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Currency")
	void InitWidget(UCoinComponent* OriginCoinComponent, UCoinComponent* DestinationCoinComponent,
	                ECurrencyType InputCurrencyType, ECurrencyType OutputCurrencyType, bool AllowForCurrencyChangeState = false);

	/**
	 * @brief Execute the coin transfer
	 * @param SelectedCoinValue Amount of coins to transfer
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Currency")
	void DoTheCoinTransfer(int32 SelectedCoinValue);
};
