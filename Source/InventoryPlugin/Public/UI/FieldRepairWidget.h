#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "UI/FieldRepairWidgetInterface.h"
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
class INVENTORYPLUGIN_API UFieldRepairWidget : public UUserWidget, public IFieldRepairWidgetInterface
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
	virtual void SetVisibility(ESlateVisibility InVisibility) override;

	// ============================================================================
	// IFieldRepairWidgetInterface Implementation
	// ============================================================================

	// Interface accessors
	virtual UProgressBar* GetItemDurabilityBar() const override { return ItemDurabilityBar; }
	virtual UProgressBar* GetRepairKitChargesBar() const override { return RepairKitChargesBar; }
	virtual UProgressBar* GetRepairProgressBar() const override { return RepairProgressBar; }
	virtual UTextBlock* GetStatusText() const override { return StatusText; }
	virtual UImage* GetRepairKitBackground() const override { return RepairKitBackground; }
	virtual const UInventoryItemEquipable* GetCurrentTargetItem() const override { return CurrentTargetItem; }
	virtual float GetCurrentTargetDurability() const override { return CurrentTargetDurability; }
	virtual float GetMaxTargetDurability() const override { return MaxTargetDurability; }
	virtual float GetCurrentRepairKitDurability() const override { return CurrentRepairKitDurability; }
	virtual float GetMaxRepairKitDurability() const override { return MaxRepairKitDurability; }
	virtual bool GetIsRepairing() const override { return IsRepairing; }
	virtual void SetIsRepairing(bool bInIsRepairing) override { IsRepairing = bInIsRepairing; }
	virtual float GetRepairProgress() const override { return RepairProgress; }
	virtual void SetRepairProgress(float InProgress) override { RepairProgress = InProgress; }
	virtual float GetElapsedRepairTime() const override { return ElapsedRepairTime; }
	virtual void SetElapsedRepairTime(float InTime) override { ElapsedRepairTime = InTime; }
	virtual float GetTotalRepairDuration() const override { return TotalRepairDuration; }
	virtual UTexture2D* GetRepairKitIcon() const override;

	// Interface function overrides (use interface implementations)
	virtual void UpdateUI_Implementation() override { IFieldRepairWidgetInterface::UpdateUI_Implementation(); }
	virtual void UpdateItemDurabilityBar_Implementation() override { IFieldRepairWidgetInterface::UpdateItemDurabilityBar_Implementation(); }
	virtual void UpdateRepairKitChargesBar_Implementation() override { IFieldRepairWidgetInterface::UpdateRepairKitChargesBar_Implementation(); }
	virtual void UpdateRepairProgressBar_Implementation() override { IFieldRepairWidgetInterface::UpdateRepairProgressBar_Implementation(); }
	virtual void UpdateStatusText_Implementation(const FText& Message) override { IFieldRepairWidgetInterface::UpdateStatusText_Implementation(Message); }
	virtual void UpdateRepairKitBackground_Implementation() override { IFieldRepairWidgetInterface::UpdateRepairKitBackground_Implementation(); }
	// Interface function overrides (_Implementation versions)
	virtual void TickRepair_Implementation() override;
	virtual void ValidateState_Implementation() override;
	virtual void OnRepairButtonClicked_Implementation() override;
	virtual bool CanStartRepair_Implementation(FText& OutReason) const override;
	virtual bool ShouldEnableRepairButton_Implementation() const override;

	// ============================================================================
	// Repair Process
	// ============================================================================

	/** Timer callback that calls TickRepair via Execute_ */
	UFUNCTION()
	void OnRepairTimerTick();

	/** Start the repair process */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair")
	void StartRepair();

	/** Complete the repair successfully */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair")
	void CompleteRepair();

	/** Cancel the repair process */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair")
	void CancelRepair();

	// ============================================================================
	// Validation
	// ============================================================================

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

	/** Called when repair fails (server-side validation or skill check) */
	UFUNCTION(BlueprintImplementableEvent, Category = "FieldRepair|Events")
	void OnRepairFailed(const FText& Reason);
};

