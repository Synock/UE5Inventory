#pragma once

#include "CoreMinimal.h"
#include "Definitions.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Items/InventoryItemBase.h"
#include "BagWidget.generated.h"

class UInventoryGridWidget;

/**
 * 
 */
UCLASS()
class INVENTORYPLUGIN_API UBagWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	FString BagName;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	int32 BagWidth = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	int32 BagHeight = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	EItemSize BagSize = EItemSize::Giant;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	EBagSlot CurrentBagSlot = EBagSlot::Unknown;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Bag")
	UUserWidget* ParentWidget = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "Inventory|Bag")
	UInventoryGridWidget* InventoryGrid = nullptr;

	/** Optional text block to display the bag name. Bind a UTextBlock named "BagNameText" in Blueprint. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Inventory|Bag|UI")
	UTextBlock* BagNameText = nullptr;


public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	void InitBagData(const FString& InBagName, int32 InBagWidth, int32 InBagHeight, EItemSize InBagSize,
	                 EBagSlot InBagSlot);

	/**
	 * Called after InitBagData to set up the visual layout.
	 * C++ default initializes InventoryGrid via InitData(); Blueprint may override for animations/extra setup.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic)
	void InitUI();
	virtual void InitUI_Implementation();

	/**
	 * Refreshes the bag grid. C++ default calls InventoryGrid->Refresh().
	 * Blueprint may override to add visual feedback.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic)
	void Refresh();
	virtual void Refresh_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	void Hide();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	void Show();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	void ToggleDisplay();

	/**
	 * Cleans up the bag grid. C++ default calls InventoryGrid->DeInitData().
	 * Blueprint may override to handle animations.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic)
	void DeInitBagData();
	virtual void DeInitBagData_Implementation();

	/**
	 * @brief Lock or unlock a specific item slot in this bag to prevent interaction.
	 * @param TopLeft The top-left index of the item in the bag grid.
	 * @param bLocked True to lock, false to unlock.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|FieldRepair")
	void LockItemSlot(int32 TopLeft, bool bLocked);
};
