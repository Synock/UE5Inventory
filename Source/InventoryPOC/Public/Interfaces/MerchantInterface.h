// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <UObject/Interface.h>

#include "Inventory/BareItem.h"
#include "Inventory/CoinValue.h"
#include "Inventory/MerchantComponent.h"

#include "MerchantInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UMerchantInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class INVENTORYPOC_API IMerchantInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FString GetMerchantName() const = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual float GetMerchantRatio() const = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void InitMerchantData(const TArray<int64>& StaticItems,
	                              const TArray<FMerchantDynamicItemStorage>& DynamicItems,
	                              const FCoinValue& Coins) = 0;

	//Get all the static items
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual const TArray<int64>& GetStaticItemsConst() const = 0;

	//Remove a given item if it belongs to the dynamic data
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void RemoveItemAmountIfNeeded(int64 ItemID) = 0;

	//Pay coin
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void PayCoin(const FCoinValue& CoinValue) = 0;

	//Receive coin
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void ReceiveCoin(const FCoinValue& CoinValue) = 0;

	//Get all the dynamic items
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual const TArray<FMerchantDynamicItemStorage>& GetDynamicItemsConst() const = 0;

	//Returns if the merchant has the given item in stock
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual bool HasItem(int64 ItemID) const = 0;

	//Return if the merchant can pay the asked price
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual bool CanPayAmount(const FCoinValue& CoinValue) const = 0;

	//Adjust the Merchant price using its internal ratio
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FCoinValue AdjustPriceBuy(const FCoinValue& CoinValue) const = 0;

	//Adjust the Merchant price using its internal ratio
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FCoinValue AdjustPriceSell(const FCoinValue& CoinValue) const = 0;

	//Get the Merchant price for buying using its internal ratio
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FCoinValue GetItemPriceBuy(int64 ItemID) const = 0;

	//Get the Merchant price for selling using its internal ratio
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FCoinValue GetItemPriceSell(int64 ItemID) const = 0;

	//Add an item to the dynamic list
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void AddDynamicItem(int64 ItemID) = 0;

	virtual FOnMerchantDynamicPoolChangedDelegate& GetMerchantDispatcher() const = 0;
};
