#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "UObject/Interface.h"
#include "Blueprint/UserWidget.h"
#include "UI/InventoryBagWindowInterface.h"
#include "UI/InventoryLootWindowInterface.h"
#include "UI/InventoryMerchantWindowInterface.h"
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

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	void SetInventoryDisplay(bool State);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	void ToggleInventoryDisplay();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	void ForceRefreshInventory();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	void SetKeyringDisplay(bool State);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	void ToggleKeyringDisplay();

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

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void RefreshAllInventoryGrids();

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
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory")
	void HideMerchantScreen();
	virtual void HideMerchantScreen_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayRepairScreen(AActor* RepairerActor);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void HideRepairScreen();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void OnRepairTransactionComplete();

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

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void OpenTradeWindow();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void CloseTradeWindow();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory|FieldRepair")
	void DisplayFieldRepairScreen(int32 RepairKitItemID, EBagSlot BagSlot, int32 TopLeft);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory|FieldRepair")
	void HideFieldRepairScreen();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory|FieldRepair")
	void NotifyFieldRepairFinished(EBagSlot RepairBagSlot, int32 RepairTopLeft, float ActualRepairAmount,
	                                float NewTargetDurability, float NewKitDurability);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|FieldRepair")
	void LockInventorySlot(EBagSlot BagSlot, int32 TopLeft, bool bLocked);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|FieldRepair")
	void LockEquipmentSlot(EEquipmentSlot EquipmentSlot, bool bLocked);

	virtual void LockEquipmentSlot_Implementation(EEquipmentSlot EquipmentSlot, bool bLocked);
};
