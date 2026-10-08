#pragma once

#include "CoreMinimal.h"
#include "GenericSlotWidget.h"
#include "Components/StagingAreaComponent.h"
#include "StagingAreaSlotWidget.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UStagingAreaSlotWidget : public UGenericSlotWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	int32 ID = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	EBagSlot BagSlot = EBagSlot::StagingArea;

	/** Server-authored escrow record represented by this slot. */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Staging")
	FInventoryEscrowItem StagedItem;

	virtual bool HandleItemDrop(class UItemWidget* InputItem) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;

public:
	void InitializeStagedItem(const FInventoryEscrowItem& InStagedItem, const UInventoryItemBase* InItem,
		AActor* InOwner, float InTileSize = 40.f);
	void ClearStagedItem(float InTileSize = 40.f);
};
