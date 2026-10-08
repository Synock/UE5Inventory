#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryItemAmmoInterface.generated.h"

enum class EAmmoType : uint8;
// This class does not need to be modified.
UINTERFACE()
class UInventoryItemAmmoInterface : public UInterface
{
	GENERATED_BODY()
};

class INVENTORYPLUGIN_API IInventoryItemAmmoInterface
{
	GENERATED_BODY()

public:
	virtual EAmmoType GetAmmoType() const = 0;
};
