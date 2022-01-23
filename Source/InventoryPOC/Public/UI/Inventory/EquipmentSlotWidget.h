// Copyright 2021 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "ItemBaseWidget.h"
#include "Inventory/BareItem.h"

#include "EquipmentSlotWidget.generated.h"

class AInventoryPOCCharacter;
/**
 * 
 */

UCLASS()
class INVENTORYPOC_API UEquipmentSlotWidget : public UItemBaseWidget
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable)
	void InitData(InventorySlot InputSlotID, float InputTileSize, AActor* InputOwner);

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Equipment")
	AInventoryPOCCharacter* PCOwner = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Equipment")
	InventorySlot SlotID = InventorySlot::Unknown;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Equipment")
	bool EnabledSlot = true;

public:
	virtual void StopDrag() override;

	UFUNCTION(BlueprintCallable)
	bool CanEquipItem(const FBareItem& InputItem) const;

	UFUNCTION(BlueprintCallable)
	static bool CanEquipItemAtSlot(const FBareItem& InputItem, InventorySlot InputSlot);

	UFUNCTION(BlueprintCallable)
	bool TryEquipItem(class UItemWidget* InputItem);

	UFUNCTION(BlueprintCallable)
	bool IsItemEquipped() const { return Item.Key > 0; }

	UFUNCTION(BlueprintCallable)
	void SetSlotWidgetStatus(bool InputStatus) { EnabledSlot = InputStatus; }
};
