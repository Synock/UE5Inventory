#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryItemActivatableInterface.h"
#include "InventoryItemFoodInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UInventoryItemFoodInterface : public UInventoryItemActivatableInterface
{
	GENERATED_BODY()
};

/**
 * Interface for food items that restore hunger.
 * Inherits from IInventoryItemActivatableInterface to provide activation behavior.
 *
 * Food items:
 * - Restore hunger when consumed
 * - Are consumed (destroyed) after use
 * - May have eating duration/animation
 *
 * Examples: Bread, meat, fruit, cooked meals
 */
class INVENTORYPLUGIN_API IInventoryItemFoodInterface : public IInventoryItemActivatableInterface
{
	GENERATED_BODY()

public:
	/**
	 * Get the amount of hunger restored by this food.
	 * @return Hunger points restored (0-100 typical range)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Food")
	float GetHungerRestoration() const;
	virtual float GetHungerRestoration_Implementation() const
	{
		return 0.0f;
	}

	/**
	 * Get the duration in seconds for eating this food.
	 * @return Time to consume (0 = instant)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Food")
	float GetConsumptionDuration() const;
	virtual float GetConsumptionDuration_Implementation() const
	{
		return 0.0f; // Instant by default
	}

	//------------------------------------------------------------------------------------------------------------------
	// IInventoryItemActivatableInterface Overrides
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * Food is always consumed when eaten.
	 */
	virtual bool IsConsumedOnUse_Implementation() const override
	{
		return true;
	}
};
