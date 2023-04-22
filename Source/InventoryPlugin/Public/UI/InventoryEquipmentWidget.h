// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryEquipmentWidget.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYPLUGIN_API UInventoryEquipmentWidget : public UUserWidget
{
 	
 GENERATED_BODY()
protected:

	UFUNCTION(BlueprintCallable)
	bool HandleItemDrop(class UItemWidget* InputItem);
	
public:

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ForceRefresh();
};
