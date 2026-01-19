#pragma once

#include "CoreMinimal.h"
#include "InventoryItemEquipable.h"
#include "Items/Interfaces/InventoryItemFieldRepairInterface.h"
#include "InventoryItemFieldRepair.generated.h"

/**
 * Field repair items are consumable tools that allow players to repair equipment in the field.
 * They have limited charges (durability, but cannot be repaired themselves), work within specific durability ranges,
 * and require time to complete repairs.
 *
 * This is the standard implementation of IInventoryItemFieldRepairInterface.
 * For custom repair logic, either:
 * - Subclass this in Blueprint and override interface methods
 * - Create a new class implementing IInventoryItemFieldRepairInterface
 *
 * Example: "Rusty Repair Kit" (10 charges, repairs 5-20%, works on 33-75% items, 3s repair time)
 */
UCLASS()
class INVENTORYPLUGIN_API UInventoryItemFieldRepair : public UInventoryItemEquipable, public IInventoryItemFieldRepairInterface
{
	GENERATED_BODY()

public:
	// ============================================================================
	// Field Repair Properties
	// ============================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|FieldRepair")
	int32 ChargeConsumption = 1;

	/** Minimum percentage of repair per use (e.g., 0.05 = 5%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|FieldRepair", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinRepairPercentage = 0.05f;

	/** Maximum percentage of repair per use (e.g., 0.20 = 20%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|FieldRepair", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxRepairPercentage = 0.20f;

	// ============================================================================
	// Durability Thresholds
	// ============================================================================

	/** Minimum durability threshold to use this repair item (e.g., 0.33 = 33%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|FieldRepair|Thresholds",
		meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "Item must be at or above this durability to be repairable"))
	float MinDurabilityThreshold = 0.33f;

	/** Maximum durability threshold - repairs stop at this point (e.g., 0.75 = 75%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|FieldRepair|Thresholds",
		meta = (ClampMin = "0.0", ClampMax = "1.0", ToolTip = "Repairs cannot exceed this durability percentage"))
	float MaxDurabilityThreshold = 0.75f;

	// ============================================================================
	// Repair Duration
	// ============================================================================

	/** Time in seconds to complete a repair. If 0, repair is instant and not interruptible. Otherwise, always interruptible. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|FieldRepair|Duration",
		meta = (ClampMin = "0.0", ToolTip = "Repair duration in seconds. 0 = instant/not interruptible, >0 = interruptible"))
	float RepairDuration = 3.0f;

	// ============================================================================
	// Restrictions
	// ============================================================================

	/** If true, can only repair weapons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|FieldRepair|Restrictions")
	bool WeaponsOnly = false;

	/** If true, can only repair armor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|FieldRepair|Restrictions")
	bool ArmorOnly = false;

	/** If true, can only repair shields */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|FieldRepair|Restrictions")
	bool ShieldsOnly = false;

	/** Equipment slot bitmask - if non-zero, can only repair items for specific slots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Meta = (Bitmask, BitmaskEnum = "/Script/InventoryPlugin.EEquipmentSlot"),
		Category = "Inventory|FieldRepair|Restrictions")
	int32 AllowedEquipmentSlotBitMask = 0;

	// ============================================================================
	// Visual/Audio Feedback
	// ============================================================================

	/** Sound to play when starting repair */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|FieldRepair|Feedback")
	USoundBase* RepairStartSound = nullptr;

	/** Sound to play when repair completes successfully */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|FieldRepair|Feedback")
	USoundBase* RepairCompleteSound = nullptr;

	/** Sound to play when repair is interrupted or fails */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|FieldRepair|Feedback")
	USoundBase* RepairFailSound = nullptr;

	/** Particle effect to spawn during repair (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|FieldRepair|Feedback")
	UParticleSystem* RepairParticleEffect = nullptr;

	// ============================================================================
	// IInventoryItemFieldRepairInterface Implementation
	// ============================================================================

	virtual bool CanRepairItem(const UInventoryItemBase* TargetItem, float TargetDurability,
		float TargetMaxDurability, float KitDurability, FText& OutReason) const override;

	virtual float CalculateRepairAmount(float TargetDurability, float TargetMaxDurability) const override;

	virtual float GetRepairDuration() const override { return RepairDuration; }

	virtual bool IsInterruptible() const override { return RepairDuration > 0.0f; }

	virtual int32 GetChargeConsumptionAmount() const override { return ChargeConsumption; }

	virtual int32 GetMaxChargeAmount() const override { return TotalDurability; }

	virtual USoundBase* GetRepairStartSound() const override { return RepairStartSound; }

	virtual USoundBase* GetRepairCompleteSound() const override { return RepairCompleteSound; }

	virtual USoundBase* GetRepairFailSound() const override { return RepairFailSound; }

	virtual UParticleSystem* GetRepairParticleEffect() const override { return RepairParticleEffect; }

	virtual FText GetRepairKitDescription() const override;

	virtual float GetMinDurabilityThreshold() const override { return MinDurabilityThreshold; }

	virtual float GetMaxDurabilityThreshold() const override { return MaxDurabilityThreshold; }
};

