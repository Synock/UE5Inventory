// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/RichTextBlock.h"
#include "InventoryBookWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCloseEvent);

/**
 *
 */
UCLASS()
class INVENTORYPLUGIN_API UInventoryBookWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	URichTextBlock* TextBlock = nullptr;

	UFUNCTION(BlueprintCallable)
	void CloseButtonCalled();

public:

	//for some reason using Pre-construct to define pointers do not work on live widget creation. Call this instead
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetupUI();

	UPROPERTY(BlueprintAssignable)
	FOnCloseEvent OnCloseEvent;

	UFUNCTION(BlueprintCallable)
	void SetText(const FText& TextToDisplay);

};
