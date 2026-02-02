#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryItemRangedWeaponInterface.generated.h"

enum class EAmmoType : uint8;
// This class does not need to be modified.
UINTERFACE(NotBlueprintable)
class UInventoryItemRangedWeaponInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class INVENTORYPLUGIN_API IInventoryItemRangedWeaponInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Inventory|RangedWeapon")
	virtual EAmmoType GetAmmoType() const = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory|RangedWeapon")
	virtual float GetMaxRange() const = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory|RangedWeapon")
	virtual float GetMinRange() const = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory|RangedWeapon")
	virtual float GetReloadDelay() const = 0;


};
