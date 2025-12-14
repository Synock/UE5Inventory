#pragma once

#include <CoreMinimal.h>
#include <Blueprint/IUserObjectListEntry.h>
#include <Blueprint/UserWidget.h>
#include <UObject/Object.h>
#include <Components/Image.h>
#include <Components/TextBlock.h>

#include "CoinValue.h"
#include "MerchantItemWidget.generated.h"

class UCoinDisplayWidget;

USTRUCT(BlueprintType)
struct FMerchantItemDataStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 Id = 0;

	UPROPERTY(BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	int32 Quantity = 0;

	UPROPERTY(BlueprintReadWrite)
	FCoinValue CoinValue;
};

/**
 *
 */
UCLASS(BlueprintType)
class INVENTORYPLUGIN_API UMerchantItemData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FMerchantItemDataStruct Data;
};


/**
 * @class UMerchantItemWidget
 *
 * Displays item icon, name, quantity, and price using dedicated UI elements.
 */
UCLASS(BlueprintType)
class INVENTORYPLUGIN_API UMerchantItemWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	//------------------------------------------------------------------------------------------------------------------
	// UI Elements (BindWidget)
	//------------------------------------------------------------------------------------------------------------------

	/** Item icon image */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Merchant|Item|UI")
	UImage* ItemIcon = nullptr;

	/** Item name text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Merchant|Item|UI")
	UTextBlock* ItemName = nullptr;

	/** Item quantity text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Merchant|Item|UI")
	UTextBlock* ItemQuantity = nullptr;

	/** Coin display widget for price */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Merchant|Item|UI")
	UCoinDisplayWidget* ItemPrice = nullptr;

	//------------------------------------------------------------------------------------------------------------------
	// Internal Data
	//------------------------------------------------------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, Category = "Merchant|Item")
	int32 ItemID = 0;

	//------------------------------------------------------------------------------------------------------------------
	// Internal Functions
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Update all UI elements with the provided data
	 * @param ItemData The merchant item data to display
	 */
	UFUNCTION(BlueprintCallable, Category = "Merchant|Item")
	void UpdateDisplay(const FMerchantItemDataStruct& ItemData);

	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	/**
	 * @brief Get the current item ID
	 * @return The item ID
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Merchant|Item")
	int32 GetItemID() const { return ItemID; }
};
