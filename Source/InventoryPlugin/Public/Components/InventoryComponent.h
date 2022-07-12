// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <Components/ActorComponent.h>
#include "InventoryItem.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeightChanged);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFullInventoryComponentChanged);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFullInventoryDispatcher_Server);

USTRUCT(BlueprintType)
struct FVariableBagStorage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	EBagSlot Slot = EBagSlot::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	class UBagStorage* Bag = nullptr;
};

///@brief Class that store the item in possession of the players in its bags
/// Only the ID are stored
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing=OnRep_ReplicatedBags, BlueprintReadWrite, Category = "Inventory|Bag")
	TArray<FVariableBagStorage> VariableBags;

	//Lookup table to find out easily the correct bag
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Bag")
	TMap<EBagSlot, UBagStorage*> BagLUT;

	UFUNCTION()
	void OnRep_ReplicatedBags();

	UFUNCTION()
	void DoBroadcastChange();

	UFUNCTION()
	void DoServerBroadcastChange();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	UBagStorage* GetRelatedBag(EBagSlot InputSlot) const;

public:
	UPROPERTY(BlueprintAssignable, Category = "Neverquest|Inventory") //this is public because its a dispatcher
	FOnFullInventoryComponentChanged FullInventoryDispatcher;

	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly, Category = "Neverquest|Inventory")
	//this is public because its a dispatcher
	FFullInventoryDispatcher_Server FullInventoryDispatcher_Server;

	UFUNCTION(BlueprintCallable, Category = "Neverquest|Inventory")
	int32 GetItemAtIndex(EBagSlot ConsideredBag, int32 ID) const;

	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Neverquest|Inventory")
	void AddItemAt(EBagSlot ConsideredBag, int32 ItemID, int32 TopLeftIndex);

	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Neverquest|Inventory")
	void RemoveItem(EBagSlot ConsideredBag, int32 TopLeftIndex);

	UFUNCTION(BlueprintCallable, Category = "Neverquest|Inventory")
	const TArray<FMinimalItemStorage>& GetBagConst(EBagSlot WantedBagSlot) const;

	//Setup bag info
	UFUNCTION(BlueprintCallable, Category = "Neverquest|Inventory")
	void BagSet(EBagSlot ConsideredBag, bool InputValidity = false, int32 InputWidth = 0, int32 InputHeight = 0,
	            EItemSize InputMaxStoreSize = EItemSize::Giant);

	UFUNCTION(BlueprintCallable, Category = "Neverquest|Inventory")
	float GetTotalWeight() const;

	//Find and return an empty slot for the item or BagSlot::Unknown
	UFUNCTION(BlueprintCallable, Category = "Neverquest|Inventory")
	EBagSlot FindSuitableSlot(const FInventoryItem& Item, int32& OutputTopLeftID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Neverquest|Inventory")
	static EEquipmentSlot GetInventorySlotFromBagSlot(EBagSlot ConsideredBag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Neverquest|Inventory")
	static EBagSlot GetBagSlotFromInventory(EEquipmentSlot ConsideredInventory);
};
