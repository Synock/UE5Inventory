// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <Components/ActorComponent.h>

#include "Items/InventoryItemEquipable.h"
#include "EquipmentComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentChanged);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentChanged_Server);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemEquiped_Server, EEquipmentSlot, Slot, const UInventoryItemEquipable*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUnEquiped_Server, EEquipmentSlot, Slot, const UInventoryItemEquipable*, Item);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neverquest|Weapon")
	UStaticMeshComponent* PrimaryWeaponComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neverquest|Weapon")
	UStaticMeshComponent* SecondaryWeaponComponent;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neverquest|Sheath")
	UStaticMeshComponent* PrimaryWeaponSheath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neverquest|Sheath")
	UStaticMeshComponent* SecondaryWeaponSheath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neverquest|Sheath")
	UStaticMeshComponent* BackWeaponSheath;

	EEquipmentSocket PrimaryWeaponOriginalSlot = EEquipmentSocket::Unknown;
	EEquipmentSocket SecondaryWeaponOriginalSlot = EEquipmentSocket::Unknown;

	

	bool Equip(const UInventoryItemEquipable* Item, EEquipmentSlot EquipSlot);

	bool UnEquip(const UInventoryItemEquipable* Item, EEquipmentSlot EquipSlot);

	static EEquipmentSocket FindBestSocketForItem(const UInventoryItemEquipable* Item, EEquipmentSlot EquipSlot);

	void Unsheath(EEquipmentSlot SlotToUnsheath);
	void Sheath();

public:

	UStaticMeshComponent* GetMeshComponentFromSocket(EEquipmentSocket Socket) const;
	
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Equipment")
	FOnEquipmentChanged EquipmentDispatcher;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Equipment")
	FOnEquipmentChanged_Server EquipmentDispatcher_Server;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Equipment")
	FOnItemEquiped_Server ItemEquipedDispatcher_Server;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Equipment")
	FOnItemUnEquiped_Server ItemUnEquipedDispatcher_Server;

	UFUNCTION()
	void OnRep_ItemList();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void EquipItem(const UInventoryItemEquipable* Item, EEquipmentSlot InSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool IsSlotEmpty(EEquipmentSlot InSlot);

	const TArray<const UInventoryItemEquipable*>& GetAllEquipment() const;

	//Return the item equipped at the given slot
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	const UInventoryItemEquipable* GetItemAtSlot(EEquipmentSlot InSlot) const;

	//Remove the item equipped at the given slot
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool RemoveItem(EEquipmentSlot InSlot);

	//Return the total weight of the equipped items
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	float GetTotalWeight() const;

	//Find and return an empty slot for the item or InventorySlot::Unknown
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	EEquipmentSlot FindSuitableSlot(const UInventoryItemEquipable* Item) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void UnsheathMelee();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void SheathMelee();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	FBoxSphereBounds GetEquipmentOverlapBox(EEquipmentSlot Slot) const;
};
