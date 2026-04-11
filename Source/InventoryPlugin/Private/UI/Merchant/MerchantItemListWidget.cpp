

#include "UI/Merchant/MerchantItemListWidget.h"

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemListWidget::AddDataToList_Implementation(const FMerchantItemDataStruct& ItemData)
{
	if (!ItemListView)
		return;

	UMerchantItemData* DataObj = NewObject<UMerchantItemData>(this);
	DataObj->Data = ItemData;
	ItemListView->AddItem(DataObj);
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemListWidget::ClearList_Implementation()
{
	if (ItemListView)
		ItemListView->ClearListItems();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemListWidget::ClearSelection_Implementation()
{
	if (ItemListView)
		ItemListView->ClearSelection();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemListWidget::UpdateDynamicElement_Implementation(int64 ItemID, int32 Quantity)
{
	if (!ItemListView)
		return;

	const TArray<UObject*>& AllItems = ItemListView->GetListItems();

	for (UObject* Obj : AllItems)
	{
		if (UMerchantItemData* Data = Cast<UMerchantItemData>(Obj))
		{
			if (static_cast<int64>(Data->Data.Id) == ItemID)
			{
				Data->Data.Quantity = Quantity;
				ItemListView->RequestRefresh();
				break;
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemListWidget::RemoveElementsFrom_Implementation(int32 DynamicStartID)
{
	if (!ItemListView)
		return;

	const TArray<UObject*> AllItems = ItemListView->GetListItems();

	// Remove items from the end to avoid index shifting
	for (int32 i = AllItems.Num() - 1; i >= DynamicStartID; --i)
	{
		ItemListView->RemoveItem(AllItems[i]);
	}
}


