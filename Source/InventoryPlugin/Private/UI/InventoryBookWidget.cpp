#include "UI/InventoryBookWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
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
	// Single-text path: fill left side, collapse all right-side chrome
	if (TextBlock)
		TextBlock->SetText(Content);
	else
		UE_LOG(LogInventoryPlugin, Warning, TEXT("UInventoryBookWidget::SetText — TextBlock is null. Ensure a URichTextBlock named 'TextBlock' exists in the Blueprint."));

	if (RightPageTitleBlock)
		RightPageTitleBlock->SetVisibility(ESlateVisibility::Hidden);
	ApplySpreadLayout(/*bSinglePageMode=*/true);
}

void UInventoryBookWidget::SetTitle_Implementation(const FText& Title)
{
	if (TitleTextBlock)
		TitleTextBlock->SetText(Title);
}

void UInventoryBookWidget::SetPages_Implementation(const TArray<FBookPage>& Pages)
{
	CachedPages = Pages;
	CurrentLeftIndex = 0;
	ApplySpreadLayout(CachedPages.Num() <= 1);
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

	// Per-side page numbers
	if (LeftPageCountBlock)
		LeftPageCountBlock->SetText(FText::AsNumber(LeftIndex + 1));
	if (RightPageCountBlock)
	{
		RightPageCountBlock->SetText(bHasRight ? FText::AsNumber(LeftIndex + 2) : FText::GetEmpty());
		RightPageCountBlock->SetVisibility(bHasRight ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
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

void UInventoryBookWidget::ApplySpreadLayout(bool bSinglePageMode)
{
	const ESlateVisibility SpreadVis = bSinglePageMode ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible;
	if (RightPage)           RightPage->SetVisibility(SpreadVis);
	if (PageDivider)         PageDivider->SetVisibility(SpreadVis);
	if (NavBar)              NavBar->SetVisibility(SpreadVis);
	// Page numbers: both hidden for single-page; ShowSpread restores them for multi-page
	if (LeftPageCountBlock)  LeftPageCountBlock->SetVisibility(SpreadVis);
	if (RightPageCountBlock) RightPageCountBlock->SetVisibility(ESlateVisibility::Collapsed);
}
