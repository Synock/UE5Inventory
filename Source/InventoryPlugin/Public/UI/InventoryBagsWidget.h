#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryBagsWidget.generated.h"

/**
 * Container widget that holds and manages multiple bag widgets.
 * C++ provides a no-op base for Refresh(); Blueprint overrides to iterate contained bags.
 */
UCLASS()
class INVENTORYPLUGIN_API UInventoryBagsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
public:
	/**
	 * Refresh all bag grids. C++ base is a no-op; Blueprint overrides to propagate to contained UBagWidgets.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic)
	void Refresh();
	virtual void Refresh_Implementation() {}
};
