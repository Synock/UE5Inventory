#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "UObject/Interface.h"
#include "Blueprint/UserWidget.h"
#include "UI/InventoryBagWindowInterface.h"
#include "UI/InventoryLootWindowInterface.h"
#include "UI/InventoryMerchantWindowInterface.h"
#include "UI/InventoryRepairWindowInterface.h"
#include "UI/FieldRepairWidgetInterface.h"
#include "UI/InventoryWindowInterface.h"
#include "UI/Keyring/KeyringWindowInterface.h"
#include "UI/TradeWindowInterface.h"
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
	// Inventory window registry

	/**
	 * Return the registered inventory window, or an empty interface if none.
	 * Default returns empty; override to expose your stored window.
	 */
	virtual TScriptInterface<IInventoryWindowInterface> GetInventoryWindow() const;

	/**
	 * Register the inventory window.
	 * Default is a no-op; override to store in your HUD.
	 */
	virtual void RegisterInventoryWindow(TScriptInterface<IInventoryWindowInterface> InventoryWindow);

	//------------------------------------------------------------------------------------------------------------------
	// Inventory

	/**
	 * Show or hide the inventory window.
	 *
	 * Default C++ flow:
	 *   - State == true:  ShowInventoryWindow → RefreshInventoryEquipments → RefreshInventoryGrids
	 *   - State == false: HideInventoryWindow
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void SetInventoryDisplay(bool State);
	virtual void SetInventoryDisplay_Implementation(bool State);

	/**
	 * Toggle the inventory window (show if hidden, hide if visible).
	 * Default calls SetInventoryDisplay(!IsInventoryWindowVisible()).
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void ToggleInventoryDisplay();
	virtual void ToggleInventoryDisplay_Implementation();

	/**
	 * Full inventory refresh: refresh equipment display, all registered bag windows, then all inventory grids.
	 *
	 * Default C++ flow:
	 *   1. (no-op at plugin level — game overrides to call InventoryWindow->RefreshAllEquipments first)
	 *   2. Iterates GetRegisteredBagSlots() and calls Execute_RefreshBagWindow on each window.
	 *   3. Calls Execute_RefreshAllInventoryGrids(this).
	 *
	 * Game side (UHUDWidget) overrides this to prepend InventoryWindow->RefreshAllEquipments().
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
	void ForceRefreshInventory();
	virtual void ForceRefreshInventory_Implementation();

	//------------------------------------------------------------------------------------------------------------------
	// Keyring window registry

	/**
	 * Return the registered keyring window, or an empty interface if none.
	 * Default returns empty; override to expose your stored window.
	 */
	virtual TScriptInterface<IKeyringWindowInterface> GetKeyringWindow() const;

	/**
	 * Register the keyring window.
	 * Default is a no-op; override to store in your HUD.
	 */
	virtual void RegisterKeyringWindow(TScriptInterface<IKeyringWindowInterface> KeyringWindow);

	/**
	 * Show or hide the keyring window.
	 * Default dispatches through IKeyringWindowInterface::Show/HideKeyringWindow.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void SetKeyringDisplay(bool State);
	virtual void SetKeyringDisplay_Implementation(bool State);

	/**
	 * Toggle the keyring window (show if hidden, hide if visible).
	 * Default calls SetKeyringDisplay(!IsKeyringWindowVisible()).
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void ToggleKeyringDisplay();
	virtual void ToggleKeyringDisplay_Implementation();

	//------------------------------------------------------------------------------------------------------------------
	// Bag window registry
	// Override these in the implementing class to expose your bag window storage.
	// The C++ defaults for the bag lifecycle events below call these to locate windows.

	/**
	 * Return the bag window registered for the given slot, or an empty interface if none.
	 * Default returns an empty TScriptInterface (no registered window).
	 */
	virtual TScriptInterface<IInventoryBagWindowInterface> GetBagWindowForSlot(EBagSlot Slot) const;

	/**
	 * Register a bag window for the given slot.
	 * Default is a no-op; override to store in your map.
	 */
	virtual void RegisterBagWindowForSlot(EBagSlot Slot,
	                                       TScriptInterface<IInventoryBagWindowInterface> BagWindow);

	/**
	 * Return all slots that have a registered bag window.
	 * Default returns an empty array.
	 */
	virtual TArray<EBagSlot> GetRegisteredBagSlots() const;

	//------------------------------------------------------------------------------------------------------------------
	// Bag lifecycle

	/**
	 * Returns the widget class to instantiate for a bag window.
	 * Must implement IInventoryBagWindowInterface.
	 * Default returns nullptr; override to supply a game-specific draggable window class.
	 * Used by DisplayBag when no window is pre-registered for the requested slot.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Bag")
	TSubclassOf<UUserWidget> GetBagWindowClass() const;
	virtual TSubclassOf<UUserWidget> GetBagWindowClass_Implementation() const;

	/**
	 * Initialize a bag window with data from the equipped bag item in the given slot.
	 *
	 * The default C++ implementation resolves the owning pawn's IEquipmentInterface,
	 * queries GetEquippedItem(slot), casts to IInventoryItemBagInterface, then calls
	 * IInventoryBagWindowInterface::Execute_InitBagData on the provided window.
	 *
	 * Override in C++ only when the default equipment query path is insufficient.
	 *
	 * @param InputBagSlot  The bag slot whose equipped item provides the dimensions.
	 * @param BagWindow     The window to initialize.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Bag")
	void HandleBag(EBagSlot InputBagSlot, const TScriptInterface<IInventoryBagWindowInterface>& BagWindow);
	virtual void HandleBag_Implementation(EBagSlot InputBagSlot,
	                                       const TScriptInterface<IInventoryBagWindowInterface>& BagWindow);

	/**
	 * Display the bag window for the given slot.
	 *
	 * Default C++ flow:
	 *   1. Look up via GetBagWindowForSlot(). If missing, lazy-create via GetBagWindowClass().
	 *   2. Call HandleBag() to populate with equipped item data.
	 *   3. Call IInventoryBagWindowInterface::Execute_ShowBagWindow().
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayBag(EBagSlot InputBagSlot);
	virtual void DisplayBag_Implementation(EBagSlot InputBagSlot);

	/**
	 * Hide the bag window for the given slot.
	 * Default calls IInventoryBagWindowInterface::Execute_HideBagWindow on the registered window.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void HideBag(EBagSlot InputBagSlot);
	virtual void HideBag_Implementation(EBagSlot InputBagSlot);

	/**
	 * Hide all registered bag windows.
	 * Default iterates GetRegisteredBagSlots() and calls HideBag() on each.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void HideAllBags();
	virtual void HideAllBags_Implementation();

	/**
	 * Unequip the bag at the given slot: deinit the grid and hide the window.
	 * Default calls DeInitBagWindow then HideBagWindow on the registered window.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void UnequipBag(EBagSlot InputBagSlot);
	virtual void UnequipBag_Implementation(EBagSlot InputBagSlot);

	/**
	 * Toggle the bag window for the given slot (show if hidden, hide if visible).
	 * Default calls IInventoryBagWindowInterface::Execute_ToggleBagWindow on the registered window.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void ToggleBag(EBagSlot InputBagSlot);
	virtual void ToggleBag_Implementation(EBagSlot InputBagSlot);

	//------------------------------------------------------------------------------------------------------------------
	// Inventory grids / loot / merchant / repair

	/**
	 * Refresh all registered bag-window grids.
	 *
	 * Default C++ implementation iterates GetRegisteredBagSlots() and calls
	 * Execute_RefreshBagWindow on each registered window (game-agnostic).
	 * Game side (UHUDWidget) overrides to also call InventoryWindow->RefreshAllInventoryGrids().
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void RefreshAllInventoryGrids();
	virtual void RefreshAllInventoryGrids_Implementation();

	//------------------------------------------------------------------------------------------------------------------
	// Loot window registry

	/**
	 * Return the registered loot window, or an empty interface if none.
	 * Default returns empty; override to expose your stored window.
	 */
	virtual TScriptInterface<IInventoryLootWindowInterface> GetLootWindow() const;

	/**
	 * Register a loot window.
	 * Default is a no-op; override to store in your HUD.
	 */
	virtual void RegisterLootWindow(TScriptInterface<IInventoryLootWindowInterface> LootWindow);

	//------------------------------------------------------------------------------------------------------------------
	// Loot lifecycle

	/**
	 * Returns the widget class to instantiate for the loot window.
	 * Must implement IInventoryLootWindowInterface.
	 * Default loads the plugin's built-in UI_LootWidget.
	 * Override to supply a game-specific draggable window class.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Loot")
	TSubclassOf<UUserWidget> GetLootWindowClass() const;
	virtual TSubclassOf<UUserWidget> GetLootWindowClass_Implementation() const;

	/**
	 * Display the loot window for the given actor.
	 *
	 * Default C++ flow:
	 *   1. Look up via GetLootWindow(). If missing, lazy-create via GetLootWindowClass().
	 *   2. Call IInventoryLootWindowInterface::Execute_InitLootWindow(Actor).
	 *   3. Call IInventoryLootWindowInterface::Execute_ShowLootWindow().
	 *   4. Position the window to the bottom-right of the current mouse cursor.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayLootScreen(class AActor* LootedActor);
	virtual void DisplayLootScreen_Implementation(AActor* LootedActor);

	/**
	 * Hide the loot window and release loot data.
	 * Default calls DeInitLootWindow then HideLootWindow on the registered window.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void HideLootScreen();
	virtual void HideLootScreen_Implementation();

	//------------------------------------------------------------------------------------------------------------------
	// Merchant window registry

	/**
	 * Return the registered merchant window, or an empty interface if none.
	 * Default returns empty; override to expose your stored window.
	 */
	virtual TScriptInterface<IInventoryMerchantWindowInterface> GetMerchantWindow() const;

	/**
	 * Register a merchant window.
	 * Default is a no-op; override to store in your HUD.
	 */
	virtual void RegisterMerchantWindow(TScriptInterface<IInventoryMerchantWindowInterface> MerchantWindow);

	//------------------------------------------------------------------------------------------------------------------
	// Merchant lifecycle

	/**
	 * Returns the widget class to instantiate for the merchant window.
	 * Must implement IInventoryMerchantWindowInterface.
	 * Default returns nullptr; game code overrides this to return its draggable window class.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Merchant")
	TSubclassOf<UUserWidget> GetMerchantWindowClass() const;
	virtual TSubclassOf<UUserWidget> GetMerchantWindowClass_Implementation() const;

	/**
	 * Display the merchant window for the given actor.
	 *
	 * Default C++ flow:
	 *   1. Look up via GetMerchantWindow(). If missing, lazy-create via GetMerchantWindowClass().
	 *   2. Call IInventoryMerchantWindowInterface::Execute_InitMerchantWindow(Actor).
	 *   3. Call IInventoryMerchantWindowInterface::Execute_ShowMerchantWindow().
	 *   4. Position the window to the bottom-right of the current mouse cursor on first creation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayMerchantScreen(AActor* MerchantActor);
	virtual void DisplayMerchantScreen_Implementation(AActor* MerchantActor);

	/**
	 * Hide the merchant window and release merchant data.
	 * Default calls DeInitMerchantWindow then HideMerchantWindow on the registered window.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Merchant")
	void HideMerchantScreen();
	virtual void HideMerchantScreen_Implementation();

	//------------------------------------------------------------------------------------------------------------------
	// Repair window registry

	/**
	 * Return the registered repair window, or an empty interface if none.
	 * Default returns empty; override to expose your stored window.
	 */
	virtual TScriptInterface<IInventoryRepairWindowInterface> GetRepairWindow() const;

	/**
	 * Register a repair window.
	 * Default is a no-op; override to store in your HUD.
	 */
	virtual void RegisterRepairWindow(TScriptInterface<IInventoryRepairWindowInterface> RepairWindow);

	//------------------------------------------------------------------------------------------------------------------
	// Repair lifecycle

	/**
	 * Returns the widget class to instantiate for the repair window.
	 * Must implement IInventoryRepairWindowInterface.
	 * Default returns URepairWidget::StaticClass().
	 * Override to supply a game-specific draggable window class.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Repair")
	TSubclassOf<UUserWidget> GetRepairWindowClass() const;
	virtual TSubclassOf<UUserWidget> GetRepairWindowClass_Implementation() const;

	/**
	 * Display the repair window for the given repairer actor.
	 *
	 * Default C++ flow:
	 *   1. Look up via GetRepairWindow(). If missing, lazy-create via GetRepairWindowClass().
	 *   2. Call IInventoryRepairWindowInterface::Execute_InitRepairWindow(Actor).
	 *   3. Call IInventoryRepairWindowInterface::Execute_ShowRepairWindow().
	 *   4. Position the window to the bottom-right of the current mouse cursor on first creation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayRepairScreen(AActor* RepairerActor);
	virtual void DisplayRepairScreen_Implementation(AActor* RepairerActor);

	/**
	 * Hide the repair window and release repairer data.
	 * Default calls DeInitRepairWindow then HideRepairWindow on the registered window.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void HideRepairScreen();
	virtual void HideRepairScreen_Implementation();

	/**
	 * Notify the repair window that a transaction has completed.
	 * Default calls OnRepairWindowTransactionComplete on the registered window.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void OnRepairTransactionComplete();
	virtual void OnRepairTransactionComplete_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void TryPresentSellItem(EBagSlot OutSlot, int32 ItemID, int32 TopLeft);
	virtual void TryPresentSellItem_Implementation(EBagSlot OutSlot, int32 ItemID, int32 TopLeft) {}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void ResetSellItem();
	virtual void ResetSellItem_Implementation() {}

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayItemDescription(const UInventoryItemBase* Item, float X, float Y);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayItemDescriptionWithDurability(const UInventoryItemBase* Item, float X, float Y,
	                                           float Durability, float MaxDurability);

	//------------------------------------------------------------------------------------------------------------------
	// Book

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
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayBookText(const UInventoryItemBase* Item, float X, float Y);
	virtual void DisplayBookText_Implementation(const UInventoryItemBase* Item, float X, float Y);

	//------------------------------------------------------------------------------------------------------------------
	// Staging / trade / field repair

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	void ForceRefreshStagingAreaPossibilities();

	//------------------------------------------------------------------------------------------------------------------
	// Trade window registry

	/**
	 * Return the registered trade window, or an empty interface if none.
	 * Default returns empty; override to expose your stored window.
	 */
	virtual TScriptInterface<ITradeWindowInterface> GetTradeWindow() const;

	/**
	 * Register the trade window.
	 * Default is a no-op; override to store in your HUD.
	 */
	virtual void RegisterTradeWindow(TScriptInterface<ITradeWindowInterface> TradeWindow);

	/**
	 * Open (init + show) the trade window.
	 *
	 * Default C++ flow:
	 *   1. Cast GetOwningPlayer() to IInventoryPlayerInterface.
	 *   2. Check IsTrading() — bail if not trading.
	 *   3. Call Execute_InitTradeWindow(GetLocalTradeComponent()).
	 *   4. Call Execute_ShowTradeWindow().
	 *
	 * Override in the implementing class only when the default flow is insufficient.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void OpenTradeWindow();
	virtual void OpenTradeWindow_Implementation();

	/**
	 * Close (deinit + hide) the trade window.
	 * Default calls Execute_DeInitTradeWindow then Execute_HideTradeWindow.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void CloseTradeWindow();
	virtual void CloseTradeWindow_Implementation();

	//------------------------------------------------------------------------------------------------------------------
	// Field repair window registry

	/**
	 * Return the registered field repair window, or an empty interface if none.
	 * Default returns empty; override to expose your stored window.
	 */
	virtual TScriptInterface<IFieldRepairWidgetInterface> GetFieldRepairWindow() const;

	/**
	 * Register a field repair window.
	 * Default is a no-op; override to store in your HUD.
	 */
	virtual void RegisterFieldRepairWindow(TScriptInterface<IFieldRepairWidgetInterface> FieldRepairWindow);

	//------------------------------------------------------------------------------------------------------------------
	// Field repair lifecycle

	/**
	 * Returns the widget class to instantiate for the field repair window.
	 * Must implement IFieldRepairWidgetInterface.
	 * Default returns UFieldRepairWidget::StaticClass().
	 * Override to supply a game-specific draggable window class.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|FieldRepair")
	TSubclassOf<UUserWidget> GetFieldRepairWidgetClass() const;
	virtual TSubclassOf<UUserWidget> GetFieldRepairWidgetClass_Implementation() const;

	/**
	 * Display the field repair window for a repair kit in the player's inventory.
	 *
	 * Default C++ flow:
	 *   1. Look up via GetFieldRepairWindow(). If missing, lazy-create via GetFieldRepairWidgetClass().
	 *   2. Call IFieldRepairWidgetInterface::Execute_InitFieldRepairWindow.
	 *   3. Call IFieldRepairWidgetInterface::Execute_ShowFieldRepairWindow.
	 *   4. Position the window at the current mouse cursor on first creation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|FieldRepair")
	void DisplayFieldRepairScreen(int32 RepairKitItemID, EBagSlot BagSlot, int32 TopLeft);
	virtual void DisplayFieldRepairScreen_Implementation(int32 RepairKitItemID, EBagSlot BagSlot, int32 TopLeft);

	/**
	 * Hide the field repair window and release all repair state.
	 * Default calls DeInitFieldRepairWindow then HideFieldRepairWindow on the registered window.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|FieldRepair")
	void HideFieldRepairScreen();
	virtual void HideFieldRepairScreen_Implementation();

	/**
	 * Forward the server-authoritative repair result to the field repair window.
	 * Default calls OnFieldRepairFinished on the registered window.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|FieldRepair")
	void NotifyFieldRepairFinished(EBagSlot RepairBagSlot, int32 RepairTopLeft, float ActualRepairAmount,
	                               float NewTargetDurability, float NewKitDurability);
	virtual void NotifyFieldRepairFinished_Implementation(EBagSlot RepairBagSlot, int32 RepairTopLeft,
	                                                      float ActualRepairAmount, float NewTargetDurability,
	                                                      float NewKitDurability);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|FieldRepair")
	void LockInventorySlot(EBagSlot BagSlot, int32 TopLeft, bool bLocked);
	virtual void LockInventorySlot_Implementation(EBagSlot BagSlot, int32 TopLeft, bool bLocked);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|FieldRepair")
	void LockEquipmentSlot(EEquipmentSlot EquipmentSlot, bool bLocked);

	virtual void LockEquipmentSlot_Implementation(EEquipmentSlot EquipmentSlot, bool bLocked);
};
