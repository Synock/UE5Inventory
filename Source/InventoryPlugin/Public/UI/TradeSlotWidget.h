#pragma once

#include "CoreMinimal.h"
#include "GenericSlotWidget.h"
#include "TradeSlotWidget.generated.h"

/**
 * @class UTradeSlotWidget
 *
 * Individual slot widget for displaying an item in the trade window.
 * Inherits from UGenericSlotWidget to reuse existing item display functionality.
 * Can be read-only (for viewing partner's items) or interactive (for our items).
 */
UCLASS()
class INVENTORYPLUGIN_API UTradeSlotWidget : public UGenericSlotWidget
{
	GENERATED_BODY()

protected:
	/** Slot index (0-7) in the trade offer */
	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	int32 SlotIndex = 0;

	/** Is this slot for our items (editable) or their items (read-only) */
	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	bool bIsOurSlot = true;

	/** Source bag slot for this item (used for drag-drop) */
	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	EBagSlot SourceBagSlot = EBagSlot::Unknown;

	/** Source top-left position for this item (used for drag-drop) */
	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	int32 SourceTopLeft = -1;

	/** Item durability (used for drag-drop) */
	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	float ItemDurability = 100.0f;

	/**
	 * @brief Handle item drops on this slot
	 */
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                          UDragDropOperation* InOperation) override;

	/**
	 * @brief Handle mouse button down events, including right-click for item inspection
	 */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/**
	 * @brief Handle drag detection to create ItemWidget drag operation for removing items from trade
	 */
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	                                  UDragDropOperation*& OutOperation) override;

public:
	/**
	 * @brief Initialize the trade slot
	 * @param InSlotIndex The slot index (0-7)
	 * @param bInIsOurSlot True if this is our slot (editable), false for their slot (read-only)
	 */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	void InitializeSlot(int32 InSlotIndex, bool bInIsOurSlot);

	/**
	 * @brief Set the item displayed in this slot using item data
	 * @param ItemData The item to display (nullptr for empty)
	 * @param OwnerActor The owner of the item
	 * @param InSourceBagSlot Source bag slot (for drag-drop)
	 * @param InSourceTopLeft Source top-left position (for drag-drop)
	 * @param InDurability Item durability (for drag-drop)
	 */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	void SetTradeItem(const UInventoryItemBase* ItemData, AActor* OwnerActor = nullptr,
	                  EBagSlot InSourceBagSlot = EBagSlot::Unknown, int32 InSourceTopLeft = -1,
	                  float InDurability = 100.0f);

	/**
	 * @brief Clear the slot
	 */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	void ClearSlot();

	/**
	 * @brief Get the slot index
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trade")
	int32 GetSlotIndex() const { return SlotIndex; }

	/**
	 * @brief Check if this is our slot (editable)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trade")
	bool IsOurSlot() const { return bIsOurSlot; }

	/**
	 * @brief Get the current item ID (0 if empty)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trade")
	int32 GetCurrentItemID() const;
};

