// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BankWidget.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UBankWidget : public UUserWidget
{

	GENERATED_BODY()
protected:

public:

	UFUNCTION(BlueprintCallable)
	void ReorganizeContent();

};
