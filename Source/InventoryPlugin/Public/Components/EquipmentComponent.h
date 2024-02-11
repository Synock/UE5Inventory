// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <Components/ActorComponent.h>

#include "Definitions.h"
#include "Items/InventoryItemEquipable.h"
#include "EquipmentComponent.generated.h"

class AInventoryLightSourceActor;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentChanged);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentChanged_Server);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemEquiped_Server, EEquipmentSlot, Slot,
                                             const UInventoryItemEquipable*, Item);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUnEquiped_Server, EEquipmentSlot, Slot,
                                             const UInventoryItemEquipable*, Item);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UEquipmentComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_ItemList)
	TArray<const UInventoryItemEquipable*> Equipment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Light")
	UChildActorComponent* SecondaryLightSource;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Weapon")
	UStaticMeshComponent* PrimaryWeaponComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Weapon")
	UStaticMeshComponent* SecondaryWeaponComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Weapon")
	USkeletalMeshComponent* PrimaryWeaponComponentSkeletal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Weapon")
	USkeletalMeshComponent* SecondaryWeaponComponentSkeletal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	UStaticMeshComponent* AmmoComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	UStaticMeshComponent* WaistBag1Component;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	UStaticMeshComponent* WaistBag2Component;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	UStaticMeshComponent* ShoulderBag1Component;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	UStaticMeshComponent* ShoulderBag2Component;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	UStaticMeshComponent* BackpackComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Sheath")
	UStaticMeshComponent* PrimaryWeaponSheath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Sheath")
	UStaticMeshComponent* SecondaryWeaponSheath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Sheath")
	UStaticMeshComponent* BackWeaponSheath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment")
	UStaticMeshComponent* EarringLComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment")
	UStaticMeshComponent* EarringRComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment")
	UStaticMeshComponent* RingLComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment")
	UStaticMeshComponent* RingRComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment")
	USkeletalMeshComponent* HeadComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment")
    USkeletalMeshComponent* RightBracerComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment")
	USkeletalMeshComponent* LeftBracerComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment")
	UStaticMeshComponent* WristLComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment")
	UStaticMeshComponent* WristRComponent;

	EEquipmentSocket PrimaryWeaponOriginalSlot = EEquipmentSocket::Unknown;
	EEquipmentSocket SecondaryWeaponOriginalSlot = EEquipmentSocket::Unknown;


	/**
	 * Equips the given equipable item to the specified equipment slot.
	 *
	 * @param Item The equipable item to be equipped.
	 * @param EquipSlot The equipment slot to which the item should be equipped.
	 * @return true if the item was successfully equipped, false otherwise.
	 */
	bool Equip(const UInventoryItemEquipable* Item, EEquipmentSlot EquipSlot);

	/**
	 * @brief UnEquips an equipable item from the specified equipment slot.
	 *
	 * @param Item Pointer to the equipable item to be unequipped.
	 * @param EquipSlot The equipment slot to unequip the item from.
	 * @return True if the item was successfully unequipped, false otherwise.
	 */
	bool UnEquip(const UInventoryItemEquipable* Item, EEquipmentSlot EquipSlot);

	/**
	 * @brief Finds the best equipment socket for the given item and equipment slot.
	 *
	 * This method determines the best equipment socket to use for a given item and equipment slot. It takes into account various factors such as the type of item and the current equipment
	 * state.
	 *
	 * @param Item                  The equipable item to find the best socket for.
	 * @param EquipSlot             The equipment slot to find the best socket for.
	 *
	 * @return                      The best equipment socket for the given item and equipment slot.
	 */
	static EEquipmentSocket FindBestSocketForItem(const UInventoryItemEquipable* Item, EEquipmentSlot EquipSlot);

	/**
	 * Unsheaths an item from the specified equipment slot and places it in the appropriate live socket.
	 *
	 * @param SlotToUnsheath The equipment slot to unsheath the item from.
	 */
	void Unsheath(EEquipmentSlot SlotToUnsheath);
	/**
	 * @brief Sheath the weapons.
	 *
	 * This method sheaths the primary and secondary weapons by swapping their static meshes with the return socket's static mesh.
	 * If the primary or secondary weapon's static mesh is already sheathed, the method will return without making any changes.
	 *
	 * @param None
	 *
	 * @return None
	 */
	void Sheath();


public:
	/**
	 * Retrieves the mesh component associated with the specified equipment socket.
	 *
	 * @param Socket The equipment socket to retrieve the mesh component for.
	 * @return A pointer to the mesh component associated with the specified socket.
	 *         Returns nullptr if no mesh component is associated with the socket.
	 */
	UStaticMeshComponent* GetMeshComponentFromSocket(EEquipmentSocket Socket) const;

	/**
	 * Retrieves the skeletal mesh component associated with the given equipment socket.
	 *
	 * @param Socket The equipment socket to retrieve the skeletal mesh component from.
	 *
	 * @return The skeletal mesh component associated with the specified equipment socket, or nullptr
	 *         if the socket is unknown or not found.
	 */
	USkeletalMeshComponent* GetSkeletalMeshComponentFromSocket(EEquipmentSocket Socket) const;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Equipment")
	FOnEquipmentChanged EquipmentDispatcher;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Equipment")
	FOnEquipmentChanged_Server EquipmentDispatcher_Server;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Equipment")
	FOnItemEquiped_Server ItemEquipedDispatcher_Server;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Equipment")
	FOnItemUnEquiped_Server ItemUnEquipedDispatcher_Server;

	UFUNCTION(NetMulticast, reliable)
	void UpdateEquipment(USkeletalMeshComponent* SkeletalSocket, USkeletalMesh* LocalItem, const FMaterialOverride& MaterialOverride);

	/**
	 *
	 */
	UFUNCTION()
	void OnRep_ItemList();

	/**
	 *
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void EquipItem(const UInventoryItemEquipable* Item, EEquipmentSlot InSlot);

	/**
	 *
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool IsSlotEmpty(EEquipmentSlot InSlot);

	/**
	 *
	 */
	const TArray<const UInventoryItemEquipable*>& GetAllEquipment() const;

	/**
	 * @brief Retrieves the inventory item equipped at the specified equipment slot.
	 *
	 * @param InSlot The equipment slot to check.
	 * @return A pointer to the inventory item equipped at the specified slot, or nullptr if no item is equipped.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	const UInventoryItemEquipable* GetItemAtSlot(EEquipmentSlot InSlot) const;

	/**
	 *
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool RemoveItem(EEquipmentSlot InSlot);

	/**
	 * @brief Removes all items from the equipment component.
	 *
	 * This function removes all items from the equipment component by iterating over each equipment slot and
	 * calling the RemoveItem function to remove the item from that slot.
	 *
	 * @note This method is BlueprintCallable, which means it can be called from Blueprint scripts.
	 * @note This method is categorized under "Inventory|Equipment" in Blueprint.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void RemoveAll();

	/**
	 * Get the total weight of all equipped items.
	 *
	 * @return The total weight as a float.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	float GetTotalWeight() const;

	/**
	 * Finds a suitable equipment slot for the given item.
	 *
	 * @param Item The item to find a suitable slot for.
	 *
	 * @return The suitable equipment slot for the item.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	EEquipmentSlot FindSuitableSlot(const UInventoryItemEquipable* Item) const;

	/**
	 * @brief Unsheathe the melee weapons.
	 *
	 * This method is used to unsheathe the primary and secondary melee weapons
	 * in the inventory. It is a Blueprint callable function that belongs to the
	 * "Inventory|Equipment" category.
	 *
	 * @param None
	 *
	 * @return None
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void UnsheathMelee();

	/**
	 * Sheaths the melee weapon.
	 *
	 * @param None
	 *
	 * @return None
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void SheathMelee();

	/**
	 * Get the overlap box for a specific equipment slot.
	 *
	 * @param Slot The equipment slot to get the overlap box for.
	 * @return The overlap box bounds for the specified equipment slot.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	FBoxSphereBounds GetEquipmentOverlapBox(EEquipmentSlot Slot) const;

	/**
	 * Sets the light source for a specific equipment slot.
	 *
	 * @param LightActor The class of the light source actor to use.
	 * @param Slot The slot where the light source should be set.
	 *
	 * @remarks This method should only be called on the server.
	 *          If LightActor is nullptr, the light source for the slot will be removed.
	 *          The light source actor is attached to the character's skeletal mesh component
	 *          using the specified attachment rules.
	 *          The specific attachment socket depends on the equipment slot provided.
	 *          Only the secondary slot is supported currently.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void SetEquipmentLight(TSubclassOf<AInventoryLightSourceActor> LightActor, EEquipmentSlot Slot) const;

	/**
	 * EquipLightItem function equips a light item specified by the LightActor parameter.
	 * It is a NetMulticast function that is reliable, meaning it will be executed on all connected clients
	 * to ensure consistency in the game state across the network.
	 *
	 * @param LightActor: The class of the light actor to be equipped.
	 * @remarks This function is constant, meaning it does not modify any member variables of the class it belongs to.
	 *          It is assumed that the class containing this function has a valid InventoryLightSourceActor reference to
	 *          equip the light actor.
	 * @see InventoryLightSourceActor
	 * @see AInventoryLightSourceActor
	 */
	UFUNCTION(NetMulticast, Reliable)
	void EquipLightItem(TSubclassOf<AInventoryLightSourceActor> LightActor) const;

	/**
	 * @brief UnEquips a light item.
	 *
	 * This method is a NetMulticast reliable function that is used to un-equip a light item.
	 * It is const and does not return any value.
	 *
	 * @param None
	 *
	 * @return None
	 */
	UFUNCTION(NetMulticast, Reliable)
	void UnEquipLightItem() const;
};
