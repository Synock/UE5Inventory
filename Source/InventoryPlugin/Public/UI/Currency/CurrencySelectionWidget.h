// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Definitions.h"
#include "Blueprint/UserWidget.h"
#include "Components/CoinComponent.h"
#include "CurrencySelectionWidget.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UCurrencySelectionWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Currency")
	UCoinComponent* CoinComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Currency")
	ECurrencyType CurrencyType = ECurrencyType::Copper;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Currency")
	int32 MaximumCoinCapacity = 0;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetupUI();

public:
	UFUNCTION(BlueprintCallable)
	void InitWidget(UCoinComponent* OriginCoinComponent, ECurrencyType InputCurrencyType);

};
