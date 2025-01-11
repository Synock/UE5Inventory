// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "GenericSlotWidget.h"
#include "ItemBaseWidget.h"
#include "Components/TextBlock.h"
#include "InventoryItem.h"

#include "EquipmentSlotWidget.generated.h"

class UInventoryItemEquipable;
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

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Equipment")
	UTextBlock* TextSlot1 = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Equipment")
	UTextBlock* TextSlot2 = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory|Equipment")
	EEquipmentSlot SlotID = EEquipmentSlot::Unknown;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory|Equipment")
	class UInventoryEquipmentWidget* ParentComponent = nullptr;

	virtual bool HandleItemDrop(class UItemWidget* InputItem) override;

	UFUNCTION(BlueprintCallable)
	bool UnEquipBagSpecific();

	UFUNCTION(BlueprintCallable)
	void OpenBag() const;

	UFUNCTION(BlueprintCallable)
	bool IsBag() const;

	virtual void InnerRefresh() override;

	virtual void HideItem() override;

	void DisableAndRefresh(const UInventoryItemEquipable* InputItem);

public:
	virtual void StopDrag() override;

	UFUNCTION(BlueprintCallable)
	virtual bool CanEquipItem(const UInventoryItemBase* InputItem) const;

	UFUNCTION(BlueprintCallable)
	static bool CanEquipItemAtSlot(const UInventoryItemBase* InputItem, EEquipmentSlot InputSlot);

	UFUNCTION(BlueprintCallable)
	bool TryEquipItem(class UItemWidget* InputItem);

	UFUNCTION(BlueprintCallable)
	void SetSlotWidgetStatus(bool InputStatus) { EnabledSlot = InputStatus; }

	UFUNCTION(BlueprintCallable)
	void UpdateTextSlots();

	[[nodiscard]] EEquipmentSlot GetSlotID() const
	{
		return SlotID;
	}

	[[nodiscard]] UInventoryEquipmentWidget* GetParentComponent() const
	{
		return ParentComponent;
	}

	void SetParentComponent(UInventoryEquipmentWidget* const NewParentComponent)
	{
		ParentComponent = NewParentComponent;
	}
};
