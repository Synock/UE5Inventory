// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "GenericSlotWidget.h"
#include "ItemBaseWidget.h"
#include "Components/TextBlock.h"
#include "InventoryItem.h"

#include "EquipmentSlotWidget.generated.h"

class APlayerCharacter;
/**
 * 
 */

UCLASS()
class INVENTORYPLUGIN_API UEquipmentSlotWidget : public UGenericSlotWidget
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable)
	void InitData();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory|Equipment")
	UTextBlock* TextSlot1 = nullptr;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory|Equipment")
	UTextBlock* TextSlot2 = nullptr;

	//this is a pointer used to handle two slots items
	// Only the first slot must reference the second to avoid cyclic calls
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Equipment")
	UEquipmentSlotWidget* SisterSlot = nullptr;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory|Equipment")
	EEquipmentSlot SlotID = EEquipmentSlot::Unknown;

	virtual bool HandleItemDrop(class UItemWidget* InputItem) override;

	UFUNCTION(BlueprintCallable)
	bool UnEquipBagSpecific();

	UFUNCTION(BlueprintCallable)
	void OpenBag() const;

	UFUNCTION(BlueprintCallable)
	bool IsBag() const;

	virtual void InnerRefresh() override;

	virtual void HideItem() override;

public:

	virtual void StopDrag() override;

	UFUNCTION(BlueprintCallable)
	void SetSisterSlot(UEquipmentSlotWidget* NewSisterSlot);
	
	UFUNCTION(BlueprintCallable)
	bool CanEquipItem(const UInventoryItemBase* InputItem) const;

	UFUNCTION(BlueprintCallable)
	static bool CanEquipItemAtSlot(const UInventoryItemBase* InputItem, EEquipmentSlot InputSlot);

	UFUNCTION(BlueprintCallable)
	bool TryEquipItem(class UItemWidget* InputItem);

	UFUNCTION(BlueprintCallable)
	void SetSlotWidgetStatus(bool InputStatus) { EnabledSlot = InputStatus; }

	UFUNCTION(BlueprintCallable)
	void UpdateTextSlots();
};
