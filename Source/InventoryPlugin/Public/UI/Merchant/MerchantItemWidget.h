// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <Blueprint/IUserObjectListEntry.h>
#include <Blueprint/UserWidget.h>
#include <UObject/Object.h>

#include "CoinValue.h"
#include "MerchantItemWidget.generated.h"


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
 *
 */
UCLASS(BlueprintType)
class INVENTORYPLUGIN_API UMerchantItemWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void InitData(const FMerchantItemDataStruct& ItemData);

	UPROPERTY(BlueprintReadOnly)
	int32 ItemID;

	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
};
