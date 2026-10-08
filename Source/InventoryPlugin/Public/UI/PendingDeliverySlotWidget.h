#pragma once

#include "CoreMinimal.h"
#include "InventoryDelivery.h"
#include "UI/GenericSlotWidget.h"
#include "PendingDeliverySlotWidget.generated.h"

/** Inspectable and draggable presentation of one immutable pending-delivery record. */
UCLASS()
class INVENTORYPLUGIN_API UPendingDeliverySlotWidget : public UGenericSlotWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Inventory|Delivery")
	void InitializeDelivery(const FPendingInventoryDelivery& Delivery, AActor* OwnerActor);

	UFUNCTION(BlueprintCallable, Category="Inventory|Delivery")
	void ClearDelivery();

	UFUNCTION(BlueprintPure, Category="Inventory|Delivery")
	FGuid GetDeliveryId() const { return DeliveryId; }

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;
	virtual bool RightClickShortEffect_Implementation() override;

private:
	UPROPERTY(Transient)
	FGuid DeliveryId;
};
