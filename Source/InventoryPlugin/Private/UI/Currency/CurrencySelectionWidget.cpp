// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/Currency/CurrencySelectionWidget.h"

void UCurrencySelectionWidget::InitWidget(UCoinComponent* OriginCoinComponent, ECurrencyType InputCurrencyType)
{

	check(OriginCoinComponent);

	CurrencyType = InputCurrencyType;
	CoinComponent = OriginCoinComponent;

	switch (CurrencyType)
	{
	case ECurrencyType::Copper: MaximumCoinCapacity = CoinComponent->GetCP(); break;
	case ECurrencyType::Silver: MaximumCoinCapacity = CoinComponent->GetSP();  break;
	case ECurrencyType::Gold: MaximumCoinCapacity = CoinComponent->GetGP();  break;
	case ECurrencyType::Platinum:  MaximumCoinCapacity = CoinComponent->GetPP(); break;
	}

	SetupUI();
}
