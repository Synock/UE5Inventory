#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryItemDescriptionWidgetInterface.generated.h"

class UInventoryItemBase;

UINTERFACE(MinimalAPI, BlueprintType)
class UInventoryItemDescriptionWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for any UUserWidget subclass that can display an item description tooltip.
 *
 * Implement this on custom description widgets to make them compatible with
 * IInventoryHUDInterface::DisplayItemDescription's default C++ flow without requiring
 * inheritance from UItemDescriptionWidget.
 */
class INVENTORYPLUGIN_API IInventoryItemDescriptionWidgetInterface
{
	GENERATED_BODY()

public:
	/**
	 * Populate the widget with the given item (no durability — uses item defaults).
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|ItemDescription")
	void InitDescription(const UInventoryItemBase* Item);
	virtual void InitDescription_Implementation(const UInventoryItemBase* Item) {}

	/**
	 * Populate the widget with the given item and explicit durability values.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|ItemDescription")
	void InitDescriptionWithDurability(const UInventoryItemBase* Item, float Durability, float MaxDurability);
	virtual void InitDescriptionWithDurability_Implementation(const UInventoryItemBase* Item, float Durability, float MaxDurability) {}
};

