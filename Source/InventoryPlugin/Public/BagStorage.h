// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include "InventoryItem.h"
#include "Components/ActorComponent.h"
#include "Items/InventoryItemBase.h"
#include "BagStorage.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBagStorageModified);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBagStorageDispatcher_Server);

class UInventoryItemBase;
/**
 * @brief Helper class to find out what slot is available
 */
class GridBagSolver
{
public:
	GridBagSolver(int32 InputWidth, int32 InputHeight);

	void RecordData(const UInventoryItemBase* Item, int32 TopLeft);

	bool IsRoomAvailable(const UInventoryItemBase* Item, int TopLeftIndex);

	int32 GetFirstValidTopLeft(const  UInventoryItemBase* Item);
private:
	int32 Width = 1;
	int32 Height = 1;
	TArray<const  UInventoryItemBase*> Grid;
};

/**
 * @brief Actual item storage for inventory bags
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API UBagStorage : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UBagStorage();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_BagData, BlueprintReadWrite, Category = "Inventory|Bag")
	TArray<FMinimalItemStorage> Items;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Bag")
	int Width = 3;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Bag")
	int Height = 2;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Bag")
	EItemSize MaxStoreSize = EItemSize::Giant;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Inventory|Bag")
	EBagSlot LocalBagSlot = EBagSlot::Unknown;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Bag")
	bool BagValidity = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Bag")
	float BagWeight = 0.f;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	bool InitializeData(EBagSlot InputBagSlot, int32 InputWidth, int32 InputHeight,
	                    EItemSize InputMaxStoreSize = EItemSize::Giant);

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Bag")
	FBagStorageModified BagDispatcher;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Bag")
	FBagStorageDispatcher_Server BagStorageDispatcher_Server;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	bool IsValidBag() const { return BagValidity; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	void SetBagValidity(bool InputValidity) { BagValidity = InputValidity; }

	UFUNCTION()
	void OnRep_BagData();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	int32 GetItemAtIndex(int32 ID) const;

	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory|Bag")
	void AddItemAt(int32 ItemID, int32 TopLeftIndex);

	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory|Bag")
	void RemoveItem(int32 TopLeftIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	EItemSize GetMaxStoreSize() const { return MaxStoreSize; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	EBagSlot GetBagSlot() const { return LocalBagSlot; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	void SetBagSlot(EBagSlot InputBagSlot) { LocalBagSlot = InputBagSlot; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	int32 GetWidth() const { return Width; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	int32 GetHeight() const { return Height; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	const TArray<FMinimalItemStorage>& GetBagConst() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	float GetBagWeight() const { return BagWeight; }

	GridBagSolver GetSolver() const;

	UFUNCTION(BlueprintCallable, Category="Inventory|Bag")
	bool HasItem(int32 ItemID);

	UFUNCTION(BlueprintCallable, Category="Inventory|Bag")
	int32 CountItems(int32 ItemID);

	UFUNCTION(BlueprintCallable, Category="Inventory|Bag")
	int32 GetFirstTopLeftID(int32 ItemID);
};
