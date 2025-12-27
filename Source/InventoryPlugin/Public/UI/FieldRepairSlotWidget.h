#pragma once

#include "CoreMinimal.h"
#include "UI/GenericSlotWidget.h"
#include "FieldRepairSlotWidget.generated.h"

class UFieldRepairWidget;
class UInventoryItemEquipable;

/**
 * Specialized slot widget for the field repair system.
 * Accepts only equipable items that can be repaired.
 * Communicates with parent UFieldRepairWidget to validate and set repair targets.
 */
UCLASS()
class INVENTORYPLUGIN_API UFieldRepairSlotWidget : public UGenericSlotWidget
{
	GENERATED_BODY()

protected:
	/**
	 * Handle item drop validation and communication with parent repair widget
	 * @param InputItem - The item widget being dropped
	 * @return True if the drop was handled successfully
	 */
	virtual bool HandleItemDrop(class UItemWidget* InputItem) override;

public:
	/**
	 * Check if an item can be placed in this repair slot
	 * @param InputItem - The item to validate
	 * @return True if the item is equipable and can potentially be repaired
	 */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair")
	bool CanAcceptRepairItem(const UInventoryItemBase* InputItem) const;

	/**
	 * Get the parent field repair widget
	 * @return Pointer to parent repair widget, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "FieldRepair")
	UFieldRepairWidget* GetParentRepairWidget() const;
};

