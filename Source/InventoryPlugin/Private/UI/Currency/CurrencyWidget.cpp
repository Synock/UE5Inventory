// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/Currency/CurrencyWidget.h"

void UCurrencyWidget::Refresh()
{
	CurrencyAmountTextBlockPointer->SetText(FText::FromString(FString::FormatAsNumber(CoinAmount)));
}

void UCurrencyWidget::UpdateCoinValue(int32 NewCoinAmount)
{
	CoinAmount = NewCoinAmount;
	Refresh();
}

void UCurrencyWidget::SetupCoinComponent(UCoinComponent* OriginCoinComponent, bool AllowForCurrencyChangeState)
{
	CoinComponent = OriginCoinComponent;
	AllowForCurrencyChange = AllowForCurrencyChangeState;
}
