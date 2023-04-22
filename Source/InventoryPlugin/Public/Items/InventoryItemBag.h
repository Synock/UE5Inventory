// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemEquipable.h"
#include "InventoryItemBag.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UInventoryItemBag : public UInventoryItemEquipable
{
public:
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	bool Bag = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	EItemSize BagSize = EItemSize::Giant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	uint8 BagWidth = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	uint8 BagHeight = 1;
};
