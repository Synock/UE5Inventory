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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory|Equipment")
	UImage* BackgroundImagePointer = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Equipment")
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

	public:
	UFUNCTION(BlueprintCallable)
	bool CanDropItem(const FInventoryItem& InputItem) const;

};
