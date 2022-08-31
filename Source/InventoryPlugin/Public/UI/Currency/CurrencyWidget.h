// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Definitions.h"
#include "Blueprint/UserWidget.h"
#include "Components/CoinComponent.h"
#include "Components/TextBlock.h"
#include "CurrencyWidget.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UCurrencyWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, Category="Inventory|Currency")
	UTextBlock* CurrencyAmountTextBlockPointer = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Inventory|Currency")
	ECurrencyType CurrencyType = ECurrencyType::Copper;

	UPROPERTY(BlueprintReadWrite, Category="Inventory|Currency")
	int32 CoinAmount = 0;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Currency")
	UCoinComponent* CoinComponent = nullptr;

	UPROPERTY(BlueprintReadWrite, Category="Inventory|Currency")
	bool AllowForCurrencyChange = false;

	UFUNCTION(BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void Refresh();

public:
	UFUNCTION(BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void UpdateCoinValue(int32 NewCoinAmount);

	UFUNCTION()
	void SetupCoinComponent(UCoinComponent* OriginCoinComponent, bool AllowForCurrencyChangeState = false);
};
