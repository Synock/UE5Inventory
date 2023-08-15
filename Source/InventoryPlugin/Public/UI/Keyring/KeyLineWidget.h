// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "KeyLineWidget.generated.h"

class UTextBlock;
class UImage;

USTRUCT(BlueprintType)
struct FkeyLineDataStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 ItemId = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 KeyId = 0;

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	UTexture2D* KeyIcon = nullptr;
};

UCLASS(BlueprintType)
class INVENTORYPLUGIN_API UKeyLineData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FkeyLineDataStruct Data;
};


UCLASS()
class INVENTORYPLUGIN_API UKeyLineWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite)
	FkeyLineDataStruct LocalData;

	UPROPERTY(BlueprintReadWrite)
	UImage* KeyIconPointer = nullptr;

	UPROPERTY(BlueprintReadWrite)
	UTextBlock* KeyNamePointer = nullptr;

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	const FkeyLineDataStruct& GetData() const {return LocalData;}

	UFUNCTION(BlueprintCallable)
	UKeyLineWidget* GetItemObject() const;
};
