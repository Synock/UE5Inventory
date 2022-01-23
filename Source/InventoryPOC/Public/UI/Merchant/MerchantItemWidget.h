// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include <CoreMinimal.h>
#include <Blueprint/IUserObjectListEntry.h>
#include <Blueprint/UserWidget.h>
#include <UObject/Object.h>

#include "Inventory/CoinValue.h"
#include "MerchantItemWidget.generated.h"


USTRUCT(BlueprintType)
struct FMerchantItemDataStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int64 Id;

	UPROPERTY(BlueprintReadWrite)
	UTexture2D* Icon;

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	int32 Quantity;

	UPROPERTY(BlueprintReadWrite)
	FCoinValue CoinValue;
};

/**
 *
 */
UCLASS(BlueprintType)
class INVENTORYPOC_API UMerchantItemData : public UObject
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
class INVENTORYPOC_API UMerchantItemWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void InitData(const FMerchantItemDataStruct& ItemData);

	UPROPERTY(BlueprintReadOnly)
	int64 ItemID;

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
};
