#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "UObject/Interface.h"
#include "InventoryGameModeInterface.generated.h"

class ULootPoolComponent;
class ULoreItemManagerComponent;
struct FCoinValue;
class ADroppedCoins;
class ADroppedItem;
class UInventoryItemBase;
// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UInventoryGameModeInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * @interface IInventoryGameModeInterface
 * @brief Interface for a game mode that handles inventory related functionality
 */
class INVENTORYPLUGIN_API IInventoryGameModeInterface
{
	GENERATED_BODY()

public:
	/**
	 * Spawns an item from the given actor at the desired drop location.
	 *
	 * @param SpawningActor  The actor from which the item will be spawned.
	 * @param ItemID         The ID of the item to be spawned.
	 * @param DesiredDropLocation  The desired location where the item should be dropped.
	 * @param ClampOnGround  Whether the item should be clamped to the ground or not. Defaults to true.
	 * @return  A pointer to the spawned ADroppedItem object, or nullptr if spawning failed.
	 */
	virtual ADroppedItem* SpawnItemFromActor(AActor* SpawningActor, uint32 ItemID, const FVector& DesiredDropLocation, bool ClampOnGround = true, float Durability = 100.0f);

	/**
	 * Spawns an item from the given actor using a raw item reference.
	 *
	 * @param SpawningActor  The actor from which the item will be spawned.
	 * @param ItemToSpawn    A pointer to the UInventoryItemBase object representing the item to be spawned.
	 * @param Durability     The durability of the spawned item. Defaults to 100.0f.
	 * @return  A pointer to the spawned ADroppedItem object, or nullptr if spawning failed.
	 */
	virtual ADroppedItem* SpawnItemFromActorRaw(AActor* SpawningActor, UInventoryItemBase* ItemToSpawn, float Durability = 100.0f);

	/**
	 * Spawns coins from an actor at a desired drop location.
	 *
	 * @param SpawningActor The actor from which to spawn the coins.
	 * @param CoinValue The value of the coins to spawn.
	 * @param DesiredDropLocation The desired location where the coins should be dropped.
	 * @param ClampOnGround Optional parameter to specify whether the coins should be clamped on the ground. Defaults to true.
	 *
	 * @return A pointer to the spawned coins.
	 */
	virtual ADroppedCoins* SpawnCoinsFromActor(AActor* SpawningActor, const FCoinValue& CoinValue, const FVector& DesiredDropLocation, bool ClampOnGround = true);

	/**
	 * Calculates the spawn location for an item to be dropped by a spawning actor.
	 *
	 * @param SpawningActor The actor that is spawning the item.
	 * @param DesiredDropLocation The desired drop location for the item.
	 * @param ClampOnGround If true, the spawn location will be clamped on the ground.
	 *
	 * @return The calculated spawn location for the item.
	 */
	virtual FVector GetItemSpawnLocation(AActor* SpawningActor,const FVector& DesiredDropLocation, bool ClampOnGround = true);

	/**
	 * @brief Fetches an inventory item based on the given ID.
	 *
	 * This method should be implemented by subclasses to retrieve an inventory item
	 * using the specified ID.
	 *
	 * @param ID The unique identifier of the inventory item to fetch.
	 * @return The inventory item with the specified ID if found, otherwise null.
	 */
	UFUNCTION(BlueprintCallable)
	virtual UInventoryItemBase* FetchItemFromID(int32 ID) = 0;

	/**
	 * @brief Registers a new inventory item.
	 *
	 * This method registers a new inventory item into the inventory system.
	 *
	 * @param NewItem The pointer to the inventory item to be registered.
	 */
	UFUNCTION(BlueprintCallable)
	virtual void RegisterItem(UInventoryItemBase* NewItem) = 0;

	/**
	 * @brief Determines if a new item can be spawned.
	 *
	 * This method checks if the specified item can be spawned in the game world.
	 * It can be used to enforce lore restrictions or other conditions on item spawning.
	 *
	 * @param NewItem The inventory item to check for spawn eligibility.
	 * @return True if the item can be spawned, false otherwise.
	 */
	UFUNCTION(BlueprintCallable)
	virtual bool CanSpawnItem(UInventoryItemBase* NewItem);

	/**
	 * @brief Validates a lore item after a delay.
	 *
	 * This method is intended to perform delayed validation of lore items, potentially
	 * in relation to loot pools or other game systems. It can be used to ensure that
	 * certain conditions are met before allowing a lore item to be considered valid.
	 *
	 * @param LocalItem The inventory item being validated.
	 * @param Origin The loot pool component that is the origin of the validation request.
	 * @return True if the lore item is valid, false otherwise.
	 */
	virtual bool DelayedLoreItemValidation(const UInventoryItemBase* LocalItem, ULootPoolComponent* Origin);

	/**
	 * @brief Retrieves the lore management component.
	 *
	 * This method returns a pointer to the lore management component, which can be used
	 * to manage lore-related functionality in the game. If no lore management component is present,
	 * this method can return nullptr.
	 *
	 * @return A pointer to the ULoreItemManagerComponent if available, otherwise nullptr.
	 */
	virtual ULoreItemManagerComponent* GetLoreManagementComponent();


	/**
	 * @brief Returns the current inflation value.
	 *
	 * This method return the inflation value. By default, it is 0, so no price increase due to inflation will happen.
	 * If this is set to 1.0, the price of the items will double.
	 * It is probably not wise to go negative.
	 *
	 * @return The actual inflation value.
	 */
	virtual float GetCurrentInflationValue();
};
