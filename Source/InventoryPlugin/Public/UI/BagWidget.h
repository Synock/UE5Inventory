// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Definitions.h"
#include "Blueprint/UserWidget.h"
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


public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	void InitBagData(const FString& InBagName, int32 InBagWidth, int32 InBagHeight, EItemSize InBagSize,
	                 EBagSlot InBagSlot);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic)
	void InitUI();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic)
	void Refresh();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	void Hide();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	void Show();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic)
	void ToggleDisplay();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic)
	void DeInitBagData();

	/**
	 * @brief Lock or unlock a specific item slot in this bag to prevent interaction.
	 * @param TopLeft The top-left index of the item in the bag grid.
	 * @param bLocked True to lock, false to unlock.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Inventory|FieldRepair")
	void LockItemSlot(int32 TopLeft, bool bLocked);
};
