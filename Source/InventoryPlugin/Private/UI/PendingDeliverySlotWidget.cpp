#include "UI/PendingDeliverySlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/InventoryNetComponent.h"
#include "GameFramework/PlayerController.h"
#include "InventoryUtilities.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Items/InventoryItemBase.h"
#include "UI/PendingDeliveryDragDropOperation.h"

void UPendingDeliverySlotWidget::InitializeDelivery(const FPendingInventoryDelivery& Delivery, AActor* OwnerActor)
{
	DeliveryId = Delivery.DeliveryId;
	const UInventoryItemBase* DeliveryItem = UInventoryUtilities::GetItemFromID(Delivery.ItemID, GetWorld());
	InitBareData(DeliveryItem, OwnerActor, TileSize, Delivery.Durability);
	SetToolTipText(DeliveryItem ? FText::FromString(DeliveryItem->Name) : FText::GetEmpty());
	Refresh();
}

void UPendingDeliverySlotWidget::ClearDelivery()
{
	DeliveryId.Invalidate();
	InitBareData(nullptr, nullptr, TileSize);
	SetToolTipText(FText::GetEmpty());
	Refresh();
}

FReply UPendingDeliverySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && DeliveryId.IsValid() && Item)
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UPendingDeliverySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	if (!DeliveryId.IsValid() || !Item)
		return;

	UPendingDeliveryDragDropOperation* Operation = Cast<UPendingDeliveryDragDropOperation>(
		UWidgetBlueprintLibrary::CreateDragDropOperation(UPendingDeliveryDragDropOperation::StaticClass()));
	if (!Operation)
		return;
	Operation->DeliveryId = DeliveryId;
	Operation->Item = Item;
	Operation->Durability = Durability;

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UPendingDeliverySlotWidget* Visual = CreateWidget<UPendingDeliverySlotWidget>(PC, GetClass()))
		{
			FPendingInventoryDelivery VisualDelivery;
			VisualDelivery.DeliveryId = DeliveryId;
			VisualDelivery.ItemID = Item->ItemID;
			VisualDelivery.Durability = Durability;
			Visual->InitializeDelivery(VisualDelivery, Owner);
			Visual->SetVisibility(ESlateVisibility::HitTestInvisible);
			Operation->DefaultDragVisual = Visual;
		}
	}
	Operation->Pivot = EDragPivot::MouseDown;
	OutOperation = Operation;
}

bool UPendingDeliverySlotWidget::RightClickShortEffect_Implementation()
{
	if (!Item || !DeliveryId.IsValid())
		return false;
	if (IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
	{
		if (UInventoryNetComponent* Net = Player->GetInventoryNetComponent())
		{
			Net->Server_ClaimPendingDelivery(DeliveryId);
			return true;
		}
	}
	return false;
}
