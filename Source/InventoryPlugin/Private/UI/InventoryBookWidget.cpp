#include "UI/InventoryBookWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "InventoryPlugin.h"

void UInventoryBookWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (CloseButton)
		CloseButton->OnClicked.AddDynamic(this, &UInventoryBookWidget::CloseButtonCalled);
	if (PrevButton)
		PrevButton->OnClicked.AddDynamic(this, &UInventoryBookWidget::OnPrevSpreadClicked);
	if (NextButton)
		NextButton->OnClicked.AddDynamic(this, &UInventoryBookWidget::OnNextSpreadClicked);
}

void UInventoryBookWidget::CloseButtonCalled()
{
	OnCloseEvent.Broadcast();
}

void UInventoryBookWidget::SetText_Implementation(const FText& Content)
{
	// Single-text path: fill left side, hide right side
	if (TextBlock)
		TextBlock->SetText(Content);
	else
		UE_LOG(LogInventoryPlugin, Warning, TEXT("UInventoryBookWidget::SetText — TextBlock is null. Ensure a URichTextBlock named 'TextBlock' exists in the Blueprint."));

	if (RightTextBlock)
	{
		RightTextBlock->SetText(FText::GetEmpty());
		RightTextBlock->SetVisibility(ESlateVisibility::Hidden);
	}
	if (RightPageTitleBlock)
		RightPageTitleBlock->SetVisibility(ESlateVisibility::Hidden);
}

void UInventoryBookWidget::SetTitle_Implementation(const FText& Title)
{
	if (TitleBlock)
		TitleBlock->SetText(Title);
}

void UInventoryBookWidget::SetPages_Implementation(const TArray<FBookPage>& Pages)
{
	CachedPages = Pages;
	CurrentLeftIndex = 0;
	ShowSpread(0);
}

void UInventoryBookWidget::ShowSpread(int32 LeftIndex)
{
	if (CachedPages.IsEmpty() || !CachedPages.IsValidIndex(LeftIndex))
		return;

	CurrentLeftIndex = LeftIndex;

	// Left page — always valid at this point
	{
		const FBookPage& Left = CachedPages[LeftIndex];
		if (TextBlock)
			TextBlock->SetText(Left.Content);
		if (PageTitleBlock)
			PageTitleBlock->SetText(Left.Title);
	}

	// Right page — only when more than one page exists and a right block is bound
	const bool bHasRight = CachedPages.IsValidIndex(LeftIndex + 1);
	if (RightTextBlock)
	{
		RightTextBlock->SetText(bHasRight ? CachedPages[LeftIndex + 1].Content : FText::GetEmpty());
		RightTextBlock->SetVisibility(bHasRight ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
	}
	if (RightPageTitleBlock)
	{
		RightPageTitleBlock->SetText(bHasRight ? CachedPages[LeftIndex + 1].Title : FText::GetEmpty());
		RightPageTitleBlock->SetVisibility(bHasRight ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
	}

	// Page count
	if (PageCountBlock)
	{
		const int32 Total = CachedPages.Num();
		PageCountBlock->SetText(bHasRight
			? FText::Format(NSLOCTEXT("InventoryBookWidget", "SpreadCount", "{0}-{1} / {2}"),
				FText::AsNumber(LeftIndex + 1), FText::AsNumber(LeftIndex + 2), FText::AsNumber(Total))
			: FText::Format(NSLOCTEXT("InventoryBookWidget", "SingleCount", "Page {0} / {1}"),
				FText::AsNumber(LeftIndex + 1), FText::AsNumber(Total)));
	}

	UpdateNavigationState();
}

void UInventoryBookWidget::UpdateNavigationState()
{
	if (PrevButton)
		PrevButton->SetIsEnabled(CurrentLeftIndex > 0);
	if (NextButton)
		NextButton->SetIsEnabled(CachedPages.IsValidIndex(CurrentLeftIndex + 2));
}

void UInventoryBookWidget::OnPrevSpreadClicked()
{
	if (CurrentLeftIndex >= 2)
		ShowSpread(CurrentLeftIndex - 2);
}

void UInventoryBookWidget::OnNextSpreadClicked()
{
	if (CachedPages.IsValidIndex(CurrentLeftIndex + 2))
		ShowSpread(CurrentLeftIndex + 2);
}
