// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "Components/ActorComponent.h"
#include "BankComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBankPoolChangedDelegate);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBankItemAdd, int32, ItemID, int32, TopLeftIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBankItemRemove, int32, TopLeftIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBankReorganize);

/**
 * UBankComponent is a blueprint spawnable component that represents a bank inventory system.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API UBankComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	/**
	 * @brief Called when the game starts or when spawned.
	 *
	 * This method is called when the game starts or when the component is spawned. It is usually used to initialize any necessary variables or perform any required setup actions.
	 *
	 */
	virtual void BeginPlay() override;

	/**
	 * @brief Array of items stored in the bank pool.
	 *
	 * This array represents the items stored in the bank pool. Each element of the array is of type FMinimalItemStorage.
	 *
	 * @see FMinimalItemStorage
	 *
	 * @note This variable is replicated using the OnRep_BankPool function.
	 * @note This variable is read and written in blueprints.
	 * @note This variable belongs to the Inventory|BankPool category.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_BankPool, BlueprintReadWrite, Category = "Inventory|BankPool")
	TArray<FMinimalItemStorage> Items;

	/**
	 * @var LocalBagSlot
	 * @brief The variable representing the current bag slot in the local inventory.
	 *
	 * LocalBagSlot is a property that determines the bag slot in the local inventory for the player.
	 * It is defined as an EditAnywhere property, allowing it to be modified in the editor.
	 * It is also marked as BlueprintReadOnly, indicating that it can only be accessed for reading, but not for writing.
	 * Moreover, it is categorized under "Inventory|BankPool" category.
	 *
	 * This variable is of type EBagSlot, which is an enumeration representing different bag slots in the inventory.
	 * The initial value assigned to LocalBagSlot is EBagSlot::BankPool, indicating that it represents the bank pool bag slot.
	 *
	 * Example usage:
	 *     LocalBagSlot = EBagSlot::PlayerInventory;
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|BankPool")
	EBagSlot LocalBagSlot = EBagSlot::BankPool;

	uint32 Width = 8;

	uint32 Height = 16;


public:
	/**
	 *
	 */
	UBankComponent();

	/**
	 * @class BankPoolDispatcher
	 * @brief Variable that represents a delegate that is used to dispatch events when the bank pool has changed.
	 *
	 * @details This variable is used in Blueprint to assign a function or event that will be triggered when the bank pool has changed. It is of type FOnBankPoolChangedDelegate and can only
	 * be accessed from Blueprint.
	 *          The FOnBankPoolChangedDelegate is a delegate type that encapsulates a function signature. It provides a way to bind multiple functions to a single delegate and execute them
	 * when the delegate is called.
	 *          When the bank pool has changed, the assigned function or event will be triggered, allowing the user to perform any necessary actions or updates.
	 *
	 * @see FOnBankPoolChangedDelegate
	 * @see BlueprintAssignable
	 */
	UPROPERTY(BlueprintAssignable)
	FOnBankPoolChangedDelegate BankPoolDispatcher;

	/**
	 * @class BankItemAddDispatcher
	 * @brief Dispatches events when a bank item is added.
	 *
	 * The BankItemAddDispatcher is a blueprint assignable property that allows for dispatching events
	 * when a bank item is added. Only the blueprint authority can assign and broadcast events through this dispatcher.
	 *
	 * Example Usage:
	 *
	 * BankItemAddDispatcher.AddDynamic(this, &MyClass::OnBankItemAdded);
	 * BankItemAddDispatcher.Broadcast(Item);
	 *
	 * Note: This variable is a part of a Blueprint object and should be used accordingly.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly)
	FBankItemAdd BankItemAddDispatcher;

	/**
	 * @brief A dispatcher for handling bank item removal events.
	 *
	 * This dispatcher allows binding and triggering events related to the removal of bank items.
	 * Only users with authority are able to execute these events.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly)
	FBankItemRemove BankItemRemoveDispatcher;

	/**
	 * @brief A variable used for dispatching events related to bank reorganization.
	 *
	 * This variable is of type FOnBankReorganize, which is a BlueprintAssignable event delegate type.
	 * It is used to delegate events related to bank reorganization.
	 * This variable can only be accessed by blueprints with authority.
	 *
	 * @see FOnBankReorganize
	 */
	UPROPERTY(BlueprintAssignable, BlueprintAuthorityOnly)
	FOnBankReorganize BankReorganizeDispatcher;

	/**
	 * @brief Replication event for the BankPool variable.
	 *
	 * This method is called when the BankPool variable is replicated across the network.
	 * It broadcasts the BankPoolDispatcher event, notifying any registered listeners.
	 *
	 * @param None
	 *
	 * @return None
	 */
	UFUNCTION()
	void OnRep_BankPool();

	/**
	 * AddItem function adds an item to the bank pool.
	 *
	 * @param ItemID The ID of the item to be added.
	 * @param TopLeftIndex The top left index of the item in the bank pool.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory|BankPool")
	void AddItem(int32 ItemID, int32 TopLeftIndex);

	/**
	 * @brief Removes an item at the specified index.
	 *
	 * Note: This function is intended to be executed on the server only.
	 *
	 * @param TopLeftIndex The index of the item to remove.
	 *
	 * @return None.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory|BankPool")
	void RemoveItem(int32 TopLeftIndex);

	/**
	 * Reorganizes the bank pool inventory.
	 *
	 * This method is used to reorganize the bank pool inventory. It is marked as a server-side function, reliable, and blueprint callable.
	 * This method belongs to the "Inventory|BankPool" category.
	 *
	 * @param None
	 *
	 * @return None
	 *
	 * @see None
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory|BankPool")
	void Reorganize();

	/**
	 * @brief Retrieves the item ID at the specified index from the inventory bank.
	 *
	 * This function searches for an item in the inventory bank based on the given index. If a matching item is found,
	 * its corresponding item ID is returned. If no item is found, -1 is returned.
	 *
	 * @param ID The index of the item to retrieve.
	 * @return The item ID at the specified index, or -1 if no item is found.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|BankPool")
	int32 GetItemAtIndex(int32 ID) const;

	/**
	 * @brief Get the constant reference to the bag's items.
	 *
	 * @return A constant reference to the array of FMinimalItemStorage representing the bag's items.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|BankPool")
	const TArray<FMinimalItemStorage>& GetBagConst() const { return Items; }

	/**
	 * @brief Checks if the inventory bank pool is empty.
	 *
	 * This function checks if the inventory bank pool is empty by invoking the IsEmpty method on the Items collection.
	 *
	 * @return True if the inventory bank pool is empty, false otherwise.
	 *
	 * @see Items
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|BankPool")
	bool IsEmpty() const { return Items.IsEmpty(); }
};
