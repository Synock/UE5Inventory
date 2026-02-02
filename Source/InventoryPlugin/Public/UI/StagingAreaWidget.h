#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StagingAreaWidget.generated.h"

class IInventoryPlayerInterface;
class UStagingAreaSlotWidget;
class UStagingAreaComponent;

/**
 * Staging area widget for displaying items in a temporary holding area.
 * Uses modern BindWidget pattern for automatic Blueprint widget binding.
 * Supports up to 8 staging slots for item preview and management.
 */
UCLASS()
class INVENTORYPLUGIN_API UStagingAreaWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// ============================================================================
	// Component References
	// ============================================================================

	/** The staging area component managing the staged items */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|StagingArea")
	TObjectPtr<UStagingAreaComponent> StagingComponent = nullptr;

	// ============================================================================
	// UI Components (BindWidget) - Slot Widgets
	// ============================================================================

	/** Staging slot 0 - automatically bound from Blueprint */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|StagingArea|Slots", Meta = (BindWidgetOptional))
	TObjectPtr<UStagingAreaSlotWidget> Slot0 = nullptr;

	/** Staging slot 1 - automatically bound from Blueprint */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|StagingArea|Slots", Meta = (BindWidgetOptional))
	TObjectPtr<UStagingAreaSlotWidget> Slot1 = nullptr;

	/** Staging slot 2 - automatically bound from Blueprint */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|StagingArea|Slots", Meta = (BindWidgetOptional))
	TObjectPtr<UStagingAreaSlotWidget> Slot2 = nullptr;

	/** Staging slot 3 - automatically bound from Blueprint */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|StagingArea|Slots", Meta = (BindWidgetOptional))
	TObjectPtr<UStagingAreaSlotWidget> Slot3 = nullptr;

	/** Staging slot 4 - automatically bound from Blueprint */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|StagingArea|Slots", Meta = (BindWidgetOptional))
	TObjectPtr<UStagingAreaSlotWidget> Slot4 = nullptr;

	/** Staging slot 5 - automatically bound from Blueprint */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|StagingArea|Slots", Meta = (BindWidgetOptional))
	TObjectPtr<UStagingAreaSlotWidget> Slot5 = nullptr;

	/** Staging slot 6 - automatically bound from Blueprint */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|StagingArea|Slots", Meta = (BindWidgetOptional))
	TObjectPtr<UStagingAreaSlotWidget> Slot6 = nullptr;

	/** Staging slot 7 - automatically bound from Blueprint */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|StagingArea|Slots", Meta = (BindWidgetOptional))
	TObjectPtr<UStagingAreaSlotWidget> Slot7 = nullptr;

	// ============================================================================
	// Initialization & Refresh
	// ============================================================================

	/**
	 * Initialize the staging area widget with component reference and bind to dispatcher
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|StagingArea")
	void InitData();

	/**
	 * Refresh all slot widgets to display current staged items
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|StagingArea")
	void Refresh();

	// ============================================================================
	// Helper Functions
	// ============================================================================

	/**
	 * Get the inventory player interface from the owning player
	 * @return The inventory player interface or nullptr if not found
	 */
	IInventoryPlayerInterface* GetInventoryPlayerInterface() const;

	/**
	 * Get a slot widget by its index
	 * @param ID - The slot index (0-7)
	 * @return The slot widget at the specified index, or nullptr if invalid
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|StagingArea|Slots")
	UStagingAreaSlotWidget* GetItemSlotFromID(int32 ID) const;

public:
	// ============================================================================
	// Public Interface
	// ============================================================================

	/** Maximum number of staging slots available */
	static constexpr int32 MaxStagingSlots = 8;
};
