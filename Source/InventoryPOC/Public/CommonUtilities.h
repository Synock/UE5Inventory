// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>
#include "Inventory/BareItem.h"
#include "Inventory/CoinValue.h"
#include "CommonUtilities.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYPOC_API UCommonUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//@brief Return a BareItem from its ID
	UFUNCTION(BlueprintCallable, Category="Inventory|Item")
	static FBareItem GetItemFromID(int64 ItemID, UWorld* WorldContext);

	//@brief Convert a float value to a CoinValue
	UFUNCTION(BlueprintCallable, Category="Inventory|Coins")
	static FCoinValue CoinValueFromFloat(float InputValue);

	//@brief Get a float value from a CoinValue
	UFUNCTION(BlueprintCallable, Category="Inventory|Coins")
	static float FloatFromCoinValue(const FCoinValue& CoinValue);

	//@brief Change coins amount to keep the same value but with less coins
	UFUNCTION(BlueprintCallable, Category="Inventory|Coins")
	static FCoinValue ReduceCoinAmount(const FCoinValue& CoinValue);

	//@brief Return if it possible to pay the price NeededCoins using AvailableCoins without changing coins.
	UFUNCTION(BlueprintCallable, Category="Inventory|Coins")
	static bool CanPay(const FCoinValue& AvailableCoins, const FCoinValue& NeededCoins);

	//@brief Return if it possible to pay the price NeededCoins using AvailableCoins allowing to change coins.
	UFUNCTION(BlueprintCallable, Category="Inventory|Coins")
	static bool CanPayWithChange(const FCoinValue& AvailableCoins, const FCoinValue& NeededCoins);

	//@brief Adjust both AvailableCoins and NeededCoins to allow for a payment without changing the actual value.
	UFUNCTION(BlueprintCallable, Category="Inventory|Coins")
	static bool ComputeChange(FCoinValue& AvailableCoins, FCoinValue& NeededCoins);
};
