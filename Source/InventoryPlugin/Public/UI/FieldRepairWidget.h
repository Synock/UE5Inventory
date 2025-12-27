#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "FieldRepairWidget.generated.h"

class UInventoryItemBase;
class IInventoryItemFieldRepairInterface;
class UInventoryItemEquipable;
class UFieldRepairSlotWidget;
class IInventoryPlayerInterface;

/**
 * Field repair widget that allows players to repair equipment using field repair items.
 *
 * Features:
 * - Drop slot for item to repair
 * - Background image of the repair kit
 * - Progress bars for: item durability, repair kit charges, repair progress
 * - Status text for feedback
 * - Repair button to start the process
 * - Interruptible repair process with timer
 *
 * Blueprint Setup:
 * - ItemSlot: UGenericSlotWidget (drop zone for item to repair)
 * - RepairKitBackground: UImage (shows the repair kit icon)
 * - ItemDurabilityBar: UProgressBar (shows target item durability 0-1)
 * - RepairKitChargesBar: UProgressBar (shows repair kit charges 0-1)
 * - RepairProgressBar: UProgressBar (shows repair progress 0-1)
 * - StatusText: UTextBlock (shows status messages)
 * - RepairButton: UButton (start repair button)
 */
UCLASS()
class INVENTORYPLUGIN_API UFieldRepairWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// ============================================================================
	// UI Components (BindWidget)
	// ============================================================================

	/** Slot for dropping the item to be repaired */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "FieldRepair|UI")
	UFieldRepairSlotWidget* ItemSlot = nullptr;

	/** Background image showing the repair kit icon */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "FieldRepair|UI")
	UImage* RepairKitBackground = nullptr;

	/** Progress bar showing the durability of the item being repaired */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "FieldRepair|UI")
	UProgressBar* ItemDurabilityBar = nullptr;

	/** Progress bar showing remaining charges in the repair kit */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "FieldRepair|UI")
	UProgressBar* RepairKitChargesBar = nullptr;

	/** Progress bar showing the current repair operation progress */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "FieldRepair|UI")
	UProgressBar* RepairProgressBar = nullptr;

	/** Text block for status messages and feedback */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "FieldRepair|UI")
	UTextBlock* StatusText = nullptr;

	/** Button to start the repair process */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "FieldRepair|UI")
	UButton* RepairButton = nullptr;

	// ============================================================================
	// State
	// ============================================================================

	/** The field repair item currently being used (via interface) */
	UPROPERTY(BlueprintReadOnly, Category = "FieldRepair|State")
	TScriptInterface<IInventoryItemFieldRepairInterface> CurrentRepairItem;

	/** Current durability (charges) of the repair kit */
	UPROPERTY(BlueprintReadWrite, Category = "FieldRepair|State")
	float CurrentRepairKitDurability = 100.0f;

	/** Maximum durability (charges) of the repair kit */
	UPROPERTY(BlueprintReadOnly, Category = "FieldRepair|State")
	float MaxRepairKitDurability = 100.0f;

	/** The item currently being repaired */
	UPROPERTY(BlueprintReadOnly, Category = "FieldRepair|State")
	const UInventoryItemEquipable* CurrentTargetItem = nullptr;

	/** Current durability of the target item */
	UPROPERTY(BlueprintReadWrite, Category = "FieldRepair|State")
	float CurrentTargetDurability = 100.0f;

	/** Maximum durability of the target item */
	UPROPERTY(BlueprintReadOnly, Category = "FieldRepair|State")
	float MaxTargetDurability = 100.0f;

	/** Whether a repair is currently in progress */
	UPROPERTY(BlueprintReadOnly, Category = "FieldRepair|State")
	bool IsRepairing = false;

	/** Current progress of the repair (0.0 to 1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "FieldRepair|State")
	float RepairProgress = 0.0f;

	/** Total duration of the current repair operation */
	UPROPERTY(BlueprintReadOnly, Category = "FieldRepair|State")
	float TotalRepairDuration = 0.0f;

	/** Time elapsed in current repair operation */
	UPROPERTY(BlueprintReadOnly, Category = "FieldRepair|State")
	float ElapsedRepairTime = 0.0f;

	/** Timer handle for the repair process */
	UPROPERTY()
	FTimerHandle RepairTimerHandle;

	/** Amount of durability that will be repaired */
	UPROPERTY(BlueprintReadOnly, Category = "FieldRepair|State")
	float PlannedRepairAmount = 0.0f;

	// ============================================================================
	// Widget Lifecycle
	// ============================================================================

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ============================================================================
	// UI Update Functions
	// ============================================================================

	/** Update all progress bars and status text */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair|UI")
	void UpdateUI();

	/** Update the item durability bar */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair|UI")
	void UpdateItemDurabilityBar();

	/** Update the repair kit charges bar */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair|UI")
	void UpdateRepairKitChargesBar();

	/** Update the repair progress bar */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair|UI")
	void UpdateRepairProgressBar();

	/** Update status text with a message */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair|UI")
	void UpdateStatusText(const FText& Message);

	/** Update the repair kit background image */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair|UI")
	void UpdateRepairKitBackground();

	// ============================================================================
	// Button Callbacks
	// ============================================================================

	/** Called when the repair button is clicked */
	UFUNCTION()
	void OnRepairButtonClicked();

	// ============================================================================
	// Repair Process
	// ============================================================================

	/** Start the repair process */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair")
	void StartRepair();

	/** Update the repair process (called by timer) */
	UFUNCTION()
	void TickRepair();

	/** Complete the repair successfully */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair")
	void CompleteRepair();

	/** Cancel the repair process */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair")
	void CancelRepair();

	/** Check if repair can be started */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair")
	bool CanStartRepair(FText& OutReason) const;

	// ============================================================================
	// Validation
	// ============================================================================

	/** Validate the current state and update UI accordingly */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair")
	void ValidateState();

	/** Get the inventory player interface from owning player */
	IInventoryPlayerInterface* GetInventoryPlayerInterface() const;

public:
	// ============================================================================
	// Public Interface
	// ============================================================================

	/**
	 * Initialize the widget with a field repair item
	 * @param RepairItem - The field repair item to use (must implement IInventoryItemFieldRepairInterface)
	 * @param RepairKitDurability - Current durability (charges) of the repair kit
	 */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair")
	void InitializeWithRepairItem(TScriptInterface<IInventoryItemFieldRepairInterface> RepairItem, float RepairKitDurability);

	/**
	 * Set the item to be repaired (called when item is dropped into slot)
	 * @param TargetItem - The item to repair
	 * @param TargetDurability - Current durability of the item
	 */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair")
	void SetTargetItem(const UInventoryItemEquipable* TargetItem, float TargetDurability);

	/**
	 * Clear the target item slot
	 */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair")
	void ClearTargetItem();

	/**
	 * Check if the widget is currently repairing
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "FieldRepair")
	bool GetIsRepairing() const { return IsRepairing; }

	// ============================================================================
	// Blueprint Events
	// ============================================================================

	/** Called when repair starts */
	UFUNCTION(BlueprintImplementableEvent, Category = "FieldRepair|Events")
	void OnRepairStarted();

	/** Called when repair completes successfully */
	UFUNCTION(BlueprintImplementableEvent, Category = "FieldRepair|Events")
	void OnRepairCompleted(float DurabilityRepaired);

	/** Called when repair is cancelled or interrupted */
	UFUNCTION(BlueprintImplementableEvent, Category = "FieldRepair|Events")
	void OnRepairCancelled();

	/** Called when the target item is set or changed */
	UFUNCTION(BlueprintImplementableEvent, Category = "FieldRepair|Events")
	void OnTargetItemChanged();
};

