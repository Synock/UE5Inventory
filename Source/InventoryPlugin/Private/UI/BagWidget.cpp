
#include "UI/BagWidget.h"
#include "UI/InventoryGridWidget.h"
#include "Components/InventoryComponent.h"
#include "Interfaces/InventoryPlayerInterface.h"

//----------------------------------------------------------------------------------------------------------------------
// IInventoryBagWindowInterface implementations
//----------------------------------------------------------------------------------------------------------------------

void UBagWidget::InitBagData_Implementation(const FString& InBagName, int32 InBagWidth,
                                              int32 InBagHeight, EItemSize InBagSize, EBagSlot InBagSlot)
{
	BagName = InBagName;
	BagWidth = InBagWidth;
	BagHeight = InBagHeight;
	BagSize = InBagSize;
	CurrentBagSlot = InBagSlot;
	IInventoryBagWindowInterface::Execute_InitUI(this);
}

//----------------------------------------------------------------------------------------------------------------------

void UBagWidget::InitUI_Implementation()
{
	if (BagNameText)
		BagNameText->SetText(FText::FromString(BagName));

	if (!InventoryGrid)
		return;

	if (AActor* OwnerActor = GetOwningPlayerPawn())
		InventoryGrid->InitData(OwnerActor, CurrentBagSlot, BagWidth, BagHeight);
}

//----------------------------------------------------------------------------------------------------------------------

void UBagWidget::RefreshBagWindow_Implementation()
{
	Refresh();
}

//----------------------------------------------------------------------------------------------------------------------

void UBagWidget::ShowBagWindow_Implementation()
{
	Show();
}

//----------------------------------------------------------------------------------------------------------------------

void UBagWidget::HideBagWindow_Implementation()
{
	Hide();
}

//----------------------------------------------------------------------------------------------------------------------

void UBagWidget::ToggleBagWindow_Implementation()
{
	ToggleDisplay();
}

//----------------------------------------------------------------------------------------------------------------------

void UBagWidget::DeInitBagWindow_Implementation()
{
	DeInitBagData();
}

//----------------------------------------------------------------------------------------------------------------------

void UBagWidget::LockBagItemSlot_Implementation(int32 TopLeft, bool bLocked)
{
	LockItemSlot(TopLeft, bLocked);
}

//----------------------------------------------------------------------------------------------------------------------
// Legacy direct API
//----------------------------------------------------------------------------------------------------------------------

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

void UBagWidget::Refresh_Implementation()
{
	if (InventoryGrid)
		InventoryGrid->Refresh();
}

//----------------------------------------------------------------------------------------------------------------------

void UBagWidget::DeInitBagData_Implementation()
{
	if (InventoryGrid)
		InventoryGrid->DeInitData();
}

//----------------------------------------------------------------------------------------------------------------------

void UBagWidget::LockItemSlot_Implementation(int32 TopLeft, bool bLocked)
{
	if (!InventoryGrid)
		return;

	if (IInventoryPlayerInterface* PC = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
	{
		if (UInventoryComponent* InventoryComp = PC->GetInventoryComponent())
			InventoryComp->SetItemLockState(CurrentBagSlot, TopLeft, bLocked);
	}

	InventoryGrid->Refresh();
}
