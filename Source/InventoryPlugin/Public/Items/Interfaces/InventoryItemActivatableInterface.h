#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryItemActivatableInterface.generated.h"

class AActor;
class UWorld;


// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UInventoryItemActivatableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Base interface for items that can be "used" or "activated".
 *
 * This interface separates activatable behavior from item data, allowing:
 * - Non-equipable items to be activatable (potions, food, keys)
 * - Equipable items to be activatable (clickable gear)
 * - Complex validation and feedback for action attempts
 *
 * Implement this interface on UInventoryItemBase-derived classes to make them usable.
 */
class INVENTORYPLUGIN_API IInventoryItemActivatableInterface
{
	GENERATED_BODY()

public:
	/**
	 * Check if this item can be activated right now.
	 * Use this for validation before attempting to use the item.
	 * @return True if item can be activated, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Activatable")
	bool CanActivate() const;
	virtual bool CanActivate_Implementation() const
	{
		return true;
	}

	/**
	 * Check if this item must be equipped before it can be activated.
	 * @return True if item must be in an equipment slot to use
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Activatable")
	bool MustBeEquippedToActivate() const;
	virtual bool MustBeEquippedToActivate_Implementation() const
	{
		return false;
	}

	/**
	 * Check if this item is consumed (destroyed/decremented) when activated.
	 * @return True if item is consumed on use
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Activatable")
	bool IsConsumedOnUse() const;
	virtual bool IsConsumedOnUse_Implementation() const
	{
		return false;
	}
};
