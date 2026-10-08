#pragma once

#include "CoreMinimal.h"
#include "ItemBaseWidget.h"
#include "InventoryItem.h"
#include "ItemWidget.generated.h"

class IInventoryPlayerInterface;
/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UItemWidget : public UItemBaseWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item|Origin")
	int32 TopLeftID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item|Origin")
	EBagSlot BagID = EBagSlot::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item|Origin")
	EEquipmentSlot OriginalSlotID = EEquipmentSlot::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item|Origin")
	class UInventoryGridWidget* ParentGrid = nullptr;

	UFUNCTION(BlueprintCallable)
	void HandleAutoEquip();

	UFUNCTION(BlueprintCallable)
	void HandleAutoLoot();

	UFUNCTION(BlueprintCallable)
	void HandleSellClick();
	
	UFUNCTION(BlueprintCallable)
	void HandleActivation();

	virtual bool RightClickShortEffect_Implementation() override;

	IInventoryPlayerInterface* GetInventoryPlayerInterface() const;

	virtual void RefreshInternal() override;

	// C++ override to forward to the Blueprint event
	virtual void NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	// C++ override to handle mouse leave
	virtual void NativeOnMouseLeave(const FPointerEvent& MouseEvent) override;
	// C++ override to handle drag-cancelled (calls StopDrag)
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

public:
	UFUNCTION(BlueprintCallable)
	void InitData(const UInventoryItemBase* InputItem, AActor* InputOwner, float InputTileSize,
	              int32 InputTopLeftID = 0, EBagSlot InputBagID = EBagSlot::Unknown,
	              EEquipmentSlot InputOriginalSlotID = EEquipmentSlot::Unknown, float InputDurability = 100.0f);

	virtual void StopDrag() override;

	UFUNCTION(BlueprintCallable)
	void GetSizeInPixels(float& WidthP, float& HeightP);

	UFUNCTION(BlueprintCallable)
	void SetParentGrid(UInventoryGridWidget* InputParentGrid);

	UFUNCTION(BlueprintCallable)
	int32 GetTopLeftID() const { return TopLeftID; }

	UFUNCTION(BlueprintCallable)
	EBagSlot GetBagID() const { return BagID; }

	UFUNCTION(BlueprintCallable)
	EEquipmentSlot GetOriginalSlot() const {return OriginalSlotID;}

	//return if the item is originating from equipment
	UFUNCTION(BlueprintCallable, Category = "Inventory|Item|Origin")
	bool IsFromEquipment() const;

	//if the item is already owned by the player (in contrast to looted/bought items)
	UFUNCTION(BlueprintCallable, Category = "Inventory|Item|Origin")
	bool IsBelongingToSelf() const;
};
