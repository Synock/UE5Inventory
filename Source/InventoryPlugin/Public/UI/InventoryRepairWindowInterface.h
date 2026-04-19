#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryRepairWindowInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UInventoryRepairWindowInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract for any widget that acts as a repair window.
 *
 * Implement on URepairWindow (game side).
 * IInventoryHUDInterface's C++ defaults dispatch all repair lifecycle calls
 * through this interface, keeping the HUD logic game-agnostic.
 *
 * The implementing widget must host a URepairWidget (or equivalent)
 * as a BindWidget member and delegate lifecycle calls to it.
 */
class INVENTORYPLUGIN_API IInventoryRepairWindowInterface
{
	GENERATED_BODY()

public:
	/**
	 * Populate the repair window with data from the given repairer actor.
	 * Called by IInventoryHUDInterface::DisplayRepairScreen_Implementation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Repair")
	void InitRepairWindow(AActor* RepairerActor);
	virtual void InitRepairWindow_Implementation(AActor* RepairerActor) {}

	/**
	 * Release repairer data and unbind delegates.
	 * Called by IInventoryHUDInterface::HideRepairScreen_Implementation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Repair")
	void DeInitRepairWindow();
	virtual void DeInitRepairWindow_Implementation() {}

	/** Make the repair window visible. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Repair")
	void ShowRepairWindow();
	virtual void ShowRepairWindow_Implementation() {}

	/** Hide the repair window. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Repair")
	void HideRepairWindow();
	virtual void HideRepairWindow_Implementation() {}

	/** Refresh the repair item list (e.g. after a transaction). */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Repair")
	void RefreshRepairWindow();
	virtual void RefreshRepairWindow_Implementation() {}

	/**
	 * Notify the window that a repair transaction has completed.
	 * Called by IInventoryHUDInterface::OnRepairTransactionComplete_Implementation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Repair")
	void OnRepairWindowTransactionComplete();
	virtual void OnRepairWindowTransactionComplete_Implementation() {}
};

