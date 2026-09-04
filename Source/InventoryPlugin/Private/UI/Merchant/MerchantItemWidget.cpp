
#include "UI/Merchant/MerchantItemWidget.h"
#include "UI/Merchant/CoinDisplayWidget.h"

#include "InventoryUtilities.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Kismet/KismetInputLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	ResetEntryState();

	UMerchantItemData* Data = Cast<UMerchantItemData>(ListItemObject);

	if (Data)
	{
		UpdateDisplay(Data->Data);
		ItemID = Data->Data.Id;
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemWidget::UpdateDisplay(const FMerchantItemDataStruct& ItemData)
{
	// Update item icon
	if (ItemIcon && ItemData.Icon)
	{
		ItemIcon->SetBrushFromTexture(ItemData.Icon);
		ItemIcon->SetVisibility(ESlateVisibility::Visible);
	}
	else if (ItemIcon)
	{
		ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Update item name
	if (ItemName)
	{
		ItemName->SetText(FText::FromString(ItemData.Name));
	}

	// Update item quantity
	if (ItemQuantity)
	{
		if (ItemData.Quantity > 0)
		{
			ItemQuantity->SetText(FText::AsNumber(ItemData.Quantity));
			ItemQuantity->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ItemQuantity->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Update item price using CoinDisplayWidget
	if (ItemPrice)
	{
		ItemPrice->SetCoinValue(ItemData.CoinValue);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemWidget::BeginRightClickHold(const FVector2D& ScreenPosition)
{
	CancelRightClickHold();
	bRightClickPending = true;
	RightClickScreenPosition = ScreenPosition;
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemWidget::CancelRightClickHold()
{
	bRightClickPending = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RightClickTimerHandle);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemWidget::ResetEntryState()
{
	CancelRightClickHold();
	ItemID = 0;
	RightClickScreenPosition = FVector2D::ZeroVector;
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemWidget::HandleRightClickHoldElapsed()
{
	if (!bRightClickPending)
	{
		return;
	}

	bRightClickPending = false;
	RequestItemInspection();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemWidget::RequestItemInspection()
{
	if (ItemID <= 0)
	{
		return;
	}

#if WITH_AUTOMATION_WORKER
	++InspectionRequestCountForTests;
#endif

	if (IInventoryPlayerInterface* PC = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
	{
		PC->GetInventoryHUDInterface()->DisplayItemDescriptionFromSource(
			UInventoryUtilities::GetItemFromID(ItemID, GetWorld()),
			RightClickScreenPosition.X,
			RightClickScreenPosition.Y, this);
	}
}

//----------------------------------------------------------------------------------------------------------------------

FReply UMerchantItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (UKismetInputLibrary::PointerEvent_GetEffectingButton(InMouseEvent) != FKey("RightMouseButton"))
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	BeginRightClickHold(InMouseEvent.GetScreenSpacePosition());
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RightClickTimerHandle, this,
			&UMerchantItemWidget::HandleRightClickHoldElapsed, RightClickHoldDuration, false);
	}

	return FReply::Handled();
}

//----------------------------------------------------------------------------------------------------------------------

FReply UMerchantItemWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (UKismetInputLibrary::PointerEvent_GetEffectingButton(InMouseEvent) != FKey("RightMouseButton"))
	{
		return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	}

	CancelRightClickHold();
	return FReply::Handled();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemWidget::NativeOnEntryReleased()
{
	ResetEntryState();
	IUserObjectListEntry::NativeOnEntryReleased();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemWidget::NativeDestruct()
{
	ResetEntryState();
	Super::NativeDestruct();
}

//----------------------------------------------------------------------------------------------------------------------

