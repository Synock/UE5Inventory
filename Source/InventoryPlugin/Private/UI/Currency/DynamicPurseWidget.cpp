#include "UI/Currency/DynamicPurseWidget.h"
#include "Components/CoinComponent.h"
#include "UI/Currency/CurrencyWidget.h"
#include "Definitions.h"

//----------------------------------------------------------------------------------------------------------------------

void UDynamicPurseWidget::InitWidget(UCoinComponent* Owner)
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
		PursePointer->PurseDispatcher.AddDynamic(this, &UDynamicPurseWidget::Refresh);
	}

	// Setup each currency widget with correct currency type
	if (CopperCurrencyWidget)
	{
		CopperCurrencyWidget->SetCurrencyType(ECurrencyType::Copper);
		CopperCurrencyWidget->SetupCoinComponent(PursePointer, AllowForCurrencyChange);
	}

	if (SilverCurrencyWidget)
	{
		SilverCurrencyWidget->SetCurrencyType(ECurrencyType::Silver);
		SilverCurrencyWidget->SetupCoinComponent(PursePointer, AllowForCurrencyChange);
	}

	if (GoldCurrencyWidget)
	{
		GoldCurrencyWidget->SetCurrencyType(ECurrencyType::Gold);
		GoldCurrencyWidget->SetupCoinComponent(PursePointer, AllowForCurrencyChange);
	}

	if (PlatinumCurrencyWidget)
	{
		PlatinumCurrencyWidget->SetCurrencyType(ECurrencyType::Platinum);
		PlatinumCurrencyWidget->SetupCoinComponent(PursePointer, AllowForCurrencyChange);
	}

	// Initial refresh
	Refresh();
}

//----------------------------------------------------------------------------------------------------------------------

void UDynamicPurseWidget::Refresh()
{
	if (!PursePointer)
		return;

	// Update each currency widget with current values
	if (CopperCurrencyWidget)
	{
		CopperCurrencyWidget->UpdateCoinValue(PursePointer->GetCP());
	}

	if (SilverCurrencyWidget)
	{
		SilverCurrencyWidget->UpdateCoinValue(PursePointer->GetSP());
	}

	if (GoldCurrencyWidget)
	{
		GoldCurrencyWidget->UpdateCoinValue(PursePointer->GetGP());
	}

	if (PlatinumCurrencyWidget)
	{
		PlatinumCurrencyWidget->UpdateCoinValue(PursePointer->GetPP());
	}
}

//----------------------------------------------------------------------------------------------------------------------
