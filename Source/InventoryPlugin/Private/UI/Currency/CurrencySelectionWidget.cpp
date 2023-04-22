// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/Currency/CurrencySelectionWidget.h"

#include "InventoryUtilities.h"
#include "Interfaces/InventoryPlayerInterface.h"

void UCurrencySelectionWidget::InitWidget(UCoinComponent* OriginCoinComponent, UCoinComponent* DestinationCoinComponent,
                                          ECurrencyType InputCurrencyType, ECurrencyType OutputCurrencyType ,bool AllowForCurrencyChangeState)
{
	check(OriginCoinComponent);
	check(DestinationCoinComponent);

	CurrencyType = InputCurrencyType;
	Origin = OriginCoinComponent;
	Destination = DestinationCoinComponent;
	DesiredCurrencyType = OutputCurrencyType;

	switch (CurrencyType)
	{
	default:
	case ECurrencyType::Copper: MaximumCoinCapacity = OriginCoinComponent->GetCP();
		break;
	case ECurrencyType::Silver: MaximumCoinCapacity = OriginCoinComponent->GetSP();
		break;
	case ECurrencyType::Gold: MaximumCoinCapacity = OriginCoinComponent->GetGP();
		break;
	case ECurrencyType::Platinum: MaximumCoinCapacity = OriginCoinComponent->GetPP();
		break;
	}

	AllowForCurrencyChange = AllowForCurrencyChangeState;
	SetupUI();
}

void UCurrencySelectionWidget::DoTheCoinTransfer(int32 SelectedCoinValue)
{
	if (SelectedCoinValue == 0)
	{
		return;
	}

	if (IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
	{
		FCoinValue RemovedValue = UInventoryUtilities::ConvertCoins(CurrencyType,SelectedCoinValue,CurrencyType);
		FCoinValue AddedValue = UInventoryUtilities::ConvertCoins(CurrencyType,SelectedCoinValue,DesiredCurrencyType);
		Player->TransferCoinTo(Origin, Destination, RemovedValue, AddedValue);
	}
}
