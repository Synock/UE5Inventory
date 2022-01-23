// Copyright 2021 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <Components/ActorComponent.h>
#include "Inventory/BareItem.h"
#include "FullInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFullInventoryComponentChanged);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFullInventoryDispatcher_Server);

USTRUCT(BlueprintType)
struct FVariableBagStorage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	BagSlot Slot = BagSlot::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	class UBagStorage* Bag = nullptr;
};

///@brief Class that store the item in possession of the players in its bags
/// Only the ID are stored
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPOC_API UFullInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFullInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing=OnRep_ReplicatedBags, BlueprintReadWrite, Category = "Inventory|Bag")
	TArray<FVariableBagStorage> VariableBags;

	//Lookup table to find out easily the correct bag
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Bag")
	TMap<BagSlot, UBagStorage*> BagLUT;

	UFUNCTION()
	void OnRep_ReplicatedBags();

	UFUNCTION()
	void DoBroadcastChange();

	UFUNCTION()
	void DoServerBroadcastChange();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	UBagStorage* GetRelatedBag(BagSlot InputSlot) const;

public:
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Inventory") //this is public because its a dispatcher
	FOnFullInventoryComponentChanged FullInventoryDispatcher;

	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly, Category = "Inventory|Inventory") //this is public because its a dispatcher
	FFullInventoryDispatcher_Server FullInventoryDispatcher_Server;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Inventory")
	int64 GetItemAtIndex(BagSlot ConsideredBag, int32 ID) const;

	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory|Inventory")
	void AddItemAt(BagSlot ConsideredBag, int64 ItemID, int32 TopLeftIndex);

	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory|Inventory")
	void RemoveItem(BagSlot ConsideredBag, int32 TopLeftIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Inventory")
	const TArray<FMinimalItemStorage>& GetBagConst(BagSlot WantedBagSlot) const;

	//Setup bag info
	UFUNCTION(BlueprintCallable, Category = "Inventory|Inventory")
	void BagSet(BagSlot ConsideredBag, bool InputValidity = false, int32 InputWidth = 0, int32 InputHeight = 0,
	            EItemSize InputMaxStoreSize = EItemSize::Giant);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Inventory")
	float GetTotalWeight() const;

	//Find and return an empty slot for the item or BagSlot::Unknown
	UFUNCTION(BlueprintCallable, Category = "Inventory|Inventory")
	BagSlot FindSuitableSlot(const FBareItem& Item, int32& OutputTopLeftID);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Inventory")
	static InventorySlot GetInventorySlotFromBagSlot(BagSlot ConsideredBag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Inventory")
	static BagSlot GetBagSlotFromInventory(InventorySlot ConsideredInventory);
};
