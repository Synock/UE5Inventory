// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeightWidget.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYPOC_API UWeightWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Weight")
	float TotalWeight;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Weight")
	class AInventoryPOCCharacter* PCOwner = nullptr;

	UFUNCTION(BlueprintCosmetic, BlueprintCallable, Category = "Inventory|Weight")
	void QueryTotalWeight();

public:
	UFUNCTION(BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void InitWidget(AInventoryPOCCharacter* Owner);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void Refresh();
};
