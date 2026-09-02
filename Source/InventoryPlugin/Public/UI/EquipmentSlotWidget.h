#pragma once

#include "CoreMinimal.h"
#include "GenericSlotWidget.h"
#include "Components/TextBlock.h"
#include "InventoryItem.h"

#include "EquipmentSlotWidget.generated.h"

class UInventoryItemEquipable;
class UInventoryEquipmentWidget;

/**
 * Widget representing a single equipment slot in the player's paperdoll UI.
 *
 * All lifecycle and input logic lives in C++; the Blueprint subclass (UI_EquipmentSlot)
 * is responsible only for asset assignments and layout.
 *
 * UMG Designer requirements:
 *   - "LowerTextBox"  — UTextBlock (optional)
 *   - "UpperTextBox"  — UTextBlock (optional)
 *   - "BackgroundImage" — UImage   (optional, inherited via UGenericSlotWidget)
 */
UCLASS()
class INVENTORYPLUGIN_API UEquipmentSlotWidget : public UGenericSlotWidget
{
	GENERATED_BODY()

protected:
	// -------------------------------------------------------------------------
	// UI Bindings (resolved automatically by UMG — no manual assignment needed)
	// -------------------------------------------------------------------------

	/** Lower label line (e.g. first word of a two-word slot name). */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory|Equipment|UI")
	TObjectPtr<UTextBlock> LowerTextBox = nullptr;

	/** Upper label line (e.g. second word of a two-word slot name). */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Inventory|Equipment|UI")
	TObjectPtr<UTextBlock> UpperTextBox = nullptr;

	// -------------------------------------------------------------------------
	// Configuration
	// -------------------------------------------------------------------------

	/** Background tint applied when the mouse hovers over this slot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment|Style")
	FLinearColor HoverColor = FLinearColor::White;

	/** Background tint applied when the mouse is not over this slot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment|Style")
	FLinearColor DefaultColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);

	/** Item tint applied when the element is disabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment|Style")
	FLinearColor ItemDisabledTint = FLinearColor(0.25f, 0.25f, 0.25f, 1.0f);

	/** Equipment slot this widget represents — set per-instance in the Designer. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory|Equipment")
	EEquipmentSlot SlotID = EEquipmentSlot::Unknown;

	/** Owning paperdoll widget — set by UInventoryEquipmentWidget::RegisterSlotWidget. */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Equipment")
	TObjectPtr<UInventoryEquipmentWidget> ParentComponent = nullptr;

	// -------------------------------------------------------------------------
	// State
	// -------------------------------------------------------------------------

	/** Last item for which the tooltip was built; used to skip redundant FText allocations. */
	UPROPERTY(Transient)
	TObjectPtr<const UInventoryItemBase> CachedTooltipItem = nullptr;

	// -------------------------------------------------------------------------
	// Lifecycle
	// -------------------------------------------------------------------------

	/** Binds to the equipment dispatcher and performs the initial refresh. */
	virtual void NativeConstruct() override;

	// -------------------------------------------------------------------------
	// Input
	// -------------------------------------------------------------------------

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	// -------------------------------------------------------------------------
	// Overrides
	// -------------------------------------------------------------------------

	virtual bool HandleItemDrop(class UItemWidget* InputItem) override;
	virtual void InnerRefresh() override;
	virtual void HideItem() override;

	/** Applies the enabled/disabled tint to ItemImage then calls InnerRefresh. */
	virtual void Refresh_Implementation() override;

	// -------------------------------------------------------------------------
	// Internal helpers
	// -------------------------------------------------------------------------

	/** Binds dispatcher and triggers initial Refresh — called from NativeConstruct. */
	void InitData();

	/**
	 * Iterates every secondary slot that shares a multi-slot item's bit-mask and invokes Func on it.
	 * No-op if Item is null, not a multi-slot item, or ParentComponent is not set.
	 */
	void ForEachSecondarySlot(const UInventoryItemEquipable* Item,
	                          TFunctionRef<void(UEquipmentSlotWidget&)> Func) const;

	/** Tries to unequip a bag from this slot via the HUD interface. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool UnEquipBagSpecific();

	/** Toggles the bag panel for a bag-type item in this slot. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void OpenBag() const;

	/** Returns true if the currently equipped item implements IInventoryItemBagInterface. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Equipment")
	bool IsBag() const;

	static bool CanHandleMerchantSaleClick(bool bLeftClick, bool bTrading, bool bSlotEnabled,
		bool bSlotLocked, bool bHasValidItem, bool bHasCanonicalSlot);

	/** Disables this slot and mirrors the multi-slot item icon into it. */
	void DisableAndRefresh(const UInventoryItemEquipable* InputItem);

public:
	// -------------------------------------------------------------------------
	// Public interface
	// -------------------------------------------------------------------------

	virtual void StopDrag() override;

	/** Returns true if InputItem can be equipped in this slot. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	virtual bool CanEquipItem(const UInventoryItemBase* InputItem) const;

	/** Returns true if InputItem can be equipped in the given slot (static utility). */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	static bool CanEquipItemAtSlot(const UInventoryItemBase* InputItem, EEquipmentSlot InputSlot);

	/**
	 * Returns true if the item referenced by InputItem can be equipped in this slot.
	 * This is a validation-only helper; it does not perform an equip operation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool CanEquipItemWidget(class UItemWidget* InputItem) const;

	/** Enables or disables this slot widget. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void SetSlotWidgetStatus(bool InputStatus) { EnabledSlot = InputStatus; }

	/** Updates the LowerTextBox/UpperTextBox labels from the slot name. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void UpdateTextSlots();

	/** Sets the widget tooltip to the equipped item name, or clears it. */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void UpdateTooltip();

	[[nodiscard]] EEquipmentSlot GetSlotID() const { return SlotID; }

	[[nodiscard]] TObjectPtr<UInventoryEquipmentWidget> GetParentComponent() const { return ParentComponent; }

	void SetParentComponent(UInventoryEquipmentWidget* NewParentComponent) { ParentComponent = NewParentComponent; }

#if WITH_AUTOMATION_WORKER
	static bool CanHandleMerchantSaleClickForTests(bool bLeftClick, bool bTrading, bool bSlotEnabled,
		bool bSlotLocked, bool bHasValidItem, bool bHasCanonicalSlot)
	{
		return CanHandleMerchantSaleClick(bLeftClick, bTrading, bSlotEnabled, bSlotLocked,
			bHasValidItem, bHasCanonicalSlot);
	}
#endif
};
