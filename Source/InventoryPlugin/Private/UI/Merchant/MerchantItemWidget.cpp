// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/Merchant/MerchantItemWidget.h"

#include "InventoryUtilities.h"
#include "Interfaces/InventoryPlayerInterface.h"
#include "Kismet/KismetInputLibrary.h"

void UMerchantItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	UMerchantItemData* Data = Cast<UMerchantItemData>(ListItemObject);

	if (Data)
	{
		InitData(Data->Data);
		ItemID = Data->Data.Id;
	}
}


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
