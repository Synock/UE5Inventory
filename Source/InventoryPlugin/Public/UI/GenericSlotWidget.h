// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "ItemBaseWidget.h"
#include "Components/Image.h"
#include "GenericSlotWidget.generated.h"

class IInventoryPlayerInterface;

/**
 * @class UGenericSlotWidget
 *
 * Base widget for inventory item slots using modern BindWidget pattern.
 * Provides common functionality for displaying items in slots with background images.
 * Can be enabled/disabled and handles item drop validation.
 */
UCLASS()
class INVENTORYPLUGIN_API UGenericSlotWidget : public UItemBaseWidget
{
	GENERATED_BODY()

protected:
	//------------------------------------------------------------------------------------------------------------------
	// UI Elements (BindWidget)
	//------------------------------------------------------------------------------------------------------------------

	/** Background image for the slot (optional) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory|Slot|UI")
	UImage* BackgroundImagePointer = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// State
	//------------------------------------------------------------------------------------------------------------------

	/** Whether this slot is currently enabled and can accept items */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Slot")
	bool EnabledSlot = true;

	//------------------------------------------------------------------------------------------------------------------
	// Protected Functions
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Updates the visibility of the item image based on whether an item is present
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	void UpdateItemImageVisibility();

	/**
	 * @brief Updates the slot state (enabled/disabled)
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	void UpdateSlotState();

	/**
	 * @brief Handles when an item is dropped onto this slot
	 * @param InputItem The item widget being dropped
	 * @return True if the drop was handled successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	virtual bool HandleItemDrop(class UItemWidget* InputItem);

	/**
	 * @brief Gets the inventory player interface from the owning player
	 * @return Pointer to the inventory player interface
	 */
	IInventoryPlayerInterface* GetInventoryPlayerInterface() const;

	/**
	 * @brief Internal refresh function that updates visibility and state
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	virtual void InnerRefresh();

	/**
	 * @brief Resets any pending transaction on the inventory interface
	 */
	UFUNCTION(Category = "Inventory|Slot")
	void ResetTransaction();

public:
	//------------------------------------------------------------------------------------------------------------------
	// Public Interface
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Checks if an item can be dropped into this slot
	 * @param InputItem The item to check
	 * @return True if the item can be dropped into this slot
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	bool CanDropItem(const UInventoryItemBase* InputItem) const;

	/**
	 * @brief Checks if this slot currently has an item equipped
	 * @return True if an item is present in this slot
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Slot")
	bool IsItemEquipped() const { return Item != nullptr; }

	/**
	 * @brief Hides the item from display
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	virtual void HideItem();

};
