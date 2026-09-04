#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryItemActivatableInterface.h"
#include "InventoryItemDrinkInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UInventoryItemDrinkInterface : public UInventoryItemActivatableInterface
{
	GENERATED_BODY()
};

/**
 * Interface for drink items that restore thirst.
 * Inherits from IInventoryItemActivatableInterface to provide activation behavior.
 *
 * Drink items:
 * - Restore thirst when consumed
 * - Are consumed (destroyed) after use
 * - May have drinking duration/animation
 *
 * Examples: Water, ale, wine, potions, elixirs
 */
class INVENTORYPLUGIN_API IInventoryItemDrinkInterface : public IInventoryItemActivatableInterface
{
	GENERATED_BODY()

public:
	/**
	 * Get the amount of thirst restored by this drink.
	 * @return Thirst points restored (0-100 typical range)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Drink")
	float GetThirstRestoration() const;
	virtual float GetThirstRestoration_Implementation() const
	{
		return 0.0f;
	}

	/**
	 * Get the duration in seconds for drinking this beverage.
	 * @return Time to consume (0 = instant)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Drink")
	float GetConsumptionDuration() const;
	virtual float GetConsumptionDuration_Implementation() const
	{
		return 0.0f; // Instant by default
	}

	/**
	 * Check if this drink is alcoholic (may have side effects).
	 * @return True if alcoholic
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Drink")
	bool IsAlcoholic() const;
	virtual bool IsAlcoholic_Implementation() const
	{
		return false;
	}

	/**
	 * Get the alcohol strength if this is an alcoholic drink.
	 * @return Alcohol level (0 = none, 10 = strong spirits)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory|Drink")
	int32 GetAlcoholStrength() const;
	virtual int32 GetAlcoholStrength_Implementation() const
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------------------------
	// IInventoryItemActivatableInterface Overrides
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * Drinks are always consumed when used.
	 */
	virtual bool IsConsumedOnUse_Implementation() const override
	{
		return true;
	}
};
