// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "UObject/Interface.h"
#include "InventoryHUDInterface.generated.h"

enum class EBagSlot : uint8;
// This class does not need to be modified.
UINTERFACE()
class UInventoryHUDInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYPLUGIN_API IInventoryHUDInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	
	//------------------------------------------------------------------------------------------------------------------
	// Inventory
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	void SetInventoryDisplay(bool State);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	void ToggleInventoryDisplay();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	void ForceRefreshInventory();

	UFUNCTION(Category = "Inventory")
	virtual void HandleBag(EBagSlot InputBagSlot, class UBagWidget* Widget) = 0;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayBag(EBagSlot InputBagSlot);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void HideBag(EBagSlot InputBagSlot);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void UnequipBag(EBagSlot InputBagSlot);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void ToggleBag(EBagSlot InputBagSlot);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void RefreshAllInventoryGrids();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayLootScreen(class AActor* LootedActor);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void HideLootScreen();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayMerchantScreen(AActor* MerchantActor);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void HideMerchantScreen();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void TryPresentSellItem(EBagSlot OutSlot, int32 ItemID, int32 TopLeft);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void ResetSellItem();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory")
	void DisplayItemDescription(const FInventoryItem& Item, float X, float Y);
};
