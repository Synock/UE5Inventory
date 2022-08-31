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
	UCoinComponent* Origin = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Currency")
	UCoinComponent* Destination = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Currency")
	ECurrencyType CurrencyType = ECurrencyType::Copper;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Currency")
	ECurrencyType DesiredCurrencyType = ECurrencyType::Copper;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Currency")
	int32 MaximumCoinCapacity = 0;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Currency")
	bool AllowForCurrencyChange = false;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetupUI();

public:
	UFUNCTION(BlueprintCallable)
	void InitWidget(UCoinComponent* OriginCoinComponent, UCoinComponent* DestinationCoinComponent,
	                ECurrencyType InputCurrencyType, ECurrencyType OutputCurrencyType, bool AllowForCurrencyChangeState = false);

	UFUNCTION(BlueprintCallable)
	void DoTheCoinTransfer(int32 SelectedCoinValue);
};
