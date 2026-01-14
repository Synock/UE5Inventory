#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FieldRepairInterface.generated.h"

class IInventoryItemFieldRepairInterface;
class UInventoryItemFieldRepairInterface;

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UFieldRepairInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for game-specific field repair logic hooks.
 * Implemented by player character to provide:
 * - Forbidden action checks during repair (combat, casting, trading, etc.)
 * - Skill system integration (skill modifiers, skill-ups)
 * - Duration/cost modifications based on skill level
 *
 * This interface keeps field repair logic separate from general inventory management.
 */
class INVENTORYPLUGIN_API IFieldRepairInterface
{
	GENERATED_BODY()

public:
	//------------------------------------------------------------------------------------------------------------------
	// Field Repair -- Component Access
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * Get the field repair component from the implementing actor.
	 * This allows direct access to the component without GetComponentByClass lookups.
	 *
	 * @return Pointer to the field repair component, or nullptr if not available
	 */
	virtual class UFieldRepairComponent* GetFieldRepairComponent() = 0;

	/**
	 * Get the field repair component (const version)
	 *
	 * @return Const pointer to the field repair component, or nullptr if not available
	 */
	virtual const class UFieldRepairComponent* GetFieldRepairComponent() const = 0;

	//------------------------------------------------------------------------------------------------------------------
	// Field Repair -- Server
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * Begin a field repair operation using a repair kit from inventory
	 * @param RepairKitItemID - The ID of the repair kit item
	 * @param RepairKitBagSlot - The bag slot containing the repair kit
	 * @param RepairKitTopLeft - The top-left index of the repair kit in the bag
	 * @param TargetEquipmentSlot - The equipment slot to repair
	 */
	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|FieldRepair")
	virtual void Server_BeginFieldRepair(int32 RepairKitItemID, EBagSlot RepairKitBagSlot,
	int32 RepairKitTopLeft, EEquipmentSlot TargetEquipmentSlot, EBagSlot TargetBagSlot,
	int32 TargetTopLeft, int32 TargetItemID) = 0;

	/**
	 * Cancel an in-progress field repair
	 */
	//UFUNCTION(Server, Reliable, Category = "Inventory|FieldRepair")
	virtual void Server_CancelFieldRepair() = 0;

	//------------------------------------------------------------------------------------------------------------------
	// Field Repair -- Interrupt/Validation Checks
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * Check if player is in a state that forbids field repair.
	 * Called periodically during repair to check for interruptions.
	 *
	 * @return true if repair should be interrupted (combat, casting, trading, looting, dead, etc.)
	 */
	virtual bool IsForbiddenActionActiveWhileRepairing() const { return false; }

	/**
	 * Convenience wrapper for Server_BeginFieldRepair (callable from anywhere)
	 * Implemented by concrete classes (default provided in .cpp)
	 */
	virtual void BeginFieldRepair(int32 RepairKitItemID, EBagSlot RepairKitBagSlot,
	int32 RepairKitTopLeft, EEquipmentSlot TargetEquipmentSlot, EBagSlot TargetBagSlot,
	int32 TargetTopLeft, int32 TargetItemID);

	/**
	 * Convenience wrapper for Server_CancelFieldRepair (callable from anywhere)
	 * Implemented by concrete classes (default provided in .cpp)
	 */
	virtual void CancelFieldRepair();

	//------------------------------------------------------------------------------------------------------------------
	// Field Repair -- RNG System Hooks
	//------------------------------------------------------------------------------------------------------------------

	virtual bool RollRepairSuccess(const IInventoryItemFieldRepairInterface* RepairKitUsed) const { return true; }

	/**
	 * Get skill-based modifier for repair amount.
	 * Higher skill = more durability restored per repair action.
	 *
	 * @return Multiplier for repair amount (1.0 = no bonus, 1.5 = +50% repair, etc.)
	 */
	virtual float GetFieldRepairSkillModifier(const IInventoryItemFieldRepairInterface* RepairKitUsed) const { return 1.0f; }
};

