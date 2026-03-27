#pragma once

#include "CoreMinimal.h"
#include "Definitions.h"
#include "Items/InventoryItemEquipable.h"
#include "UObject/Interface.h"
#include "InventoryModularCharacterInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInventoryModularCharacterInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class INVENTORYPLUGIN_API IInventoryModularCharacterInterface
{
	GENERATED_BODY()
public:

	/// these are character component that get replaced by stuff
	virtual USkeletalMeshComponent* GetHeadComponent();
	virtual USkeletalMeshComponent* GetTorsoComponent();
	virtual USkeletalMeshComponent* GetArmsComponent();
	virtual USkeletalMeshComponent* GetHandsComponent();
	virtual USkeletalMeshComponent* GetLegsComponent();
	virtual USkeletalMeshComponent* GetFootComponent();

	/// these are stuff that get on top of other stuff
	virtual USkeletalMeshComponent* GetHelmetComponent();
	virtual USkeletalMeshComponent* GetShoulderPadComponent();
	virtual USkeletalMeshComponent* GetNeckComponent();
	virtual USkeletalMeshComponent* GetRightBracerComponent();
	virtual USkeletalMeshComponent* GetLeftBracerComponent();
	virtual USkeletalMeshComponent* GetBackComponent();

	virtual void SetEquipment(const UInventoryItemEquipable* Item, EEquipmentSlot Slot);

	virtual USkeletalMeshComponent* GetEquipmentComponentFromSlot(EEquipmentSlot Slot);

	/**
	 * Returns the skeletal mesh to display as a dynamic overlay component for the given slot.
	 * The plugin calls this immediately on equip/unequip to drive UpdateSingleOverlayMesh.
	 *
	 * Default: returns Item->EquipmentMesh (raw).
	 * Override to return nullptr for slots kept in a merged mesh path (e.g. Head),
	 * or a race/gender-corrected mesh for overlay-eligible slots (Shoulders, Neck, Back, Face, Wrists).
	 *
	 * @param Slot  The equipment slot changing.
	 * @param Item  The item being equipped, or nullptr when the slot is being cleared.
	 * @return      Mesh to use as overlay, or nullptr to suppress overlay for this slot.
	 */
	virtual USkeletalMesh* GetEquipmentOverlayMesh(EEquipmentSlot Slot, const UInventoryItemEquipable* Item) const;

	virtual bool IsBodyPart(EEquipmentSlot Slot);

	virtual bool IsEquipmentPart(EEquipmentSlot Slot);

};
