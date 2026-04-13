#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "UObject/Interface.h"
#include "Blueprint/UserWidget.h"
#include "InventoryHUDInterface.generated.h"

class UInventoryItemBase;
enum class EBagSlot : uint8;
// This class does not need to be modified.
UINTERFACE()
class UInventoryHUDInterface : public UInterface
{
	GENERATED_BODY()
};


class INVENTORYPLUGIN_API IInventoryHUDInterface
{
	GENERATED_BODY()

public:

	//------------------------------------------------------------------------------------------------------------------
	// Inventory
	/**
	 * Sets the display state of the inventory.
	 *
	 * This method is a blueprint implementable event, meaning it can be directly implemented in blueprint scripts.
	 * It can also be called from blueprint scripts as it is marked as BlueprintCallable.
	 *
	 * @param State The state to set the inventory display to. True for visible, false for invisible.
	 * @see UFUNCTION()
	 * @see BlueprintImplementableEvent()
	 * @see BlueprintCallable()
	 * @see Category()
	 */

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	void SetInventoryDisplay(bool State);

	/**
	 * @brief Toggles the display of the inventory.
	 *
	 * This function can be called to toggle the visibility of the inventory display.
	 * It is an implementable event which can be used in blueprints.
	 * This function is also callable from blueprint scripts and is categorized under "Inventory".
	 *
	 * @param None
	 * @return None
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	void ToggleInventoryDisplay();

	/**
	 * @brief Forces a refresh of the inventory.
	 *
	 * This method is a Blueprint implementable event which can be called to force a refresh of the inventory.
	 * By calling this method, any changes made to the inventory will be updated and reflected in the UI.
	 *
	 * @param None
	 *
	 * @return None
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	void ForceRefreshInventory();

	/**
	 * @brief Sets the display state of the keyring.
	 *
	 * This function allows for changing the display state of the keyring in the inventory.
	 * The display state can be toggled between visible and hidden.
	 *
	 * @param State The new display state of the keyring.
	 *
	 * @see UFUNCTION
	 * @see BlueprintImplementableEvent
	 * @see BlueprintCallable
	 * @see Category
	 *
	 * @note This function is implemented as a blueprint event, which allows for customization in blueprint scripts.
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	void SetKeyringDisplay(bool State);

	/**
	 * @brief Toggles the display of the keyring in the inventory.
	 *
	 * This method is used to toggle the visibility of the keyring display within the inventory screen.
	 * The keyring is an optional feature that allows the player to view and manage their collection
	 * of keys or key-like objects within the game.
	 *
	 * @param None
	 * @return None
	 *
	 * @note This method can be called from Blueprint scripts.
	 * @note This method is implemented as an event and can be overridden in Blueprint.
	 * @note This method is categorized under "Inventory" in Blueprint.
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	void ToggleKeyringDisplay();

	/**
	 * Handles a bag slot for the inventory.
	 *
	 * @param InputBagSlot The bag slot to handle.
	 * @param Widget       The bag widget to interact with.
	 */
	UFUNCTION(Category = "Inventory")
	virtual void HandleBag(EBagSlot InputBagSlot, class UBagWidget* Widget) = 0;

	/**
	 * Displays the content of the bag in the specified bag slot.
	 *
	 * @param InputBagSlot The bag slot to display.
	 *
	 * @see EBagSlot
	 *
	 * @note This method is a blueprint callable and blueprint implementable event.
	 * @note This method is used for displaying the bag content in the inventory.
	 * @note This method is part of the "Inventory" category.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayBag(EBagSlot InputBagSlot);

	/**
	 * @brief Hides the bag at the specified slot.
	 *
	 * This method hides the bag located at the specified slot in the inventory.
	 *
	 * @param InputBagSlot The bag slot to hide.
	 *
	 * @return None
	 *
	 * @see ShowBag()
	 * @see IsBagVisible()
	 * @see EBagSlot
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void HideBag(EBagSlot InputBagSlot);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void HideAllBags();

	/**
	 * Unequips a bag from a specified bag slot.
	 *
	 * This method is a BlueprintCallable and BlueprintImplementableEvent which allows it to be called
	 * and implemented within Blueprint classes. It is also marked as BlueprintCosmetic to indicate that
	 * it is used for cosmetic purposes.
	 *
	 * @param InputBagSlot The bag slot from which the bag will be unequipped.
	 *
	 * @note This method is part of the "Inventory" category.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void UnequipBag(EBagSlot InputBagSlot);

	/**
	 * @brief Toggles the specified bag slot in the inventory.
	 *
	 * This function allows the player to toggle the visibility of a bag slot in the inventory. When a bag slot is toggled, it will either appear or disappear from the inventory UI.
	 *
	 * @param InputBagSlot The bag slot to toggle.
	 * @note This function is a blueprint callable and blueprint implementable event that can be accessed from blueprint scripts. It is categorized under "Inventory" and is marked as a blueprint
	 * cosmetic.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void ToggleBag(EBagSlot InputBagSlot);

	/**
	 * @brief Refreshes all inventory grids.
	 *
	 * This method will refresh all inventory grids. This can be used to update the display of the inventory grids
	 * after changes have been made to the underlying data.
	 *
	 * @param None
	 *
	 * @return None
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void RefreshAllInventoryGrids();

	/**
	 * @brief Displays a loot screen for the specified looted actor.
	 *
	 * This method is a BlueprintCallable, BlueprintImplementableEvent, and BlueprintCosmetic, allowing it to
	 * be used in Blueprint graphs and customized in Blueprint subclasses. It is categorized under "Inventory".
	 *
	 * @param LootedActor The actor from which the loot is obtained.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayLootScreen(class AActor* LootedActor);

	/**
	 * @brief Hides the loot screen in the inventory.
	 *
	 * This method is used to hide the loot screen in the inventory. The loot screen is typically
	 * displayed when the player receives loot in a game or after completing a quest. Hiding the loot
	 * screen allows the player to continue their gameplay without any interruptions.
	 *
	 * @param None.
	 *
	 * @return None.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void HideLootScreen();

	/**
	 * Displays the merchant screen for the given merchant actor.
	 *
	 * @param MerchantActor The merchant actor to display the screen for.
	 *
	 * @see UFUNCTION
	 * @see BlueprintCallable
	 * @see BlueprintImplementableEvent
	 * @see BlueprintCosmetic
	 * @see Category
	 * @see AActor
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayMerchantScreen(AActor* MerchantActor);

	/**
	 * @brief Hides the merchant screen.
	 *
	 * This function is a blueprint callable and implementable event used to hide the merchant screen in the inventory category.
	 *
	 * @note This function is blueprint cosmetic.
	 *
	 * @see ShowMerchantScreen()
	 *
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void HideMerchantScreen();

	/**
	 * @brief Displays the repair screen for a repairer NPC.
	 *
	 * @param RepairerActor The repairer actor to display the screen for.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayRepairScreen(AActor* RepairerActor);

	/**
	 * @brief Hides the repair screen.
	 *
	 * This function is a blueprint callable and implementable event used to hide the repair screen.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void HideRepairScreen();

	/**
	 * @brief Called when a repair transaction completes on the server.
	 *
	 * This event notifies the HUD that repairs have finished processing server-side.
	 * Use this to re-enable UI buttons and refresh the repair widget display.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void OnRepairTransactionComplete();

	/**
	 * UFUNCTION macro for the TryPresentSellItem method.
	 *
	 * This method is a BlueprintCallable, BlueprintImplementableEvent, and BlueprintCosmetic.
	 * It is used to present and sell an item from the inventory.
	 *
	 * @param OutSlot The bag slot on which the item is located.
	 * @param ItemID The unique identifier of the item.
	 * @param TopLeft The top-left coordinates of the item's UI display.
	 *
	 * @category Inventory
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void TryPresentSellItem(EBagSlot OutSlot, int32 ItemID, int32 TopLeft);

	/**
	 * @brief Resets the sell item action in the inventory.
	 *
	 * This method is used to reset the sell item action in the inventory.
	 *
	 * @param None
	 *
	 * Remarks:
	 * - This method is a BlueprintCallable, which means it can be called from Blueprint graphs.
	 * - This method is a BlueprintImplementableEvent, which means it can be overridden in Blueprint subclasses.
	 * - This method is BlueprintCosmetic, which means it is intended for cosmetic purposes and does not affect gameplay.
	 * - This method belongs to the Inventory category.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void ResetSellItem();

	/**
	 * Display the description of an inventory item at a specified position on the screen.
	 *
	 * @param Item The inventory item whose description needs to be displayed.
	 * @param X The X coordinate of the position where the description should be displayed.
	 * @param Y The Y coordinate of the position where the description should be displayed.
	 *
	 * This function is marked as a BlueprintCallable, BlueprintImplementableEvent, and BlueprintCosmetic,
	 * so it can be called from Blueprint graphs and overridden in Blueprint subclasses. It is categorized
	 * under "Inventory" in Blueprint editor for easy organization.
	 *
	 * Example usage in Blueprint:
	 * ![Example usage in Blueprint](../Images/DisplayItemDescriptionExample.png)
	 *
	 * Example usage in C++:
	 * ```cpp
	 * UInventoryItemBase* Item = ...; // Get the inventory item you want to display
	 * float X = ...;                // Specify the X coordinate of the position where the description should be displayed
	 * float Y = ...;                // Specify the Y coordinate of the position where the description should be displayed
	 *
	 * DisplayItemDescription(Item, X, Y);
	 * ```
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayItemDescription(const UInventoryItemBase* Item, float X, float Y);

	/**
	 * Display the description of an inventory item with durability information at a specified position on the screen.
	 *
	 * @param Item The inventory item whose description needs to be displayed.
	 * @param X The X coordinate of the position where the description should be displayed.
	 * @param Y The Y coordinate of the position where the description should be displayed.
	 * @param Durability The current durability of the item (absolute value).
	 * @param MaxDurability The maximum durability of the item (to calculate percentage).
	 *
	 * This function extends DisplayItemDescription by including durability information.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayItemDescriptionWithDurability(const UInventoryItemBase* Item, float X, float Y, float Durability, float MaxDurability);

	/**
	 * Returns the widget class to instantiate when displaying book text.
	 * The returned class must implement IInventoryBookWidgetInterface.
	 * Override to supply a custom class; the default loads the plugin's built-in UI_BookWidget.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Book")
	TSubclassOf<UUserWidget> GetBookWidgetClass() const;
	virtual TSubclassOf<UUserWidget> GetBookWidgetClass_Implementation() const;

	/**
	 * Displays the text of a book item at the specified viewport location.
	 *
	 * The default C++ implementation creates a widget via GetBookWidgetClass(), populates it
	 * with the item's content (IInventoryItemBookInterface::GetSimpleContent), and places it
	 * at (X, Y) in the viewport. Override in Blueprint or C++ for custom presentation.
	 *
	 * @param Item The book item to display.
	 * @param X    Viewport X position (DPI-independent).
	 * @param Y    Viewport Y position (DPI-independent).
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayBookText(const UInventoryItemBase* Item, float X, float Y);
	virtual void DisplayBookText_Implementation(const UInventoryItemBase* Item, float X, float Y);

	/**
	 * @brief Forces a refresh of the staging area possibilities.
	 *
	 * This method is used to manually trigger a refresh of the possibilities in the staging area of the inventory.
	 * It is a blueprint implementable event and can be called from blueprint scripts.
	 *
	 * @note This method does not return any value.
	 *
	 * @see UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	void ForceRefreshStagingAreaPossibilities();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void OpenTradeWindow();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void CloseTradeWindow();

	/**
	 * @brief Displays the field repair window for a repair kit in inventory.
	 * @param RepairKitItemID The item ID of the repair kit.
	 * @param BagSlot The bag slot which contains the repair kit.
	 * @param TopLeft The top-left index of the repair kit in the bag grid.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory|FieldRepair")
	void DisplayFieldRepairScreen(int32 RepairKitItemID, EBagSlot BagSlot, int32 TopLeft);

	/**
	 * @brief Hides the field repair window.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory|FieldRepair")
	void HideFieldRepairScreen();

	/**
	 * @brief Hides the field repair window.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory|FieldRepair")
	void NotifyFieldRepairFinished(EBagSlot RepairBagSlot, int32 RepairTopLeft, float ActualRepairAmount,
											   float NewTargetDurability, float NewKitDurability);

	/**
	 * @brief Lock or unlock a specific inventory UI slot so its widget cannot be dragged/moved.
	 * @param BagSlot The bag slot where the item resides.
	 * @param TopLeft The top-left index in the bag grid for the item.
	 * @param bLocked True to lock (disable interactions), false to unlock.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|FieldRepair")
	void LockInventorySlot(EBagSlot BagSlot, int32 TopLeft, bool bLocked);

	/**
	 * Lock or unlock an equipment slot (client-side UI state for field repair, etc.).
	 * @param EquipmentSlot The equipment slot to lock/unlock.
	 * @param bLocked True to lock (disable interactions), false to unlock.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|FieldRepair")
	void LockEquipmentSlot(EEquipmentSlot EquipmentSlot, bool bLocked);

	virtual void LockEquipmentSlot_Implementation(EEquipmentSlot EquipmentSlot, bool bLocked);
};
