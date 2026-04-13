#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "UI/InventoryBookWidgetInterface.h"
#include "InventoryBookWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCloseEvent);

/**
 * Plugin book widget. Supports both single-page and two-page spread display.
 *
 * Behavior driven by SetPages():
 *   - 1 page  → single-page view: right side hidden, navigation disabled.
 *   - > 1 pages → two-page spread: left + right shown (if RightTextBlock is bound),
 *                 navigation jumps by two pages per click.
 *
 * Required Blueprint binding:
 *   - "TextBlock"          (URichTextBlock) — left / only page content
 * Optional Blueprint bindings:
 *   - "RightTextBlock"     (URichTextBlock) — right page content; hidden on single/last odd page
 *   - "TitleBlock"         (UTextBlock)     — book-level title (SetTitle)
 *   - "PageTitleBlock"     (UTextBlock)     — left page FBookPage::Title
 *   - "RightPageTitleBlock"(UTextBlock)     — right page FBookPage::Title
 *   - "PageCountBlock"     (UTextBlock)     — e.g. "1-2 / 6" or "Page 1 / 1"
 *   - "CloseButton"        (UButton)        — broadcasts OnCloseEvent
 *   - "PrevButton"         (UButton)        — previous spread; disabled on first spread
 *   - "NextButton"         (UButton)        — next spread; disabled on last spread
 */
UCLASS()
class INVENTORYPLUGIN_API UInventoryBookWidget : public UUserWidget, public IInventoryBookWidgetInterface
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	/** Required — left / only page content. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (BindWidget), Category = "Inventory|Book|UI")
	URichTextBlock* TextBlock = nullptr;

	/** Optional right-page content — hidden when only one page remains. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Inventory|Book|UI")
	URichTextBlock* RightTextBlock = nullptr;

	/** Book-level title (SetTitle). */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Inventory|Book|UI")
	UTextBlock* TitleBlock = nullptr;

	/** Left page per-page title from FBookPage::Title. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Inventory|Book|UI")
	UTextBlock* PageTitleBlock = nullptr;

	/** Right page per-page title from FBookPage::Title. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Inventory|Book|UI")
	UTextBlock* RightPageTitleBlock = nullptr;

	/** Page range indicator, e.g. "1-2 / 6" or "Page 1 / 1". */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Inventory|Book|UI")
	UTextBlock* PageCountBlock = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Inventory|Book|UI")
	UButton* CloseButton = nullptr;

	/** Disabled on the first spread. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Inventory|Book|UI")
	UButton* PrevButton = nullptr;

	/** Disabled on the last spread. */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Inventory|Book|UI")
	UButton* NextButton = nullptr;

	UFUNCTION(BlueprintCallable)
	void CloseButtonCalled();

	UFUNCTION()
	void OnPrevSpreadClicked();

	UFUNCTION()
	void OnNextSpreadClicked();

private:
	TArray<FBookPage> CachedPages;
	int32 CurrentLeftIndex = 0;

	void ShowSpread(int32 LeftIndex);
	void UpdateNavigationState();

public:
	// IInventoryBookWidgetInterface
	virtual void SetupUI_Implementation() override {}
	virtual void SetText_Implementation(const FText& Content) override;
	virtual void SetTitle_Implementation(const FText& Title) override;
	virtual void SetPages_Implementation(const TArray<FBookPage>& Pages) override;

	UPROPERTY(BlueprintAssignable)
	FOnCloseEvent OnCloseEvent;

};
