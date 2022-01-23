// Copyright 2022 Maximilien (Synock) Guislain


#include "Inventory/CoinValue.h"

FCoinValue::FCoinValue(float ValueAsFloat)
{
	int32 ValueAsInt = static_cast<int32>(ValueAsFloat);
	CopperPieces = ValueAsInt % 10;
	ValueAsInt -= CopperPieces;
	ValueAsInt /= 10;

	SilverPieces = ValueAsInt % 10;
	ValueAsInt -= SilverPieces;
	ValueAsInt /= 10;

	GoldPieces = ValueAsInt % 10;
	ValueAsInt -= GoldPieces;
	ValueAsInt /= 10;

	PlatinumPieces = ValueAsInt % 10;
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue& FCoinValue::operator+=(const FCoinValue& OtherCoinValue)
{
	CopperPieces += OtherCoinValue.CopperPieces;
	SilverPieces += OtherCoinValue.SilverPieces;
	GoldPieces += OtherCoinValue.GoldPieces;
	PlatinumPieces += OtherCoinValue.PlatinumPieces;
	return *this;
}

//----------------------------------------------------------------------------------------------------------------------

FCoinValue& FCoinValue::operator*=(float Ratio)
{
	float FVal = ToFloat();
	FVal *= Ratio;
	if (Ratio >= 1.0f)
		FVal += 0.5f;
	else
		FVal -= 0.5f;

	*this = FCoinValue(std::max(FVal, 0.f));
	return *this;
}

//----------------------------------------------------------------------------------------------------------------------

void FCoinValue::ReduceCoinAmount()
{
	if (CopperPieces >= 10)
	{
		SilverPieces += CopperPieces / 10;
		CopperPieces = CopperPieces % 10;
	}
	if (SilverPieces >= 10)
	{
		GoldPieces += SilverPieces / 10;
		SilverPieces = SilverPieces % 10;
	}
	if (GoldPieces >= 10)
	{
		PlatinumPieces += GoldPieces / 10;
		GoldPieces = GoldPieces % 10;
	}
}

//----------------------------------------------------------------------------------------------------------------------

float FCoinValue::ToFloat() const
{
	return CopperPieces + SilverPieces * 10.f + GoldPieces * 100.f + PlatinumPieces * 1000.f;
}

//----------------------------------------------------------------------------------------------------------------------

bool FCoinValue::RetrieveValue(FCoinValue& AvailableCoins, FCoinValue& NeededCoins)
{
	if (AvailableCoins.CopperPieces < NeededCoins.CopperPieces)
	{
		const int32 Factor = std::max((NeededCoins.CopperPieces - AvailableCoins.CopperPieces) / 10, 1);
		AvailableCoins.CopperPieces += Factor * 10;
		NeededCoins.SilverPieces += Factor;
	}

	if (AvailableCoins.SilverPieces < NeededCoins.SilverPieces)
	{
		const int32 Factor = std::max((NeededCoins.SilverPieces - AvailableCoins.SilverPieces) / 10, 1);
		AvailableCoins.SilverPieces += Factor * 10;
		NeededCoins.GoldPieces += Factor;
	}

	if (AvailableCoins.GoldPieces < NeededCoins.GoldPieces)
	{
		const int32 Factor = std::max((NeededCoins.GoldPieces - AvailableCoins.GoldPieces) / 10, 1);
		AvailableCoins.GoldPieces += Factor * 10;
		NeededCoins.PlatinumPieces += Factor;
	}

	if (AvailableCoins.PlatinumPieces < NeededCoins.PlatinumPieces)
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool FCoinValue::CanPay(const FCoinValue& AvailableCoins, const FCoinValue& NeededCoins)
{
	return NeededCoins.CopperPieces <= AvailableCoins.CopperPieces
		&& NeededCoins.SilverPieces <= AvailableCoins.SilverPieces
		&& NeededCoins.GoldPieces <= AvailableCoins.GoldPieces
		&& NeededCoins.PlatinumPieces <= AvailableCoins.PlatinumPieces;
}

//----------------------------------------------------------------------------------------------------------------------

bool FCoinValue::CanPayWithChange(const FCoinValue& AvailableCoins, const FCoinValue& NeededCoins)
{
	FCoinValue CAvailableCoins = AvailableCoins;
	FCoinValue CNeededCoins = NeededCoins;

	return RetrieveValue(CAvailableCoins, CNeededCoins);
}
