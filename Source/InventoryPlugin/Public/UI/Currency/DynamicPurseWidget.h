#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DynamicPurseWidget.generated.h"

class UCurrencyWidget;
class UCoinComponent;

/**
 * @class UDynamicPurseWidget
 *
 * Widget displaying a purse using four separate UCurrencyWidget instances (one per currency type).
 * Uses BindWidget pattern for automatic UI element binding.
 * Automatically syncs with a UCoinComponent and updates when coins change.
 * Each currency type is displayed in its own UCurrencyWidget with icon and value.
 */
UCLASS()
class INVENTORYPLUGIN_API UDynamicPurseWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	//------------------------------------------------------------------------------------------------------------------
	// UI Elements (BindWidget)
	//------------------------------------------------------------------------------------------------------------------

	/** Widget displaying copper currency */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Purse|UI")
	UCurrencyWidget* CopperCurrencyWidget = nullptr;

	/** Widget displaying silver currency */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Purse|UI")
	UCurrencyWidget* SilverCurrencyWidget = nullptr;

	/** Widget displaying gold currency */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Purse|UI")
	UCurrencyWidget* GoldCurrencyWidget = nullptr;

	/** Widget displaying platinum currency */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Purse|UI")
	UCurrencyWidget* PlatinumCurrencyWidget = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// Configuration & Data
	//------------------------------------------------------------------------------------------------------------------

	/** Reference to the coin component this widget is displaying */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Purse")
	UCoinComponent* PursePointer = nullptr;

	/** Whether this is a dynamic purse (unused legacy property) */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Purse")
	bool Dynamic = false;

	/** Allow currency type changes in the currency widgets */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory|Purse")
	bool AllowForCurrencyChange = false;

	//------------------------------------------------------------------------------------------------------------------
	// Internal Functions
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Refresh all currency widget displays from the coin component
	 * Called automatically when coin component changes
	 */
	UFUNCTION(BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void Refresh();

public:
	/**
	 * @brief Initialize the widget with a coin component
	 * Binds to the component's PurseDispatcher for automatic updates
	 * Sets up all four currency widgets with correct types
	 * @param Owner The coin component to display
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	void InitWidget(UCoinComponent* Owner);

	/**
	 * @brief Get the current purse pointer
	 * @return The coin component this widget is displaying
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|UI")
	UCoinComponent* GetPursePointer() const { return PursePointer; }
};
