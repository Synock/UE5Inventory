// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeightWidget.generated.h"

class IInventoryPlayerInterface;
class APlayerCharacter;
/**
 * 
 */
UCLASS()
class INVENTORYPLUGIN_API UWeightWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Weight")
	float TotalWeight;
	
	UFUNCTION(BlueprintCosmetic, BlueprintCallable, Category = "Inventory|Weight")
	void QueryTotalWeight();

	
	IInventoryPlayerInterface* GetInventoryPlayerInterface() const;

public:
	UFUNCTION(BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void InitWidget();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void Refresh();
};
