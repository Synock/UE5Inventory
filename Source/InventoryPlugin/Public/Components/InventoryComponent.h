// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <Components/ActorComponent.h>
#include "InventoryItem.h"
#include "Items/InventoryItemBase.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeightChanged);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFullInventoryComponentChanged);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFullInventoryDispatcher_Server);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FInventoryItemAdd, EBagSlot, ConsideredBag, int32, ItemID, int32, TopLeftIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInventoryItemRemove, EBagSlot, ConsideredBag, int32, TopLeftIndex);

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

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	const UBagStorage* GetRelatedBagConst(EBagSlot InputSlot) const;


	UPROPERTY(BlueprintAssignable, Category = "Inventory") //this is public because its a dispatcher
	FOnFullInventoryComponentChanged FullInventoryDispatcher;

	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly, Category = "Inventory")
	//this is public because its a dispatcher
	FFullInventoryDispatcher_Server FullInventoryDispatcher_Server;

	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly, Category = "Inventory")
	FInventoryItemAdd InventoryItemAdd;

	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly, Category = "Inventory")
	FInventoryItemRemove InventoryItemRemove;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemAtIndex(EBagSlot ConsideredBag, int32 ID) const;

	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory")
	void AddItemAt(EBagSlot ConsideredBag, int32 ItemID, int32 TopLeftIndex);

	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory")
	void RemoveItem(EBagSlot ConsideredBag, int32 TopLeftIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const TArray<FMinimalItemStorage>& GetBagConst(EBagSlot WantedBagSlot) const;

	//Setup bag info
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void BagSet(EBagSlot ConsideredBag, bool InputValidity = false, int32 InputWidth = 0, int32 InputHeight = 0,
	            EItemSize InputMaxStoreSize = EItemSize::Giant);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	float GetTotalWeight() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasAnyItem(const TArray<int32>& ItemID);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasItem(int32 ItemID);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasItems(int32 ItemId, int32 ItemAmount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItemIfPossible(int32 ItemID);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool PlayerRemoveAnyItemIfPossible(const TArray<int32>& ItemID);

	//Find and return an empty slot for the item or BagSlot::Unknown
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	EBagSlot FindSuitableSlot(const UInventoryItemBase* Item, int32& OutputTopLeftID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	static EEquipmentSlot GetInventorySlotFromBagSlot(EBagSlot ConsideredBag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	static EBagSlot GetBagSlotFromInventory(EEquipmentSlot ConsideredInventory);

	UFUNCTION(BlueprintCallable,BlueprintPure, Category="Inventory")
	TArray<int32> GetAllItems() const;

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void RemoveAllItems();

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void ClearAllBags();

	UFUNCTION(Blueprintable, Category="Inventory")
	bool IsBagValid(EBagSlot InputSlot) const;
};
