// Copyright 2023 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KeyringWidget.generated.h"

struct FkeyLineDataStruct;
class UKeyLineData;
class UListView;

UCLASS()
class INVENTORYPLUGIN_API UKeyringWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadWrite)
	UListView* KeyDataList = nullptr;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ClearList();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void AddKey(const FkeyLineDataStruct& SkillLineDataStruct);

	UFUNCTION(BlueprintCallable)
	void InternalSetup();

public:

	UFUNCTION(BlueprintCallable)
	void RefreshList();

	bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                  UDragDropOperation* InOperation);
};
