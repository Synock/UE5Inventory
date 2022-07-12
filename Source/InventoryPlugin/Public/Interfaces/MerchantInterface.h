// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <UObject/Interface.h>
#include "CoinValue.h"
#include "InventoryItem.h"

#include "MerchantInterface.generated.h"

class FOnMerchantDynamicPoolChangedDelegate;
class UCoinComponent;
class UMerchantComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UMerchantInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class INVENTORYPLUGIN_API IMerchantInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual UWorld* GetMerchantWorldContext() const = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FString GetMerchantName() const = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual float GetMerchantRatio() const = 0;

	//Get coin component
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual UCoinComponent* GetCoinComponent() = 0;

	//Get merchant component
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual UMerchantComponent* GetMerchantComponent() = 0;

	//Get coins
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual UCoinComponent* GetCoinComponentConst() const = 0;

	//Get coins
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual UMerchantComponent* GetMerchantComponentConst() const = 0;

	virtual FOnMerchantDynamicPoolChangedDelegate& GetMerchantDispatcher() const = 0;


	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void InitMerchantData(const TArray<int32>& StaticItems,
	                              const TArray<FMerchantDynamicItemStorage>& DynamicItems,
	                              const FCoinValue& Coins);

	//Get all the static items
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual const TArray<int32>& GetStaticItemsConst() const;

	//Get all the dynamic items
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual const TArray<FMerchantDynamicItemStorage>& GetDynamicItemsConst() const;

	//Remove a given item if it belongs to the dynamic data
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void RemoveItemAmountIfNeeded(int32 ItemID);

	//Pay coin
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void PayCoin(const FCoinValue& CoinValue);

	//Receive coin
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void ReceiveCoin(const FCoinValue& CoinValue);

	//Returns if the merchant has the given item in stock
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual bool HasItem(int32 ItemID) const;

	//Return if the merchant can pay the asked price
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual bool CanPayAmount(const FCoinValue& CoinValue) const;

	//Adjust the Merchant price using its internal ratio
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FCoinValue AdjustPriceBuy(const FCoinValue& CoinValue) const;

	//Adjust the Merchant price using its internal ratio
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FCoinValue AdjustPriceSell(const FCoinValue& CoinValue) const;

	//Get the Merchant price for buying using its internal ratio
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FCoinValue GetItemPriceBuy(int32 ItemID) const;

	//Get the Merchant price for selling using its internal ratio
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FCoinValue GetItemPriceSell(int32 ItemID) const;

	//Add an item to the dynamic list
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void AddDynamicItem(int32 ItemID);
};
