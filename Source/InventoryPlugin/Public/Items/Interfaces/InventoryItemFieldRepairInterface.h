#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryItemFieldRepairInterface.generated.h"

class UInventoryItemBase;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInventoryItemFieldRepairInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for field repair items.
 * Allows custom implementations with various repair logic and restrictions.
 *
 * Pure virtual interface - all methods must be overridden in C++.
 * Implementers must provide:
 * - Validation logic (CanRepairItem)
 * - Repair amount calculation
 * - Duration configuration
 * - Visual/audio feedback hooks
 */
class INVENTORYPLUGIN_API IInventoryItemFieldRepairInterface
{
	GENERATED_BODY()

public:
	// ============================================================================
	// Core Repair Functionality
	// ============================================================================

	/**
	 * Check if this repair item can repair the target item
	 * @param TargetItem - The item to check for repair eligibility
	 * @param TargetDurability - Current durability of the target item
	 * @param TargetMaxDurability - Maximum durability of the target item
	 * @param KitDurability - Current durability (charges) of this repair kit
	 * @param OutReason - If false, contains the human-readable reason why repair is not possible
	 * @return True if the target item can be repaired with this kit
	 */
	virtual bool CanRepairItem(const UInventoryItemBase* TargetItem, float TargetDurability,
		float TargetMaxDurability, float KitDurability, FText& OutReason) const = 0;

	/**
	 * Calculate how much durability will be restored to the target item
	 * @param TargetDurability - Current durability of target
	 * @param TargetMaxDurability - Max durability of target
	 * @return Amount of durability to add (should respect thresholds)
	 */
	virtual float CalculateRepairAmount(float TargetDurability, float TargetMaxDurability) const = 0;

	/**
	 * Get the duration in seconds for this repair operation
	 * @return Duration in seconds (0 = instant, >0 = timed)
	 */
	virtual float GetRepairDuration() const = 0;

	/**
	 * Check if the repair process can be interrupted
	 * @return True if repair can be cancelled mid-process
	 */
	virtual bool IsInterruptible() const = 0;

	// ============================================================================
	// Charge Management
	// ============================================================================

	/**
	 * Get the amount of durability consumed per repair use
	 * @return Durability to subtract from kit per repair
	 */
	virtual float GetChargeConsumptionAmount() const = 0;

	// ============================================================================
	// Visual & Audio Feedback
	// ============================================================================

	/**
	 * Get the sound to play when starting a repair
	 * @return Sound asset, or nullptr for silent start
	 */
	virtual USoundBase* GetRepairStartSound() const = 0;

	/**
	 * Get the sound to play when repair completes successfully
	 * @return Sound asset, or nullptr for silent completion
	 */
	virtual USoundBase* GetRepairCompleteSound() const = 0;

	/**
	 * Get the sound to play when repair is interrupted or fails
	 * @return Sound asset, or nullptr for silent failure
	 */
	virtual USoundBase* GetRepairFailSound() const = 0;

	/**
	 * Get the particle effect to spawn during repair
	 * @return Particle system, or nullptr for no effect
	 */
	virtual UParticleSystem* GetRepairParticleEffect() const = 0;

	// ============================================================================
	// Display Information
	// ============================================================================

	/**
	 * Get a short description of this repair kit's capabilities
	 * Used for tooltips and UI display
	 * Example: "Repairs weapons 10-25%, works on items 25-80% durability"
	 * @return Human-readable description
	 */
	virtual FText GetRepairKitDescription() const = 0;

	/**
	 * Get the minimum durability threshold for repairs (as percentage 0.0-1.0)
	 * Items below this threshold cannot be repaired
	 * @return Minimum threshold (e.g., 0.33 = 33%)
	 */
	virtual float GetMinDurabilityThreshold() const = 0;

	/**
	 * Get the maximum durability threshold for repairs (as percentage 0.0-1.0)
	 * Repairs cannot exceed this threshold
	 * @return Maximum threshold (e.g., 0.75 = 75%)
	 */
	virtual float GetMaxDurabilityThreshold() const = 0;
};

