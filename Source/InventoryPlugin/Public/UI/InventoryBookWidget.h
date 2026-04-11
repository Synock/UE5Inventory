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
	/** Bind a URichTextBlock named "TextBlock" in the Blueprint for automatic text updates. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (BindWidget), Category = "Inventory|Book|UI")
	URichTextBlock* TextBlock = nullptr;

	UFUNCTION(BlueprintCallable)
	void CloseButtonCalled();

public:
	/**
	 * Called post-construction to allow Blueprint to run visual/animation setup.
	 * C++ no longer needs this for pointer initialization — TextBlock is bound via BindWidget.
	 * Blueprint subclasses may still override for additional visual setup.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetupUI();
	virtual void SetupUI_Implementation() {}

	UPROPERTY(BlueprintAssignable)
	FOnCloseEvent OnCloseEvent;

	UFUNCTION(BlueprintCallable)
	void SetText(const FText& TextToDisplay);

};
