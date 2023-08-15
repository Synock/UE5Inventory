// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemBase.h"
#include "InventoryItemKey.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UInventoryItemKey : public UInventoryItemBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Key")
	int32 KeyID = -1;
};
