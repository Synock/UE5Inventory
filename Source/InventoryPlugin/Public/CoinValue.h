// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include "CoinValue.generated.h"

/**
 *@brief  Structure that represent a set of coins (either a value or an actual stack)
 * There are 4 different type of coin each higher value being 10x more valuable than the lesser valued
 * Copper < Silver < Gold < Platinum
 */
USTRUCT(BlueprintType)
struct INVENTORYPLUGIN_API FCoinValue
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory|Coins")
	int32 CopperPieces = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory|Coins")
	int32 SilverPieces = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory|Coins")
	int32 GoldPieces = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory|Coins")
	int32 PlatinumPieces = 0;

	FCoinValue() = default;

	FCoinValue(int32 CP, int32 SP, int32 GP, int32 PP): CopperPieces(CP), SilverPieces(SP), GoldPieces(GP),
	                                                    PlatinumPieces(PP)
	{
	}

	FCoinValue(float ValueAsFloat);

	FCoinValue& operator+=(const FCoinValue& OtherCoinValue);

	FCoinValue& operator*=(float Ratio);

	//@brief Change low value coins into higher value coins if possible.
	void ReduceCoinAmount();

	//@brief Convert coins to float.
	float ToFloat() const;

	//@brief return if the coin value is empty.
	bool IsEmpty() const;

	//@brief Adjust both AvailableCoins and NeededCoins to allow for a payment without changing the actual value.
	static bool RetrieveValue(FCoinValue& AvailableCoins, FCoinValue& NeededCoins);

	//@brief Return if it possible to pay the price NeededCoins using AvailableCoins without changing coins.
	static bool CanPay(const FCoinValue& AvailableCoins, const FCoinValue& NeededCoins);

	//@brief Return if it possible to pay the price NeededCoins using AvailableCoins allowing to change coins.
	static bool CanPayWithChange(const FCoinValue& AvailableCoins, const FCoinValue& NeededCoins);
};
