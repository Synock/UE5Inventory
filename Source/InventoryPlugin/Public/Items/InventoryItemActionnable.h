// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemBase.h"
#include "InventoryItemActionnable.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UInventoryItemActionnable : public UInventoryItemBase
{
public:
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Actionnable")
	bool Actionnable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Actionnable")
	float HungerValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Actionnable")
	float ThirstValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Actionnable")
	FString BookText;
};
