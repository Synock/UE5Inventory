// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Actors/MerchantActor.h"
#include "Blueprint/UserWidget.h"
#include "MerchantWidget.generated.h"

/**
 *
 */
UCLASS()
class INVENTORYPOC_API UMerchantWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Merchant")
	AMerchantActor* MerchantActor = nullptr;

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void InitMerchantData(AMerchantActor* InputMerchantActor);

	UFUNCTION(BlueprintCallable,BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory|Merchant")
	void DeInitMerchantData();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory|Merchant")
	void InitUI();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory|Merchant")
	void Refresh();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Inventory|Merchant")
	void AssignSellData(int64 ItemID, int32 TopLeft, BagSlot OriginBag);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Inventory|Merchant")
	void ResetSellData();
};
