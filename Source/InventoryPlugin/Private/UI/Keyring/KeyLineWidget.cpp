// Copyright 2023 Maximilien (Synock) Guislain


#include "UI/Keyring/KeyLineWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UKeyLineWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	if (const UKeyLineData* Data = Cast<UKeyLineData>(ListItemObject))
	{
		LocalData = Data->Data;

		if(KeyIconPointer)
			KeyIconPointer->SetBrushFromTexture(LocalData.KeyIcon);

		if(KeyNamePointer)
			KeyNamePointer->SetText(FText::FromString(LocalData.Name));
	}
}

UKeyLineWidget* UKeyLineWidget::GetItemObject() const
{
	return GetListItem<UKeyLineWidget>();
}
