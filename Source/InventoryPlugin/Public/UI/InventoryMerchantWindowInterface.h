#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryMerchantWindowInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UInventoryMerchantWindowInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract for any widget that acts as a merchant window.
 *
 * Implement on UMerchantWindow (game side).
 * IInventoryHUDInterface's C++ defaults dispatch all merchant lifecycle calls
 * through this interface, keeping the HUD logic game-agnostic.
 *
 * The implementing widget must host a UMerchantSellWidget (or equivalent)
 * as a BindWidget member and delegate lifecycle calls to it.
 */
class INVENTORYPLUGIN_API IInventoryMerchantWindowInterface
{
	GENERATED_BODY()

public:
	/**
	 * Populate the merchant window with data from the given merchant actor.
	 * Called by IInventoryHUDInterface::DisplayMerchantScreen_Implementation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Merchant")
	void InitMerchantWindow(AActor* MerchantActor);
	virtual void InitMerchantWindow_Implementation(AActor* MerchantActor) {}

	/**
	 * Release merchant data and unbind delegates.
	 * Called by IInventoryHUDInterface::HideMerchantScreen_Implementation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Merchant")
	void DeInitMerchantWindow();
	virtual void DeInitMerchantWindow_Implementation() {}

	/** Make the merchant window visible. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Merchant")
	void ShowMerchantWindow();
	virtual void ShowMerchantWindow_Implementation() {}

	/** Hide the merchant window. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Merchant")
	void HideMerchantWindow();
	virtual void HideMerchantWindow_Implementation() {}

	/** Refresh the merchant item list (e.g. after a transaction). */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|Merchant")
	void RefreshMerchantWindow();
	virtual void RefreshMerchantWindow_Implementation() {}
};

