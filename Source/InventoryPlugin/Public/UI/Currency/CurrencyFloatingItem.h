// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Definitions.h"
#include "Blueprint/UserWidget.h"
#include "Components/CoinComponent.h"
#include "CurrencyFloatingItem.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UCurrencyFloatingItem : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category="Inventory|Currency")
	ECurrencyType CurrencyType = ECurrencyType::Copper;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Currency")
	int32 CoinAmount = 0;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Currency")
	UCoinComponent* CoinOrigin = nullptr;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetupUI();

public:
	UFUNCTION(BlueprintCallable)
	void InitWidget(UCoinComponent* CoinOriginPointer, int32 InputCoinAmount, ECurrencyType InputCurrencyType);
};
