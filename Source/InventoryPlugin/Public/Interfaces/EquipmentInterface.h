// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "UObject/Interface.h"
#include "EquipmentInterface.generated.h"

class UInventoryItemEquipable;
class UEquipmentComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UEquipmentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * @class IEquipmentInterface
 * @brief Interface for handling equipment functionality.
 *
 * This interface provides methods for retrieving, equipping, and unequipping items in specific equipment slots.
 */
class INVENTORYPLUGIN_API IEquipmentInterface
{
	GENERATED_BODY()

public:
	virtual UEquipmentComponent* GetEquipmentComponent() = 0;
	virtual const UEquipmentComponent* GetEquipmentComponentConst() const = 0;

	//------------------------------------------------------------------------------------------------------------------
	// Equipment
	//------------------------------------------------------------------------------------------------------------------

	virtual const TArray<const UInventoryItemEquipable*>& GetAllEquipment() const;

	/**
	 * Retrieves the equipped item for the specified equipment slot.
	 *
	 * @param Slot The equipment slot to retrieve the equipped item from.
	 * @return A constant pointer to the UInventoryItemEquipable object representing the equipped item. Returns nullptr if no item is equipped in the specified slot.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	virtual const UInventoryItemEquipable* GetEquippedItem(EEquipmentSlot Slot) const;

	/**
	 * \brief Equip an item in the given equipment slot.
	 * \param InSlot The equipment slot to equip the item in.
	 * \param InItemId The ID of the item to equip.
	 *
	 * This method equips an item in the specified equipment slot. It first checks if the equipment has the necessary authority to perform the operation. If not, the method returns without
	 * making any changes. Next, it retrieves the item with the specified ID from the inventory using the GetItemFromID method of the UInventoryUtilities class. If the item is not found,
	 * the method returns without making any changes. Finally, it calls the EquipItem method of the equipment component to actually equip the item. Note that this method does not handle any
	 * equipment effect associated with equipping the item.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	virtual void EquipItem(EEquipmentSlot InSlot, int32 InItemId);

	/**
	 * Tries to automatically equip an item based on its ID.
	 *
	 * @param InItemId The ID of the item to try to equip.
	 * @param PossibleEquipment [out] The possible equipment slot where the item can be equipped.
	 * @return True if the item was successfully equipped, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	virtual bool TryAutoEquip(int32 InItemId, EEquipmentSlot& PossibleEquipment) const;

	/**
	 * Removes the item from the given equipment slot.
	 *
	 * @param OutSlot The equipment slot from which the item will be removed.
	 *
	 * @note This method requires the equipment system to have authority.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	virtual void UnequipItem(EEquipmentSlot OutSlot);

	/**
	 * @brief Swaps the equipment between two equipment slots.
	 *
	 * This method is used to swap the equipment between two specified equipment slots.
	 * The equipment in the InSlot is moved to the OutSlot, and the equipment in the OutSlot is moved to the InSlot.
	 *
	 * @param InSlot The equipment slot to move the equipment from.
	 * @param OutSlot The equipment slot to move the equipment to.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	virtual void SwapEquipment(EEquipmentSlot InSlot, EEquipmentSlot OutSlot);

	/**
	 * @brief FindSuitableSlot method.
	 *
	 * This method finds a suitable equipment slot for the given equipable item.
	 * It will return the appropriate equipment slot for the item based on some criteria.
	 *
	 * @param Item The equipable item to find a suitable slot for.
	 *
	 * @return The suitable equipment slot found for the item.
	 */
	virtual EEquipmentSlot FindSuitableSlot(const UInventoryItemEquipable* Item) const;

	/**
	 * Handles the equipment of a two-slot item.
	 *
	 * This method is called when a two-slot item is being equipped.
	 * It first checks if the equipment has the authority to perform the operation.
	 * If the item is a two-slot item, it determines the corresponding primary and secondary slots
	 * based on the provided input slot.
	 *
	 * @param Item The two-slot item to be equipped.
	 * @param InSlot The input slot where the item is being equipped.
	 *
	 * @return None.
	 */
	virtual void HandleTwoSlotItemEquip(const UInventoryItemEquipable* Item, EEquipmentSlot& InSlot);
	/**
	 * Handles unequipping of a two-slot item.
	 *
	 * @param Item The inventory item being unequipped.
	 * @param InSlot The equipment slot from which the item is being unequipped.
	 */
	virtual void HandleTwoSlotItemUnequip(const UInventoryItemEquipable* Item, EEquipmentSlot InSlot);

	/**
	 * @brief Handles the effects of equipping an item in the specified equipment slot.
	 *
	 * This method is called when an item is equipped in the specified equipment slot.
	 * It performs various tasks based on the type of item being equipped.
	 *
	 * @param InSlot The equipment slot in which the item is being equipped.
	 * @param LocalItem The item being equipped.
	 *
	 * @note This method should only be called by the server.
	 *       Clients should not directly call this method.
	 */
	UFUNCTION()
	virtual void HandleEquipmentEffect(EEquipmentSlot InSlot, const UInventoryItemEquipable* LocalItem);

	/**
	 * Handles the effects of un-equipping an item from the specified equipment slot.
	 *
	 * This method is called when an item is un-equipped from a specific equipment slot.
	 * It should handle any special effects or actions that need to be taken when
	 * un-equipping the item, such as updating attributes, triggering events, etc.
	 *
	 * @param InSlot The equipment slot from which the item is being un-equipped.
	 * @param LocalItem The item being un-equipped from the equipment slot.
	 */
	UFUNCTION()
	virtual void HandleUnEquipmentEffect(EEquipmentSlot InSlot, const UInventoryItemEquipable* LocalItem);

	/**
	 * @brief Gets the total weight of the equipment.
	 *
	 * This method iterates over all the equipment items and calculates the total weight by summing up the weight of each item.
	 *
	 * @return The total weight of the equipment as a float value.
	 */
	float GetTotalWeight() const;

	/**
		 * @brief If needed, override this to handle special mesh query of the interface owner
		 *
		 * @return The static mesh to use.
		 */
	virtual UStaticMesh* GetPreferedMesh(UStaticMesh* OriginalMesh) const;

protected:
	/**
	 * Check if the equipment has authority.
	 *
	 * @return True if the equipment has authority, false otherwise.
	 */
	virtual bool EquipmentHasAuthority();

	/**
	 * Get the world context for the equipment.
	 *
	 * @return The UWorld object that the equipment belongs to.
	 */
	virtual UWorld* EquipmentGetWorldContext() const;
};
