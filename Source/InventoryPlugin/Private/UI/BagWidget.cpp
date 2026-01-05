// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/BagWidget.h"
#include "UI/InventoryGridWidget.h"
#include "UI/ItemWidget.h"
#include "Components/InventoryComponent.h"
#include "Interfaces/InventoryPlayerInterface.h"

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

//----------------------------------------------------------------------------------------------------------------------

void UBagWidget::LockItemSlot_Implementation(int32 TopLeft, bool bLocked)
{
	if (!InventoryGrid)
		return;

	// First, update the lock state in the underlying data storage
	// This ensures the lock state persists across refreshes
	if (IInventoryPlayerInterface* PC = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
	{
		// Get the inventory component and update the lock state
		UInventoryComponent* InventoryComp = PC->GetInventoryComponent();
		if (InventoryComp)
		{
			InventoryComp->SetItemLockState(CurrentBagSlot, TopLeft, bLocked);
		}
	}
	// Now refresh to update the visual state
	InventoryGrid->Refresh();
}

