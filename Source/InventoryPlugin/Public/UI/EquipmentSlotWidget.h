// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "ItemBaseWidget.h"
#include "Components/TextBlock.h"
#include "InventoryItem.h"

#include "EquipmentSlotWidget.generated.h"

class IInventoryPlayerInterface;
class APlayerCharacter;
/**
 * 
 */

UCLASS()
class INVENTORYPLUGIN_API UEquipmentSlotWidget : public UItemBaseWidget
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
	UImage* BackgroundImagePointer = nullptr;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory|Equipment")
	EEquipmentSlot SlotID = EEquipmentSlot::Unknown;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Equipment")
	bool EnabledSlot = true;
	
	UFUNCTION(BlueprintCallable)
	void UpdateItemImageVisibility();

	UFUNCTION(BlueprintCallable)
	void UpdateSlotState();

	UFUNCTION(BlueprintCallable)
	bool HandleItemDrop(class UItemWidget* InputItem);

	UFUNCTION(BlueprintCallable)
	bool UnEquipBagSpecific();

	UFUNCTION(BlueprintCallable)
	void OpenBag() const;

	UFUNCTION(BlueprintCallable)
	bool IsBag() const;

	UFUNCTION(BlueprintCallable)
	void InnerRefresh();

	UFUNCTION(BlueprintCallable)
	void HideItem();

	UFUNCTION()
	void ResetTransaction();
	
	IInventoryPlayerInterface* GetInventoryPlayerInterface() const;

public:
	virtual void StopDrag() override;

	UFUNCTION(BlueprintCallable)
	void SetSisterSlot(UEquipmentSlotWidget* NewSisterSlot);
	
	UFUNCTION(BlueprintCallable)
	bool CanEquipItem(const FInventoryItem& InputItem) const;

	UFUNCTION(BlueprintCallable)
	static bool CanEquipItemAtSlot(const FInventoryItem& InputItem, EEquipmentSlot InputSlot);

	UFUNCTION(BlueprintCallable)
	bool TryEquipItem(class UItemWidget* InputItem);

	UFUNCTION(BlueprintCallable)
	bool IsItemEquipped() const { return Item.ItemID > 0; }

	UFUNCTION(BlueprintCallable)
	void SetSlotWidgetStatus(bool InputStatus) { EnabledSlot = InputStatus; }

	UFUNCTION(BlueprintCallable)
	void UpdateTextSlots();
};
