// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>
#include "Definitions.h"
#include "CoinValue.h"
#include "Items/InventoryItemBase.h"
#include "InventoryUtilities.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UInventoryUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	static FVector2D GetItemFootPrint(EItemSize Size);

	static FString GetItemSizeString(EItemSize Size);

	static FString GetSlotName(EEquipmentSlot Slot);

	//@brief Return a BareItem from its ID
	UFUNCTION(BlueprintCallable, Category="Inventory|Item")
	static UInventoryItemBase* GetItemFromID(int32 ItemID, UWorld* WorldContext);

	//@brief Convert a float value to a CoinValue
	UFUNCTION(BlueprintCallable, Category="Inventory|Coins")
	static FCoinValue CoinValueFromFloat(float InputValue);

	//@brief Get a float value from a CoinValue
	UFUNCTION(BlueprintCallable, Category="Inventory|Coins")
	static float FloatFromCoinValue(const FCoinValue& CoinValue);

	//@brief Change coins amount to keep the same value but with less coins
	UFUNCTION(BlueprintCallable, Category="Inventory|Coins")
	static FCoinValue ReduceCoinAmount(const FCoinValue& CoinValue);

	//@brief Convert an amount of currency type to another
	UFUNCTION(BlueprintCallable, Category="Inventory|Coins")
	static FCoinValue ConvertCoins(ECurrencyType InputCurrency, int32 InputValue, ECurrencyType OutputCurrency);

	//@brief Return if it possible to pay the price NeededCoins using AvailableCoins without changing coins.
	UFUNCTION(BlueprintCallable, Category="Inventory|Coins")
	static bool CanPay(const FCoinValue& AvailableCoins, const FCoinValue& NeededCoins);

	//@brief Return if it possible to pay the price NeededCoins using AvailableCoins allowing to change coins.
	UFUNCTION(BlueprintCallable, Category="Inventory|Coins")
	static bool CanPayWithChange(const FCoinValue& AvailableCoins, const FCoinValue& NeededCoins);

	//@brief Adjust both AvailableCoins and NeededCoins to allow for a payment without changing the actual value.
	UFUNCTION(BlueprintCallable, Category="Inventory|Coins")
	static bool ComputeChange(FCoinValue& AvailableCoins, FCoinValue& NeededCoins);

	UFUNCTION(BlueprintCallable, Category="Inventory|General")
	static UTexture2D* LoadTexture2D(const FString& Path);

	static FString GetCoinValueAsString(const FCoinValue& CoinValue);

};
