// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include "Engine/DataTable.h"
#include "Definitions.h"
#include "InventoryItem.generated.h"

USTRUCT(BlueprintType)
struct FMerchantDynamicItemStorage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	int32 ItemID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	int32 Quantity = 0;
};

USTRUCT(BlueprintType)
struct FMinimalItemStorage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	int32 ItemID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	int32 TopLeftID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item|Durability")
	float Durability = 100.0f;
};
