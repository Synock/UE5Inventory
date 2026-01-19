#pragma once

#include <CoreMinimal.h>
#include <Components/ActorComponent.h>
#include "InventoryItem.h"
#include "Items/InventoryItemBase.h"
#include "InventoryComponent.generated.h"

class IInventoryItemAmmoInterface;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeightChanged);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFullInventoryComponentChanged);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFullInventoryDispatcher_Server);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FInventoryItemAdd, EBagSlot, ConsideredBag, int32, ItemID, int32, TopLeftIndex, float, Durability);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInventoryItemRemove, EBagSlot, ConsideredBag, int32, TopLeftIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FInventoryItemDurabilityUpdate, EBagSlot, ConsideredBag, int32, ItemID, int32, TopLeftIndex, float, NewDurability);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInventoryBagUsageChanged, EBagSlot, ConsideredBag, float, BagUsage);

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

	UFUNCTION()
	void InventoryBagUsageChange(EBagSlot ConsideredBag, float BagUsage);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	const UBagStorage* GetRelatedBagConst(EBagSlot InputSlot) const;
	float GetBagUsage(EBagSlot Quiver);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	bool HasCompatibleAmmoInQuiver(EAmmoType Ammo) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	TScriptInterface<IInventoryItemAmmoInterface> RemoveAmmoFromQuiver(EAmmoType Ammo);

	// Check if we can receive all items in the array
	UFUNCTION(BlueprintCallable, Category = "Inventory|Bag")
	bool CanReceiveAllItems(TArray<UInventoryItemBase*> ItemArray);

	UPROPERTY(BlueprintAssignable, Category = "Inventory") //this is public because its a dispatcher
	FOnFullInventoryComponentChanged FullInventoryDispatcher;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	//this is public because its a dispatcher
	FFullInventoryDispatcher_Server FullInventoryDispatcher_Server;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FInventoryItemAdd InventoryItemAdd;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FInventoryItemRemove InventoryItemRemove;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FInventoryItemDurabilityUpdate InventoryItemDurabilityUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FInventoryBagUsageChanged InventoryBagUsageChanged;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemAtIndex(EBagSlot ConsideredBag, int32 ID) const;

	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory")
	void AddItemAt(EBagSlot ConsideredBag, int32 ItemID, int32 TopLeftIndex, float Durability = 100.0f);

	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Inventory")
	void RemoveItem(EBagSlot ConsideredBag, int32 TopLeftIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const TArray<FMinimalItemStorage>& GetBagConst(EBagSlot WantedBagSlot) const;

	/**
	 * Update the lock state for an item in a specific bag at a specific position
	 * @param BagSlot - The bag containing the item
	 * @param TopLeft - The grid position of the item
	 * @param bLocked - Whether the item should be locked
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Lock")
	void SetItemLockState(EBagSlot BagSlot, int32 TopLeft, bool bLocked);

	/**
	 * Update the durability of an item in a specific bag at a specific position
	 * This is more efficient than removing and re-adding the item
	 * Authority check is performed inside the function
	 * @param BagSlot - The bag containing the item
	 * @param TopLeft - The grid position of the item
	 * @param ItemID - The item ID to verify we're updating the correct item
	 * @param NewDurability - The new durability value to set
	 * @return True if the item was found and updated, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Durability")
	bool UpdateItemDurability(EBagSlot BagSlot, int32 TopLeft, int32 ItemID, float NewDurability);

	//Setup bag info
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void BagSet(EBagSlot ConsideredBag, bool InputValidity = false, int32 InputWidth = 0, int32 InputHeight = 0,
	            EItemSize InputMaxStoreSize = EItemSize::Giant, float WeightReduction = 1.f);

	/// @brief Initialize the bag with the given quiver parameters
	/// This function is only used by the quiver item, it will set the ammo type limitation
	/// If the ammo type is unknown, it will set the bag as a normal bag
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void QuiverSpecificSetup(EBagSlot ConsideredBag = EBagSlot::Quiver, EAmmoType NewAmmoType = EAmmoType::Unknown);

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
