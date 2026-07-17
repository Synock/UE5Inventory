#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "PendingDeliveryDragDropOperation.generated.h"

class UInventoryItemBase;

/** Drag payload for an item that does not exist in inventory until its claim is acknowledged. */
UCLASS()
class INVENTORYPLUGIN_API UPendingDeliveryDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category="Inventory|Delivery")
	FGuid DeliveryId;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Delivery")
	TObjectPtr<const UInventoryItemBase> Item = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Delivery")
	float Durability = 100.0f;
};
