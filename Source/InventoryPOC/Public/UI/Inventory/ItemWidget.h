// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "ItemBaseWidget.h"
#include "Inventory/BareItem.h"
#include "ItemWidget.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYPOC_API UItemWidget : public UItemBaseWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Origin")
	int32 TopLeftID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Origin")
	BagSlot BagID = BagSlot::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Origin")
	InventorySlot OriginalSlotID = InventorySlot::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Origin")
	class UInventoryGridWidget* ParentGrid = nullptr;

public:
	UFUNCTION(BlueprintCallable)
	void InitData(const FBareItem& InputItem, AActor* InputOwner, float InputTileSize,
	              int32 InputTopLeftID = 0, BagSlot InputBagID = BagSlot::Unknown,
	              InventorySlot InputOriginalSlotID = InventorySlot::Unknown);

	virtual void StopDrag() override;

	UFUNCTION(BlueprintCallable)
	void GetSizeInPixels(float& WidthP, float& HeightP);

	UFUNCTION(BlueprintCallable)
	void SetParentGrid(UInventoryGridWidget* InputParentGrid);

	UFUNCTION(BlueprintCallable)
	int32 GetTopLeftID() const { return TopLeftID; }

	UFUNCTION(BlueprintCallable)
	BagSlot GetBagID() const { return BagID; }

	//return if the item is originating from equipment
	UFUNCTION(BlueprintCallable, Category = "Inventory|Origin")
	bool IsFromEquipment() const;

	//if the item is already owned by the player (in contrast to looted/bought items)
	UFUNCTION(BlueprintCallable, Category = "Inventory|Origin")
	bool IsBelongingToSelf() const;
};
