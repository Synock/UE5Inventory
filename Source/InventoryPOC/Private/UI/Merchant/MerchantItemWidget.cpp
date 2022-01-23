// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/Merchant/MerchantItemWidget.h"

void UMerchantItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	UMerchantItemData* Data = Cast<UMerchantItemData>(ListItemObject);

	if(Data)
	{
		InitData(Data->Data);
		ItemID = Data->Data.Id;
	}

}
