// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "MerchantItemWidget.h"
#include "Blueprint/UserWidget.h"
#include "MerchantItemListWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectionChangedDelegate, int32, ItemID);

/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UMerchantItemListWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void AddDataToList(const FMerchantItemDataStruct& ItemData);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ClearList();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ClearSelection();

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnSelectionChangedDelegate SelectionChangedDelegate;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void UpdateDynamicElement(int64 ItemID, int32 Quantity);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void RemoveElementsFrom(int32 DynamicStartID);
};
