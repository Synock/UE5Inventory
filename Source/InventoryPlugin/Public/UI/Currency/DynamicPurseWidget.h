// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DynamicPurseWidget.generated.h"

class UCurrencyWidget;
/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UDynamicPurseWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category= "Inventory|Purse")
	class UCoinComponent* PursePointer = nullptr;

	UPROPERTY(BlueprintReadWrite, Category= "Inventory|Purse")
	UCurrencyWidget* CopperCurrencyWidget = nullptr;

	UPROPERTY(BlueprintReadWrite, Category= "Inventory|Purse")
	UCurrencyWidget* SilverCurrencyWidget = nullptr;

	UPROPERTY(BlueprintReadWrite, Category= "Inventory|Purse")
	UCurrencyWidget* GoldCurrencyWidget = nullptr;

	UPROPERTY(BlueprintReadWrite, Category= "Inventory|Purse")
	UCurrencyWidget* PlatinumCurrencyWidget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category= "Inventory|Purse")
	bool Dynamic = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category= "Inventory|Purse")
	bool AllowForCurrencyChange = false;

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	void InitWidget(UCoinComponent* Owner);

	UFUNCTION(BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void Refresh();
};
