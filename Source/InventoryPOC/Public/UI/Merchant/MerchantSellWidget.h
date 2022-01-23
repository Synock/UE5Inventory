// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <Blueprint/UserWidget.h>

#include "MerchantItemListWidget.h"
#include "MerchantItemWidget.h"
#include "Inventory/BareItem.h"
#include "MerchantSellWidget.generated.h"

UENUM(BlueprintType)
enum class EMerchantWindowMode : uint8
{
	Sell = 0,
	Buy = 1,
};

class AMerchantActor;
/**
 *
 */
UCLASS()
class INVENTORYPOC_API UMerchantSellWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Merchant")
	AMerchantActor* MerchantActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Merchant")
	int64 SelectedItemId = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Sell")
	int32 DynamicStartID = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Buy")
	bool MerchantCanBuy = true;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Buy")
	BagSlot MerchantBuyOriginSlot = BagSlot::Unknown;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Buy")
	int32 MerchantBuyOriginTopLeft = -1;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Merchant")
	EMerchantWindowMode MerchantMode = EMerchantWindowMode::Sell;

	//------------------------------------------------------------------------------------------------------------------
	// Internal Functions - Sell
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	TArray<FMerchantItemDataStruct> GetStaticDataDisplayable();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	TArray<FMerchantItemDataStruct> GetDynamicDataDisplayable();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	UTexture2D* GetTextureFomName(const FString& TextureRef);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void InitListFromStatic(UMerchantItemListWidget* InputListWidget);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void InitListFromDynamic(UMerchantItemListWidget* InputListWidget);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	bool MerchantCanSell(int64 ItemID) const;

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void InitMerchantData(AMerchantActor* InputMerchantActor);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void DeInitMerchantData();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory|Merchant")
	void InitUI();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory|Merchant")
	void Refresh();

	//------------------------------------------------------------------------------------------------------------------
	// Buy
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Inventory|Buy")
	void AssignSellData(int64 ItemID, int32 TopLeft, BagSlot OriginBag);
};
