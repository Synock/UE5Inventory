#pragma once

#include "CoreMinimal.h"
#include "Definitions.h"
#include "UObject/Interface.h"
#include "InventoryItemAmmoBagInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInventoryItemAmmoBagInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class INVENTORYPLUGIN_API IInventoryItemAmmoBagInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual EAmmoType GetAmmoType() const = 0;
	virtual UStaticMesh* GetEmptyBagMesh() const =0;
	virtual UStaticMesh* GetLowMesh() const =0;
	virtual UStaticMesh* GetMidMesh() const =0;
	virtual UStaticMesh* GetFullMesh() const =0;

	virtual float GetLowMeshThreshold() const { return 0.1f; }
	virtual float GetMidMeshThreshold() const { return 0.33f; }
	virtual float GetFullMeshThreshold() const { return 0.66f; }
};
