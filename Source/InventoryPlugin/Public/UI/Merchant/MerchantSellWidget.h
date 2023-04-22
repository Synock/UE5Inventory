// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <Blueprint/UserWidget.h>

#include "MerchantItemListWidget.h"
#include "MerchantItemWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "InventoryPlugin/Public/Interfaces/MerchantInterface.h"
#include "InventoryItem.h"
#include "UI/PurseWidget.h"
#include "MerchantSellWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNotEnoughPlayerMoney);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNotEnoughPlayerSpace);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNotEnoughMerchantMoney);

UENUM(BlueprintType)
enum class EMerchantWindowMode : uint8
{
	Sell = 0,
	Buy = 1,
};

/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UMerchantSellWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	//------------------------------------------------------------------------------------------------------------------
	// UI Elements
	//------------------------------------------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, Category="Inventory|Merchant|UI")
	UButton* BuySellButtonPointer = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Merchant|UI")
	UImage* ImageIconPreviewPointer = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Merchant|UI")
	UTextBlock* CurrentItemNamePointer = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Merchant|UI")
	UTextBlock* BuySellButtonTextPointer = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Merchant|UI")
	UTextBlock* CurrentItemPricePointer = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Merchant|UI")
	UTextBlock* MerchantNameTextPointer = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Merchant|UI")
	UMerchantItemListWidget* ListWidgetPointer = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Inventory|Merchant|UI")
	UPurseWidget* MerchantPursePointer = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// Internal data
	//------------------------------------------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Merchant")
	TScriptInterface<IMerchantInterface> MerchantActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Merchant")
	int32 SelectedItemId = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Merchant|Sell")
	int32 DynamicStartID = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Merchant|Buy")
	bool MerchantCanBuy = true;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Merchant|Buy")
	EBagSlot MerchantBuyOriginSlot = EBagSlot::Unknown;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Merchant|Buy")
	int32 MerchantBuyOriginTopLeft = -1;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Merchant")
	EMerchantWindowMode MerchantMode = EMerchantWindowMode::Sell;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void InitUIInternal(UButton* BuySellButton, UImage* ImageIconPreview, UTextBlock* CurrentItemName,
	                    UTextBlock* BuySellButtonText, UTextBlock* CurrentItemPrice,
	                    UMerchantItemListWidget* ItemListWidget,
	                    UPurseWidget* MerchantPurseWidget, UTextBlock* MerchantNameText);
	//------------------------------------------------------------------------------------------------------------------
	// Internal Functions - Sell
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	TArray<FMerchantItemDataStruct> GetStaticDataDisplayable();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	TArray<FMerchantItemDataStruct> GetDynamicDataDisplayable();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void InitListFromStatic(UMerchantItemListWidget* InputListWidget);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void InitListFromDynamic(UMerchantItemListWidget* InputListWidget);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	bool MerchantCanSell(int32 ItemID) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	FCoinValue GetCorrectPrice(float FloatValue) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	void HandleBuyClick();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	void HandleSellClick();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	void UpdateItemPreview();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	void HideItemPreview();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	void UpdatePricePreview();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	FCoinValue GetSelectedItemPrice() const;

	bool IsWorthless();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	void StopTrading();

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void InitMerchantData(AActor* InputMerchantActor);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void DeInitMerchantData();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic, Category = "Inventory|Merchant")
	void InitUI();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Inventory|Merchant")
	void Refresh();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant")
	void ResetSellData();

	//------------------------------------------------------------------------------------------------------------------
	// Buy
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Inventory|Merchant|Buy")
	void AssignSellData(int32 ItemID, int32 TopLeft, EBagSlot OriginBag);

	//these functions are intended for player notification of the possible results.
	virtual void OnNotEnoughPlayerMoney();
	virtual void OnNotEnoughPlayerSpace();
	virtual void OnNotEnoughMerchantMoney();

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Merchant")
	FNotEnoughPlayerMoney OnNotEnoughPlayerMoneyDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Merchant")
	FNotEnoughPlayerSpace OnNotEnoughPlayerSpaceDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Merchant")
	FNotEnoughMerchantMoney OnNotEnoughMerchantMoneyDelegate;
};
