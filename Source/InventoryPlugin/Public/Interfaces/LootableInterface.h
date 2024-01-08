// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "CoinValue.h"

#include "UObject/Interface.h"
#include "LootableInterface.generated.h"

class UCoinComponent;
class ULootPoolComponent;
class APlayerCharacter;
class FOnLootPoolChangedDelegate;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class ULootableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * @class ILootableInterface
 *
 * @brief This interface provides functions for interacting with lootable objects.
 */
class INVENTORYPLUGIN_API ILootableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * @brief Retrieves the coin component associated with this object.
	 *
	 * @return The coin component attached to this object. Returns nullptr if no coin component is found.
	 *
	 * @note The returned coin component can be used to manage the number of coins associated with the object.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual UCoinComponent* GetCoinComponent() = 0;
	/**
	 * @brief Retrieves the LootPoolComponent associated with this object.
	 *
	 * @return The LootPoolComponent object.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual ULootPoolComponent* GetLootPoolComponent() = 0;
	/**
	 * Retrieves the constant reference to the CoinComponent.
	 *
	 * @return A constant pointer to the CoinComponent.
	 *
	 * @see UCoinComponent
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual UCoinComponent* GetCoinComponentConst() const = 0;
	/**
	 * @brief Get the loot pool component of the object.
	 *
	 * This method retrieves the loot pool component of the object. The loot pool component
	 * holds all the information about the loot items that can be obtained from the object.
	 *
	 * @return A pointer to the loot pool component. If the object does not have a loot pool component, nullptr is returned.
	 *
	 * @see ULootPoolComponent
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual ULootPoolComponent* GetLootPoolComponentConst() const = 0;

	/**
	 * @brief Check if the item is being looted.
	 *
	 * This function returns a boolean value indicating whether the item is currently being looted or not.
	 *
	 * @return true if the item is being looted, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual bool GetIsBeingLooted() const = 0;
	/**
	 * Sets the loot status of the inventory.
	 *
	 * This method is used to set the loot status of the inventory.
	 * When an inventory is being looted, its loot status is set to true.
	 * Similarly, when the looting is complete, the loot status is set to false.
	 *
	 * @param LootStatus A boolean value indicating whether the inventory is being looted or not.
	 *
	 * @see IsBeingLooted
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void SetIsBeingLooted(bool LootStatus) = 0;

	/**
	 * @brief Determines if the object is destroyable.
	 *
	 * @return true if the object is destroyable, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual bool GetIsDestroyable() const = 0;

	/**
	 * @brief Destroys the loot actor.
	 *
	 * This method should be called to destroy the loot actor. It is a blueprint callable function
	 * that is used to destroy the loot actor in the game. It belongs to the "Inventory|Loot" category.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void DestroyLootActor() = 0;

	/**
	 * Retrieves the name of the loot actor.
	 *
	 * @return The name of the loot actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual FString GetLootActorName() const = 0;

	/**
	 * @brief Initializes the loot pool for the lootable interface.
	 *
	 * This method is used to initialize the loot pool for an object that implements the lootable interface.
	 * The loot pool determines what items can be looted from the object.
	 *
	 * @param LootableItems The array of lootable item IDs to be added to the loot pool.
	 *
	 * @return None.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void InitLootPool(const TArray<int32>& LootableItems);

	/**
	 * @brief Initializes the cash pool with the given coin value.
	 *
	 * This method is responsible for initializing the cash pool with the given coin value.
	 * It adds the specified coin value to the existing cash pool.
	 *
	 * @param CoinValue The coin value to add to the cash pool.
	 *
	 * @note This method is only applicable for objects that implement the ILootableInterface.
	 *
	 * @see ILootableInterface, GetCoinComponent, AddCoins
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void InitCashPool(const FCoinValue& CoinValue);

	/**
	 * Starts the looting process for the specified looter.
	 *
	 * This function is called when a looter actor wants to start looting this instance of a lootable object.
	 * If this lootable object is already being looted, the function simply returns and does nothing.
	 *
	 * @param Looter The actor that wants to start looting this lootable object.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void StartLooting(AActor* Looter);
	/**
	 * Stops the looting process for the specified looter.
	 *
	 * @param Looter The actor that was looting.
	 *
	 * @return None.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void StopLooting(AActor* Looter);

	/**
	 * Returns the item data for the specified TopLeftID.
	 *
	 * @param TopLeftID The ID of the item.
	 * @return The item data of the specified TopLeftID.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual int32 GetItemData(int32 TopLeftID) const;

	/**
	 * \brief Removes an item from the loot pool.
	 *
	 * This method removes an item from the loot pool identified by the given TopLeftID.
	 * The item is removed from the loot pool associated with the lootable object.
	 *
	 * \param TopLeftID The ID of the item to be removed from the loot pool.
	 *
	 * \return None.
	 *
	 * \see ILootableInterface
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
	virtual void RemoveItem(int32 TopLeftID);

	/**
	 * @brief Retrieves the delegate for the loot pool changed event.
	 *
	 * This method returns a reference to the loot pool changed delegate, which can be used to register or unregister
	 * callback functions for when the loot pool changes. The loot pool represents the available items that can be
	 * obtained from a lootable object.
	 *
	 * @return A reference to the loot pool changed delegate.
	 */
	virtual FOnLootPoolChangedDelegate& GetLootPoolDelegate();
};
