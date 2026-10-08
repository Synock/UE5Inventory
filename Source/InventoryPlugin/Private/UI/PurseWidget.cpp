#include "UI/PurseWidget.h"
#include "Components/CoinComponent.h"

//----------------------------------------------------------------------------------------------------------------------

void UPurseWidget::InitWidget(UCoinComponent* Owner)
{
	// Unbind from previous purse if any
	if (PursePointer)
	{
		PursePointer->PurseDispatcher.RemoveAll(this);
	}

	// Set new purse pointer
	PursePointer = Owner;

	// Bind to new purse dispatcher
	if (PursePointer)
	{
		PursePointer->PurseDispatcher.AddDynamic(this, &UPurseWidget::Refresh);
	}

	// Initial refresh
	Refresh();
}

//----------------------------------------------------------------------------------------------------------------------

void UPurseWidget::Refresh()
{
	if (PursePointer)
	{
		// Get coin values from the coin component and update display
		// CoinDisplayWidget's SetCoinValue will handle all UI updates including icons
		SetCoinValue(PursePointer->GetCoinValue());
	}
	else
	{
		// No purse, clear display
		Clear();
	}
}

//----------------------------------------------------------------------------------------------------------------------

