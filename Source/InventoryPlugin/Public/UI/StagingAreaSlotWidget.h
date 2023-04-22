// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "GenericSlotWidget.h"
#include "Components/StagingAreaComponent.h"
#include "StagingAreaSlotWidget.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UStagingAreaSlotWidget : public UGenericSlotWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	int32 ID = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	EBagSlot BagSlot = EBagSlot::StagingArea;

	virtual bool HandleItemDrop(class UItemWidget* InputItem) override;

};
