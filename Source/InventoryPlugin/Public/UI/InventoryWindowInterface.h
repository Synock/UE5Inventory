#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryWindowInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UInventoryWindowInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract for the main inventory window widget.
 *
 * Implement on UInventoryWindow (game side).
 * IInventoryHUDInterface's C++ defaults dispatch all inventory window
 * lifecycle calls through this interface, keeping HUD logic game-agnostic.
 *
 * The implementing widget must expose:
 *   - Equipment slot display  (via RefreshInventoryEquipments)
 *   - Pocket/bag grid display (via RefreshInventoryGrids)
 */
class INVENTORYPLUGIN_API IInventoryWindowInterface
{
	GENERATED_BODY()

public:
	/** Make the inventory window visible. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Window")
	void ShowInventoryWindow();
	virtual void ShowInventoryWindow_Implementation() {}

	/** Hide the inventory window. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Window")
	void HideInventoryWindow();
	virtual void HideInventoryWindow_Implementation() {}

	/**
	 * Refresh all equipment slot displays.
	 * Called before RefreshInventoryGrids when the window is shown.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Window")
	void RefreshInventoryEquipments();
	virtual void RefreshInventoryEquipments_Implementation() {}

	/**
	 * Refresh all pocket/bag grid displays.
	 * Called after RefreshInventoryEquipments when the window is shown.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Window")
	void RefreshInventoryGrids();
	virtual void RefreshInventoryGrids_Implementation() {}

	/** Returns true if the window is currently visible. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Window")
	bool IsInventoryWindowVisible() const;
	virtual bool IsInventoryWindowVisible_Implementation() const { return false; }
};

