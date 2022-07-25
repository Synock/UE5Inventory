// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/Currency/CurrencySelectionWidget.h"
#include "Interfaces/InventoryPlayerInterface.h"

void UCurrencySelectionWidget::InitWidget(UCoinComponent* OriginCoinComponent, UCoinComponent* DestinationCoinComponent,
                                          ECurrencyType InputCurrencyType)
{
	check(OriginCoinComponent);
	check(DestinationCoinComponent);

	CurrencyType = InputCurrencyType;
	Origin = OriginCoinComponent;
	Destination = DestinationCoinComponent;

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

	SetupUI();
}

void UCurrencySelectionWidget::DoTheCoinTransfer(int32 SelectedCoinValue)
{
	if (SelectedCoinValue == 0)
		return;

	if (IInventoryPlayerInterface* Player = Cast<IInventoryPlayerInterface>(GetOwningPlayer()))
	{
		FCoinValue TemporaryValue;
		switch (CurrencyType)
		{
		default:
		case ECurrencyType::Copper: TemporaryValue.CopperPieces = SelectedCoinValue;
			break;
		case ECurrencyType::Silver: TemporaryValue.SilverPieces = SelectedCoinValue;
			break;
		case ECurrencyType::Gold: TemporaryValue.GoldPieces = SelectedCoinValue;
			break;
		case ECurrencyType::Platinum: TemporaryValue.PlatinumPieces = SelectedCoinValue;
			break;
		}
		Player->TransferCoinTo(Destination, TemporaryValue);
	}
}
