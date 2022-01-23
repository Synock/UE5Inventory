// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "BareItem.h"
#include "Components/ActorComponent.h"
#include "MerchantComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMerchantDynamicPoolChangedDelegate);

///@brief Represent a merchant inventory with static item pool (eg bread for bakers)
///Dynamic item pool are the items sold by the player.
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPOC_API UMerchantComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMerchantComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Inventory|Merchant")
	TArray<int64> StaticMerchantPool;

	UPROPERTY(ReplicatedUsing=OnRep_DynamicPool, BlueprintReadWrite, Category = "Inventory|Merchant")
	TArray<FMerchantDynamicItemStorage> DynamicMerchantPool;

	const int32 DynamicPoolLimit = 20;

public:
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Merchant")
	FOnMerchantDynamicPoolChangedDelegate MerchantPoolDispatcher;

	UFUNCTION()
	void OnRep_DynamicPool();

	//Add item to the dynamic table
	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory|Merchant")
	void AddItem(int64 ItemID);

	//Remove an item from the list (either completely or one unit)
	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory|Merchant")
	void RemoveItem(int32 ListIndex);

	//Remove an item from the list (either completely or one unit)
	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory|Merchant")
	void RemoveItemID(int64 ItemID);

	//Initialize the static merchant item list
	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory|Merchant")
	void InitStatic(const TArray<int64>& MerchantStaticItems);

	//Initialize the dynamic merchant item list
	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory|Merchant")
	void InitDynamic(const TArray<FMerchantDynamicItemStorage>& MerchantDynamicItems);

	//Get item at given Index, by convention it is static items first, then dynamic items
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	int64 GetItemAtIndex(int32 ListIndex) const;

	//Get item at given Index in the static items list
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	int64 GetItemFromStaticPool(int32 ListIndex) const;

	//Get item at given Index in the dynamic items list
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	FMerchantDynamicItemStorage GetItemFromDynamicPool(int32 ListIndex) const;

	//Get all the static items
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	const TArray<int64>& GetStaticItemsConst() const { return StaticMerchantPool; }

	//Get all the static items
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	const TArray<FMerchantDynamicItemStorage>& GetDynamicItemsConst() const { return DynamicMerchantPool; }

	//Get all the static items
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	bool HasItem(int64 ItemID) const;
};
