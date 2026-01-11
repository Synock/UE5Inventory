#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FieldRepairWidgetInterface.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;
class UInventoryItemEquipable;

/**
 * Interface for field repair widgets that share common repair logic.
 * Allows plugin to handle core repair flow (progress tracking, UI updates, timer management)
 * while game-specific implementations can override for multiplayer, skill checks, etc.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UFieldRepairWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

class INVENTORYPLUGIN_API IFieldRepairWidgetInterface
{
	GENERATED_BODY()

public:
	// ============================================================================
	// Core Repair Flow (Implemented in Plugin)
	// ============================================================================

	/**
	 * Update all progress bars and status text
	 * Common implementation in plugin
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FieldRepair|UI")
	void UpdateUI();
	virtual void UpdateUI_Implementation();

	/**
	 * Update the item durability bar
	 * Common implementation in plugin
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FieldRepair|UI")
	void UpdateItemDurabilityBar();
	virtual void UpdateItemDurabilityBar_Implementation();

	/**
	 * Update the repair kit charges bar
	 * Common implementation in plugin
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FieldRepair|UI")
	void UpdateRepairKitChargesBar();
	virtual void UpdateRepairKitChargesBar_Implementation();

	/**
	 * Update the repair progress bar
	 * Common implementation in plugin
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FieldRepair|UI")
	void UpdateRepairProgressBar();
	virtual void UpdateRepairProgressBar_Implementation();

	/**
	 * Update status text with a message
	 * Common implementation in plugin
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FieldRepair|UI")
	void UpdateStatusText(const FText& Message);
	virtual void UpdateStatusText_Implementation(const FText& Message);

	/**
	 * Update the repair kit background image
	 * Common implementation in plugin
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FieldRepair|UI")
	void UpdateRepairKitBackground();
	virtual void UpdateRepairKitBackground_Implementation();

	/**
	 * Update the repair process timer (called by timer)
	 * Common implementation in plugin
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FieldRepair")
	void TickRepair();
	virtual void TickRepair_Implementation();

	/**
	 * Validate the current state and update UI accordingly
	 * Common implementation in plugin
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FieldRepair")
	void ValidateState();
	virtual void ValidateState_Implementation();

	// ============================================================================
	// Accessors for Common State (Must be implemented by derived classes)
	// ============================================================================

	/** Get the item durability progress bar widget */
	virtual UProgressBar* GetItemDurabilityBar() const = 0;

	/** Get the repair kit charges progress bar widget */
	virtual UProgressBar* GetRepairKitChargesBar() const = 0;

	/** Get the repair progress bar widget */
	virtual UProgressBar* GetRepairProgressBar() const = 0;

	/** Get the status text widget */
	virtual UTextBlock* GetStatusText() const = 0;

	/** Get the repair kit background image widget */
	virtual UImage* GetRepairKitBackground() const = 0;

	/** Get current target item being repaired */
	virtual const UInventoryItemEquipable* GetCurrentTargetItem() const = 0;

	/** Get current target durability */
	virtual float GetCurrentTargetDurability() const = 0;

	/** Get maximum target durability */
	virtual float GetMaxTargetDurability() const = 0;

	/** Get current repair kit durability (charges) */
	virtual float GetCurrentRepairKitDurability() const = 0;

	/** Get maximum repair kit durability */
	virtual float GetMaxRepairKitDurability() const = 0;

	/** Get whether repair is currently in progress */
	virtual bool GetIsRepairing() const = 0;

	/** Set repair in progress state */
	virtual void SetIsRepairing(bool bInIsRepairing) = 0;

	/** Get current repair progress (0.0 to 1.0) */
	virtual float GetRepairProgress() const = 0;

	/** Set repair progress */
	virtual void SetRepairProgress(float InProgress) = 0;

	/** Get elapsed repair time */
	virtual float GetElapsedRepairTime() const = 0;

	/** Set elapsed repair time */
	virtual void SetElapsedRepairTime(float InTime) = 0;

	/** Get total repair duration */
	virtual float GetTotalRepairDuration() const = 0;

	/** Get repair kit icon texture */
	virtual UTexture2D* GetRepairKitIcon() const = 0;

	// ============================================================================
	// Game-Specific Hooks (Override in derived classes for custom behavior)
	// ============================================================================

	/**
	 * Called when repair button is clicked
	 * Override for game-specific button handling
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FieldRepair")
	void OnRepairButtonClicked();
	virtual void OnRepairButtonClicked_Implementation() {}

	/**
	 * Called when repair starts (after validation)
	 * Override for server communication, sound effects, etc.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FieldRepair")
	void OnRepairStartedInternal();
	virtual void OnRepairStartedInternal_Implementation() {}

	/**
	 * Called when repair completes successfully
	 * Override for server communication, persistence, etc.
	 * @param DurabilityRepaired - Amount of durability that was restored
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FieldRepair")
	void OnRepairCompletedInternal(float DurabilityRepaired);
	virtual void OnRepairCompletedInternal_Implementation(float DurabilityRepaired) {}

	/**
	 * Called when repair is cancelled or interrupted
	 * Override for server communication, cleanup, etc.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FieldRepair")
	void OnRepairCancelledInternal();
	virtual void OnRepairCancelledInternal_Implementation() {}

	/**
	 * Check if repair can be started
	 * Override for custom validation logic
	 * @param OutReason - Reason why repair cannot start (if false)
	 * @return True if repair can start
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FieldRepair")
	bool CanStartRepair(FText& OutReason) const;
	virtual bool CanStartRepair_Implementation(FText& OutReason) const { return false; }

	/**
	 * Check if repair button should be enabled
	 * Override for custom button state logic
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FieldRepair")
	bool ShouldEnableRepairButton() const;
	virtual bool ShouldEnableRepairButton_Implementation() const { return false; }
};
