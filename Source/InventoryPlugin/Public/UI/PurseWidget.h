// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PurseWidget.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORYPLUGIN_API UPurseWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, Category="Inventory|Purse")
	int32 CP = 0;

	UPROPERTY(BlueprintReadWrite, Category="Inventory|Purse")
	int32 SP = 0;

	UPROPERTY(BlueprintReadWrite, Category="Inventory|Purse")
	int32 GP = 0;

	UPROPERTY(BlueprintReadWrite, Category="Inventory|Purse")
	int32 PP = 0;

	UPROPERTY(BlueprintReadOnly, Category= "Inventory|Purse")
	class UCoinComponent* PursePointer = nullptr;

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|UI")
	void InitWidget(UCoinComponent* Owner);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, BlueprintCallable, Category = "Inventory|UI")
	void Refresh();
};
