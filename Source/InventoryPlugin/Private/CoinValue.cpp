// Copyright 2022 Maximilien (Synock) Guislain

#include "CoinValue.h"

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

	PlatinumPieces = ValueAsInt;
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

	*this = FCoinValue(FMath::CeilToFloat(FMath::Max(FVal, 0.f)));
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

bool FCoinValue::IsEmpty() const
{
	return CopperPieces == 0 && SilverPieces == 0 && GoldPieces == 0 && PlatinumPieces == 0;
}

//----------------------------------------------------------------------------------------------------------------------

void MakeChange(int32& AvailableLow, const int32& NeededLow, int32& AvailableCurrent, const int32& NeededCurrent,
                int32& NeededHigher, int32 ConversionFactor = 10)
{
	//if we don't have enough coin of the current value
	if (AvailableCurrent < NeededCurrent)
	{
		const int32 SpareLow = AvailableLow - NeededLow;
		const int32 MissingCurrent = NeededCurrent - AvailableCurrent;

		//if we can use lower value coins to make up for current values
		if (SpareLow >= MissingCurrent * ConversionFactor)
		{
			AvailableLow -= MissingCurrent * ConversionFactor; //we use ConversionFactor time more lower value coin
			AvailableCurrent += MissingCurrent;
		}
		else //otherwise we can try to use higher value coin to compensate
		{
			const int32 Factor = FMath::Max((NeededCurrent - AvailableCurrent) / ConversionFactor, 1);
			AvailableCurrent += Factor * ConversionFactor;
			NeededHigher += Factor;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool FCoinValue::RetrieveValue(FCoinValue& AvailableCoins, FCoinValue& NeededCoins)
{

	if (AvailableCoins.ToFloat() < NeededCoins.ToFloat())
		return false;


	//if copper coin is missing, the only option is to convert at least one silver into 10 copper
	if (AvailableCoins.CopperPieces < NeededCoins.CopperPieces)
	{
		const int32 Factor = FMath::Max((NeededCoins.CopperPieces - AvailableCoins.CopperPieces) / 10, 1);
		AvailableCoins.CopperPieces += Factor * 10;
		NeededCoins.SilverPieces += Factor;
	}

	//Silver can be obtained using either copper or gold
	MakeChange(AvailableCoins.CopperPieces, NeededCoins.CopperPieces,
	           AvailableCoins.SilverPieces, NeededCoins.SilverPieces,
	           NeededCoins.GoldPieces);

	//Gold can be obtained using either silver or platinum
	MakeChange(AvailableCoins.SilverPieces, NeededCoins.SilverPieces,
	           AvailableCoins.GoldPieces, NeededCoins.GoldPieces,
	           NeededCoins.PlatinumPieces);

	//if platinum coin is missing, we have to figure out if we have enough gold to cover the cost
	if (AvailableCoins.PlatinumPieces < NeededCoins.PlatinumPieces)
	{
		const int32 SpareGold = AvailableCoins.GoldPieces - NeededCoins.GoldPieces;
		const int32 MissingPlatinum = NeededCoins.PlatinumPieces - AvailableCoins.PlatinumPieces;

		if (SpareGold >= MissingPlatinum * 10)
		{
			AvailableCoins.GoldPieces -= MissingPlatinum * 10;
			AvailableCoins.PlatinumPieces += MissingPlatinum;
			return true;
		}
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
	return AvailableCoins.ToFloat() >= NeededCoins.ToFloat();
}
