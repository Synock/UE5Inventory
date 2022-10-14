// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "UObject/Interface.h"
#include "EquipmentInterface.generated.h"

class UEquipmentComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UEquipmentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class INVENTORYPLUGIN_API IEquipmentInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual UEquipmentComponent* GetEquipmentComponent() = 0;

	virtual const UEquipmentComponent* GetEquipmentComponentConst() const = 0;

	virtual AActor* GetEquipmentOwningActor() = 0;
	virtual AActor const* GetEquipmentOwningActorConst() const = 0;

	//------------------------------------------------------------------------------------------------------------------
	// Equipment
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	virtual TArray<FInventoryItem> GetAllEquipment() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	virtual const FInventoryItem& GetEquippedItem(EEquipmentSlot Slot) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	virtual void EquipItem(EEquipmentSlot InSlot, int32 InItemId);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	virtual bool TryAutoEquip(int32 InItemId, EEquipmentSlot& PossibleEquipment) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	virtual void UnequipItem(EEquipmentSlot OutSlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	virtual void SwapEquipment(EEquipmentSlot InSlot, EEquipmentSlot OutSlot);

	virtual EEquipmentSlot FindSuitableSlot(const FInventoryItem& Item) const;

	virtual void HandleTwoSlotItemEquip(const FInventoryItem& Item, EEquipmentSlot& InSlot);
	virtual void HandleTwoSlotItemUnequip(const FInventoryItem& Item, EEquipmentSlot InSlot);

	UFUNCTION()
	virtual void HandleEquipmentEffect(EEquipmentSlot InSlot, const FInventoryItem& LocalItem);

	UFUNCTION()
	virtual void HandleUnEquipmentEffect(EEquipmentSlot InSlot, const FInventoryItem& LocalItem);
	
	float GetTotalWeight() const;

protected:
	virtual bool EquipmentHasAuthority();

	virtual UWorld* EquipmentGetWorldContext() const;
};
