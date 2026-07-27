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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentDurabilityChanged_Server, EEquipmentSlot, Slot,
                                             float, NewDurability);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEquipmentDurabilityWarning, EEquipmentSlot, Slot,
                                               float, DurabilityPercent, const UInventoryItemEquipable*, Item);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPLUGIN_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UEquipmentComponent();

	void UpdateMasterMeshComponent(USkeletalMeshComponent* Mesh);

	/// Try to update the dynamic meshes handled by the equipment component
	void TryUpdateDynamicMeshes(const TMap<EEquipmentSlot, USkeletalMesh*>& MeshArray,const TMap<EEquipmentSlot, TArray<FMaterialOverride>>& OverrideArray);
	void SellMaterialForAllMeshes(int MaterialID, UMaterialInstance* MaterialInstance);
	bool AttachEquipmentComponentsToOwnerMeshIfReady();

private:
	/**
	 * Allocates, configures, attaches, and registers a new USkeletalMeshComponent for the given slot.
	 * Sets both ECC_Camera and ECC_Pawn to ECR_Ignore and sets LeaderPoseComponent.
	 * Adds the result to VariableMeshesMap. Returns nullptr if the owner is not a valid ACharacter.
	 */
	USkeletalMeshComponent* CreateAndRegisterOverlayComponent(EEquipmentSlot Slot, USkeletalMesh* Mesh);

	/**
	 * Applies a list of material overrides to a component.
	 * Creates a UMaterialInstanceDynamic per entry to support tint/intensity parameters.
	 */
	static void ApplyMaterialOverrides(USkeletalMeshComponent* MeshComp, const TArray<FMaterialOverride>& Overrides);

	/**
	 * Creates, updates, or removes the dynamic overlay USkeletalMeshComponent for a single slot,
	 * without touching any other slot in VariableMeshesMap.
	 * Called directly on equip/unequip so the overlay appears before UpdateMeshFromInternal rebuilds.
	 *
	 * @param Slot      Equipment slot to target.
	 * @param Mesh      Mesh to display; nullptr removes the existing overlay component.
	 * @param Overrides Material overrides to apply after setting the mesh.
	 */
	void UpdateSingleOverlayMesh(EEquipmentSlot Slot, USkeletalMesh* Mesh, const TArray<FMaterialOverride>& Overrides);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_ItemList)
	TArray<const UInventoryItemEquipable*> Equipment;

	// Durability tracking for equipped items (current condition only)
	UPROPERTY(ReplicatedUsing = OnRep_EquipmentDurability, BlueprintReadOnly, Category = "Inventory|Equipment|Durability")
	TArray<float> EquipmentDurability;

	/** Client-side lock state for equipment slots (not replicated, UI-only) */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory|Equipment|Lock")
	TMap<EEquipmentSlot, bool> EquipmentLockStates;

	/** Server-only equipment cells reserved while a pending delivery is committed externally. */
	TMap<FGuid, TArray<EEquipmentSlot>> PendingDeliveryReservations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Light")
	UChildActorComponent* SecondaryLightSource;

	/// Weapons and Sheaths
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Inventory|Weapon")
	USkeletalMeshComponent* PrimaryWeaponComponent;

	UPROPERTY(EditAnywhere,Replicated, BlueprintReadWrite, Category = "Inventory|Weapon")
	USkeletalMeshComponent* SecondaryWeaponComponent;

	UPROPERTY(EditAnywhere,Replicated, BlueprintReadWrite, Category = "Inventory|Sheath")
	USkeletalMeshComponent* PrimaryWeaponSheath;

	UPROPERTY(EditAnywhere,Replicated, BlueprintReadWrite, Category = "Inventory|Sheath")
	USkeletalMeshComponent* SecondaryWeaponSheath;

	UPROPERTY(EditAnywhere,Replicated, BlueprintReadWrite, Category = "Inventory|Sheath")
	USkeletalMeshComponent* BackWeaponSheath;

	UPROPERTY(EditAnywhere,Replicated, BlueprintReadWrite, Category = "Inventory|Sheath")
	USkeletalMeshComponent* RangedWeaponSheath;

	/// Bags and Ammo

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Bag")
	UStaticMeshComponent* AmmoComponent;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Inventory|Bag")
	UStaticMeshComponent* AmmoVariableComponent;

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

	/// Jewelry
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment")
	UStaticMeshComponent* EarringLComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment")
	UStaticMeshComponent* EarringRComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment")
	UStaticMeshComponent* RingLComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment")
	UStaticMeshComponent* RingRComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment")
	UStaticMeshComponent* WristLComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment")
	UStaticMeshComponent* WristRComponent;

	EEquipmentSocket PrimaryWeaponOriginalSlot = EEquipmentSocket::Unknown;
	EEquipmentSocket SecondaryWeaponOriginalSlot = EEquipmentSocket::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Equipment")
	TMap<EEquipmentSlot, USkeletalMeshComponent*> VariableMeshesMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Inventory|Equipment")
	bool IsHoldingATwoHandedWeapon = false;

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
	virtual EEquipmentSocket FindBestSocketForItem(const UInventoryItemEquipable* Item, EEquipmentSlot EquipSlot);

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

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Equipment")
	FOnEquipmentDurabilityChanged_Server EquipmentDurabilityChangedDispatcher_Server;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Equipment")
	FOnEquipmentDurabilityWarning DurabilityWarningDispatcher;

	UFUNCTION()
	void OnRep_EquipmentDurability();

	UFUNCTION(NetMulticast, reliable)
	void UpdateEquipment(USkeletalMeshComponent* SkeletalSocket, USkeletalMesh* LocalItem, const TArray<FMaterialOverride>& MaterialOverride);

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
	 * Equip an item with specific durability value
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void EquipItemWithDurability(const UInventoryItemEquipable* Item, EEquipmentSlot InSlot, float Durability);

	/**
	 * Get current durability for an equipped item
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool GetEquipmentDurability(EEquipmentSlot InSlot, float& OutDurability) const;

	/**
	 * Set current durability for an equipped item
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void SetEquipmentDurability(EEquipmentSlot InSlot, float Durability);

	/**
	 * Reduce equipment durability for a specific slot
	 * @param InSlot The equipment slot to reduce durability for
	 * @param DurabilityReduction Amount of durability to reduce (already calculated with modifiers)
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void ReduceEquipmentDurability(EEquipmentSlot InSlot, float DurabilityReduction);

	/**
	 * Set lock state for an equipment slot (client-side UI state only, not persisted)
	 * @param InSlot The equipment slot to lock/unlock
	 * @param bLocked Whether the slot should be locked
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment|Lock")
	void SetEquipmentLockState(EEquipmentSlot InSlot, bool bLocked);

	/**
	 * Get lock state for an equipment slot
	 * @param InSlot The equipment slot to check
	 * @return True if the slot is locked, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment|Lock")
	bool GetEquipmentLockState(EEquipmentSlot InSlot) const;

	/**
	 *
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool IsSlotEmpty(EEquipmentSlot InSlot);

	/** Validate the complete equipment footprint, including multi-slot occupancy and reservations. */
	bool CanEquipItemAt(const UInventoryItemEquipable* Item, EEquipmentSlot InSlot,
		const FGuid& IgnoredReservation = FGuid()) const;

	/** Reserve every equipment cell occupied by Item until its delivery claim completes. */
	bool ReservePendingDelivery(const FGuid& DeliveryId, const UInventoryItemEquipable* Item, EEquipmentSlot InSlot);
	void ReleasePendingDeliveryReservation(const FGuid& DeliveryId);
	bool HasPendingDeliveryReservation(const FGuid& DeliveryId) const;
	bool IsEquipmentSlotReserved(EEquipmentSlot InSlot, const FGuid& IgnoredReservation = FGuid()) const;

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
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void UnsheathMelee();

	/**
	 * Sheaths the melee weapon.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void SheathMelee();

	/**
	 * @brief Unsheathe the ranged weapon.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void UnsheathRanged();

	/**
	 * Sheaths the melee weapon.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void SheathRanged();

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

	UFUNCTION(BlueprintCallable)
	void SetAllEquipmentCollisionDisabled();

	UFUNCTION(BlueprintCallable)
	bool IsWeaponTwoHanded() const;

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	FTransform GetOffHandTransform() const;

	/**
	 * World-space location of the primary weapon's tip.
	 * Reads the "WeaponTip" socket from PrimaryWeaponComponent (weapon skeletal mesh).
	 * Falls back to the weapon component root, then to SOCKET_RightHandWeapon if no weapon is drawn.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|FX")
	FVector GetWeaponTipLocation() const;

	/**
	 * World-space location of the defender's contact point.
	 * Reads "ShieldCenter" from SecondaryWeaponComponent if a shield is equipped,
	 * "WeaponTip" if an off-hand weapon is equipped, or falls back to SOCKET_LeftHandWeapon.
	 * Used by ComputeDefenseContactPoint to spawn ClashFX at the right midpoint.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|FX")
	FVector GetDefenseContactLocation() const;

	UFUNCTION(BlueprintCallable)
	void UpdateBagUsage(EBagSlot BagSlot, float BagUsage);

	/**
	 * Re-attaches a rigid (static-mesh) bag/backpack component to the named anchor bone on the
	 * owner's skeletal mesh, then applies a location offset and an XY-plane scale override.
	 * Pass FVector::OneVector for ScaleXY when no scaling is needed (backpacks, hip bags).
	 * Pass (BulkScale, BulkScale, 1) for belt/girdle items that must expand radially.
	 *
	 * @param Slot        The equipment slot whose static mesh component to reposition.
	 * @param BoneName    Anchor bone on the owner CharacterMesh to attach to.
	 * @param LocationOffset  Relative location offset applied after attachment (component space).
	 * @param ScaleXY     Relative scale applied to the component (Z kept from w passed value).
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment|Fitting")
	void ApplyRigidItemFitting(EEquipmentSlot Slot, FName BoneName, FVector LocationOffset, FVector Scale);

	/**
	 * Returns the UStaticMeshComponent responsible for the given bag/backpack slot, or nullptr
	 * for slots that do not use a static-mesh component (weapons, skeletal overlays, jewellery).
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment|Fitting")
	UStaticMeshComponent* GetStaticMeshComponentForSlot(EEquipmentSlot Slot) const;

	/**
	 * Returns the dynamic skeletal overlay component for the given slot from VariableMeshesMap,
	 * or nullptr if no overlay component exists for that slot.
	 * Used by the fitting system to apply scale/attachment to belt and other overlays.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment|Fitting")
	USkeletalMeshComponent* GetOverlayComponentForSlot(EEquipmentSlot Slot) const;
};
