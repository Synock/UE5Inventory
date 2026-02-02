#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BankWidget.generated.h"

class UInventoryGridWidget;
class UDynamicPurseWidget;
class UButton;

/**
 * @class UBankWidget
 * @brief Widget for displaying and managing the player's bank storage
 *
 * Contains an inventory grid for items and a dynamic purse for bank currency.
 * Automatically initializes on construction with the player's bank data.
 */
UCLASS()
class INVENTORYPLUGIN_API UBankWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	//------------------------------------------------------------------------------------------------------------------
	// UI Elements (BindWidget)
	//------------------------------------------------------------------------------------------------------------------

	/** Grid widget displaying bank storage items */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Bank|UI")
	UInventoryGridWidget* InventoryGrid = nullptr;

	/** Widget displaying bank currency */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory|Bank|UI")
	UDynamicPurseWidget* DynamicPurse = nullptr;

	/** Button to reorganize bank contents */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory|Bank|UI")
	UButton* ReorganiseButton = nullptr;

	/** Button to close the bank window */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory|Bank|UI")
	UButton* DoneButton = nullptr;

	/** Delay in seconds before reorganise button can be pressed again */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bank|UI")
	float ReorganiseDelay = 5.f;

	/** Timer handle for reorganise button cooldown */
	FTimerHandle ReorganiseTimerHandle;

	//------------------------------------------------------------------------------------------------------------------
	// Lifecycle
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Initialize the widget on construction
	 * Called automatically by UMG when the widget is constructed
	 * Sets up the inventory grid and dynamic purse with bank data
	 */
	virtual void NativeConstruct() override;

	/**
	 * @brief Clean up when widget is destroyed
	 * Called automatically by UMG when the widget is destroyed
	 */
	virtual void NativeDestruct() override;

	//------------------------------------------------------------------------------------------------------------------
	// Button Handlers
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Handle ReorganiseButton click - calls ReorganizeContent
	 */
	UFUNCTION()
	void OnReorganiseButtonClicked();

	/**
	 * @brief Re-enable the reorganise button after cooldown
	 */
	UFUNCTION()
	void EnableReorganiseButton();

public:
	/**
	 * @brief Reorganize bank contents to optimize space
	 * Calls the bank component's reorganize function to compact items
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Bank")
	void ReorganizeContent();

	/**
	 * @brief Get the inventory grid widget
	 * @return The inventory grid widget displaying bank items
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Bank")
	UInventoryGridWidget* GetInventoryGrid() const { return InventoryGrid; }

	/**
	 * @brief Get the dynamic purse widget
	 * @return The purse widget displaying bank currency
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Bank")
	UDynamicPurseWidget* GetDynamicPurse() const { return DynamicPurse; }
};
