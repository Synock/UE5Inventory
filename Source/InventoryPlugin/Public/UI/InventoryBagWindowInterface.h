#pragma once

#include "CoreMinimal.h"
#include "Definitions.h"
#include "UObject/Interface.h"
#include "InventoryBagWindowInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UInventoryBagWindowInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract for any widget that acts as a bag window.
 *
 * Implement on UBagWidget or a game-side. IInventoryHUDInterface's C++ defaults dispatch all bag
 * lifecycle calls through this interface, keeping the HUD logic game-agnostic.
 *
 * The implementing widget must bind:
 *   - "InventoryGrid"  (UInventoryGridWidget, mandatory)
 * Optionally:
 *   - "BagNameText"    (UTextBlock, optional) — title/label shown in the window
 */
class INVENTORYPLUGIN_API IInventoryBagWindowInterface
{
	GENERATED_BODY()

public:
	/**
	 * Populate bag data fields and trigger grid setup.
	 * Called by IInventoryHUDInterface::HandleBag with data from the equipped bag item.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Bag")
	void InitBagData(const FString& InBagName, int32 InBagWidth, int32 InBagHeight,
	                 EItemSize InBagSize, EBagSlot InBagSlot);
	virtual void InitBagData_Implementation(const FString& InBagName, int32 InBagWidth,
	                                         int32 InBagHeight, EItemSize InBagSize,
	                                         EBagSlot InBagSlot) {}

	/**
	 * Configure the grid widget after bag data is set.
	 * C++ default is a no-op; implementations call InventoryGrid->InitData().
	 * Blueprint may override for open animations or extra setup.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Bag")
	void InitUI();
	virtual void InitUI_Implementation() {}

	/** Refresh the grid display to reflect current inventory state. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Bag")
	void RefreshBagWindow();
	virtual void RefreshBagWindow_Implementation() {}

	/** Make the bag window visible. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Bag")
	void ShowBagWindow();
	virtual void ShowBagWindow_Implementation() {}

	/** Hide the bag window. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Bag")
	void HideBagWindow();
	virtual void HideBagWindow_Implementation() {}

	/** Toggle the bag window between visible and hidden. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Bag")
	void ToggleBagWindow();
	virtual void ToggleBagWindow_Implementation() {}

	/**
	 * Release grid data. Called when the bag item is unequipped.
	 * C++ default is a no-op; implementations call InventoryGrid->DeInitData().
	 * Blueprint may override for close animations.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Bag")
	void DeInitBagWindow();
	virtual void DeInitBagWindow_Implementation() {}

	/**
	 * Lock or unlock a specific item slot in this bag (used by field repair, etc.).
	 * @param TopLeft The top-left grid index of the item to lock.
	 * @param bLocked True to prevent interaction, false to restore it.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Bag")
	void LockBagItemSlot(int32 TopLeft, bool bLocked);
	virtual void LockBagItemSlot_Implementation(int32 TopLeft, bool bLocked) {}
};

