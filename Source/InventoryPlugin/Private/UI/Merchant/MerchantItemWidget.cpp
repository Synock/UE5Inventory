
#include "UI/Merchant/MerchantItemWidget.h"
#include "UI/Merchant/CoinDisplayWidget.h"

#include "InventoryUtilities.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Kismet/KismetInputLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

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

FReply UMerchantItemWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (UKismetInputLibrary::PointerEvent_GetEffectingButton(InMouseEvent) == FKey("RightMouseButton"))
	{
		if (IInventoryPlayerInterface* PC = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
		{
			PC->GetInventoryHUDInterface()->Execute_DisplayItemDescription(
				PC->GetInventoryHUDObject(), UInventoryUtilities::GetItemFromID(ItemID, GetWorld()),
				InMouseEvent.GetScreenSpacePosition().X,
				InMouseEvent.GetScreenSpacePosition().Y);
		}
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

//----------------------------------------------------------------------------------------------------------------------

