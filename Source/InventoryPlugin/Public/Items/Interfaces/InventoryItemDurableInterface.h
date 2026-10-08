#pragma once

#include "UObject/Interface.h"
#include "InventoryItemDurableInterface.generated.h"

/**
 * Interface for items that have durability (wear and tear).
 * Can be applied to any item type - equipable, tools, consumables with charges, etc.
 */
UINTERFACE(MinimalAPI)
class UInventoryItemDurableInterface : public UInterface
{
	GENERATED_BODY()
};

class INVENTORYPLUGIN_API IInventoryItemDurableInterface
{
	GENERATED_BODY()

public:
	/**
	 * Get the maximum durability of this item when fully repaired/new.
	 * @return Maximum durability value
	 */
	virtual float GetTotalDurability() const = 0;

	/**
	 * Get the durability modifier for this item.
	 * Affects how quickly durability degrades during use.
	 * @return Modifier value (1.0f = normal, >1.0f = degrades slower, <1.0f = degrades faster)
	 */
	virtual float GetDurabilityModifier() const = 0;
};
