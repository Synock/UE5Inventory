#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CoinValue.h"
#include "Items/InventoryItemEquipable.h"
#include "RepairInterface.generated.h"

class URepairComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class URepairInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * @class IRepairInterface
 *
 * Interface for NPCs that can repair player equipment and inventory items.
 * Similar to merchant system but focused on durability restoration.
 */
class INVENTORYPLUGIN_API IRepairInterface
{
	GENERATED_BODY()

public:
	/**
	 * @brief Retrieves the world context associated with the repairer.
	 *
	 * @return A pointer to the UWorld object representing the world context, if available, nullptr otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Repair")
	virtual UWorld* GetRepairerWorldContext() const = 0;

	/**
	 * @brief Retrieves the name of the repairer NPC.
	 *
	 * @return FString The name of the repairer.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Repair")
	virtual FString GetRepairerName() const = 0;

	/**
	 * @brief Retrieves the repair cost multiplier.
	 *
	 * This determines how expensive repairs are at this NPC.
	 * 1.0 = normal cost, 1.5 = 50% more expensive, 0.8 = 20% cheaper
	 *
	 * @return The repair cost multiplier as a floating point value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Repair")
	virtual float GetRepairCostMultiplier() const = 0;

	/**
	 * @brief Get the repair component associated with this repairer.
	 *
	 * @return The repair component, or nullptr if not found.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Repair")
	virtual URepairComponent* GetRepairComponent() = 0;

	/**
	 * @brief Retrieves the const pointer to the repair component.
	 *
	 * @return The const pointer to the repair component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Repair")
	virtual URepairComponent* GetRepairComponentConst() const = 0;

	/**
	 * @brief Calculate repair cost for a specific item.
	 *
	 * @param ItemID The ID of the item to repair
	 * @param CurrentDurability Current durability value
	 * @param MaxDurability Maximum durability value
	 * @return The cost to fully repair the item
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Repair")
	virtual FCoinValue CalculateRepairCost(int32 ItemID, float CurrentDurability, float MaxDurability) const;

	/**
	 * @brief Calculate total repair cost for all equipped items.
	 *
	 * @param Equipment Array of equipped items with their durability
	 * @param EquipmentDurability
	 * @return Total cost to repair all equipment
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Repair")
	virtual FCoinValue CalculateRepairAllCost(TArray<UInventoryItemEquipable*>& Equipment,
	                                         const TArray<float>& EquipmentDurability) const;
};


