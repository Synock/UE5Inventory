// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/BagWidget.h"

void UBagWidget::Hide()
{
	SetVisibility(ESlateVisibility::Hidden);
}

//----------------------------------------------------------------------------------------------------------------------

void UBagWidget::Show()
{
	SetVisibility(ESlateVisibility::Visible);
}

//----------------------------------------------------------------------------------------------------------------------

void UBagWidget::ToggleDisplay()
{
	if (IsInViewport())
		Hide();
	else
		Show();
}

//----------------------------------------------------------------------------------------------------------------------

void UBagWidget::InitBagData(const FString& InBagName, int32 InBagWidth, int32 InBagHeight, EItemSize InBagSize,
                             EBagSlot InBagSlot)
{
	BagName = InBagName;
	BagWidth = InBagWidth;
	BagHeight = InBagHeight;
	BagSize = InBagSize;
	CurrentBagSlot = InBagSlot;
	InitUI();
}
