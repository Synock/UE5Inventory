// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryEquipmentWidget.generated.h"

class UEquipmentSlotWidget;
enum class EEquipmentSlot : uint8;
/**
 * 
 */
UCLASS()
class INVENTORYPLUGIN_API UInventoryEquipmentWidget : public UUserWidget
{
 	
 GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadOnly)
	TMap<EEquipmentSlot, UEquipmentSlotWidget*> KnownEquipmentSlot;

	UFUNCTION(BlueprintCallable)
	virtual bool HandleItemDrop(class UItemWidget* InputItem);

	UFUNCTION(BlueprintCallable)
	void RegisterSlotWidget(UEquipmentSlotWidget* NewSlotWidget);


public:

	UFUNCTION(BlueprintCallable)
	void ForceRefresh();

	UFUNCTION(BlueprintCallable)
	UEquipmentSlotWidget* GetSlotWidget(EEquipmentSlot WantedSlot) const;

};
