

#include "UI/Merchant/MerchantItemListWidget.h"

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindListViewSelection();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemListWidget::NativeDestruct()
{
	UnbindListViewSelection();

	Super::NativeDestruct();
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemListWidget::BindListViewSelection()
{
	if (!ItemListView)
		return;

	ItemListView->OnItemSelectionChanged().RemoveAll(this);
	ItemListView->OnItemSelectionChanged().AddUObject(this, &UMerchantItemListWidget::HandleListItemSelectionChanged);
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemListWidget::UnbindListViewSelection()
{
	if (ItemListView)
		ItemListView->OnItemSelectionChanged().RemoveAll(this);
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemListWidget::HandleListItemSelectionChanged(UObject* SelectedItem)
{
	if (const UMerchantItemData* Data = Cast<UMerchantItemData>(SelectedItem))
	{
#if WITH_AUTOMATION_WORKER
		LastSelectionItemIDForTests = Data->Data.Id;
#endif
		SelectionChangedDelegate.Broadcast(Data->Data.Id);
	}
	else
	{
#if WITH_AUTOMATION_WORKER
		LastSelectionItemIDForTests = 0;
#endif
		SelectionChangedDelegate.Broadcast(0);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UMerchantItemListWidget::AddDataToList_Implementation(const FMerchantItemDataStruct& ItemData)
{
	if (!ItemListView)
		return;

	UMerchantItemData* DataObj = NewObject<UMerchantItemData>(this);
	DataObj->Data = ItemData;
	ItemListView->AddItem(DataObj);
	ItemListView->RequestRefresh();
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


