// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryBagsWidget.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYPLUGIN_API UInventoryBagsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic)
	void Refresh();
};
