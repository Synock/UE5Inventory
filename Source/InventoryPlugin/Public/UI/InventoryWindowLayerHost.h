#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "InventoryWindowLayerHost.generated.h"

class UUserWidget;

/**
 * Generic bridge implemented by a game's draggable window base.  InventoryPlugin
 * uses it only when an item UI opens a child popup, keeping game-specific window
 * classes out of the plugin.
 */
UINTERFACE(BlueprintType)
class INVENTORYPLUGIN_API UInventoryWindowLayerHost : public UInterface
{
	GENERATED_BODY()
};

class INVENTORYPLUGIN_API IInventoryWindowLayerHost
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory|Window")
	int32 GetChildWindowZOrder() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory|Window")
	void AttachChildWindow(UUserWidget* ChildWindow);
};
