// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "EquipmentInterface.h"
#include "InventoryHUDInterface.h"
#include "Components/CoinComponent.h"
#include "Components/InventoryComponent.h"
#include "CoinValue.h"
#include "Components/StagingAreaComponent.h"
#include "UI/WeightWidget.h"
#include "UObject/Interface.h"
#include "Items/InventoryItemBase.h"
#include "InventoryPlayerInterface.generated.h"

class UKeyringComponent;
class UBankComponent;
// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInventoryPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * @*/
class INVENTORYPLUGIN_API IInventoryPlayerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * @brief Retrieves the inventory component associated with the object.
	 *
	 * This method is virtual and must be implemented by derived classes.
	 *
	 * @return A pointer to the UInventoryComponent object, or nullptr if the object has no inventory component.
	 *
	 * @par Usage Example:
	 * @code
	 * UInventoryComponent* InventoryComponent = GetInventoryComponent();
	 * if (InventoryComponent != nullptr)
	 * {
	 *     // Perform operations on the inventory component
	 * }
	 * else
	 * {
	 *     // Object has no inventory component
	 * }
	 * @endcode
	 */
	virtual UInventoryComponent* GetInventoryComponent() = 0;
	/**
	 *  @brief Retrieves a constant pointer to the inventory component.
	 *
	 *  This method returns a constant pointer to the inventory component object which
	 *  provides access to the inventory functionality.
	 *
	 *  @return A constant pointer to the UInventoryComponent object. This pointer can be used
	 *  to access the inventory functionality but cannot be used to modify the component.
	 *
	 *  @note To modify the component, use the non-const version of this method: GetInventoryComponent().
	 */
	virtual const UInventoryComponent* GetInventoryComponentConst() const = 0;

	/**
	 * @brief Get the Coin Component object
	 *
	 * This function is used to get the Coin Component object.
	 *
	 * @note The function is pure virtual and needs to be implemented by derived classes.
	 *
	 * @return UCoinComponent* A pointer to the Coin Component object.
	 */
	virtual UCoinComponent* GetCoinComponent() = 0;
	/**
	 * @brief Retrieves the constant pointer to the coin component of the object.
	 *
	 * This method returns a constant pointer to the `UCoinComponent` of the object.
	 *
	 * @return A constant pointer to the coin component.
	 * @see UCoinComponent
	 */
	virtual const UCoinComponent* GetCoinComponentConst() const = 0;

	/**
	 * @brief Get the actor that owns the inventory.
	 *
	 * This method returns a pointer to the actor that owns the inventory.
	 * The owning actor is responsible for managing the inventory and its items.
	 *
	 * @return A pointer to the owning actor.
	 * @note This method is virtual and must be implemented by child classes.
	 */
	virtual AActor* GetInventoryOwningActor() = 0;
	/**
	 * @brief Get the owning actor of the inventory.
	 *
	 * This function returns a pointer to the AActor class which owns the inventory.
	 * The pointer is constant, meaning it cannot be used to modify the owning actor.
	 *
	 * @return A constant pointer to the owning actor of the inventory.
	 */
	virtual AActor const* GetInventoryOwningActorConst() const = 0;

	/**
	 * @brief Retrieves the boolean value associated with a transaction.
	 *
	 * This virtual function should be implemented by derived classes to provide
	 * the boolean value of a transaction.
	 *
	 * @return The boolean value associated with the transaction.
	 */
	virtual bool GetTransactionBoolean() = 0;
	/**
	 * @brief A pure virtual function to set a transaction boolean value.
	 *
	 * @param Value The boolean value to be set.
	 */
	virtual void SetTransactionBoolean(bool Value) = 0;

	/**
	 * @brief Retrieves the merchant actor of the current instance.
	 *
	 * This method returns a pointer to the merchant actor that represents the merchant in the game.
	 *
	 * @return A pointer to the merchant actor.
	 *
	 * @see AActor
	 */
	virtual AActor* GetMerchantActor() = 0;
	/**
	 * @brief Retrieves the const merchant actor.
	 *
	 * @return The const merchant actor.
	 */
	virtual const AActor* GetMerchantActorConst() const = 0;
	/**
	 * \brief Sets the merchant actor for interaction.
	 *
	 * This method sets the merchant actor that the player can interact with.
	 *
	 * \param Actor The actor representing the merchant.
	 *
	 * \return void
	 */
	virtual void SetMerchantActor(AActor* Actor) = 0;

	/**
	 * @brief Retrieves the actor that has been looted.
	 *
	 * This is a pure virtual function that must be implemented by a derived class.
	 *
	 * @return A pointer to the looted actor.
	 */
	virtual AActor* GetLootedActor() = 0;
	/**
	 * @brief Retrieves the currently looted actor.
	 *
	 * @return The currently looted actor as a constant AActor pointer.
	 */
	virtual const AActor* GetLootedActorConst() const = 0;
	/**
	 * \brief Sets the looted actor for the current instance.
	 *
	 * This method is a pure virtual function that needs to be implemented by the derived classes.
	 *
	 * \param Actor A pointer to the actor that represents the loot being interacted with.
	 *              This can be nullptr if there is no actor to associate with the loot.
	 *
	 * \remarks The implementation of this method should handle the logic of setting the looted actor,
	 *          such as storing it in a variable or updating any relevant data structures.
	 *
	 * \see GetLootedActor()
	 */
	virtual void SetLootedActor(AActor* Actor) = 0;

	/**
	 * Retrieves the interface for controlling the inventory HUD.
	 *
	 * @return A pointer to the Inventory HUD interface.
	 */
	virtual IInventoryHUDInterface* GetInventoryHUDInterface() = 0;
	/**
	 * This method retrieves the Inventory HUD object.
	 *
	 * @return The Inventory HUD object as a UObject pointer.
	 */
	virtual UObject* GetInventoryHUDObject() = 0;

	/**
	 * @brief Retrieves the staging area coin component.
	 *
	 * This method returns a pointer to the staging area coin component of the class.
	 *
	 * @return A pointer to the staging area coin component.
	 *
	 * @note The return value may be nullptr if the staging area coin component is not set.
	 */
	virtual UCoinComponent* GetStagingAreaCoin() = 0;
	/**
	 * @brief Retrieves the staging area items.
	 *
	 * This method returns the UStagingAreaComponent object representing the staging area items.
	 *
	 * @return The staging area items as a UStagingAreaComponent object.
	 */
	virtual UStagingAreaComponent* GetStagingAreaItems() = 0;

	/**
	 * @brief Retrieves the bank coin of the player.
	 *
	 * This method is used to retrieve the UCoinComponent representing the bank coin
	 * of the player. The bank coin is a component used in the player's inventory system
	 * to store virtual currency.
	 *
	 * @return Pointer to the UCoinComponent representing the player's bank coin.
	 *         Returns nullptr if no bank coin is found.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Bank")
	virtual UCoinComponent* GetBankCoin() const;

	/**
	 * @brief Get the bank component of the inventory player.
	 *
	 * @return UBankComponent* Pointer to the bank component. Returns nullptr if the bank component is not found.
	 *
	 * @note This method is implemented by classes that implement the IInventoryPlayerInterface interface.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Bank")
	virtual UBankComponent* GetBankComponent() const;

	/**
	 * @brief Gets the keyring component of the inventory player.
	 *
	 * This method returns the keyring component of the inventory player.
	 *
	 * @return The keyring component of the inventory player.
	 *
	 * @see UKeyringComponent
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Keys")
	virtual UKeyringComponent* GetKeyring() const;

	//------------------------------------------------------------------------------------------------------------------
	// Equipment
	/**
	 * @brief Unequips an item from the player's inventory.
	 *
	 * This method is responsible for unequipping an item from the player's inventory.
	 * It takes in the top left slot index, the bag slot, item ID, and the equipment slot
	 * where the item will be unequipped. It then checks if a transaction is already in progress
	 * and if it is, the method returns without doing anything. If a transaction is not in progress,
	 * it sets the transaction boolean to true and calls the server method Server_PlayerUnequipItem
	 * passing in the given parameters.
	 *
	 * @param InTopLeft The top left slot index.
	 * @param InSlot The bag slot where the item is currently equipped.
	 * @param InItemId The item ID of the item to be unequipped.
	 * @param OutSlot The equipment slot where the item will be unequipped.
	 * @return None.
	 */

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	virtual void PlayerUnequipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, EEquipmentSlot OutSlot);

	/**
	 * \brief Equips an item from the inventory to the specified equipment slot.
	 *
	 * This function equips an item from the player's inventory to the specified equipment slot.
	 * The item is identified by its unique item ID. The equipment slot is specified by the enumeration EEquipmentSlot.
	 * Upon successful execution, the previous item in the equipment slot is unequipped and moved to the specified bag slot.
	 *
	 * \param InItemId The unique ID of the item to be equipped.
	 * \param InSlot The equipment slot where the item should be equipped.
	 * \param OutTopLeft The top-left position of the bag slot where the previous item in the equipment slot should be moved to.
	 * \param OutSlot The bag slot where the previous item in the equipment slot should be moved to.
	 *
	 * \return None.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	virtual void PlayerEquipItemFromInventory(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft,
	                                          EBagSlot OutSlot);

	/**
	 * @brief Swaps equipment items between slots for the player character.
	 *
	 * This method is invoked to swap equipment items between slots for the player character. It takes in the IDs of the dropped
	 * item and the swapped item, as well as the equipment slots they are being dropped and dragged out from. The swap is performed
	 * by calling the Server_PlayerSwapEquipment method.
	 *
	 * @param DroppedItemId The ID of the dropped item.
	 * @param DroppedInSlot The equipment slot where the dropped item is being placed.
	 * @param SwappedItemId The ID of the swapped item.
	 * @param DraggedOutSlot The equipment slot from where the swapped item is being dragged out.
	 *
	 * @see Server_PlayerSwapEquipment
	 */
	virtual void PlayerSwapEquipment(int32 DroppedItemId, EEquipmentSlot DroppedInSlot, int32 SwappedItemId,
	                                 EEquipmentSlot DraggedOutSlot);

	//------------------------------------------------------------------------------------------------------------------
	// Helpers
	/**
	 * Determines if the player can put an item somewhere in their inventory.
	 *
	 * @param ItemID - The ID of the item to be put.
	 * @return True if the player can put the item somewhere, false otherwise.
	 */

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool PlayerCanPutItemSomewhere(int32 ItemID);

	/**
	 * Determines whether the player can pay the specified amount using their inventory.
	 *
	 * @param CoinValue The amount to be paid, represented by a FCoinValue struct.
	 * @return true if the player can pay the amount with the items in their inventory, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool PlayerCanPayAmount(const FCoinValue& CoinValue) const;

	/**
	 * Tries to automatically equip an item for the player.
	 *
	 * @param InItemId The ID of the item to be equipped.
	 * @param PossibleEquipment [out] The equipment slot where the item can be equipped.
	 * @return True if the item was successfully equipped, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool PlayerTryAutoEquip(int32 InItemId, EEquipmentSlot& PossibleEquipment);

	/**
	 * @brief Automatically equips an item for the player.
	 *
	 * This method is used to automatically equip an item for the player.
	 *
	 * @param InTopLeft The top left coordinate of the item in the inventory grid.
	 * @param InSlot The slot where the item will be equipped.
	 * @param InItemId The ID of the item to be equipped.
	 *
	 * @note This method can only be called by the player and is part of the inventory functionality.
	 * @note If another transaction is in progress, the method will return without doing anything.
	 *
	 * @see IInventoryPlayerInterface::GetTransactionBoolean, IInventoryPlayerInterface::SetTransactionBoolean, IInventoryPlayerInterface::Server_PlayerAutoEquipItem
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void PlayerAutoEquipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId);

	/**
	 * @brief Resets the current transaction.
	 *
	 * This method sets the transaction boolean to false, indicating that the current transaction
	 * has been reset.
	 *
	 * @param None
	 * @return None
	 */
	void ResetTransaction();

	/**
	 * @brief Get the equipment for the inventory.
	 *
	 * @return Pointer to an object implementing the IEquipmentInterface.
	 */
	IEquipmentInterface* GetEquipmentForInventory();

	/**
	 * @brief Get the weight changed delegate.
	 *
	 * This method returns a reference to a weight changed delegate.
	 *
	 * @return A reference to the weight changed delegate.
	 */
	virtual FOnWeightChanged& GetWeightChangedDelegate() = 0;

	//------------------------------------------------------------------------------------------------------------------
	// Inventory
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * Retrieves all items from the player's inventory.
	 *
	 * @return An array containing pointers to all the items in the inventory.
	 */
	virtual TArray<const UInventoryItemBase*> GetAllItems() const;

	/**
	 * Retrieves all items in the specified bag slot.
	 *
	 * @param Slot The bag slot to retrieve items from.
	 * @return An array of FMinimalItemStorage objects representing the items in the bag slot.
	 * @see FMinimalItemStorage
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual const TArray<FMinimalItemStorage>& GetAllItemsInBag(EBagSlot Slot) const;

	/**
	 * Checks if the specified equipment slot can be unequipped.
	 *
	 * @param Slot The equipment slot to check for unequippability.
	 * @return True if the slot can be unequipped, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool CanUnequipBag(EEquipmentSlot Slot) const;

	/**
	 * PlayerTryAutoLootFunction is a method of the IInventoryPlayerInterface interface.
	 * It is used to determine if an item can be auto-looted and, if so, find the suitable equipment and bag slots to store the item.
	 *
	 * @param InItemId The ID of the item to be auto-looted.
	 * @param PossibleEquipment The variable that will be set to the suitable equipment slot for the item.
	 * @param InTopLeft The variable that will be set to the top-left position in the bag slot for the item.
	 * @param PossibleBag The variable that will be set to the suitable bag slot for the item.
	 * @return True if the auto-loot is successful and suitable slots are found, otherwise false.
	 */
	UFUNCTION()
	virtual bool PlayerTryAutoLootFunction(int32 InItemId, EEquipmentSlot& PossibleEquipment, int32& InTopLeft,
	                                       EBagSlot& PossibleBag);

	/**
	 * @brief Checks if the player has a specific item.
	 *
	 * This method is used to determine if the player possesses a specified item given its item ID.
	 *
	 * @param ItemId The ID of the item to check for.
	 *
	 * @return True if the player has the item, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool PlayerHasItem(int32 ItemId);

	/**
	 * @brief Checks if the player has at least a specific amount of an item.
	 *
	 * @param ItemId The ID of the item to check for.
	 * @param ItemAmounts The ID of the item to check for.
	 *
	 * @return True if the player has the item, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool PlayerHasItems(int32 ItemId, int32 ItemAmount);

	/**
	 * @brief Checks if the player has any of the specific items.
	 *
	 * This method is used to determine if the player possesses any of the specific items given their item ID.
	 *
	 * @param ItemId The ID of the items to check for.
	 *
	 * @return True if the player has any of the items, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool PlayerHasAnyItem(const TArray<int32>& ItemID);

	/**
	 * Removes an item from the player's inventory if it is possible.
	 *
	 * @param ItemID The ID of the item to be removed.
	 *
	 * @return True if the item was successfully removed, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool PlayerRemoveItemIfPossible(int32 ItemID);

	/**
	 * @brief Removes any item from the player's inventory if possible.
	 *
	 * This method removes any item specified by the ItemID from the player's inventory.
	 *
	 * @param ItemID The IDs of the items to be removed.
	 * @return True if any item is successfully removed, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool PlayerRemoveAnyItemIfPossible(const TArray<int32>& ItemID);

	/**
	 * Adds an item to the player's inventory.
	 *
	 * This method is responsible for adding an item to the player's inventory. The item will be added at the specified location within the inventory grid.
	 *
	 * @param InTopLeft The top left corner of the grid location where the item will be added.
	 * @param InSlot The slot in which the item will be added. It can be one of the EBagSlot enumerated values.
	 * @param InItemId The ID of the item to be added.
	 *
	 * @remark This method will only execute if the inventory owning actor has authority.
	 *
	 * @see IInventoryPlayerInterface::GetInventoryOwningActor
	 * @see UInventoryComponent::AddItemAt
	 * @see UBankComponent::AddItem
	 */
	UFUNCTION()
	virtual void PlayerAddItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId);

	/**
	 * \brief Removes an item from the player's inventory or bank.
	 *
	 * This function checks if the owning actor has authority before removing the item.
	 *
	 * If the slot is EBagSlot::BankPool, the item is removed from the bank component using the TopLeft index.
	 * Otherwise, the item is removed from the inventory component using the specified slot and TopLeft index.
	 *
	 * \param TopLeft The index of the item to be removed.
	 * \param Slot The slot from which the item is to be removed.
	 *
	 * \return None.
	 */
	UFUNCTION()
	virtual void PlayerRemoveItem(int32 TopLeft, EBagSlot Slot);

	/**
	 * Retrieves the item at the specified position in the player's inventory.
	 *
	 * @param TopLeft The position of the item in the inventory.
	 * @param Slot The bag slot where the item is located.
	 *
	 * @return The ID of the item at the specified position. If the specified slot is the BankPool, returns the item ID in the player's bank.
	 *         If the specified slot is not the BankPool, returns the item ID in the player's inventory at the specified position.
	 */
	UFUNCTION()
	virtual int32 PlayerGetItem(int32 TopLeft, EBagSlot Slot) const;

	/**
	 * @brief Moves an item from one slot to another in the player's inventory.
	 *
	 * @param InTopLeft  The top left index of the grid where the item is located.
	 * @param InSlot     The slot in the grid where the item is located.
	 * @param InItemId   The unique ID of the item to be moved.
	 * @param OutTopLeft The top left index of the grid where the item should be moved.
	 * @param OutSlot    The slot in the grid where the item should be moved.
	 *
	 * @see IInventoryPlayerInterface::Server_PlayerMoveItem()
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void PlayerMoveItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft, EBagSlot OutSlot);

	/**
	 * Transfers coins from one coin component to another.
	 *
	 * @param GivingComponent - The coin component from which coins are being transferred.
	 * @param ReceivingComponent - The coin component receiving the transferred coins.
	 * @param RemovedCoinValue - The value of coins being removed from the giving component.
	 * @param AddedCoinValue - The value of coins being added to the receiving component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void TransferCoinTo(UCoinComponent* GivingComponent, UCoinComponent* ReceivingComponent,
	                            const FCoinValue& RemovedCoinValue, const FCoinValue& AddedCoinValue);

	/**
	 * Cancels the staging area for the inventory player.
	 *
	 * This method is used to cancel the staging area for the inventory player. It will clear any items that were staged for placement in the inventory.
	 *
	 * @param None
	 *
	 * @return None
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void CancelStagingArea();

	/**
	 * Transfers staged inventory items to the specified target actor.
	 *
	 * @param TargetActor The actor to transfer the inventory items to
	 * @note This function is callable from Blueprint and belongs to the "Inventory" category.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void TransferStagingToActor(AActor* TargetActor);

	/**
	 * Moves equipment to the staging area.
	 *
	 * This function is used to move equipment from the player's inventory to the staging area.
	 * The staging area is a temporary storage for equipment that is waiting to be equipped.
	 * The equipment is specified by the item ID and the equipment slot where it should be placed.
	 *
	 * @param InItemId The ID of the equipment item to be moved.
	 * @param OutSlot The equipment slot where the item should be placed in the staging area.
	 *
	 * @see Server_MoveEquipmentToStagingArea()
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void MoveEquipmentToStagingArea(int32 InItemId, EEquipmentSlot OutSlot);

	/**
	 * Moves an inventory item to the staging area.
	 *
	 * @param InItemId The ID of the inventory item to be moved.
	 * @param OutTopLeft The top left position in the staging area to move the item to.
	 * @param OutSlot The bag slot in the staging area to move the item to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void MoveInventoryItemToStagingArea(int32 InItemId, int32 OutTopLeft, EBagSlot OutSlot);

	/**
	 * Handles the activation of an item in the player's inventory.
	 *
	 * This function is used to handle the activation of an item based on its ID, top left position, and bag slot.
	 *
	 * @param ItemID The ID of the item being activated.
	 *
	 * @param TopLeft The top left position of the item in the player's inventory.
	 *
	 * @param BagSlot The bag slot in which the item is located.
	 *
	 * @note This function must be implemented in classes that inherit from IInventoryPlayerInterface.
	 *
	 * @see IInventoryPlayerInterface
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void HandleActivation(int32 ItemID, int32 TopLeft, EBagSlot BagSlot);

	//------------------------------------------------------------------------------------------------------------------
	// Loot -- Client
	/**
	 * Checks if the player is currently looting.
	 *
	 * @return true if the player is currently looting, false otherwise.
	 */

	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual bool IsLooting() const;

	/**
	 * \brief Starts looting the given Actor.
	 *
	 * This method is responsible for initiating the looting process for a specific Actor.
	 *
	 * \param Actor The Actor to be looted.
	 * \return None.
	 *
	 * \details This method first checks if the given Actor is valid and implements the ILootableInterface.
	 * If so, it further checks if the looting process is not already underway for another Actor and if the given Actor is not being looted at the moment.
	 * If these conditions are met, the method triggers the Server_LootActor method to start the looting process on the server.
	 *
	 * \note This method is a BlueprintCallable function.
	 * \note This method is part of the Inventory|Loot category.
	 * \note This method is a virtual function.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void StartLooting(AActor* Actor);

	/**
	 * @brief Stops looting an actor.
	 *
	 * This method is used to stop looting an actor. If the specified actor is being looted or no actor is specified, the looting process will be stopped.
	 *
	 * @param Actor The actor being looted. If nullptr, the currently looted actor will be stopped.
	 *
	 * @return void
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void StopLooting(AActor* Actor = nullptr);

	/**
	 * @brief Automatically loots an item for the player.
	 *
	 * This function attempts to automatically loot an item for the player. It checks if the player is currently in a transaction and returns if true.
	 * The function then calls PlayerTryAutoLootFunction to determine the slot and bag to place the item in.
	 * If a valid equipment slot is returned, the function sets the transaction boolean to true and calls Server_PlayerEquipItemFromLoot to equip the item from the loot.
	 * If a valid bag slot is returned, the function sets the transaction boolean to true and calls Server_PlayerLootItem to transfer the item to the specified bag.
	 *
	 * @param InItemId The ID of the item to be looted.
	 * @param OutTopLeft The top-left position of the item in the slot or bag.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void PlayerAutoLootItem(int32 InItemId, int32 OutTopLeft);

	/**
	 * @brief Automatically loots all items for the player.
	 *
	 * This function is used to automatically loot all items for the player.
	 *
	 * @param None
	 *
	 * @return None
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void PlayerAutoLootAll();

	/**
	 *
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void PlayerLootItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft);

	/**
	 * PlayerEquipItemFromLoot method is used to equip an item from the loot to the player's inventory.
	 *
	 * @param InItemId - The unique identifier of the item to be equipped.
	 * @param InSlot - The equipment slot where the item should be equipped.
	 * @param OutTopLeft - The position of the top-left corner of the item in the loot interface.
	 *
	 * @return None.
	 *
	 * @see IInventoryPlayerInterface, EEquipmentSlot
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void PlayerEquipItemFromLoot(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft);

	//------------------------------------------------------------------------------------------------------------------
	// Merchant -- Client
	/**
	 * @brief Checks if the player is currently trading.
	 *
	 * @return true if the player is trading, false otherwise.
	 */

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual bool IsTrading() const;

	/**
	 * Tries to present and sell an item in the merchant's inventory.
	 *
	 * @param OutSlot The slot where the purchased item will be placed.
	 * @param ItemId The ID of the item being sold.
	 * @param TopLeft The reference position where the item should be presented.
	 *
	 * This method is responsible for handling the logic of presenting and selling an item in the merchant's inventory.
	 * It forwards the request to the InventoryHUDInterface::TryPresentSellItem method, passing the necessary arguments.
	 *
	 * @see InventoryHUDInterface::TryPresentSellItem
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	virtual void TryPresentSellItem(EBagSlot OutSlot, int32 ItemId, int32 TopLeft);

	/**
	 * @brief Resets the sell item in the inventory merchant category.
	 *
	 * This function is a BlueprintCallable function that can be accessed in Blueprint scripts.
	 * It is used to reset the sell item in the inventory merchant category.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	virtual void ResetSellItem();

	/**
	 * @brief Initiates a purchase transaction for the specified item from a merchant.
	 *
	 * This method allows the player to buy an item from a merchant. The item to be purchased is identified by its unique ID and the price of the item is specified using the FCoinValue struct
	 *. The purchase transaction is initiated by calling the Server_PlayerBuyFromMerchant method on the server-side.
	 *
	 * @param ItemId The unique ID of the item to be purchased.
	 * @param Price The price of the item specified using the FCoinValue struct.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void PlayerBuyFromMerchant(int32 ItemId, const FCoinValue& Price);

	/**
	 * @brief Function called when a player sells an item to a merchant's inventory.
	 *
	 * This function is responsible for handling the player's request to sell an item to a merchant. It will invoke the
	 * server-side function `Server_PlayerSellToMerchant` to perform the actual selling process.
	 *
	 * @param OutSlot The bag slot from which the item will be removed.
	 * @param ItemId The ID of the item to be sold.
	 * @param TopLeft The top-left index of the rectangular region representing the bag slot.
	 * @param Price The price at which the item will be sold, represented by a `FCoinValue` structure.
	 *
	 * @see Server_PlayerSellToMerchant
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void PlayerSellToMerchant(EBagSlot OutSlot, int32 ItemId, int32 TopLeft, const FCoinValue& Price);

	/**
	 * @brief Initiates a trade with a merchant actor.
	 *
	 * This method is used to start a trade process with a merchant actor. The specified merchant actor will be notified that a trade has been initiated.
	 *
	 * @param InputMerchantActor The merchant actor with whom the trade is being initiated.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void MerchantTrade(AActor* InputMerchantActor);

	/**
	 * @brief Stops the merchant trade.
	 *
	 * This function is used to stop the merchant trade.
	 *
	 * @param None
	 *
	 * @return None
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	virtual void StopMerchantTrade();

	/**
	 * \brief Calculate the total weight of the player's inventory.
	 *
	 * This method calculates the total weight of the player's inventory by summing up the weights
	 * of the inventory items, equipment items, and coins.
	 *
	 * \return The total weight of the player's inventory.
	 *
	 * \note The weight is returned as a float value.
	 */
	float GetTotalWeight();

	/**
	 * @brief Adds a key from inventory to the player.
	 *
	 * This function is used to add a key to the player from their inventory. The key is removed from the inventory
	 * if it is successfully added to the player's keyring.
	 *
	 * @param InTopLeft The top left index of the item in the inventory.
	 * @param InSlot The slot identifier of the item in the inventory.
	 * @param InItemId The ID of the item to be added as a key.
	 */
	// Keys
	//------------------------------------------------------------------------------------------------------------------
	UFUNCTION()
	virtual void Internal_PlayerAddKeyFromInventory(int32 InTopLeft, EBagSlot InSlot, int32 InItemId);

	/**
	 * @brief Removes a key from the player's inventory.
	 *
	 * This method is called when the player wants to remove a specific key from their inventory.
	 * The key is identified by the provided KeyId.
	 *
	 * @param KeyId The ID of the key to be removed.
	 *
	 * @note This method only executes if the owning actor has authority, the player has a keyring, and the key exists in the keyring.
	 * Additionally, the method attempts to automatically loot the item associated with the key, if possible.
	 * If the item cannot be looted or the destination bag is unknown, no action is taken.
	 */
	UFUNCTION()
	virtual void Internal_PlayerRemoveKeyToInventory(int32 KeyId);

	/**
	 * @brief PlayerAddKeyFromInventory function is used to add a key from inventory to the player.
	 *
	 * @param InTopLeft The top left position of the inventory where the key is located.
	 * @param InSlot The slot in the inventory where the key is located.
	 * @param InItemId The ID of the key to be added.
	 */
	UFUNCTION(BlueprintCallable)
	virtual void PlayerAddKeyFromInventory(int32 InTopLeft, EBagSlot InSlot, int32 InItemId);

	/**
	 * @brief Removes a key from the player's inventory.
	 *
	 * This function is a blueprint callable function that removes a key from the player's inventory.
	 *
	 * @param KeyId The ID of the key to be removed from the inventory.
	 *
	 * @note This function is an interface function and should be implemented in classes that inherit from `IInventoryPlayerInterface`.
	 */
	UFUNCTION(BlueprintCallable)
	virtual void PlayerRemoveKeyToInventory(int32 KeyId);

	UFUNCTION(BlueprintCallable)
	virtual void DisplayItemDescription(const UInventoryItemBase* Item, float X, float Y);

	UFUNCTION(Blueprintable)
	virtual bool TryToEat();

	UFUNCTION(Blueprintable)
	virtual bool TryToDrink();

protected:
	//------------------------------------------------------------------------------------------------------------------
	// Merchant -- Server
	//------------------------------------------------------------------------------------------------------------------
	//UFUNCTION(Server, Reliable, Category = "Inventory|Merchant")
	virtual void Server_MerchantTrade(AActor* InputMerchantActor) = 0;

	//UFUNCTION(Server, Reliable, Category = "Inventory|Merchant")
	virtual void Server_StopMerchantTrade() = 0;

	//Buy selected stuff from merchant
	//UFUNCTION(Server, Reliable, WithValidation,  Category = "Inventory|Merchant")
	virtual void Server_PlayerBuyFromMerchant(int32 ItemId, const FCoinValue& Price) = 0;

	//Sell selected stuff to merchant
	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Merchant")
	virtual void Server_PlayerSellToMerchant(EBagSlot OutSlot, int32 ItemId, int32 TopLeft, const FCoinValue& Price) =
	0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	virtual void Server_PlayerAutoEquipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId) = 0;

	//------------------------------------------------------------------------------------------------------------------
	// Loot -- Server
	//------------------------------------------------------------------------------------------------------------------
	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Loot")
	virtual void Server_LootActor(AActor* InputLootedActor) = 0;

	//UFUNCTION(Server, Reliable, Category = "Inventory|Loot")
	virtual void Server_StopLooting() = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Loot")
	virtual void Server_PlayerLootItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft) = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Loot")
	virtual void Server_PlayerEquipItemFromLoot(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft) = 0;

	//Try to loot all items in the lootPool, return false is at least one item cannot be looted
	//UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory|Loot")
	virtual void Server_PlayerAutoLootAll() = 0;

	//------------------------------------------------------------------------------------------------------------------
	// Inventory -- Server
	//------------------------------------------------------------------------------------------------------------------
	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	virtual void Server_PlayerMoveItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, int32 OutTopLeft,
	                                   EBagSlot OutSlot) = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Equipment")
	virtual void Server_PlayerUnequipItem(int32 InTopLeft, EBagSlot InSlot, int32 InItemId, EEquipmentSlot OutSlot) = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Equipment")
	virtual void Server_PlayerEquipItemFromInventory(int32 InItemId, EEquipmentSlot InSlot, int32 OutTopLeft,
	                                                 EBagSlot OutSlot) = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Equipment")
	virtual void Server_PlayerSwapEquipment(int32 DroppedItemId, EEquipmentSlot DroppedInSlot, int32 SwappedItemId,
	                                        EEquipmentSlot DraggedOutSlot) = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	virtual void Server_TransferCoinTo(UCoinComponent* GivingComponent, UCoinComponent* ReceivingComponent,
	                                   const FCoinValue& RemovedCoinValue, const FCoinValue& AddedCoinValue) = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	virtual void Server_DropItemFromInventory(int32 TopLeft, EBagSlot Slot, FVector DropLocation = {}) = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	virtual void Server_DropItemFromEquipment(EEquipmentSlot Slot, FVector DropLocation = {}) = 0;

	//------------------------------------------------------------------------------------------------------------------
	// Staging -- Server
	//------------------------------------------------------------------------------------------------------------------

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Staging")
	virtual void Server_CancelStagingArea() = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Staging")
	virtual void Server_TransferStagingToActor(AActor* TargetActor) = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Staging")
	virtual void Server_MoveEquipmentToStagingArea(int32 InItemId, EEquipmentSlot OutSlot) = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Staging")
	virtual void Server_MoveInventoryItemToStagingArea(int32 InItemId, int32 OutTopLeft, EBagSlot OutSlot) = 0;

	//------------------------------------------------------------------------------------------------------------------
	// Keys
	//------------------------------------------------------------------------------------------------------------------
	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Key")
	virtual void Server_PlayerAddKeyFromInventory(int32 InTopLeft, EBagSlot InSlot, int32 InItemId) = 0;

	//UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Key")
	virtual void Server_PlayerRemoveKeyToInventory(int32 KeyId) = 0;
};
