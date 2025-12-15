#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CoinValue.h"
#include "RepairComponent.generated.h"

/**
 * @brief Component for NPCs that can repair equipment
 *
 * Handles repair cost calculations and provides repair services to players.
 * Similar pattern to MerchantComponent but focused on durability restoration.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API URepairComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URepairComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/**
	 * Base cost multiplier for repairs.
	 * Formula: RepairCost = (ItemBaseValue * DurabilityLost / MaxDurability) * BaseCostMultiplier * RepairCostMultiplier
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Repair")
	float BaseCostMultiplier = 0.5f;  // Repair costs 50% of proportional item value by default

	/**
	 * Repairer-specific cost multiplier (set per NPC)
	 * Master smiths might have 0.8, apprentices might have 1.2
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Repair")
	float RepairCostMultiplier = 1.0f;

public:
	/**
	 * @brief Calculate the cost to repair an item from current to max durability
	 *
	 * @param ItemBaseValue The base value of the item in copper pieces
	 * @param CurrentDurability Current durability value
	 * @param MaxDurability Maximum durability value
	 * @return Cost to repair in coin value
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Repair")
	FCoinValue CalculateSingleItemRepairCost(float ItemBaseValue, float CurrentDurability, float MaxDurability) const;

	/**
	 * @brief Get the base cost multiplier
	 *
	 * @return Base cost multiplier for repairs
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Repair")
	float GetBaseCostMultiplier() const { return BaseCostMultiplier; }

	/**
	 * @brief Get the repair cost multiplier (NPC-specific)
	 *
	 * @return Repair cost multiplier
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Repair")
	float GetRepairCostMultiplier() const { return RepairCostMultiplier; }

	/**
	 * @brief Set the repair cost multiplier
	 *
	 * @param NewMultiplier New repair cost multiplier value
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Repair")
	void SetRepairCostMultiplier(float NewMultiplier);
};

