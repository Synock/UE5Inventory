#pragma once

#include "CoreMinimal.h"
#include "InventoryItemBase.h"
#include "InventoryItemEquipable.h"
#include "InventoryItemActionnable.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UInventoryItemActionnable : public UInventoryItemEquipable
{
public:
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Actionnable")
	bool Actionnable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Actionnable")
	bool NeedToBeEquipped = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Actionnable")
	float HungerValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Actionnable")
	float ThirstValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Actionnable")
	FString BookText;
};
