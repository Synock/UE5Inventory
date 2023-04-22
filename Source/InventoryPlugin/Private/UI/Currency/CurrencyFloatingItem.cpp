// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/Currency/CurrencyFloatingItem.h"

void UCurrencyFloatingItem::InitWidget(UCoinComponent* CoinOriginPointer, int32 InputCoinAmount,
                                       ECurrencyType InputCurrencyType)
{
	CoinOrigin = CoinOriginPointer;
	CoinAmount = InputCoinAmount;
	CurrencyType = InputCurrencyType;
	SetupUI();
}
