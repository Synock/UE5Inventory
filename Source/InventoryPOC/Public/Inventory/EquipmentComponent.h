// Copyright 2021 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/BareItem.h"
#include "EquipmentComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentChanged);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentChanged_Server);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYPOC_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UEquipmentComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_ItemList)
	TArray<FBareItem> Equipment;

public:
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Equipment")
	FOnEquipmentChanged EquipmentDispatcher;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Equipment")
	FOnEquipmentChanged_Server EquipmentDispatcher_Server;

	UFUNCTION()
	void OnRep_ItemList();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void EquipItem(const FBareItem& Item, InventorySlot InSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool IsSlotEmpty(InventorySlot InSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	TArray<FBareItem> GetAllEquipment() const;

	//Return the item equipped at the given slot
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	const FBareItem& GetItemAtSlot(InventorySlot InSlot);

	//Remove the item equipped at the given slot
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool RemoveItem(InventorySlot InSlot);

	//Return the total weight of the equipped items
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	float GetTotalWeight();

	//Find and return an empty slot for the item or InventorySlot::Unknown
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	InventorySlot FindSuitableSlot(const FBareItem& Item);
};
