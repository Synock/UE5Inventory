// Copyright 2025 Maximilien (Synock) Guislain


#include "UI/Craft/BaseCraftWidget.h"

#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "UI/InventoryGridWidget.h"

void UBaseCraftWidget::SetupUI(AActor* Owner, UButton* CratButton, UButton* CancelButton, UProgressBar* ProgressBar,
                               UInventoryGridWidget* InputGrid, UInventoryGridWidget* OutputGrid)
{
	InputGridPointer = InputGrid;
	OutputGridPointer = OutputGrid;

	if (InputGridPointer)
	{
		InputGridPointer->SetCanAcceptDrop(true);
		InputGridPointer->InitData(Owner, EBagSlot::CraftInput);
	}

	if (OutputGridPointer)
	{
		OutputGridPointer->SetCanAcceptDrop(false);
		OutputGridPointer->InitData(Owner, EBagSlot::CraftOutput);
	}

	CratButtonPointer = CratButton;
	CancelButtonPointer = CancelButton;
	ProgressBarPointer = ProgressBar;

	if (CratButtonPointer)
		CratButtonPointer->OnClicked.AddDynamic(this, &UBaseCraftWidget::DisableCraftButton);
}

void UBaseCraftWidget::EnableCraftButton()
{
	if (CratButtonPointer)
		CratButtonPointer->SetIsEnabled(true);
}

void UBaseCraftWidget::DisableCraftButton()
{
	if (CratButtonPointer)
		CratButtonPointer->SetIsEnabled(false);
	if (ProgressBarPointer)
		ProgressBarPointer->SetPercent(0.f);
}
