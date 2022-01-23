// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/MerchantInterface.h"
#include "Inventory/BareItem.h"
#include "Inventory/CoinValue.h"
#include "Inventory/MerchantComponent.h"
#include "MerchantActor.generated.h"

class UPurseComponent;

UCLASS()
class INVENTORYPOC_API AMerchantActor : public AActor, public IMerchantInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMerchantActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Contains the lootable items.
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Inventory|Merchant")
	TObjectPtr<UMerchantComponent> MerchantComponent;

	//Contains the lootable coins.
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Inventory|Merchant")
	TObjectPtr<UPurseComponent> CashContent_Merchant;

	//Merchant name.
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Inventory|Merchant")
	FString MerchantName = "Merchant";

	//Merchant Sell/Buy ratio. 0.9 means it will buy items at 90% of their price and sell them at 110%
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Inventory|Merchant")
	float MerchantRatio = 0.9f;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void InitMerchantData(const TArray<int64>& StaticItems, const TArray<FMerchantDynamicItemStorage>& DynamicItems,
	                      const FCoinValue& Coins) override;

public:

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FString GetMerchantName() const {return MerchantName;}

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual float GetMerchantRatio() const {return MerchantRatio;}

	//Get all the static items
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual const TArray<int64>& GetStaticItemsConst() const override;

	//Remove a given item if it belongs to the dynamic data
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void RemoveItemAmountIfNeeded(int64 ItemID) override;

	//Pay coin
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void PayCoin(const FCoinValue& CoinValue) override;

	//Receive coin
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void ReceiveCoin(const FCoinValue& CoinValue) override;

	//Get all the dynamic items
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual const TArray<FMerchantDynamicItemStorage>& GetDynamicItemsConst() const override;

	//Returns if the merchant has the given item in stock
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual bool HasItem(int64 ItemID) const override;

	//Return if the merchant can pay the asked price
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual bool CanPayAmount(const FCoinValue& CoinValue) const override;

	//Adjust the Merchant price using its internal ratio
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FCoinValue AdjustPriceBuy(const FCoinValue& CoinValue) const override;

	//Adjust the Merchant price using its internal ratio
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FCoinValue AdjustPriceSell(const FCoinValue& CoinValue) const override;

	//Get the Merchant price for buying using its internal ratio
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FCoinValue GetItemPriceBuy(int64 ItemID) const override;

	//Get the Merchant price for selling using its internal ratio
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual FCoinValue GetItemPriceSell(int64 ItemID) const override;

	//Add an item to the dynamic list
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void AddDynamicItem(int64 ItemID) override;

	virtual FOnMerchantDynamicPoolChangedDelegate& GetMerchantDispatcher() const override;
};
