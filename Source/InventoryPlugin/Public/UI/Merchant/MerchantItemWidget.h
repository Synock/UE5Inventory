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

	/** Match regular inventory items: inspection opens only after this right-click hold duration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant|Item|Click")
	float RightClickHoldDuration = 0.5f;

	bool bRightClickPending = false;
	FVector2D RightClickScreenPosition = FVector2D::ZeroVector;
	FTimerHandle RightClickTimerHandle;

#if WITH_AUTOMATION_WORKER
	int32 InspectionRequestCountForTests = 0;
#endif

	//------------------------------------------------------------------------------------------------------------------
	// Internal Functions
	//------------------------------------------------------------------------------------------------------------------

	/**
	 * @brief Update all UI elements with the provided data
	 * @param ItemData The merchant item data to display
	 */
	UFUNCTION(BlueprintCallable, Category = "Merchant|Item")
	void UpdateDisplay(const FMerchantItemDataStruct& ItemData);

	void BeginRightClickHold(const FVector2D& ScreenPosition);
	void CancelRightClickHold();
	void ResetEntryState();

	UFUNCTION()
	void HandleRightClickHoldElapsed();

	void RequestItemInspection();

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnEntryReleased() override;
	virtual void NativeDestruct() override;

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	/**
	 * @brief Get the current item ID
	 * @return The item ID
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Merchant|Item")
	int32 GetItemID() const { return ItemID; }

#if WITH_AUTOMATION_WORKER
	void BeginRightClickHoldForTests(int32 InItemID)
	{
		ItemID = InItemID;
		BeginRightClickHold(FVector2D(640.f, 360.f));
	}
	void ReleaseRightClickHoldForTests() { CancelRightClickHold(); }
	void ExpireRightClickHoldForTests() { HandleRightClickHoldElapsed(); }
	void RecycleEntryForTests() { ResetEntryState(); }
	int32 GetInspectionRequestCountForTests() const { return InspectionRequestCountForTests; }
	bool IsRightClickPendingForTests() const { return bRightClickPending; }
#endif
};
