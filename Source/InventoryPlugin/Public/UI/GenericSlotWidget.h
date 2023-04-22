// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "ItemBaseWidget.h"
#include "GenericSlotWidget.generated.h"

class IInventoryPlayerInterface;
/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UGenericSlotWidget : public UItemBaseWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	UImage* BackgroundImagePointer = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	bool EnabledSlot = true;

	UFUNCTION(BlueprintCallable)
	void UpdateItemImageVisibility();

	UFUNCTION(BlueprintCallable)
	void UpdateSlotState();

	UFUNCTION(BlueprintCallable)
	virtual bool HandleItemDrop(class UItemWidget* InputItem);

	IInventoryPlayerInterface* GetInventoryPlayerInterface() const;

	UFUNCTION(BlueprintCallable)
	virtual void InnerRefresh();

	UFUNCTION()
	void ResetTransaction();

	UFUNCTION(BlueprintCallable)
	virtual void HideItem();

	public:

	UFUNCTION(BlueprintCallable)
	bool CanDropItem(const UInventoryItemBase* InputItem) const;

	UFUNCTION(BlueprintCallable)
	bool IsItemEquipped() const { return Item != nullptr; }



};
