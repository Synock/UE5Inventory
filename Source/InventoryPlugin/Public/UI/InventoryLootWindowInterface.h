#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryLootWindowInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UInventoryLootWindowInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract for any widget that acts as a loot window.
 *
 * Implement on ULootScreenWidget.
 * IInventoryHUDInterface's C++ defaults dispatch all loot lifecycle calls
 * through this interface, keeping the HUD logic game-agnostic.
 *
 * The implementing widget must bind:
 *   - "LootContent"  (ULootScreenWidget, mandatory if using the game-side wrapper)
 * OR extend ULootScreenWidget directly.
 */
class INVENTORYPLUGIN_API IInventoryLootWindowInterface
{
	GENERATED_BODY()

public:
	/**
	 * Populate the loot window with data from the given actor and show it.
	 * Called by IInventoryHUDInterface::DisplayLootScreen_Implementation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Loot")
	void InitLootWindow(AActor* LootedActor);
	virtual void InitLootWindow_Implementation(AActor* LootedActor) {}

	/**
	 * Release loot data. Called by IInventoryHUDInterface::HideLootScreen_Implementation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Loot")
	void DeInitLootWindow();
	virtual void DeInitLootWindow_Implementation() {}

	/** Make the loot window visible. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Loot")
	void ShowLootWindow();
	virtual void ShowLootWindow_Implementation() {}

	/** Hide the loot window. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Loot")
	void HideLootWindow();
	virtual void HideLootWindow_Implementation() {}

	/** Refresh the loot grid (e.g. after an item is taken). */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Loot")
	void RefreshLootWindow();
	virtual void RefreshLootWindow_Implementation() {}
};

