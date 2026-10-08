#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryItemWeaponInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(NotBlueprintable)
class UInventoryItemWeaponInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for weapon-specific properties.
 * Separates weapon concerns from general equipable items.
 */
class INVENTORYPLUGIN_API IInventoryItemWeaponInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Inventory|Weapon")
	virtual bool IsWeapon() const = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Weapon")
	virtual bool IsUnsheathable() const = 0;
};
