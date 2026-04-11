
#include "UI/InventoryBookWidget.h"

void UInventoryBookWidget::CloseButtonCalled()
{
	OnCloseEvent.Broadcast();
}

void UInventoryBookWidget::SetText(const FText& TextToDisplay)
{
	if (TextBlock)
		TextBlock->SetText(TextToDisplay);
	else
		UE_LOG(LogTemp, Warning, TEXT("UInventoryBookWidget::SetText — TextBlock is null. Ensure a URichTextBlock named 'TextBlock' exists in the Blueprint."));
}
