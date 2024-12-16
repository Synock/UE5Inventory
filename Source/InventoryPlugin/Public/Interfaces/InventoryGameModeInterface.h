// Copyright 2022 Maximilien (Synock) Guislain

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
	 *
	 * This method is a pure virtual function and needs to be implemented by the derived classes.
	 * It allows an item to be spawned from a specific actor at a desired location.
	 *
	 * Usage example:
	 *     AActor* SpawningActor = GetPlayerCharacter();
	 *     FVector DesiredDropLocation = SpawningActor->GetActorLocation();
	 *     uint32 ItemID = GetRandomItemID();
	 *     ADroppedItem* DroppedItem = SpawnItemFromActor(SpawningActor, ItemID, DesiredDropLocation);
	 *     if (DroppedItem != nullptr) {
	 *         // Item was successfully spawned.
	 *     } else {
	 *         // Item spawning failed.
	 *     }
	 */
	virtual ADroppedItem* SpawnItemFromActor(AActor* SpawningActor, uint32 ItemID, const FVector& DesiredDropLocation, bool ClampOnGround = true) = 0;

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
	virtual ADroppedCoins* SpawnCoinsFromActor(AActor* SpawningActor, const FCoinValue& CoinValue, const FVector& DesiredDropLocation, bool ClampOnGround = true) = 0;

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

	UFUNCTION(BlueprintCallable)
	virtual bool CanSpawnItem(UInventoryItemBase* NewItem);

	virtual bool DelayedLoreItemValidation(const UInventoryItemBase* LocalItem, ULootPoolComponent* Origin);

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
