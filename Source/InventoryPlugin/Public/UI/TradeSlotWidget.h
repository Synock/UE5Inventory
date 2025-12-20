// Copyright 2025 Maximilien (Synock) Guislain

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

	/**
	 * @brief Handle item drops on this slot
	 */
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                          UDragDropOperation* InOperation) override;

	/**
	 * @brief Handle mouse button down events, including right-click for item inspection
	 */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

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
	 */
	UFUNCTION(BlueprintCallable, Category = "Trade")
	void SetTradeItem(const UInventoryItemBase* ItemData, AActor* OwnerActor = nullptr);

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

