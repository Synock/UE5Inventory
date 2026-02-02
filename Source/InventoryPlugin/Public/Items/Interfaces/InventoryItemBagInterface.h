#pragma once

#include "CoreMinimal.h"
#include "Items/InventoryItemBase.h"
#include "UObject/Interface.h"
#include "InventoryItemBagInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInventoryItemBagInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class INVENTORYPLUGIN_API IInventoryItemBagInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool IsBag() const = 0;

	virtual UStaticMesh* GetStandardMesh() const = 0;

	virtual EItemSize GetBagSize() const =0;
	virtual uint8 GetBagWidth() const =0;
	virtual uint8 GetBagHeight() const =0;

	virtual float GetWeightReduction() const { return 0.f; }
	virtual bool IsVariableBagShape() const { return false; }
	virtual UStaticMesh* GetEmptyBagMesh() const { return nullptr; }
	virtual UStaticMesh* GetLowBagMesh() const { return nullptr; }
	virtual UStaticMesh* GetMidBagMesh() const { return nullptr; }
	virtual UStaticMesh* GetFullBagMesh() const { return nullptr; }
};
